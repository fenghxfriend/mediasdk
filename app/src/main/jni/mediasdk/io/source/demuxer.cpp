/*******************************************************************************
 *        Module: mediasdk
 *          File: clipparse.cpp
 * Functionality: parse clip.
 *       Related: mp4v2
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-20  v1.0        huangxuefeng  created
 ******************************************************************************/

#include <cstring>
#include <autolog.h>
#include <constant.h>
#include <unistd.h>
#include "demuxer.h"

#define PROCESS_RAW_TO_NAT 0

namespace paomiantv {

    static bool handleRaw(u8 *raw, u32 &size) {
#if PROCESS_RAW_TO_NAT
        u32 slicesize = raw[0] << 24 | raw[1] << 16 | raw[2] << 8 | raw[3];
        if (slicesize + 4 == size) {
            raw[0] = 0x00;
            raw[1] = 0x00;
            raw[2] = 0x00;
            raw[3] = 0x01;
        } else if (slicesize + 4 < size) {
            u32 originsize = size;
            u32 tmpsize = 0;
            size = 0;
            u8 *tmp = raw;

            tmp[0] = 0x00;
            tmp[1] = 0x00;
            tmp[2] = 0x00;
            tmp[3] = 0x01;
            size += (slicesize + 4);
            tmpsize += (slicesize + 4);
            tmp += (slicesize + 4);
            while (tmpsize + 4 < originsize) {
                slicesize = tmp[0] << 24 | tmp[1] << 16 | tmp[2] << 8 | tmp[3];
                tmp[0] = 0x00;
                tmp[1] = 0x00;
                tmp[2] = 0x01;
                memmove(tmp + 3, tmp + 4, originsize - tmpsize - 4);
                tmpsize += (slicesize + 4);
                tmp += (slicesize + 3);
                size += (slicesize + 3);
            }
        } else {
            return false;
        }
#endif
        return true;
    }

    CDemuxer::CDemuxer() {
        m_pLock = new CLock;
        m_Handle = MP4_INVALID_FILE_HANDLE;
        m_pchSrc = (s8 *) malloc(MAX_LEN_FILE_PATH);
        m_pbySPS = (u8 *) malloc(MAX_SPS_SIZE);
        m_pbyPPS = (u8 *) malloc(MAX_PPS_SIZE);
        m_pbyESDS = (u8 *) malloc(MAX_ESDS_SIZE);
        m_uSPSLen = 0;
        m_uSPSLen = 0;
        m_uESDSLen = 0;
        m_ullVRealDuration = 0;
        m_ullARealDuration = 0;
        m_uHeight = 0;
        m_uWidth = 0;
        memset(m_pchSrc, 0, MAX_LEN_FILE_PATH);
        memset(m_pbySPS, 0, MAX_SPS_SIZE);
        memset(m_pbyPPS, 0, MAX_PPS_SIZE);
        memset(m_pbyESDS, 0, MAX_ESDS_SIZE);
        m_uVSyncSize = 0;
        m_pUSampleSyncIds = NULL;
    }

    CDemuxer::~CDemuxer() {

        if (m_pUSampleSyncIds != NULL) {
            free(m_pUSampleSyncIds);
            m_pUSampleSyncIds = NULL;
        }

        if (m_pchSrc != NULL) {
            free(m_pchSrc);
            m_pchSrc = NULL;
        }

        if (m_pbySPS != NULL) {
            free(m_pbySPS);
            m_pbySPS = NULL;
        }
        if (m_pbyPPS != NULL) {
            free(m_pbyPPS);
            m_pbyPPS = NULL;
        }

        if (MP4_IS_VALID_FILE_HANDLE(m_Handle)) {
            MP4Close(m_Handle);
            m_Handle = MP4_INVALID_FILE_HANDLE;
        }
        if (m_pLock != NULL) {
            delete m_pLock;
            m_pLock = NULL;
        }
    }

