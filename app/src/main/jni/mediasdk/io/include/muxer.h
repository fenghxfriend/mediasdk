//
// Created by ASUS on 2018/1/4.
//

#ifndef _PAOMIANTV_CMUXER_H
#define _PAOMIANTV_CMUXER_H

#include <vector>
#include <frame.h>
#include <autolock.h>
#include <enum.h>
#include <constant.h>
#include "mp4v2/mp4v2.h"

namespace paomiantv {

    typedef struct tagH264Metadata {
        // video, must be h264 type
        u16 m_wSpsSize;
        u8 m_abySps[MAX_SPS_SIZE];
        u16 m_wPpsSize;
        u8 m_abyPps[MAX_PPS_SIZE];

        tagH264Metadata() {
            memset(this, 0, sizeof(tagH264Metadata));
        }
    } TH264Metadata;

    typedef struct tagH264SliceInfo {
        // video slice, must be h264 type
        u32 m_uSize;
        u32 m_uOffset;
        u8 *m_pbySlice;

        tagH264SliceInfo() {
            memset(this, 0, sizeof(tagH264SliceInfo));
        }

        void reset() {
            memset(this, 0, sizeof(tagH264SliceInfo));
        }
    } TH264SliceInfo;


    class CMuxer {
    public:
        CMuxer();

        virtual ~CMuxer();

        BOOL32 init(s8 *pchDstPath);

        BOOL32 setDescription(const s8 *pchDescription);

        u32
        addH264VideoTrack(u16 nWidth, u16 nHeight, u8 byLevel, const TH264Metadata tH264Metadata);

        u32 addAACAudioTrack(u32 nSampleHZ, u8 byLevel, u8 *mbyESDS, u32 uSize);

        BOOL32 writeH264Frame(CH264Frame *pFrame);

        BOOL32 writeAACFrame(CAACFrame *pFrame);

//        void copyFromSource(s8 *pSrcPath, EMTrack type, s64 startTimeUS, s64 durationUS);

        inline s8 *getDst();

        inline MP4FileHandle getHandler();

    private:
        ILock *m_pLock;

        s8 *m_pchDstPath;
        MP4FileHandle m_pnFile;

        MP4TrackId m_uVTrackId;
        u32 m_uVTimeScale;
        u32 m_uVFPS;

        CH264Frame *m_pH264Frame;

        MP4TrackId m_uATrackId;

        void uninit();

        u32 dealH264FrameSlice(CH264Frame *&pFrame);

        TH264SliceInfo *createSliceInfo();

        void releaseSliceInfo(TH264SliceInfo *pframe);

        void clearSliceInfo();

        std::vector<TH264SliceInfo *> m_svPool;

//        void copyTrack(MP4FileHandle pSrcHandle, MP4TrackId trackId, s64 startTimeUS, s64 durationUS);
    };

    inline s8 *CMuxer::getDst() {
        return m_pchDstPath;
    }

    inline MP4FileHandle CMuxer::getHandler() {
        return m_pnFile;
    }
}

#endif //_PAOMIANTV_CMUXER_H
