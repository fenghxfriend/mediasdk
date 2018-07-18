/*******************************************************************************
 *        Module: render
 *          File:
 * Functionality: rendering video onto opengl context.
 *       Related: GLES2
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2013-07-12  v1.0        huangxuefeng  created
 * 2014-01-19  v1.1        huangxuefeng  aggregates render-objects
 ******************************************************************************/

#ifndef _PAOMIANTV_RENDERER_H_
#define _PAOMIANTV_RENDERER_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include "../../common/safequeue.h"
#include <set>

#include "../../common/typedef.h"
#include "../../common/autolock.h"
#include "../../common/thread.h"
#include "vlayer.h"
#include "image.h"

namespace paomiantv {
    /*!
 * \brief   renderer of OpenGLES for YUV420Planar frames.
 * \note    it dependents GLContext created in java by GLSurfaceView.
 *          currently, it only support rendering YUV420P frames.
 * \author  huangxuefeng
 * \date    2018-01-29
 */
    class CRenderer {
    public:
        CRenderer();

        virtual ~CRenderer();

        void updateGLRect(u32 uGLWidth, u32 uGLHeight);

        void bindEGLNativeWindow(EGLNativeWindowType pWindow, u32 uWidth, u32 uHeight);

        BOOL32 prepare(bool isSync = false);

        void start();

        void stop();

        void pause();

        void resume();

        inline EGLNativeWindowType getEGLWin();

        BOOL32 addImage(CImage *pImage);

        void flush();

        void setCaching(bool flag);

        bool isCaching() {
            return m_isCaching;
        }

        void setOnMessageCB(OnMessageCB pFunction, void *pVoid);

    private:
        static void *ThreadWrapper(void *pThis);

    protected:
        //opengles
        volatile EGLNativeWindowType m_eglWin;
        EGLContext m_eglContext;
        EGLDisplay m_eglDisplay;
        EGLSurface m_eglSurface;

        CTexture *m_pOriRGBATex;
        CTexture *m_pOriYUVTex;
//        CVFilter *m_pNonRGBAFilter;
//        CVFilter *m_pNonYUVFilter;

        CVLayer *m_pLayer;

        u32 m_uGLWidth;
        u32 m_uGLHeight;

        ILock *m_pLock;
        CThread *m_pThread;

        BOOL32 m_bIsStarted;
        BOOL32 m_bIsStopped;
        BOOL32 m_bIsPaused;
        CSafeQueue<CImage> *m_pQueue;

        BOOL32 m_bIsDirty;
        bool m_isCaching;
        bool m_isSync;

        struct timeval m_tRenderStartTm;

        OnMessageCB m_cbOnMessage;
        void *m_cbDelegate;

    protected:

        virtual BOOL32 draw();

        virtual BOOL32 drawImage(const CImage *pImage);

        virtual void *run();

        virtual BOOL32 initEGL();

        virtual BOOL32 initGLComponents();

        virtual void uninitGLComponents();

        virtual void uninitEGL();

        void onDrawnMessage(s64 sllTimeStampUS);

        void onSendEOSMessage();

        void onSaveMessage(s64 sllTimeStampUS);
    };

    inline EGLNativeWindowType CRenderer::getEGLWin() {
        return m_eglWin;
    }

} // namespace paomiantv

#endif // _PAOMIANTV_RENDERER_H_