    BOOL32 CDemuxer::init(s8 *pSrcPath) {
        CAutoLock autoLock(m_pLock);
        strncpy(m_pchSrc, pSrcPath, MAX_LEN_FILE_PATH);
        if (access(m_pchSrc, F_OK | R_OK) != -1) {
            return TRUE;
        } else {
            LOGE("%s does not exist or can not read.\n", m_pchSrc);
        }
        return FALSE;
    }

    void CDemuxer::reset() {
        memset(m_pchSrc, 0, MAX_LEN_FILE_PATH);
        m_Handle = MP4_INVALID_FILE_HANDLE;

        m_uTimeScale = 0;
        m_uTrackNum = 0;

        m_uVTimeScale = 0;
        m_uVTrackId = 0;
        m_uVSampleNum = 0;
        m_uVSampleMaxSize = 0;
        m_ullVRealDuration = 0;

        m_uATimeScale = 0;
        m_uATrackId = 0;
        m_uASampleNum = 0;
        m_uASampleMaxSize = 0;
        m_ullARealDuration = 0;
    }

    void CDemuxer::initSpsPps() {

        const u8 nalHeader[] = {0x00, 0x00, 0x00, 0x01};
        // read sps/pps
        uint8_t **seqheader;
        uint8_t **pictheader;
        uint32_t *pictheadersize;
        uint32_t *seqheadersize;
        uint32_t ix;
        MP4GetTrackH264SeqPictHeaders(m_Handle, m_uVTrackId, &seqheader, &seqheadersize,
                                      &pictheader,
                                      &pictheadersize);

        //sps
        u32 count = 0;
        BOOL32 flag = TRUE;
        memcpy(m_pbySPS + count, nalHeader, 4);
        count += 4;
        u8 *temp = NULL;
        for (ix = 0; seqheadersize[ix] != 0; ix++) {

            if (!flag) {
                goto bailSPS;
            }

            if (count + seqheadersize[ix] > MAX_SPS_SIZE) {
                temp = (u8 *) realloc(m_pbySPS, count + seqheadersize[ix]);
                if (temp == NULL) {
                    flag = FALSE;
                    count = 0;
                    goto bailSPS;
                }
                m_pbySPS = temp;
            }
            memcpy(m_pbySPS + count, seqheader[ix], seqheadersize[ix]);
            count += seqheadersize[ix];
            bailSPS:
            free(seqheader[ix]);
        }
        m_uSPSLen = count;
        free(seqheader);
        free(seqheadersize);


        //pps
        count = 0;
        flag = TRUE;
        temp = NULL;
        memcpy(m_pbyPPS + count, nalHeader, 4);
        count += 4;
        for (ix = 0; pictheadersize[ix] != 0; ix++) {

            if (!flag) {
                goto bailPPS;
            }
            if (count + pictheadersize[ix] > MAX_PPS_SIZE) {
                temp = (u8 *) realloc(m_pbyPPS, count + pictheadersize[ix]);
                if (temp == NULL) {
                    flag = FALSE;
                    count = 0;
                    goto bailPPS;
                }
                m_pbyPPS = temp;
            }
            memcpy(m_pbyPPS + count, pictheader[ix], pictheadersize[ix]);
            count += pictheadersize[ix];
            bailPPS:
            free(pictheader[ix]);
        }
        m_uPPSLen = count;
        free(pictheader);
        free(pictheadersize);
    }

