//
// Created by ASUS on 2018/4/16.
//

#ifndef MEDIAENGINE_AUDIOTRACK_H
#define MEDIAENGINE_AUDIOTRACK_H

#include <set>
#include <thread.h>
#include <autolock.h>
#include <safequeue.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <queue>
#include <afilter.h>
#include "sound.h"
#include "bufferplayer.h"
#include "filtercomplex.h"
#include "bufferwriter.h"

namespace paomiantv {


    /*!
 * \brief   player of OpenSLES for PCM frames.
 * \note    it dependents GLContext created in java by GLSurfaceView.
 *          currently, it only support rendering YUV420P frames.
 * \author  huangxuefeng
 * \date    2018-01-29
 */
    class CAudioTrack {
    public:
        CAudioTrack();

        virtual ~CAudioTrack();

        BOOL32 configure(const u8 inputs, const s32 *ids, TAudioParams *trackParams,
                         TAudioParams outParams, BOOL32 isPlay = TRUE, BOOL32 isWrite = FALSE);

        void start();

        void pause();

        void resume();

        void stop();

        void flush();

        void setCaching(bool flag) {
            m_isCaching = flag;
        }

        bool isCaching() {
            return m_isCaching;
        }

        BOOL32 writeSound(CSound *pSound);

        void setOnMessageCB(OnMessageCB cbOnMessage, OnWritePCMCB cbOnWrite, void *cbDelegate);

    private:

        static void *ThreadWrapper(void *pThis);

        void release();

    protected:


        ILock *m_pLock;
        CThread *m_pThread;

        CAFilterComplex *m_pMixSound;

        BOOL32 m_bIsPlay;
        CBufferPlayer *m_pBufferPlayer;

        BOOL32 m_bIsWrite;
        CBufferWriter *m_pBufferWriter;

        BOOL32 m_bIsStarted;
        BOOL32 m_bIsStopped;
        BOOL32 m_bIsPaused;

        CSafeQueue<CSound> *m_pQueue;

        u32 *m_puInputIds;
        u8 **m_ppbyInputBuffer;
        int *m_pnInputFrameSize;

        bool m_isCaching;

        //key track,value audio source filter id
        std::unordered_map<s32, u32> m_mapSourceIds;

        OnMessageCB m_cbOnMessage;
        OnWritePCMCB m_cbOnWrite;
        void *m_cbDelegate;

    protected:

        virtual BOOL32 process();

        virtual BOOL32 processSound(const CSound *pSound);

        virtual void *run();
    };

} // namespace paomiantv

#endif //MEDIAENGINE_AUDIOTRACK_H
