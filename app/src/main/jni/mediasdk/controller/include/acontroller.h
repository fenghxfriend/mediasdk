/*******************************************************************************
 *        Module: mediasdk
 *          File: 
 * Functionality: audio controller
 *       Related: 
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-24  v1.0        huangxuefeng  created
 ******************************************************************************/
#ifndef _PAOMIANTV_ACONTROLLER_H_
#define _PAOMIANTV_ACONTROLLER_H_


#include <adec.h>
#include "controller.h"
#include <deque>

#define USING_AUDIO_DECODER_EX 1

namespace paomiantv {

    class IDecoder;

    class CAController : public CController {
    public:
        CAController();

        virtual ~CAController();

        virtual BOOL32 init(CStoryboard **ppStoryboard, CAudioTrack **ppAudiotrack);

        virtual void prepare(BOOL32 isPlay = TRUE, BOOL32 isWrite = FALSE);

        virtual void start();

        virtual void stop();

        virtual void resume();

        virtual void pause();

        virtual bool seekTo(s64 sllMicrosecond);

        virtual bool locPreview(s64 sllMicrosecond);

    protected:
        void _seekTo(s64 sllMicrosecond);

    private:

        virtual long run();

        CAudioTrack **m_ppAudioTrack;
#if USING_AUDIO_DECODER_EX
        std::vector<IDecoder *> m_decoders;
#endif
        std::vector<CDec *> m_vDec;
        std::deque<TMessage *> _seekList;

        const AVSampleFormat m_ePlayFormat = AV_SAMPLE_FMT_S16;
        const u64 m_ullPlayChannelLayout = AV_CH_LAYOUT_STEREO;
        const EMSampleRate m_ePlaySampleRate = EM_SAMPLE_RATE_44_1;

        void sendSoundDecodeCompleteMessage();
    };
}

#endif /* _PAOMIANTV_ACONTROLLER_H_ */