    static u32 avpriv_mpeg4audio_sample_rates[] = {
            96000, 88200, 64000, 48000, 44100, 32000,
            24000, 22050, 16000, 12000, 11025, 8000, 7350
    };
    static const char *mpeg4AudioNames[] = {
            "MPEG-4 AAC main",
            "MPEG-4 AAC LC",
            "MPEG-4 AAC SSR",
            "MPEG-4 AAC LTP",
            "MPEG-4 AAC HE",
            "MPEG-4 AAC Scalable",
            "MPEG-4 TwinVQ",
            "MPEG-4 CELP",
            "MPEG-4 HVXC",
            NULL, NULL,
            "MPEG-4 TTSI",
            "MPEG-4 Main Synthetic",
            "MPEG-4 Wavetable Syn",
            "MPEG-4 General MIDI",
            "MPEG-4 Algo Syn and Audio FX",
            "MPEG-4 ER AAC LC",
            NULL,
            "MPEG-4 ER AAC LTP",
            "MPEG-4 ER AAC Scalable",
            "MPEG-4 ER TwinVQ",
            "MPEG-4 ER BSAC",
            "MPEG-4 ER ACC LD",
            "MPEG-4 ER CELP",
            "MPEG-4 ER HVXC",
            "MPEG-4 ER HILN",
            "MPEG-4 ER Parametric",
            "MPEG-4 SSC",
            "MPEG-4 PS",
            "MPEG-4 MPEG Surround",
            NULL,
            "MPEG-4 Layer-1",
            "MPEG-4 Layer-2",
            "MPEG-4 Layer-3",
            "MPEG-4 DST",
            "MPEG-4 Audio Lossless",
            "MPEG-4 SLS",
            "MPEG-4 SLS non-core",
    };
    static const uint8_t mpegAudioTypes[] = {
            MP4_MPEG2_AAC_MAIN_AUDIO_TYPE,  // 0x66
            MP4_MPEG2_AAC_LC_AUDIO_TYPE,    // 0x67
            MP4_MPEG2_AAC_SSR_AUDIO_TYPE,   // 0x68
            MP4_MPEG2_AUDIO_TYPE,           // 0x69
            MP4_MPEG1_AUDIO_TYPE,           // 0x6B
            // private types
            MP4_PCM16_LITTLE_ENDIAN_AUDIO_TYPE,
            MP4_VORBIS_AUDIO_TYPE,
            MP4_ALAW_AUDIO_TYPE,
            MP4_ULAW_AUDIO_TYPE,
            MP4_G723_AUDIO_TYPE,
            MP4_PCM16_BIG_ENDIAN_AUDIO_TYPE,
    };
    static const char *mpegAudioNames[] = {
            "MPEG-2 AAC Main",
            "MPEG-2 AAC LC",
            "MPEG-2 AAC SSR",
            "MPEG-2 Audio (13818-3)",
            "MPEG-1 Audio (11172-3)",
            // private types
            "PCM16 (little endian)",
            "Vorbis",
            "G.711 aLaw",
            "G.711 uLaw",
            "G.723.1",
            "PCM16 (big endian)",
    };
    static uint8_t numMpegAudioTypes =
            sizeof(mpegAudioTypes) / sizeof(uint8_t);

