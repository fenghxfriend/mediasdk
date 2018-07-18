/*******************************************************************************
 *        Module: mediasdk
 *          File: engine.cpp
 * Functionality: engine entity.
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-08-02  v1.0        huangxuefeng  created
 ******************************************************************************/

#include <version.h>
#include <engine.h>
#include <frame.h>
#include <core.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#ifdef __cplusplus
}
#endif

namespace paomiantv {

    CEngine::Garbo CEngine::garbo; // 一定要初始化，不然程序结束时不会析构garbo

    CEngine *CEngine::m_pInstance = NULL;

    CEngine *CEngine::getInstance() {
        if (m_pInstance == NULL)
            m_pInstance = new CEngine();
        return m_pInstance;
    }


    CEngine::CEngine() : m_bIsInitialize(false) {
        USE_LOG;

    }

    CEngine::~CEngine() {
        USE_LOG;
    }

    BOOL32 CEngine::init(s32 nVersion, s32 nSampleRate, s32 nBufferSize) {
        if (m_bIsInitialize) {
            LOGW("the engine is initialized!");
            return false;
        }
        MP4SetLogCallback(log_cb);
        av_register_all();
        avfilter_register_all();
        av_log_set_callback(avlog_cb);
        if (g_sdkVersion != 0) {
            g_sdkVersion = nVersion;
        }
        if (nSampleRate != 0) {
            g_sampleRate = nSampleRate;
        }
        if (g_bufferSize != 0) {
            g_bufferSize = nBufferSize;
        }
        CCore::getInstance()->setup();
        m_bIsInitialize = true;
        return TRUE;
    }

    void CEngine::uninit() {
        if (!m_bIsInitialize) {
            LOGW("the engine is uninitialized!");
        }
        CCore::getInstance()->shutdown();
        MP4SetLogCallback(NULL);
        av_log_set_callback(NULL);
        CFrameManager::clearFrame();
        CSoundManager::clearSound();
        CImageManager::clearImage();
        m_bIsInitialize = false;
    }

    void CEngine::log_cb(MP4LogLevel loglevel, const char *fmt, va_list ap) {
        switch (loglevel) {
            case MP4_LOG_NONE:
                break;
            case MP4_LOG_ERROR:
                VLOGE(fmt, ap);
                break;
            case MP4_LOG_WARNING:
                VLOGW(fmt, ap);
                break;
            case MP4_LOG_INFO:
                VLOGI(fmt, ap);
                break;
            case MP4_LOG_VERBOSE1:
            case MP4_LOG_VERBOSE2:
            case MP4_LOG_VERBOSE3:
            case MP4_LOG_VERBOSE4:
                VLOGV(fmt, ap);
                break;
            default:
                VLOGV(fmt, ap);
                break;
        }
    }

    void CEngine::avlog_cb(void *p, int loglevel, const char *fmt, va_list ap) {
        switch (loglevel) {
            case AV_LOG_QUIET:
                break;
            case AV_LOG_PANIC:
            case AV_LOG_FATAL:
            case AV_LOG_ERROR:
                VLOGE(fmt, ap);
                break;
            case AV_LOG_WARNING:
                VLOGW(fmt, ap);
                break;
            case AV_LOG_INFO:
                VLOGI(fmt, ap);
                break;
            case AV_LOG_VERBOSE:
                VLOGV(fmt, ap);
                break;
            case AV_LOG_DEBUG:
                VLOGD(fmt, ap);
                break;
            default:
                VLOGV(fmt, ap);
                break;
        }
    }
}