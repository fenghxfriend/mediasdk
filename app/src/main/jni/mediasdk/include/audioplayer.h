//
// Created by John.Huang on 2017/8/29.
//

#ifndef _PAOMIANTV_PLAYER_H
#define _PAOMIANTV_PLAYER_H

#include <SLES/OpenSLES.h>
#include "../../common/typedef.h"
#include "../../common/constant.h"
#include "../../common/safequeue.h"

namespace paomiantv {

    class IAudioPlayer {
    public:
        IAudioPlayer() {
        }

        virtual ~IAudioPlayer() {
        }

        virtual void setPlaying(BOOL32 isPlaying)=0;

    };

    class IBQAudioPlayer : public IAudioPlayer {
    public:
        IBQAudioPlayer() {
        }

        virtual ~IBQAudioPlayer() {
        }

        virtual void addToQueue(void *pframe)=0;

        virtual void start()=0;

        virtual void stop()=0;

        virtual void popQueue(void *&pframe)=0;

        virtual void setPlaying(BOOL32 isPlaying)=0;

    };

    class IURIAudioPlayer : public IAudioPlayer {
    public:
        IURIAudioPlayer() {

        }

        IURIAudioPlayer(const s8 *pbyUri, const u32 uStart, const u32 uEnd, const BOOL32 bIsLoop) {

        }

        virtual ~IURIAudioPlayer() {
        }

        virtual void setParams(const s8 *pbyUri, const u32 uStart, const u32 uEnd, const BOOL32 bIsLoop) = 0;

        virtual void init(SLEngineItf engineEngine, SLObjectItf outputMixObject) = 0;

        virtual void setPlaying(BOOL32 isPlaying)=0;

        virtual void setVolume(float volume)=0;

        virtual void seekTo(u32 time)=0;

        inline s8 *getSrc();

        inline u32 getStart();

        inline u32 getEnd();

    protected:
        s8 m_pbyUri[MAX_LEN_FILE_PATH];
        u32 m_uStart;
        u32 m_uEnd;
        BOOL32 m_bIsLoop;
    };

    inline s8 *IURIAudioPlayer::getSrc() {
        return m_pbyUri;
    }

    inline u32 IURIAudioPlayer::getStart() {
        return m_uStart;
    }

    inline u32 IURIAudioPlayer::getEnd() {
        return m_uEnd;
    }
}
#endif //_PAOMIANTV_PLAYER_H