    void CDemuxer::setADTS(u8 *&buffer, const s32 datasize) {
        int id = 0;
        int layer = 0;
        int protection_absent = 1;
        int profile = 3;  //AAC LC，MediaCodecInfo.CodecProfileLevel.AACObjectLC;
        int freqIdx = 15;  //见后面注释avpriv_mpeg4audio_sample_rates中32000对应的数组下标，来自ffmpeg源码
        int chanCfg = 2;  //见后面注释channel_configuration，Stero双声道立体声
        int packet_len = datasize + MAX_ADTS_SIZE;
        int type = MP4GetTrackEsdsObjectTypeId(m_Handle, m_uATrackId);
        const char *name;
        switch (type) {
            case MP4_INVALID_AUDIO_TYPE:
                name = "AAC from .mov";
                break;
            case MP4_MPEG4_AUDIO_TYPE: {

                type = MP4GetTrackAudioMpeg4Type(m_Handle, m_uATrackId);
                if (type == MP4_MPEG4_INVALID_AUDIO_TYPE ||
                    type > ((sizeof((mpeg4AudioNames))) / (sizeof(*(mpeg4AudioNames)))) ||
                    mpeg4AudioNames[type - 1] == NULL) {
                    name = "MPEG-4 Unknown Profile";
                } else {
                    id = 0;
                    profile = type - 1;
                    name = mpeg4AudioNames[type - 1];
                }
                break;
            }
                // fall through
            default:
                for (uint8_t i = 0; i < numMpegAudioTypes; i++) {
                    if (type == mpegAudioTypes[i]) {
                        id = 1;
                        profile = i;
                        name = mpegAudioNames[i];
                        break;
                    }
                }
        }
        int sample_num = sizeof(avpriv_mpeg4audio_sample_rates) / sizeof(int);
        for (int i = 0; i < sample_num; i++) {
            if (m_uATimeScale == avpriv_mpeg4audio_sample_rates[i]) {
                freqIdx = i;
                break;
            }
        }
        chanCfg = MP4GetTrackAudioChannels(m_Handle, m_uATrackId);

        /*
        int avpriv_mpeg4audio_sample_rates[] = {
        96000, 88200, 64000, 48000, 44100, 32000,
        24000, 22050, 16000, 12000, 11025, 8000, 7350
        };
        channel_configuration: 表示声道数chanCfg
        0: Defined in AOT Specifc Config
        1: 1 channelCount: front-center
        2: 2 channels: front-left, front-right
        3: 3 channels: front-center, front-left, front-right
        4: 4 channels: front-center, front-left, front-right, back-center
        5: 5 channels: front-center, front-left, front-right, back-left, back-right
        6: 6 channels: front-center, front-left, front-right, back-left, back-right, LFE-channelCount
        7: 8 channels: front-center, front-left, front-right, side-left, side-right, back-left, back-right, LFE-channelCount
        8-15: Reserved
        */

        buffer[0] = (uint8_t) 0xFF;
        buffer[1] = (uint8_t) (0xF0 + ((id & 1) << 3) + ((layer & 3) << 1) +
                               (protection_absent & 1));
        buffer[2] = (uint8_t) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
        buffer[3] = (uint8_t) (((chanCfg & 3) << 6) + (packet_len >> 11));
        buffer[4] = (uint8_t) ((packet_len & 0x7FF) >> 3);
        buffer[5] = (uint8_t) (((packet_len & 7) << 5) + 0x1F);
        buffer[6] = (uint8_t) 0xFC;
    }

    void CDemuxer::initEsds() {
        int profile = 3;
        int freqIdx = 15;
        int chanCfg = 2;
        int type = MP4GetTrackEsdsObjectTypeId(m_Handle, m_uATrackId);
        switch (type) {
            case MP4_INVALID_AUDIO_TYPE:
                break;
            case MP4_MPEG4_AUDIO_TYPE: {

                type = MP4GetTrackAudioMpeg4Type(m_Handle, m_uATrackId);
                if (type == MP4_MPEG4_INVALID_AUDIO_TYPE ||
                    type > ((sizeof((mpeg4AudioNames))) / (sizeof(*(mpeg4AudioNames)))) ||
                    mpeg4AudioNames[type - 1] == NULL) {
                } else {
                    profile = type - 1;
                }
                break;
            }
                // fall through
            default:
                for (uint8_t i = 0; i < numMpegAudioTypes; i++) {
                    if (type == mpegAudioTypes[i]) {
                        profile = i;
                        break;
                    }
                }
        }
        int sample_num = sizeof(avpriv_mpeg4audio_sample_rates) / sizeof(int);
        for (int i = 0; i < sample_num; i++) {
            if (m_uATimeScale == avpriv_mpeg4audio_sample_rates[i]) {
                freqIdx = i;
                break;
            }
        }
        chanCfg = MP4GetTrackAudioChannels(m_Handle, m_uATrackId);

        m_pbyESDS[0] = (uint8_t) (((profile + 1) << 3) | freqIdx >> 1);
        m_pbyESDS[1] = (uint8_t) (((freqIdx << 7) & 0x80) | chanCfg << 3);
        m_uESDSLen = 2;
    }

