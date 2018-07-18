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
#ifndef _PAOMIANTV_EXPORTER_H_
#define _PAOMIANTV_EXPORTER_H_

#include "vcontroller.h"
#include "acontroller.h"

namespace paomiantv {
    class CExporter {
    public:
        CExporter();

        virtual ~CExporter();

    private:
        CStoryboard **m_ppStoryboard;

        CVController *m_pVController;
        CAController *m_pAController;

        // is playing or not
        BOOL32 m_bIsPlaying;


        CRenderer *m_pRender;//the renderer for rendering video
        CAudioTrack *m_pAudioTrack;//the audiotrack for play audio

        OnMessageCB m_cbOnMessage;
        OnWritePCMCB m_cbOnWrite;
        void *m_cbDelegate;

    public:

        void setDataSource(CStoryboard **ppStoryboard);

        void start();

        void stop();

        virtual void bindSurface(EGLNativeWindowType pWindow, u32 uWidth, u32 uHeight);

        void setOnCB(void *cbDelegate = NULL, OnMessageCB cbOnMessage = NULL,
                     OnWritePCMCB cbOnWrite = NULL);

        BOOL32
        setVideoEncoderParams(s8 string[32], jint i, jint i1, jint i2, jint i3, jint i4, jint i5,
                              jint i6);

        BOOL32 setAudioEncoderParams(s8 string[32], jint i, jint i1, jint i2, jint i3);
    };
}

#endif //_PAOMIANTV_EXPORTER_H_
