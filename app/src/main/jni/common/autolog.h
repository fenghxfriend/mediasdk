/*******************************************************************************
 *        Module: common
 *          File: 
 * Functionality: auto log
 *       Related: 
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 PAOMIANTV, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-20  v1.0        huangxuefeng  created
 ******************************************************************************/
#ifndef _PAOMIANTV_AUTOLOG_H_
#define _PAOMIANTV_AUTOLOG_H_

#include <sys/time.h>
#include <typedef.h>

#ifndef LOG_ENABLE
#define LOG_ENABLE 0
#endif

#ifndef LOG_LEVEL
#define LOG_LEVEL 4
#endif

#if defined(__ANDROID__)

#include <android/log.h>

#ifndef android_printLog
#define android_printLog(prio, tag, fmt...) \
    __android_log_print(prio, tag, fmt)
#endif

#ifndef ALOG
#define ALOG(priority, tag, ...) \
    android_printLog(ANDROID_##priority, tag, __VA_ARGS__)
#endif

#ifndef android_vprintLog
#define android_vprintLog(prio, tag, ap, fmt) \
    __android_log_vprint(prio, tag, fmt, ap)
#endif

#ifndef VALOG
#define VALOG(priority, tag, ap, ...) \
    android_vprintLog(ANDROID_##priority, tag, ap, __VA_ARGS__)
#endif

#elif defined(__MACOSX__)

#elif defined(__IPHONEOS__)

#elif defined(__WIN32__)

#elif defined(__NINTENDODS__)
#else

#endif /* platform log */

namespace paomiantv {

    class CAutoLog {
    public:
        CAutoLog(const char *szFile, const char *szFunction, const s32 nLine)
                : m_szCaption(szFile),
                  m_szFunction(szFunction) {
            ALOG(LOG_DEBUG, m_szCaption, "[%s@%d] enter", m_szFunction, nLine);
        }

        ~CAutoLog() {
            ALOG(LOG_DEBUG, m_szCaption, "[%s] leave", m_szFunction);
        }

        const char *m_szCaption;
        const char *m_szFunction;
    };

    class CAutoLogReportTime {
    public:
        CAutoLogReportTime(char *szFile, char *szFunction, s32 nLine)
                : m_szCaption(szFile),
                  m_szFunction(szFunction) {
            ALOG(LOG_DEBUG, m_szCaption, "[%s@%d] enter", m_szFunction, nLine);
            gettimeofday(&begin, NULL);
        }

        ~CAutoLogReportTime() {
            struct timeval end;
            gettimeofday(&end, NULL);
            int span = 1000000 * (end.tv_sec - begin.tv_sec) + end.tv_usec - begin.tv_usec;
            ALOG(LOG_DEBUG, m_szCaption, "[%s] leave with time elapsed: %dus", m_szFunction, span);
        }

        char *m_szCaption;
        char *m_szFunction;
        struct timeval begin;
    };

#define USE_LOG (void(0))
#define USE_LOG_REPORT_TIME (void(0))

#define LOGV(fmt, ...) (void(0))
#define LOGD(fmt, ...) (void(0))
#define LOGI(fmt, ...) (void(0))
#define LOGW(fmt, ...) (void(0))
#define LOGE(fmt, ...) ALOG(LOG_ERROR, __FILE__, "[%s@%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);

#define VLOGV(fmt, ap) (void(0))
#define VLOGD(fmt, ap) (void(0))
#define VLOGI(fmt, ap) (void(0))
#define VLOGW(fmt, ap) (void(0))
#define VLOGE(fmt, ap) VALOG(LOG_ERROR, __FILE__, ap, fmt);

#if LOG_ENABLE

    #undef USE_LOG
    #define USE_LOG CAutoLog this_log(__FILE__, __FUNCTION__, __LINE__)
    #undef USE_LOG_REPORT_TIME
    #define USE_LOG_REPORT_TIME CAutoLogReportTime this_log(__FILE__, __FUNCTION__, __LINE__)

    #if (LOG_LEVEL >= 0)
        #undef LOGE
        #define LOGE(fmt, ...) ALOG(LOG_ERROR, __FILE__, "[%s@%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
        #undef VLOGE
        #define VLOGE(fmt, ap) VALOG(LOG_ERROR, __FILE__, ap, fmt);
    #endif

    #if (LOG_LEVEL >= 1)
        #undef LOGW
        #define LOGW(fmt, ...) ALOG(LOG_WARN, __FILE__, "[%s@%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
        #undef VLOGW
        #define VLOGW(fmt, ap) VALOG(LOG_WARN, __FILE__, ap, fmt);
    #endif

    #if (LOG_LEVEL >= 2)
        #undef LOGI
        #define LOGI(fmt, ...) ALOG(LOG_INFO, __FILE__, "[%s@%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
        #undef VLOGI
        #define VLOGI(fmt, ap) VALOG(LOG_INFO, __FILE__, ap, fmt);
    #endif

    #if (LOG_LEVEL >= 3)
        #undef LOGD
        #define LOGD(fmt, ...) ALOG(LOG_DEBUG, __FILE__, "[%s@%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
        #undef VLOGD
        #define VLOGD(fmt, ap) VALOG(LOG_DEBUG, __FILE__, ap, fmt);
    #endif

    #if (LOG_LEVEL >= 4)
        #undef LOGV
        #define LOGV(fmt, ...) ALOG(LOG_VERBOSE, __FILE__, "[%s@%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
        #undef VLOGV
        #define VLOGV(fmt, ap) VALOG(LOG_VERBOSE, __FILE__, ap, fmt);
    #endif

#endif

} // namespace paomiantv

#endif // _PAOMIANTV_AUTOLOG_H_