    BOOL32 CDemuxer::parse() {
        CAutoLock autoLock(m_pLock);
        BOOL32 re = FALSE;
        do {
            if (m_pchSrc == NULL || strlen(m_pchSrc) <= 0) {
                LOGE("clip parameter is invalid, parse failed!");
                break;
            }
            if (MP4_IS_VALID_FILE_HANDLE(m_Handle)) {
                return TRUE;
            } else {
                m_Handle = MP4Read(m_pchSrc);
            }

            if (!MP4_IS_VALID_FILE_HANDLE(m_Handle)) {
                LOGE("read clip failed!");
                break;
            }
            m_uTimeScale = MP4GetTimeScale(m_Handle);
            m_uTrackNum = MP4GetNumberOfTracks(m_Handle);
            MP4TrackId trackId = MP4_INVALID_TRACK_ID;
            for (u32 i = 0; i < m_uTrackNum; i++) {
                trackId = MP4FindTrackId(m_Handle, i);
                const s8 *trackType = MP4GetTrackType(m_Handle, trackId);
                if (MP4_IS_VIDEO_TRACK_TYPE(trackType)) {
                    m_uVTrackId = trackId;
                    m_uVTimeScale = MP4GetTrackTimeScale(m_Handle, m_uVTrackId);
                    m_uVSampleMaxSize = MP4GetTrackMaxSampleSize(m_Handle, m_uVTrackId);
                    m_uVSampleNum = MP4GetTrackNumberOfSamples(m_Handle, m_uVTrackId);
                    m_ullVRealDuration =
                            MP4GetTrackDuration(m_Handle, m_uVTrackId) * 1000000 / m_uVTimeScale;
                    m_uWidth = MP4GetTrackVideoWidth(m_Handle, m_uVTrackId);
                    m_uHeight = MP4GetTrackVideoHeight(m_Handle, m_uVTrackId);
                    MP4GetSyncSampleIds(m_Handle, m_uVTrackId, &m_pUSampleSyncIds, &m_uVSyncSize);
                    initSpsPps();
                } else if (MP4_IS_AUDIO_TRACK_TYPE(trackType)) {
                    m_uATrackId = trackId;
                    m_uATimeScale = MP4GetTrackTimeScale(m_Handle, m_uATrackId);
                    m_uASampleMaxSize = MP4GetTrackMaxSampleSize(m_Handle, m_uATrackId);
                    m_uASampleNum = MP4GetTrackNumberOfSamples(m_Handle, m_uATrackId);
                    m_ullARealDuration =
                            MP4GetTrackDuration(m_Handle, m_uATrackId) * 1000000 / m_uATimeScale;
                    initEsds();
                }
            }
            if (m_uSPSLen == 0 || m_uPPSLen == 0) {
                break;
            }
            re = TRUE;
        } while (0);

        if (!re) {
            reset();
        }
        return re;
    }

    void CDemuxer::copyTrack(
            MP4TrackId trackId,
            MP4FileHandle pDstHandle,
            s64 startTime,
            s64 duration) {
        if (!MP4_IS_VALID_TRACK_ID(trackId)) {
            return;
        }
        MP4Timestamp trackStartTime = MP4ConvertToTrackTimestamp(
                m_Handle,
                trackId,
                startTime,
                MP4_MSECS_TIME_SCALE);

        MP4Duration trackDuration = MP4ConvertToTrackDuration(
                m_Handle,
                trackId,
                duration,
                MP4_MSECS_TIME_SCALE);

        MP4TrackId dstTrackId = MP4CloneTrack(m_Handle, trackId, pDstHandle);

        if (!MP4_IS_VALID_TRACK_ID(dstTrackId)) {
            return;
        }

        MP4Timestamp when = 0;
        const MP4SampleId numSamples = MP4GetTrackNumberOfSamples(m_Handle, trackId);
        MP4SampleId sampleId = MP4GetSampleIdFromTime(m_Handle, trackId, trackStartTime, true);

        while (true) {
            if (when >= trackDuration) {
                break;
            }
            if (sampleId > numSamples) {
                break;
            }
            MP4Duration sampleDuration = MP4GetSampleDuration(m_Handle, trackId, sampleId);
            bool rc = MP4CopySample(
                    m_Handle,
                    trackId,
                    sampleId,
                    pDstHandle,
                    dstTrackId,
                    sampleDuration);
            if (!rc) {
                MP4DeleteTrack(pDstHandle, dstTrackId);
                break;
            }
            sampleId++;
            when += sampleDuration;
        }
    }

