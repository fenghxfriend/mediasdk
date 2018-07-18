/*******************************************************************************
 *        Module: render
 *          File: renderer.cpp
 * Functionality: rendering video onto opengl context.
 *       Related: GLES2
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-08-01  v1.0        huangxuefeng  created
 ******************************************************************************/

#include <stdlib.h>
#include <sys/time.h>
#include <typeinfo>
#include <unistd.h>
#include <lutfilter.h>
#include <blurfilter.h>
#include <maskfilter.h>
#include "autolog.h"
#include "autolock.h"
#include "renderer.h"

#ifdef __cplusplus
extern "C"
{
#endif
#ifdef __cplusplus
}
#endif
namespace paomiantv {

    static void setFilterParams(CVFilter *filter, const CVLayerParam *params) {
        filter->setScale(params->m_afScale[EM_DIRECT_X],
                         params->m_afScale[EM_DIRECT_Y],
                         params->m_afScale[EM_DIRECT_Z]);
        filter->setRotation(
                params->m_afRotate[EM_DIRECT_X],
                params->m_afRotate[EM_DIRECT_Y],
                params->m_afRotate[EM_DIRECT_Z]);
        filter->setTranslate(
                params->m_afTranslate[EM_DIRECT_X],
                params->m_afTranslate[EM_DIRECT_Y],
                params->m_afTranslate[EM_DIRECT_Z]);
        filter->setUVCropScale(
                params->m_afUVCropScale[EM_DIRECT_X],
                params->m_afUVCropScale[EM_DIRECT_Y],
                params->m_afUVCropScale[EM_DIRECT_Z]);
        filter->setUVCropRotation(
                params->m_afUVCropRotate[EM_DIRECT_X],
                params->m_afUVCropRotate[EM_DIRECT_Y],
                params->m_afUVCropRotate[EM_DIRECT_Z]);
        filter->setUVCropTranslate(
                params->m_afUVCropTranslate[EM_DIRECT_X],
                params->m_afUVCropTranslate[EM_DIRECT_Y],
                params->m_afUVCropTranslate[EM_DIRECT_Z]);
        filter->setAlpha(params->m_fAlpha);
    }

    CRenderer::CRenderer()
            : m_eglWin(NULL),
              m_cbOnMessage(NULL),
              m_cbDelegate(NULL),
              m_pOriRGBATex(NULL),
              m_pOriYUVTex(NULL),
              m_bIsDirty(FALSE),
              m_isCaching(false),
              m_isSync(false),
              m_uGLWidth(0),
              m_uGLHeight(0),
              m_bIsStopped(FALSE),
              m_bIsPaused(FALSE),
              m_bIsStarted(FALSE),
              m_eglContext(EGL_NO_CONTEXT),
              m_eglDisplay(EGL_NO_DISPLAY),
              m_eglSurface(EGL_NO_SURFACE) {
        USE_LOG;
        memset(&m_tRenderStartTm, 0, sizeof(m_tRenderStartTm));
        m_pQueue = new CSafeQueue<CImage>(10);
        m_pLock = new CLock;
        m_pThread = new CThread(ThreadWrapper, this);
        m_pLayer = new CVLayer;
    }

    CRenderer::~CRenderer() {
        USE_LOG;
        stop();
        if (m_pLayer != NULL) {
            delete m_pLayer;
            m_pLayer = NULL;
        }
        if (m_pThread != NULL) {
            delete m_pThread;
            m_pThread = NULL;
        }
        if (m_pQueue != NULL) {
            delete m_pQueue;
            m_pQueue = NULL;
        }
        if (m_pLock != NULL) {
            m_pLock = new CLock;
            m_pLock = NULL;
        }
    }

    BOOL32 CRenderer::prepare(bool isSync) {
        m_pQueue->clear();
        m_pQueue->enable();
        CAutoLock autoLock(m_pLock);
//        if (m_bIsStarted) {
//            return FALSE;
//        }
//        if (!m_pThread->start()) {
//            LOGE("start renderer thread failed!");
//            return FALSE;
//        }
        m_isSync = isSync;
        return TRUE;
    }

