/*******************************************************************************
 *        Module: mediasdk
 *          File:
 * Functionality: producer
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-08-03  v1.0        huangxuefeng  created
 ******************************************************************************/
#ifndef _PAOMIANTV_REVERSER_H_
#define _PAOMIANTV_REVERSER_H_

#include <revcontroller.h>

namespace paomiantv {
    class CReverser {
    public:
        CReverser();

        virtual ~CReverser();

    private:
        CDemuxer *m_pDemuxer;

        CRenderer *m_pRender;//the renderer for rendering video
        CAudioTrack *m_pAudioTrack;//the audiotrack for play audio

        CRevController *m_pRevController;
        OnMessageCB m_cbOnMessage;
        OnWritePCMCB m_cbOnWritePCM;
        OnStartVEncodeCB m_cbOnStartVEncode;
        OnStartAEncodeCB m_cbOnStartAEncode;
        void *m_cbDelegate;

    public:

        void
        reverse(CMuxer *muxer, s8 *tempPath, s8 *src, BOOL32 isVideoReverse = TRUE, BOOL32 isAudioReverse = FALSE);

        void start();

        void stop();

        virtual void bindSurface(EGLNativeWindowType pWindow, u32 uWidth, u32 uHeight);

        void setOnCB(void *cbDelegate = NULL, OnMessageCB cbOnMessage = NULL,
                     OnWritePCMCB cbOnWrite = NULL, OnStartVEncodeCB cbStartVEncodeCB = NULL,
                     OnStartAEncodeCB cbStartAEncodeCB = NULL);
    };
}

#endif //_PAOMIANTV_REVERSER_H_