    void CDemuxer::cut(const s8 *pchDstPath, s64 nllStart, s64 nllDuration) {
        CAutoLock autoLock(m_pLock);
        MP4FileHandle dstHandle = MP4_INVALID_FILE_HANDLE;
        if (pchDstPath == NULL || strlen(pchDstPath) <= 0) {
            return;
        }
        char *p[6];
        p[0] = (char *) "mp42";
        p[1] = (char *) "isom";
        p[2] = (char *) "iso2";
        p[3] = (char *) "avc1";
        p[5] = (char *) "mp41";
        dstHandle = MP4CreateEx(pchDstPath, 0, 1, 1, (char *) "mp42", 0, p, 6);
        if (!MP4_IS_VALID_FILE_HANDLE(dstHandle)) {
            return;
        }

        MP4SetTimeScale(dstHandle, m_uTimeScale);
        for (u32 i = 0; i < m_uTrackNum; i++) {
            copyTrack(MP4FindTrackId(m_Handle, i), dstHandle, nllStart, nllDuration);
        }
        MP4Close(dstHandle);
    }

    BOOL32 CDemuxer::getVidoeSampleById(u32 nId, u8 *&buff, u32 &size, u64 &starttime,
                                        u64 &duration, u64 &renderoffset, BOOL32 &isSync) {
        CAutoLock autoLock(m_pLock);
        BOOL32 re = MP4ReadSample(m_Handle, m_uVTrackId, nId, &buff, &size, &starttime, &duration,
                                  &renderoffset, (bool *) &isSync);
        if (re) {
            starttime = starttime * 1000000 / m_uVTimeScale;
            duration = duration * 1000000 / m_uVTimeScale;
            renderoffset = renderoffset * 1000000 / m_uVTimeScale;
            if (buff && size > 4) {
                //标准的264帧，前面几个字节就是frame的长度.
                //需要替换为标准的264 nal 头.
                handleRaw(buff, size);
            }
        }
        return re;
    }


    BOOL32 CDemuxer::getNextSyncVidoeSampleIdFromTime(u64 starttime, u32 &id) {
        CAutoLock autoLock(m_pLock);
        MP4Timestamp trackStartTime = MP4ConvertToTrackTimestamp(
                m_Handle,
                m_uVTrackId,
                starttime,
                MP4_MSECS_TIME_SCALE);
        id = MP4GetNextSyncSampleIdFromTime(m_Handle, m_uVTrackId, trackStartTime);
        if (MP4_IS_VALID_SAMPLE_ID(id)) {
            return false;
        }
        return true;
    }

    BOOL32 CDemuxer::getPreviousSyncVidoeSampleIdFromTime(u64 starttime, u32 &id) {
        CAutoLock autoLock(m_pLock);
        MP4Timestamp trackStartTime = MP4ConvertToTrackTimestamp(
                m_Handle,
                m_uVTrackId,
                starttime,
                MP4_MSECS_TIME_SCALE);
        id = MP4GetPreviousSyncSampleIdFromTime(m_Handle, m_uVTrackId, trackStartTime);
        if (MP4_IS_VALID_SAMPLE_ID(id)) {
            return false;
        }
        return true;
    }

    BOOL32 CDemuxer::getAudioSampleIdFromTime(u64 starttime, u32 &id) {
        CAutoLock autoLock(m_pLock);
        MP4Timestamp trackStartTime = MP4ConvertToTrackTimestamp(
                m_Handle,
                m_uATrackId,
                starttime,
                MP4_MSECS_TIME_SCALE);
        id = MP4GetSampleIdFromTime(m_Handle, m_uATrackId, trackStartTime, true);
        if (MP4_IS_VALID_SAMPLE_ID(id)) {
            return false;
        }
        return true;
    }

