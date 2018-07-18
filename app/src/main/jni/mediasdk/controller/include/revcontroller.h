//
// Created by ASUS on 2018/6/1.
//

#ifndef MEDIAENGINE_REVCONTROLLER_H
#define MEDIAENGINE_REVCONTROLLER_H

#include <muxer.h>
#include <cutter.h>
#include <controller.h>
#include <renderer.h>
#include <VideoReverseDecoder.h>
#include <AudioReverseDecoder.h>

namespace paomiantv {

    class CRevController : public CController {
    public:
        CRevController();

        virtual ~CRevController();

        BOOL32 init(CDemuxer **ppDemuxer, CAudioTrack **ppAudioTrack, CRenderer **ppRender);

        virtual void prepare(CMuxer *muxer,s8 *pchTempPath,BOOL32 isVideoReverse = TRUE, BOOL32 isAudioReverse = FALSE);
        virtual void prepare(BOOL32 isPlay = TRUE, BOOL32 isWrite = FALSE){};

        virtual void start();

        virtual void stop();

        virtual void resume();

        virtual void pause();

        virtual bool seekTo(s64 sllMicrosecond);

        virtual bool locPreview(s64 sllMicrosecond);

        void setOnEncodeThread(OnStartVEncodeCB cbOnStartVEncode, OnStartAEncodeCB cbOnStartAEncode,
                           void *cbDelegate);

    protected:
        CDemuxer **m_ppDemuxer;
        CRenderer **m_ppRender;
        CAudioTrack **m_ppAudioTrack;
        VideoReverseDecoder *m_pVRevDec;
        AudioReverseDecoder *m_pARevDec;
        CCutter *m_pVCutter;
        CCutter *m_pACutter;
        const AVSampleFormat m_ePlayFormat = AV_SAMPLE_FMT_S16;
        const u64 m_ullPlayChannelLayout = AV_CH_LAYOUT_STEREO;
        const EMSampleRate m_ePlaySampleRate = EM_SAMPLE_RATE_44_1;

        OnStartVEncodeCB m_cbOnStartVEncode;
        OnStartAEncodeCB m_cbOnStartAEncode;
        void *m_cbDelegateEncode;

        virtual long run();
        void sendImageDecodeCompleteMessage();

        void sendSoundDecodeCompleteMessage();
    };
}

#endif //MEDIAENGINE_REVCONTROLLER_H
