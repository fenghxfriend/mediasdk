//
// Created by ASUS on 2018/1/4.
//

#include <typedef.h>
#include <stdlib.h>
#include <constant.h>
#include <muxer.h>
#include <frame.h>
#include <math.h>
#include <enum.h>

namespace paomiantv {

    CMuxer::CMuxer() : m_uVTrackId(MP4_INVALID_TRACK_ID),
                       m_uATrackId(MP4_INVALID_TRACK_ID),
                       m_pH264Frame(NULL),
                       m_uVTimeScale(90000),
                       m_uVFPS(VIDEO_FRAME_RATE),
                       m_pnFile(MP4_INVALID_FILE_HANDLE) {
        USE_LOG;
        m_pLock = new CLock;
        m_pchDstPath = new s8[MAX_LEN_FILE_PATH];
    }

    CMuxer::~CMuxer() {
        uninit();
        delete[]m_pchDstPath;
        delete m_pLock;
        clearSliceInfo();
    }

    BOOL32 CMuxer::init(s8 *pchDstPath) {
        BOOL32 re = FALSE;
        CAutoLock autoLock(m_pLock);
        if (pchDstPath != NULL && strlen(pchDstPath)) {
            strncpy(m_pchDstPath, pchDstPath, MAX_LEN_FILE_PATH);
            char *p[6];
            p[0] = (char *) "mp42";
            p[1] = (char *) "isom";
            p[2] = (char *) "iso2";
            p[3] = (char *) "avc1";
            p[5] = (char *) "mp41";
            m_pnFile = MP4CreateEx(m_pchDstPath, 0, 1, 1, (char *) "mp42", 0, p, 6);
            if (m_pnFile != MP4_INVALID_FILE_HANDLE) {
                MP4SetTimeScale(m_pnFile, 90000);
                re = TRUE;
            }
        }
        return re;
    }

    BOOL32 CMuxer::setDescription(const s8 *pchDescription) {
        BOOL32 re = FALSE;
        CAutoLock autoLock(m_pLock);
        if (pchDescription != NULL && strlen(pchDescription)) {
            /* Read out the existing metadata */
            const MP4Tags *mdata = MP4TagsAlloc();
            if (mdata != NULL &&
                MP4TagsFetch(mdata, m_pnFile) &&
                MP4TagsSetDescription(mdata, pchDescription) &&
                MP4TagsStore(mdata, m_pnFile)) {
                re = TRUE;
            }
            MP4TagsFree(mdata);
        }
        return re;
    }

    void CMuxer::uninit() {
        CAutoLock autoLock(m_pLock);
        if (m_pnFile != MP4_INVALID_FILE_HANDLE) {
            if (m_pH264Frame != NULL) {
                CH264Frame *pFrame = CH264Frame::create();
                pFrame->size = 0;
                pFrame->isEos = TRUE;
                writeH264Frame(pFrame);
            }
            MP4Close(m_pnFile);
            m_pnFile = NULL;
            if (!MP4Optimize((const char *) m_pchDstPath, NULL)) {
                LOGE("optimize failed!");
            }
        }
    }