    BOOL32 CDemuxer::getVidoeSampleByTime(u64 ullTimestamp, u8 *&buff, u32 &size, u64 &starttime,
                                          u64 &duration, u64 &renderoffset, BOOL32 &isSync) {
        CAutoLock autoLock(m_pLock);
        BOOL32 re = MP4ReadSampleFromTime(m_Handle, m_uVTrackId, ullTimestamp, &buff, &size,
                                          &starttime,
                                          &duration,
                                          &renderoffset, (bool *) &isSync);
        if (re) {
            starttime = starttime * 1000000 / m_uVTimeScale;
            duration = duration * 1000000 / m_uVTimeScale;
            renderoffset = renderoffset * 1000000 / m_uVTimeScale;
            if (buff && size > 4) {
                //标准的264帧，前面几个字节就是frame的长度.
                //需要替换为标准的264 nal 头.
                handleRaw(buff, size);
            }
        }
        return re;
    }

    BOOL32 CDemuxer::getAudioSampleById(u32 nId, u8 *&buff, u32 &size, u64 &starttime,
                                        u64 &duration, u64 &renderoffset, BOOL32 &isSync) {
        CAutoLock autoLock(m_pLock);
        BOOL32 re = MP4ReadSample(m_Handle, m_uATrackId, nId, &buff, &size, &starttime, &duration,
                                  &renderoffset, (bool *) &isSync);
        if (re) {
            starttime = starttime * 1000000 / m_uATimeScale;
            duration = duration * 1000000 / m_uATimeScale;
            renderoffset = renderoffset * 1000000 / m_uATimeScale;
        }
        return re;
    }

    BOOL32 CDemuxer::getAudioSampleByTime(u64 ullTimestamp, u8 *&buff, u32 &size, u64 &starttime,
                                          u64 &duration, u64 &renderoffset, BOOL32 &isSync) {
        CAutoLock autoLock(m_pLock);
        BOOL32 re = MP4ReadSampleFromTime(m_Handle, m_uATrackId, ullTimestamp, &buff, &size,
                                          &starttime,
                                          &duration,
                                          &renderoffset, (bool *) &isSync);
        if (re) {
            starttime = starttime * 1000000 / m_uATimeScale;
            duration = duration * 1000000 / m_uATimeScale;
            renderoffset = renderoffset * 1000000 / m_uATimeScale;
        }
        return re;
    }

    BOOL32 CDemuxer::getVideoSPS(u8 *&sps, u32 &size) {
        CAutoLock autoLock(m_pLock);
        if (m_uSPSLen == 0) {
            return false;
        } else {
            if (!sps) {
                sps = (u8 *) malloc(m_uSPSLen);
            }
            memcpy(sps, m_pbySPS, m_uSPSLen);
            size = m_uSPSLen;
            return true;
        }
    }

    BOOL32 CDemuxer::getVideoPPS(u8 *&pps, u32 &size) {
        CAutoLock autoLock(m_pLock);
        if (m_uPPSLen == 0) {
            return false;
        } else {
            if (!pps) {
                pps = (u8 *) malloc(m_uPPSLen);
            }
            memcpy(pps, m_pbyPPS, m_uPPSLen);
            size = m_uPPSLen;
            return true;
        }
    }

    BOOL32 CDemuxer::getAudioESDS(u8 *&esds, u32 &size) {
        CAutoLock autoLock(m_pLock);
        if (m_uESDSLen == 0) {
            return false;
        } else {
            if (!esds) {
                esds = (u8 *) malloc(m_uESDSLen);
            }
            memcpy(esds, m_pbyESDS, m_uESDSLen);
            size = m_uESDSLen;
            return true;
        }
    }
}
