/*******************************************************************************
 *        Module: mediasdk
 *          File:
 * Functionality: clip parser
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-20  v1.0        huangxuefeng  created
 ******************************************************************************/
#ifndef _PAOMIANTV_DEMUXER_H_
#define _PAOMIANTV_DEMUXER_H_

#include <mp4v2/mp4v2.h>
#include <typedef.h>
#include <autolock.h>
#include <constant.h>

namespace paomiantv {

    class CTrack;

    class CDemuxer {
    public:
        CDemuxer();

        virtual ~CDemuxer();

        BOOL32 init(s8 *pSrcPath);

        BOOL32 parse();

        void cut(const s8 *pchDstPath, s64 nllStart, s64 nllDuration);

        BOOL32 getVidoeSampleById(u32 nId, u8 *&buff, u32 &size, u64 &starttime, u64 &duration,
                                  u64 &renderoffset, BOOL32 &isSync);

        BOOL32
        getVidoeSampleByTime(u64 ullTimestamp, u8 *&buff, u32 &size, u64 &starttime, u64 &duration,
                             u64 &renderoffset, BOOL32 &isSync);

        BOOL32 getNextSyncVidoeSampleIdFromTime(u64 starttime, u32 &id);

        BOOL32 getPreviousSyncVidoeSampleIdFromTime(u64 starttime, u32 &id);

        BOOL32 getAudioSampleById(u32 nId, u8 *&buff, u32 &size, u64 &starttime, u64 &duration,
                                  u64 &renderoffset, BOOL32 &isSync);

        BOOL32
        getAudioSampleByTime(u64 ullTimestamp, u8 *&buff, u32 &size, u64 &starttime, u64 &duration,
                             u64 &renderoffset, BOOL32 &isSync);

        BOOL32 getAudioSampleIdFromTime(u64 starttime, u32 &id);

        inline u32 getVSampleNum();

        inline u32 getVSampleMaxSize();

        inline u64 getVRealDuration();

        inline u32 getVWidth();

        inline u32 getVHeight();

        inline MP4SampleId *getVSyncIds();

        inline u32 getVSyncNum();


        inline u32 getASampleNum();

        inline u32 getASampleMaxSize();

        inline u64 getARealDuration();


        inline s8 *getSrc();

        inline MP4FileHandle getHandler();


        BOOL32 getVideoSPS(u8 *&sps, u32 &size);

        BOOL32 getVideoPPS(u8 *&sps, u32 &size);

        BOOL32 getAudioESDS(u8 *&esds, u32 &size);

    private:
        ILock *m_pLock;

        s8 *m_pchSrc;

        MP4FileHandle m_Handle;

        u32 m_uTimeScale;
        u32 m_uTrackNum;

        u32 m_uVTimeScale;
        MP4TrackId m_uVTrackId;
        u32 m_uVSampleNum;
        u32 m_uVSampleMaxSize;
        u64 m_ullVRealDuration;
        u32 m_uWidth;
        u32 m_uHeight;

        u32 m_uATimeScale;
        MP4TrackId m_uATrackId;
        u32 m_uASampleNum;
        u32 m_uASampleMaxSize;
        u64 m_ullARealDuration;

        u8 *m_pbySPS;
        u32 m_uSPSLen;
        u8 *m_pbyPPS;
        u32 m_uPPSLen;
        u8 *m_pbyESDS;
        u32 m_uESDSLen;

        u32 m_uVSyncSize;
        MP4SampleId *m_pUSampleSyncIds;

    private:

        void copyTrack(MP4TrackId trackId, MP4FileHandle pDstHandle, s64 startTime = 0,
                       s64 duration = INTEGER32_MAX_VALUE);

        void reset();

        void setADTS(u8 *&buffer, const s32 datasize);

        void initSpsPps();

        void initEsds();
    };

    inline u32 CDemuxer::getVSampleNum() {
        return m_uVSampleNum;
    }

    inline u32 CDemuxer::getVSampleMaxSize() {
        return m_uVSampleMaxSize;
    }


    inline u64 CDemuxer::getVRealDuration() {
        return m_ullVRealDuration;
    }

    inline u32 CDemuxer::getVWidth() {
        return m_uWidth;
    }

    inline u32 CDemuxer::getVHeight() {
        return m_uHeight;
    }

    inline u32 CDemuxer::getASampleNum() {
        return m_uASampleNum;
    }

    inline u32 CDemuxer::getASampleMaxSize() {
        return m_uASampleMaxSize;
    }

    inline u64 CDemuxer::getARealDuration() {
        return m_ullARealDuration;
    }

    inline s8 *CDemuxer::getSrc() {
        return m_pchSrc;
    }

    inline MP4FileHandle CDemuxer::getHandler() {
        return m_Handle;
    }

    inline MP4SampleId *CDemuxer::getVSyncIds() {
        return m_pUSampleSyncIds;
    }

    inline u32 CDemuxer::getVSyncNum() {
        return m_uVSyncSize;
    }
}

#endif /* _PAOMIANTV_DEMUXER_H_ */
