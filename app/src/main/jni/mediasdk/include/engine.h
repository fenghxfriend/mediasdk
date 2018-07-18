/*******************************************************************************
 *        Module: mediasdk
 *          File:
 * Functionality: define engine entity.
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

#ifndef _PAOMIANTV_ENGINE_H
#define _PAOMIANTV_ENGINE_H

#include <mp4v2/mp4v2.h>
#include <atomic>
#include "exporter.h"
#include "player.h"

namespace paomiantv {

    class CEngine {
    private:
        CEngine();

        ~CEngine();

        static CEngine *m_pInstance;

        class Garbo {
        public:
            ~Garbo() {
                if (CEngine::m_pInstance) {
                    delete CEngine::m_pInstance;
                }
            }
        };

        static Garbo garbo;
        std::atomic<bool> m_bIsInitialize;
    public:
        static CEngine *getInstance();

    private:
        static void log_cb(MP4LogLevel loglevel, const char *fmt, va_list ap);

        static void avlog_cb(void *p, int loglevel, const char *fmt, va_list ap);

    public:
        BOOL32 init(s32 nVersion, s32 nSampleRate, s32 nBufferSize);

        void uninit();

    };


} // namespace paomiantv

#endif //_PAOMIANTV_ENGINE_H