    MP4TrackId
    CMuxer::addH264VideoTrack(u16 nWidth, u16 nHeight, u8 byLevel,
                              const TH264Metadata tH264Metadata) {
        CAutoLock autoLock(m_pLock);
        u16 offset = 0;
        if (m_pnFile != MP4_INVALID_FILE_HANDLE) {
            if (tH264Metadata.m_abySps[0] == 0x00
                && tH264Metadata.m_abySps[1] == 0x00
                && ((tH264Metadata.m_abySps[2] == 0x01)
                    || ((tH264Metadata.m_abySps[2] == 0x00)
                        && (tH264Metadata.m_abySps[3] == 0x01)))) {
                if ((tH264Metadata.m_abySps[2] == 1)) {
                    offset = 3;
                }
                if ((tH264Metadata.m_abySps[2] == 0 && tH264Metadata.m_abySps[3] == 1)) {
                    offset = 4;
                }

            }

            m_uVTrackId = MP4AddH264VideoTrack(m_pnFile, m_uVTimeScale, m_uVTimeScale / m_uVFPS,
                                               nWidth, nHeight,
                                               tH264Metadata.m_abySps[offset + 1],
                                               tH264Metadata.m_abySps[offset + 2],
                                               tH264Metadata.m_abySps[offset + 3],
                                               3);
            if (m_uVTrackId != MP4_INVALID_TRACK_ID) {
                // write sps
                MP4AddH264SequenceParameterSet(m_pnFile, m_uVTrackId,
                                               &(tH264Metadata.m_abySps[offset]),
                                               tH264Metadata.m_wSpsSize - offset);
                offset = 0;
                if (tH264Metadata.m_abyPps[0] == 0x00
                    && tH264Metadata.m_abyPps[1] == 0x00
                    && ((tH264Metadata.m_abyPps[2] == 0x01)
                        || ((tH264Metadata.m_abyPps[2] == 0x00)
                            && (tH264Metadata.m_abyPps[3] == 0x01)))) {
                    if ((tH264Metadata.m_abyPps[2] == 0x01)) {
                        offset = 3;
                    }
                    if ((tH264Metadata.m_abyPps[2] == 0x00)
                        && (tH264Metadata.m_abyPps[3] == 0x01)) {
                        offset = 4;
                    }

                }
                // write pps
                MP4AddH264PictureParameterSet(m_pnFile, m_uVTrackId,
                                              &(tH264Metadata.m_abyPps[offset]),
                                              tH264Metadata.m_wPpsSize - offset);
//                MP4SetVideoProfileLevel(m_pnFile, byLevel);

            }
        }

        return m_uVTrackId;
    }

    MP4TrackId CMuxer::addAACAudioTrack(u32 nSampleHZ, u8 byLevel, u8 *mbyESDS, u32 uSize) {
        CAutoLock autoLock(m_pLock);
        if (m_pnFile != MP4_INVALID_FILE_HANDLE) {
            m_uATrackId = MP4AddAudioTrack(m_pnFile, nSampleHZ, AUDIO_SAMPLE_COUNT_PER_FRAME,
                                           MP4_MPEG4_AUDIO_TYPE);
            if (m_uATrackId != MP4_INVALID_TRACK_ID) {
//                MP4SetAudioProfileLevel(m_pnFile, byLevel);
                int offset = 0;
                if (uSize > 4
                    && mbyESDS[0] == 0x00
                    && mbyESDS[1] == 0x00
                    && ((mbyESDS[2] == 0x01)
                        || ((mbyESDS[2] == 0x00)
                            && (mbyESDS[3] == 0x01)))) {
                    if ((mbyESDS[2] == 0x01)) {
                        offset = 3;
                    }
                    if ((mbyESDS[2] == 0x00) && (mbyESDS[3] == 0x01)) {
                        offset = 4;
                    }

                }
                MP4SetTrackESConfiguration(m_pnFile, m_uATrackId, mbyESDS + offset, uSize - offset);
            }
        }
        return m_uATrackId;
    }