    void CRenderer::start() {
        LOGI("CRenderer::startThread");
        m_pQueue->enable();
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted) {
            return;
        }
        if (!m_pThread->start()) {
            LOGE("start renderer thread failed!");
            return;
        }
        m_bIsStopped = FALSE;
    }

    void CRenderer::stop() {
        LOGI("CRenderer::stopThread");
        m_pQueue->disable();
        m_pLock->lock();
        if (m_bIsStarted && !m_bIsStopped) {
            m_bIsStopped = TRUE;
            m_pLock->acttive();
        }
        m_pLock->unlock();
        m_pThread->join();
        m_pQueue->clear();
        memset(&m_tRenderStartTm, 0, sizeof(m_tRenderStartTm));
    }

    void CRenderer::pause() {
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted && !m_bIsStopped && !m_bIsPaused) {
            m_bIsPaused = TRUE;
        }
    }

    void CRenderer::resume() {
        CAutoLock autoLock(m_pLock);
        memset(&m_tRenderStartTm, 0, sizeof(m_tRenderStartTm));
        if (m_bIsStarted && !m_bIsStopped && m_bIsPaused) {
            m_bIsPaused = FALSE;
            m_pLock->acttive();
        }
    }

    BOOL32 CRenderer::addImage(CImage *pImage) {
        if (m_pQueue != NULL) {
            return m_pQueue->push(pImage);
        }
        return FALSE;
    }

    void CRenderer::setCaching(bool flag) {
        if (flag == m_isCaching) {
            return;
        }
        if (!flag) {
            memset(&m_tRenderStartTm, 0, sizeof(m_tRenderStartTm));
        }
        m_isCaching = flag;
    }

    void CRenderer::flush() {
        CAutoLock autoLock(m_pLock);
        m_pQueue->disable();
        m_pQueue->clear();
        m_pQueue->enable();
    }

    BOOL32 CRenderer::drawImage(const CImage *pImage) {
        CAutoLock autoLock(m_pLock);
        s32 slayerSize = pImage->m_vVLayerParam.size();
        if (slayerSize > 0 && m_eglWin != NULL) {
            glViewport(0, 0, m_uGLWidth, m_uGLHeight);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            for (s32 i = 0; i < slayerSize; i++) {
                if (pImage->m_vVLayerParam[i] != NULL &&
                    pImage->m_vVLayerParam[i]->m_pbyPicture != NULL &&
                    pImage->m_vVLayerParam[i]->m_uSize > 0 &&
                    pImage->m_vVLayerParam[i]->m_uWidth > 0 &&
                    pImage->m_vVLayerParam[i]->m_uHeight > 0) {
                    CTexture *texture = NULL;
                    m_pLayer->reset();
                    m_pLayer->updateLayerSize(m_uGLWidth, m_uGLHeight);
                    switch (pImage->m_vVLayerParam[i]->m_eFormat) {
                        case EM_PIXEL_FORMAT_I420: {
                            texture = m_pOriYUVTex;
                        }
                            break;
                        case EM_PIXEL_FORMAT_RGBA_8888: {
                            texture = m_pOriRGBATex;
                        }
                            break;
                        default:
                            continue;
                    }
                    texture->fillTexture(pImage->m_vVLayerParam[i]->m_pbyPicture,
                                         pImage->m_vVLayerParam[i]->m_uWidth,
                                         pImage->m_vVLayerParam[i]->m_uHeight);

                    s32 nFilterCount = pImage->m_vVLayerParam[i]->m_vFilterParams.size();
                    for (s32 j = 0; j < nFilterCount; j++) {
                        if (pImage->m_vVLayerParam[i]->m_vFilterParams[j] != NULL) {
                            switch (pImage->m_vVLayerParam[i]->m_vFilterParams[j]->m_eType) {
                                case EM_V_FILTER_LUT: {
                                    if (pImage->m_vVLayerParam[i]->m_vFilterParams[j]->m_pFilterSource !=
                                        NULL) {
                                        CLUTFilter *lutFilter = new CLUTFilter;
                                        if (m_pLayer->getFilterSize() > 0) {
                                            lutFilter->init(EM_PIXEL_FORMAT_RGBA_8888);
                                        } else {
                                            lutFilter->init(texture->getFormat());
                                            setFilterParams(lutFilter, pImage->m_vVLayerParam[i]);
                                        }

                                        lutFilter->changeOutputSize(m_uGLWidth, m_uGLHeight);
                                        lutFilter->fillMapTexture(
                                                pImage->m_vVLayerParam[i]->m_vFilterParams[j]->m_pFilterSource->m_pbyPicture,
                                                pImage->m_vVLayerParam[i]->m_vFilterParams[j]->m_pFilterSource->m_uWidth,
                                                pImage->m_vVLayerParam[i]->m_vFilterParams[j]->m_pFilterSource->m_uHeight,
                                                pImage->m_vVLayerParam[i]->m_vFilterParams[j]->m_pFilterSource->m_eFormat);
                                        m_pLayer->pushFilter(lutFilter);
                                    }
                                }
                                    break;
                                case EM_V_FILTER_MASK: {
                                    if (pImage->m_vVLayerParam[i]->m_vFilterParams[j]->m_pFilterSource !=
                                        NULL) {
                                        CMaskFilter *maskFilter = new CMaskFilter;
                                        if (m_pLayer->getFilterSize() > 0) {
                                            maskFilter->init(EM_PIXEL_FORMAT_RGBA_8888);
                                        } else {
                                            maskFilter->init(texture->getFormat());
                                            setFilterParams(maskFilter, pImage->m_vVLayerParam[i]);
                                        }

                                        maskFilter->changeOutputSize(m_uGLWidth, m_uGLHeight);
                                        maskFilter->fillMaskTexture(
                                                pImage->m_vVLayerParam[i]->m_vFilterParams[j]->m_pFilterSource->m_pbyPicture,
                                                pImage->m_vVLayerParam[i]->m_vFilterParams[j]->m_pFilterSource->m_uWidth,
                                                pImage->m_vVLayerParam[i]->m_vFilterParams[j]->m_pFilterSource->m_uHeight,
                                                pImage->m_vVLayerParam[i]->m_vFilterParams[j]->m_pFilterSource->m_eFormat);
                                        m_pLayer->pushFilter(maskFilter);
                                    }
                                }
                                    break;
                                case EM_V_FILTER_BLUR: {
                                    if (m_pLayer->getFilterSize() <= 0) {
                                        // fill the pos and uv
                                        CVFilter *filter = new CVFilter;
                                        filter->init(texture->getFormat());
                                        setFilterParams(filter, pImage->m_vVLayerParam[i]);
                                        filter->changeOutputSize(m_uGLWidth, m_uGLHeight);
                                        m_pLayer->pushFilter(filter);
                                    }
                                    CBlurFilter *blurFilter = new CBlurFilter;
                                    blurFilter->init(EM_PIXEL_FORMAT_RGBA_8888);
                                    blurFilter->changeOutputSize(m_uGLWidth, m_uGLHeight);
                                    blurFilter->setBlurOffset(DEFAULT_BLUR_OFFSET);
                                    blurFilter->setItNumber(DEFAULT_ITERATE_TIME);
                                    m_pLayer->pushFilter(blurFilter);
                                }
                                    break;
                                default: {

                                }
                                    break;

                            }
                        }
                    }
                    if (m_pLayer->getFilterSize() <= 0) {
                        // fill the pos and uv
                        CVFilter *filter = new CVFilter;
                        filter->init(texture->getFormat());
                        filter->changeOutputSize(m_uGLWidth, m_uGLHeight);
                        setFilterParams(filter, pImage->m_vVLayerParam[i]);
                        m_pLayer->pushFilter(filter);
                    }
                    m_pLayer->draw(texture);
                    CVFilter *filter = NULL;
                    while (m_pLayer->popFilter(filter)) {
                        if (filter != NULL /*&&
                            filter != m_pNonYUVFilter &&
                            filter != m_pNonRGBAFilter*/) {
                            delete filter;
                        }
                    }
                }
            }
            glDisable(GL_BLEND);
            //send event to upgrade UI
            if (m_isSync) {
                onDrawnMessage(pImage->m_sllTimeStampUS);
            } else {
                onSaveMessage(pImage->m_sllTimeStampUS);
            }

            eglSwapBuffers(m_eglDisplay, m_eglSurface);
        }
        return TRUE;
    }

    BOOL32 CRenderer::draw() {
        if (m_isSync && m_isCaching) {
            //正在缓存数据，主动等待一段时间以避免卡死线程
            usleep(3000);
            return FALSE;
        }

        CImage *pImage = NULL;
        m_pQueue->pop(pImage);
        if (pImage == NULL || pImage->m_vVLayerParam.size() <= 0) {
            if (pImage != NULL && pImage->isEOS) {
                LOGE("Video stream is EOS! ");
                onSendEOSMessage();
                CImage::release(pImage);
                return FALSE;
            }
            LOGE("frame is invalid! ");
            if (pImage != NULL) {
                CImage::release(pImage);
            }
            return FALSE;
        }
        if (m_tRenderStartTm.tv_sec == 0 && m_tRenderStartTm.tv_usec == 0) {
            gettimeofday(&m_tRenderStartTm, NULL);
            m_tRenderStartTm.tv_usec -= (pImage->m_sllTimeStampUS % 1000000);
            m_tRenderStartTm.tv_sec -= (pImage->m_sllTimeStampUS / 1000000);
        }
        struct timeval now;
        gettimeofday(&now, NULL);
        s64 usDelay =
                (m_tRenderStartTm.tv_usec + (pImage->m_sllTimeStampUS % 1000000) - now.tv_usec) +
                ((m_tRenderStartTm.tv_sec + (pImage->m_sllTimeStampUS / 1000000) - now.tv_sec) *
                 1000000);
        if (m_isSync && usDelay > 0) {
            usleep((unsigned long) (usDelay));
        }

        drawImage(pImage);
        CImage::release(pImage);
        return TRUE;
    }

    void *CRenderer::ThreadWrapper(void *pThis) {
        CRenderer *p = (CRenderer *) pThis;
        return p->run();
    }

    void *CRenderer::run() {
        USE_LOG;
        m_pThread->setName("CRenderer");
        m_pLock->lock();
        LOGI("render thread is started");
        m_bIsStarted = TRUE;
        while (!m_bIsStopped) {
            if (m_eglWin != NULL) {
                bool loop = true;
                if (!initEGL()) {
                    LOGE("init egl failed!");
                    loop = false;
                }
                if (!initGLComponents()) {
                    LOGE("init gl components failed!");
                    loop = false;
                }
                if (loop) {
                    while (!m_bIsStopped && m_eglWin != NULL) {
                        while (!m_bIsStopped && m_bIsPaused && m_eglWin != NULL) {
                            m_pLock->wait(1000);
                        }
                        if (!m_bIsStopped && m_uGLHeight != 0 && m_uGLWidth != 0 &&
                            m_eglWin != NULL) {
                            m_pLock->unlock();
                            if (m_eglWin != NULL) {
                                draw();
                            }
                            m_pLock->lock();
                        }
                    }
                }
                uninitGLComponents();
                uninitEGL();
            } else {
                m_pLock->unlock();
                usleep(2000);
                m_pLock->lock();
            }

        }
        m_bIsStarted = FALSE;
        m_bIsStopped = FALSE;
        LOGI("render thread is stopped");
        m_pLock->unlock();
        return 0;
    }

    BOOL32 CRenderer::initEGL() {
        m_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (m_eglDisplay == EGL_NO_DISPLAY) {
            // Unable to open connection to local windowing system
            LOGE("Unable to open connection to local windowing system");
            return FALSE;
        }
        EGLint majorVersion;
        EGLint minorVersion;
        if (!eglInitialize(m_eglDisplay, &majorVersion, &minorVersion)) {
            // Unable to initialize EGL. Handle and recover
            LOGE("Unable to initialize EGL. Handle and recover");
            return FALSE;
        }

        EGLint configAttribList[] = {
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, 0,
                EGL_STENCIL_SIZE, 0,
                EGL_NONE      //总是以EGL10.EGL_NONE结尾
        };
        const EGLint MaxConfigs = 10;
        EGLConfig configs[MaxConfigs]; // We'll only accept 10 configs
        EGLint numConfigs;
        EGLConfig bestMatch = NULL;
        if (!eglChooseConfig(m_eglDisplay, configAttribList, configs, MaxConfigs, &numConfigs)) {
            // Something didn't work … handle error situation
            LOGE("Something didn't work … handle error situation");
            return FALSE;
        } else {
            // Everything's okay. Continue to create a rendering surface
            if (numConfigs <= 0) {
                LOGE("No match EGLConfig for our display!");
                return FALSE;
            }
            int i = 0;
            for (; i < numConfigs; i++) {
                EGLint r, g, b, a, d, s;
                if (eglGetConfigAttrib(m_eglDisplay, configs[i], EGL_RED_SIZE, &r) &&
                    eglGetConfigAttrib(m_eglDisplay, configs[i], EGL_GREEN_SIZE, &g) &&
                    eglGetConfigAttrib(m_eglDisplay, configs[i], EGL_BLUE_SIZE, &b) &&
                    eglGetConfigAttrib(m_eglDisplay, configs[i], EGL_ALPHA_SIZE, &a) &&
                    eglGetConfigAttrib(m_eglDisplay, configs[i], EGL_DEPTH_SIZE, &d) &&
                    eglGetConfigAttrib(m_eglDisplay, configs[i], EGL_STENCIL_SIZE, &s) &&
                    r == 8 && g == 8 && b == 8 && a == 8 && d == 0 && s == 0) {
                    bestMatch = configs[i];
                    break;
                }
            }
            if (i == numConfigs) {
                bestMatch = configs[0];
            }
        }


        EGLint surfaceAttribList[] = {
                EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
                EGL_NONE
        };
        m_eglSurface = eglCreateWindowSurface(m_eglDisplay, bestMatch, m_eglWin, surfaceAttribList);
        if (m_eglSurface == EGL_NO_SURFACE) {
            switch (eglGetError()) {
                case EGL_BAD_MATCH:
                    // Check window and EGLConfig attributes to determine
                    // compatibility, or verify that the EGLConfig
                    // supports rendering to a window,
                    LOGE("Check window and EGLConfig attributes to determine compatibility, "\
                                 "or verify that the EGLConfig supports rendering to a window");
                    break;
                case EGL_BAD_CONFIG:
                    // Verify that provided EGLConfig is valid
                    LOGE("Verify that provided EGLConfig is valid");
                    break;
                case EGL_BAD_NATIVE_WINDOW:
                    // Verify that provided EGLNativeWindow is valid
                    LOGE("Verify that provided EGLNativeWindow is valid");
                    break;
                case EGL_BAD_ALLOC:
                    // Not enough resources available. Handle and recover
                    LOGE("Not enough resources available. Handle and recover");
                    break;
            }
            return FALSE;
        }
        const EGLint contextAttribList[] = {
                EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL_NONE
        };
        m_eglContext = eglCreateContext(m_eglDisplay, bestMatch, EGL_NO_CONTEXT, contextAttribList);
        if (m_eglContext == EGL_NO_CONTEXT) {
            EGLint error = eglGetError();
            if (error == EGL_BAD_CONFIG) {
                // Handle error and recover
                LOGE("eglCreateContext failed");
                return FALSE;
            }
        }
        if (!eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext)) {
            LOGE("eglMakeCurrent failed");
            return FALSE;
        }
        return TRUE;
    }

    void CRenderer::uninitEGL() {
        if (m_eglDisplay != EGL_NO_DISPLAY) {
            eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            eglDestroySurface(m_eglDisplay, m_eglSurface);
            m_eglSurface = EGL_NO_SURFACE;
        }
        if (m_eglContext != EGL_NO_CONTEXT) {
            eglDestroyContext(m_eglDisplay, m_eglContext);
            m_eglContext = EGL_NO_CONTEXT;
        }
        if (m_eglDisplay != EGL_NO_DISPLAY) {
            eglTerminate(m_eglDisplay);
            m_eglDisplay = EGL_NO_DISPLAY;
        }
    }

    BOOL32 CRenderer::initGLComponents() {
//        BOOL32 re;
        m_pOriYUVTex = new CTexture(EM_PIXEL_FORMAT_I420);
        m_pOriRGBATex = new CTexture(EM_PIXEL_FORMAT_RGBA_8888);
////        m_pMapTex = new CTexture(EM_PIXEL_FORMAT_RGBA_8888);
//        m_pNonRGBAFilter = new CVFilter;
//        re = (m_pNonRGBAFilter != NULL) ? m_pNonRGBAFilter->init() : FALSE;
//        m_pNonYUVFilter = new CVFilter;
//        re = re && ((m_pNonYUVFilter != NULL) ? m_pNonYUVFilter->init(DEFAULT_YUV_FRAGMENT_SHADER)
//                                              : FALSE);
        return (m_pOriYUVTex != NULL && m_pOriRGBATex != NULL
//                && m_pMapTex != NULL
                /* && re*/);
    }

    void CRenderer::uninitGLComponents() {
        if (m_pOriYUVTex != NULL) {
            delete m_pOriYUVTex;
            m_pOriYUVTex = NULL;
        }
        if (m_pOriRGBATex != NULL) {
            delete m_pOriRGBATex;
            m_pOriRGBATex = NULL;
        }
//        if (m_pNonRGBAFilter != NULL) {
//            delete m_pNonRGBAFilter;
//            m_pNonRGBAFilter = NULL;
//        }
//        if (m_pNonYUVFilter != NULL) {
//            delete m_pNonYUVFilter;
//            m_pNonYUVFilter = NULL;
//        }
    }

    void CRenderer::updateGLRect(u32 uGLWidth, u32 uGLHeight) {
        CAutoLock autoLock(m_pLock);
        m_uGLWidth = uGLWidth;
        m_uGLHeight = uGLHeight;
    }

    void CRenderer::onDrawnMessage(s64 sllTimeStampUS) {
        CAutoLock autoLock(m_pLock);
        if (m_cbOnMessage != NULL && m_cbDelegate != NULL) {
            char message[128];
            sprintf(message, "{timestamp:%lld}", sllTimeStampUS / 1000 + 1);
            m_cbOnMessage(m_cbDelegate, EM_MESSAGE_ID_V_RENDER_PROGRESS, message);
        }
    }

    void CRenderer::onSaveMessage(s64 sllTimeStampUS) {
        CAutoLock autoLock(m_pLock);
        if (m_cbOnMessage != NULL && m_cbDelegate != NULL) {
            char message[128];
            sprintf(message, "{timestamp:%lld}", sllTimeStampUS);
            m_cbOnMessage(m_cbDelegate, EM_MESSAGE_ID_V_SAVE_RENDER_PROGRESS, message);
        }
    }

    void CRenderer::onSendEOSMessage() {
        CAutoLock autoLock(m_pLock);
        if (m_cbOnMessage != NULL && m_cbDelegate != NULL) {
            char message[128];
            sprintf(message, "{isEOS:%d}", 1);
            m_cbOnMessage(m_cbDelegate, EM_MESSAGE_ID_V_RENDER_COMPLETE, message);
        }
    }

    void CRenderer::bindEGLNativeWindow(EGLNativeWindowType pWindow, u32 uWidth, u32 uHeight) {
        CAutoLock autoLock(m_pLock);
        m_eglWin = pWindow;
        m_uGLWidth = uWidth;
        m_uGLHeight = uHeight;
    }

    void CRenderer::setOnMessageCB(OnMessageCB cbOnMessage, void *cbDelegate) {
        CAutoLock autoLock(m_pLock);
        m_cbOnMessage = cbOnMessage;
        m_cbDelegate = cbDelegate;
    }

} // namespace paomiantv

