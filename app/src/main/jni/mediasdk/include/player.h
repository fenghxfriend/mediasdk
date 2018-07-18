//
// Created by ASUS on 2018/2/8.
//

#ifndef MEDIAENGINE_PLAYER_H
#define MEDIAENGINE_PLAYER_H

#include <storyboard.h>
#include <controller.h>
#include <renderer.h>
#include <audiotrack.h>
#include <vcontroller.h>
#include <acontroller.h>

namespace paomiantv {

    class IPlayer {
    protected:
        IPlayer() {
        }

    public:
        virtual ~IPlayer() {
        }

        virtual void prepare()=0;

        virtual void start()=0;

        virtual void pause()=0;

        virtual void resume()=0;

        virtual void stop()=0;
    };

    class CPlayer {
    public:
        CPlayer();

    public:
        static CPlayer *Create();

        virtual ~CPlayer();

        virtual void setDataSource(CStoryboard **ppStoryboard);

        virtual void prepare();

        virtual void start();

        virtual void pause();

        virtual void resume();

        virtual void stop();

        virtual void setOnMessageCB(OnMessageCB cbOnMessage, void *cbDelegate);

        virtual void bindSurface(EGLNativeWindowType pWindow, u32 uWidth, u32 uHeight);

        virtual void surfaceSizeChanged(u32 uGLWidth, u32 uGLHeight);

        virtual void unbindSurface(void (*releaseWindow)(EGLNativeWindowType));

        virtual void seekTo(s64 sllMicrosecond);

        virtual void locPreview(s64 sllMicrosecond);

    protected:
        int init();

        void onAudioReady();

        void onVideoReady();

    protected:
        // either storyboard or url is effective
        CStoryboard **m_ppStoryboard;// the storyboard for play
        // is playing or not
        BOOL32 m_bIsPlaying;

        CVController *m_pVController;//video ctrl for dealing video data
        CAController *m_pAController;//audio ctrl for dealing audio data

        CRenderer *m_pRender;//the renderer for rendering video
        CAudioTrack *m_pAudioTrack;//the audiotrack for play audio

        ILock *m_playerLock;
        bool m_isAudioReady;
        bool m_isVideoReady;
        bool m_isAudioPreparing;
        bool m_isVideoPreparing;

        EMPlayerStatus m_eStatus;//player status

//        OnMessageCB m_cbOnMessage;//send message to UI,for update UI
//        void *m_cbDelegate;//the delegate for sending message to UI
    };

}

#endif //MEDIAENGINE_PLAYER_H
