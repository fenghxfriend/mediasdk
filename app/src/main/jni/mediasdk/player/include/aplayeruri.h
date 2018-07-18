#ifndef _PAOMIANTV_AUDIOPLAYERURI_H
#define _PAOMIANTV_AUDIOPLAYERURI_H

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "audioplayer.h"
#include "../../../common/autolock.h"

namespace paomiantv {

    class CAPlayerUri : public IURIAudioPlayer {
    public:
        CAPlayerUri();
        CAPlayerUri(const s8 *pbyBGM,const u32 uStart,const u32 uEnd,const BOOL32 bIsLoop);

        virtual ~CAPlayerUri();
        void setParams(const s8 *pbyUri, const u32 uStart, const u32 uEnd, const BOOL32 bIsLoop);
        void setPlaying(BOOL32 isPlaying);
        void setVolume(float volume);
        void seekTo(u32 time);

    public:

        void init(SLEngineItf engineEngine,SLObjectItf outputMixObject);
    private:
// uri player interfaces
        SLObjectItf uriPlayerObject;
        SLPlayItf uriPlayerPlay;
        SLSeekItf uriPlayerSeek;
        SLMuteSoloItf uriPlayerMuteSolo;
        SLVolumeItf uriPlayerVolume;

        void destory();
    };

}
#endif //_PAOMIANTV_AUDIOPLAYERURI_H
