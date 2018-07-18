//
// Created by ASUS on 2018/6/7.
//

#ifndef MEDIAENGINE_CUTTER_H
#define MEDIAENGINE_CUTTER_H

#include <autolock.h>
#include <enum.h>
#include <thread.h>
#include <vector>
#include "mp4v2/mp4v2.h"

namespace paomiantv {
    class CCutter {
    public:
        CCutter(MP4FileHandle pFile);

        virtual ~CCutter();

        long run();

        virtual BOOL32 prepare(s8 *pSrcPath, EMTrack type, s64 startTimeUS, s64 durationUS,
                               OnMessageCB cbOnMessage = NULL,
                               void *cbDelegate = NULL);

        virtual void start();

        virtual void stop();

        virtual void resume();

        virtual void pause();

    private:
        ILock *m_pLock;
        CThread *m_pThread;

        BOOL32 m_bIsStarted;
        BOOL32 m_bIsStopped;
        BOOL32 m_bIsPaused;

        const MP4FileHandle m_pDstFile;

        MP4FileHandle m_pSrcFile;
        EMTrack m_eType;
        s64 m_sllStartTimeUS;
        s64 m_sllDurationUS;
        std::vector<MP4TrackId> m_vTracks;

        OnMessageCB m_cbOnMessage;
        void *m_cbDelegate;

        static void *ThreadWrapper(void *pData);

        void release();
    };
}

#endif //MEDIAENGINE_CUTTER_H