    BOOL32
    CMuxer::writeH264Frame(CH264Frame *pFrame) {
        BOOL32 re = FALSE;
        CAutoLock autoLock(m_pLock);
        if (m_pnFile != MP4_INVALID_FILE_HANDLE && m_uVTrackId != MP4_INVALID_TRACK_ID &&
            ((pFrame->data != NULL && pFrame->size > 0) || pFrame->isEos)) {
            dealH264FrameSlice(pFrame);
            if (m_pH264Frame == NULL) {
                // first frame
                m_pH264Frame = pFrame;
                re = TRUE;
            } else {
                if (pFrame->isEos) {
                    u64 duration = 0;
                    if (pFrame->data != NULL && pFrame->size > 0) {
                        duration = pFrame->startTm - m_pH264Frame->startTm;
                        if (((s64) duration) >= 0) {
                            // next frame
                            if (MP4WriteSample(m_pnFile, m_uVTrackId,
                                               m_pH264Frame->data + m_pH264Frame->offset,
                                               m_pH264Frame->size,
                                               (u64) round(
                                                       duration / 1000000.0f * m_uVTimeScale),
                                               0,
                                               m_pH264Frame->type == EM_TYPE_I)) {
                                if (MP4WriteSample(m_pnFile, m_uVTrackId,
                                                   pFrame->data + pFrame->offset,
                                                   pFrame->size,
                                                   m_uVTimeScale / m_uVFPS,
                                                   0,
                                                   pFrame->type == EM_TYPE_I)) {
                                    re = TRUE;
                                }
                            }
                        } else {
                            // error next frame is earlier than current frame
                            LOGE("error next frame is earlier than current frame!")
                        }
                    } else {
                        if (MP4WriteSample(m_pnFile, m_uVTrackId,
                                           m_pH264Frame->data + m_pH264Frame->offset,
                                           m_pH264Frame->size,
                                           m_uVTimeScale / m_uVFPS,
                                           0,
                                           m_pH264Frame->type == EM_TYPE_I)) {
                            re = TRUE;
                        }
                    }

                    CH264Frame::release(pFrame);
                    CH264Frame::release(m_pH264Frame);
                    m_pH264Frame = NULL;
                } else {
                    switch (pFrame->type) {
                        case EM_TYPE_I:
                        case EM_TYPE_P:
                        case EM_TYPE_B:
                        case EM_TYPE_PB:
                        case EM_TYPE_OTHER: {
                            u64 duration = pFrame->startTm - m_pH264Frame->startTm;
                            if (((s64) duration) >= 0) {
                                // next frame
                                if (MP4WriteSample(m_pnFile, m_uVTrackId,
                                                   m_pH264Frame->data + m_pH264Frame->offset,
                                                   m_pH264Frame->size,
                                                   (u64) round(
                                                           duration / 1000000.0f * m_uVTimeScale),
                                                   0,
                                                   m_pH264Frame->type == EM_TYPE_I)) {
                                    re = TRUE;
                                }
                            } else {
                                // error next frame is earlier than current frame
                                LOGE("error next frame is earlier than current frame!")
                            }
                        }
                            break;
                        default:
                            break;
                    }
                    CH264Frame::release(m_pH264Frame);
                    m_pH264Frame = pFrame;
                }

            }
        }
        return re;
    }

