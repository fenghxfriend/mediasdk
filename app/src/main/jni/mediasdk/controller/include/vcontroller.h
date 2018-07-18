/*******************************************************************************
 *        Module: paomiantv
 *          File: 
 * Functionality: video controller
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

#ifndef _PAOMIANTV_VCONTROLLER_H_
#define _PAOMIANTV_VCONTROLLER_H_


#include <vdec.h>
#include "controller.h"
#include <deque>

namespace paomiantv {

#define USING_VIDEO_DECODER_EX 1

    class IDecoder;

    class CVController : public CController {

    public:
        CVController();

        virtual ~CVController();

        virtual BOOL32 init(CStoryboard **ppStoryboard, CRenderer **ppRenderer);

        virtual void prepare(BOOL32 isPlay = TRUE, BOOL32 isWrite = FALSE);

        virtual void start();

        virtual void stop();

        virtual void resume();

        virtual void pause();

        virtual bool seekTo(s64 sllMicrosecond);

        virtual bool locPreview(s64 sllMicrosecond);

    protected:
        void _seekTo(int64_t sllMicrosecond);

    private:
        CRenderer **m_ppRender;
#if USING_VIDEO_DECODER_EX
        std::vector<IDecoder*> m_decoders;
#endif
        std::vector<CDec *> m_vDec;
        std::deque<TMessage*> _seekList;

        virtual long run();

        void sendImageDecodeCompleteMessage();
    };
}

#endif /* _PAOMIANTV_VCONTROLLER_H_ */
