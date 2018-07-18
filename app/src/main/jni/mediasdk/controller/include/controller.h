/*******************************************************************************
 *        Module: mediasdk
 *          File: 
 * Functionality: controller
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
#ifndef _PAOMIANTV_CONTROLLER_H_
#define _PAOMIANTV_CONTROLLER_H_

#include <typeinfo>
#include <functional>
#include <autolock.h>
#include <thread.h>
#include <storyboard.h>
#include <renderer.h>
#include <audiotrack.h>

namespace paomiantv {
    typedef struct tagMessage {
        EMCtrlMsg type;
        s64 timestamp;

        tagMessage() {
            memset(this, 0, sizeof(tagMessage));
        }
    } TMessage;

    class CController {
    public:
        CController();

        virtual ~CController();

        void setOnMessageCB(OnMessageCB cbOnMessage, void *cbDelegate);

        virtual void prepare(BOOL32 isPlay = TRUE, BOOL32 isWrite = FALSE)=0;

        virtual void start();

        virtual void stop();

        virtual void resume();

        virtual void pause();

        virtual bool seekTo(s64 sllMicrosecond) = 0;

        virtual bool locPreview(s64 sllMicrosecond)=0;


        void setCacheCount(size_t count) {
            m_cacheCount = count;
        }

        size_t getCacheCount() {
            return m_cacheCount;
        }

        size_t getCurrentCacheNumber() {
            return m_cacheNumber;
        }

        void setReadyCallback(std::function<void()> cacheReadyFunc) {
            m_cacheReadyCallback = cacheReadyFunc;
        }

    protected:
        void _resetCacheNumber();

        void _increaseCacheNumber();

    protected:
        BOOL32 m_bIsStarted;
        BOOL32 m_bIsStopped;
        BOOL32 m_bIsPaused;

        //when preview mode, the timestamp of storyboard
        s64 m_sllStartTS;
        //when preview mode, the timestamp of storyboard
        s64 m_sllEndTS;

        s64 m_sllCurTS;

        ILock *m_pLock;
        CThread *m_pThread;

        OnMessageCB m_cbOnMessage;
        void *m_cbDelegate;

        size_t m_cacheCount;
        size_t m_cacheNumber;
        std::function<void()> m_cacheReadyCallback;
        CSafeList<ITrack> *m_pTrackList;
    private:
        static void *ThreadWrapper(void *pData);

    protected:
        virtual long run()=0;

        void sendErrorMessage(s32 id);
    };
}

#endif /* _PAOMIANTV_CONTROLLER_H_ */