    BOOL32 CMuxer::writeAACFrame(CAACFrame *pFrame) {
        BOOL32 re = FALSE;
        CAutoLock autoLock(m_pLock);
        if (m_pnFile != MP4_INVALID_FILE_HANDLE && m_uATrackId != MP4_INVALID_TRACK_ID &&
            pFrame->data != NULL && pFrame->size > 0) {
            if ((pFrame->data[pFrame->offset] == 0xFF)
                && (pFrame->data[pFrame->offset + 1] == 0xF0)) {
                //cut adts task
                if (MP4WriteSample(m_pnFile, m_uATrackId, pFrame->data + pFrame->offset + 7,
                                   pFrame->size - 7,
                                   MP4_INVALID_DURATION,
                                   0, 1)) {
                    re = TRUE;
                }
            } else if (MP4WriteSample(m_pnFile, m_uATrackId, pFrame->data + pFrame->offset,
                                      pFrame->size,
                                      MP4_INVALID_DURATION,
                                      0, 1)) {
                re = TRUE;
            }

        }
        return re;
    }

//    void
//    CMuxer::copyTrack(MP4FileHandle pSrcHandle, MP4TrackId trackId, s64 startTimeUS, s64 durationUS) {
//        if (!MP4_IS_VALID_TRACK_ID(trackId)) {
//            return;
//        }
//        MP4Timestamp trackStartTime = MP4ConvertToTrackTimestamp(
//                pSrcHandle,
//                trackId,
//                startTimeUS,
//                MP4_USECS_TIME_SCALE);
//
//        MP4Duration trackDuration = MP4ConvertToTrackDuration(
//                pSrcHandle,
//                trackId,
//                durationUS,
//                MP4_USECS_TIME_SCALE);
//
//        MP4TrackId dstTrackId = MP4CloneTrack(pSrcHandle, trackId, m_pnFile);
//
//        if (!MP4_IS_VALID_TRACK_ID(dstTrackId)) {
//            return;
//        }
//
//        MP4Timestamp when = 0;
//        const MP4SampleId numSamples = MP4GetTrackNumberOfSamples(pSrcHandle, trackId);
//        MP4SampleId sampleId = MP4GetSampleIdFromTime(pSrcHandle, trackId, trackStartTime, true);
//
//        while (true) {
//            if (when >= trackDuration) {
//                break;
//            }
//            if (sampleId > numSamples) {
//                break;
//            }
//            MP4Duration sampleDuration = MP4GetSampleDuration(pSrcHandle, trackId, sampleId);
//            bool rc = MP4CopySample(
//                    pSrcHandle,
//                    trackId,
//                    sampleId,
//                    m_pnFile,
//                    dstTrackId,
//                    sampleDuration);
//            if (!rc) {
//                MP4DeleteTrack(m_pnFile, dstTrackId);
//                break;
//            }
//            sampleId++;
//            when += sampleDuration;
//        }
//    }
//
//    void CMuxer::copyFromSource(s8 *pSrcPath, EMTrack type, s64 startTimeUS, s64 durationUS) {
//        MP4FileHandle handle = MP4_INVALID_FILE_HANDLE;
//        if (pSrcPath == NULL || strlen(pSrcPath) <= 0 || durationUS < 0 || type < EM_TRACK_VIDEO ||
//            type > EM_TRACK_AUDIO) {
//            LOGE("parameter is invalid!");
//            return;
//        }
//
//        handle = MP4Read(pSrcPath);
//
//        if (!MP4_IS_VALID_FILE_HANDLE(handle)) {
//            LOGE("read source failed!");
//            return;
//        }
//        u32 uTrackNum = MP4GetNumberOfTracks(handle);
//        MP4TrackId trackId = MP4_INVALID_TRACK_ID;
//        for (u32 i = 0; i < uTrackNum; i++) {
//            trackId = MP4FindTrackId(handle, i);
//            const s8 *trackType = MP4GetTrackType(handle, trackId);
//            if (type == EM_TRACK_AUDIO && MP4_IS_AUDIO_TRACK_TYPE(trackType)) {
//                copyTrack(handle, trackId, startTimeUS, durationUS);
//            }
//            if (type == EM_TRACK_VIDEO && MP4_IS_VIDEO_TRACK_TYPE(trackType)) {
//                copyTrack(handle, trackId, startTimeUS, durationUS);
//            }
//        }
//    }

    u32 CMuxer::dealH264FrameSlice(CH264Frame *&pFrame) {
        if (pFrame->data == NULL || pFrame->size <= 0) {
            return 0;
        }
        u32 offset = 0;
        std::vector<TH264SliceInfo *> slices;

        TH264SliceInfo *sliceInfo = createSliceInfo();
        sliceInfo->m_pbySlice = pFrame->data;
        if (!(pFrame->data[pFrame->offset] == 0x00
              && pFrame->data[pFrame->offset + 1] == 0x00
              && ((pFrame->data[pFrame->offset + 2] == 0x01)
                  || (pFrame->data[pFrame->offset + 2] == 0x00 &&
                      pFrame->data[pFrame->offset + 3] == 0x01)))) {
//            pFrame->drift(4);
            sliceInfo->m_uOffset = pFrame->offset;
        } else {
            if ((pFrame->data[pFrame->offset + 2] == 0x01)) {
                sliceInfo->m_uOffset = pFrame->offset + 3;
            }
            if ((pFrame->data[pFrame->offset + 2] == 0x00) &&
                (pFrame->data[pFrame->offset + 3] == 0x01)) {
                sliceInfo->m_uOffset = pFrame->offset + 4;
            }

        }
        offset = sliceInfo->m_uOffset;

        while (offset + 4 <= pFrame->size + pFrame->offset) {
            if (pFrame->data[offset] == 0x00 && pFrame->data[offset + 1] == 0x00 &&
                ((pFrame->data[offset + 2] == 0x01) || (pFrame->data[offset + 2] == 0x00 &&
                                                        pFrame->data[offset + 3] == 0x01))) {
                sliceInfo->m_uSize = offset - sliceInfo->m_uOffset;
                slices.push_back(sliceInfo);
                sliceInfo = createSliceInfo();
                sliceInfo->m_pbySlice = pFrame->data;
                if ((pFrame->data[offset + 2] == 0x01)) {
                    offset += 3;
                    sliceInfo->m_uOffset = offset;
                    continue;
                }
                if ((pFrame->data[offset + 2] == 0x00 && pFrame->data[offset + 3] == 0x01)) {
                    offset += 4;
                    sliceInfo->m_uOffset = offset;
                    continue;
                }
            } else {
                offset++;
            }
        }
        sliceInfo->m_uSize = pFrame->size + pFrame->offset - sliceInfo->m_uOffset;
        slices.push_back(sliceInfo);
        u32 frameSize = 0;
        for (u32 i = 0; i < slices.size(); i++) {
            frameSize += (4 + slices[i]->m_uSize);
        }
        CH264Frame *tmp = NULL;
        if (frameSize != pFrame->size) {
            tmp = CH264Frame::create();
            tmp->reset(frameSize);
            tmp->offset = 0;
            tmp->size = frameSize;
            tmp->isEos = pFrame->isEos;
            tmp->type = pFrame->type;
            tmp->startTm = pFrame->startTm;
            tmp->clipId = pFrame->clipId;
            tmp->duration = pFrame->duration;
//            tmp->isLast = pFrame->isLast;
            tmp->renderOffset = pFrame->renderOffset;
            tmp->sampleId = pFrame->sampleId;

            offset = 0;
            for (int i = 0; i < slices.size(); ++i) {
                sliceInfo = slices[i];
                tmp->data[offset] = (u8) ((sliceInfo->m_uSize >> 24) & 0xFF);
                tmp->data[offset + 1] = (u8) ((sliceInfo->m_uSize >> 16) & 0xFF);
                tmp->data[offset + 2] = (u8) ((sliceInfo->m_uSize >> 8) & 0xFF);
                tmp->data[offset + 3] = (u8) (sliceInfo->m_uSize & 0xFF);
                memcpy(tmp->data + offset + 4,
                       sliceInfo->m_pbySlice + sliceInfo->m_uOffset,
                       sliceInfo->m_uSize);
                offset += (4 + sliceInfo->m_uSize);
                releaseSliceInfo(sliceInfo);
            }
            CH264Frame::release(pFrame);
            pFrame = tmp;
        } else {
            offset = pFrame->offset;
            for (int i = 0; i < slices.size(); ++i) {
                sliceInfo = slices[i];
                pFrame->data[offset] = (u8) ((sliceInfo->m_uSize >> 24) & 0xFF);
                pFrame->data[offset + 1] = (u8) ((sliceInfo->m_uSize >> 16) & 0xFF);
                pFrame->data[offset + 2] = (u8) ((sliceInfo->m_uSize >> 8) & 0xFF);
                pFrame->data[offset + 3] = (u8) (sliceInfo->m_uSize & 0xFF);
                offset += (4 + sliceInfo->m_uSize);
                releaseSliceInfo(sliceInfo);
            }
        }
        return frameSize;
    }

    TH264SliceInfo *CMuxer::createSliceInfo() {
        TH264SliceInfo *re = NULL;
        if (!m_svPool.empty()) {
            re = m_svPool.back();
            m_svPool.pop_back();
        } else {
            re = new TH264SliceInfo();
        }
        return re;
    }

    void CMuxer::releaseSliceInfo(TH264SliceInfo *pframe) {
        if (pframe) {
            pframe->m_uOffset = 0;
            pframe->m_uSize = 0;
            pframe->m_pbySlice = NULL;
            m_svPool.push_back(pframe);
        }
    }

    void CMuxer::clearSliceInfo() {
        int i = 0;
        while (i < m_svPool.size()) {
            delete m_svPool[i];
            i++;
        }
        m_svPool.clear();
    }

}