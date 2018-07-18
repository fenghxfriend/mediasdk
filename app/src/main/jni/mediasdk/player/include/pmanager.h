//
// Created by John.Huang on 2017/9/2.
//

#ifndef _PAOMIANTV_JNIAUDIOPLAYER_H
#define _PAOMIANTV_JNIAUDIOPLAYER_H

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "audioplayer.h"
#include "../../../common/autolock.h"

namespace paomiantv {

    typedef enum {
        kVideo,
        kAudio,
        kEnd,
    } EMPlayer;

    class CPManager {
    private:
        CPManager();

        virtual ~CPManager();

        static CPManager *m_pInstance;

        class Garbo {
        public:
            ~Garbo() {
                if (CPManager::m_pInstance) {
                    delete CPManager::m_pInstance;
                }
            }
        };

        static Garbo garbo;

    public:
        static CPManager *getInstance();
        IBQAudioPlayer *CreateBQPlayer(EMPlayer type = kAudio);
        IURIAudioPlayer *CreateUriPlayer(EMPlayer type,const s8* pbyUri,const u32 start,const u32 end,const BOOL32 bIsLoop);
        IURIAudioPlayer * CreateUriPlayer(EMPlayer type);
        void initUriPlayer(IURIAudioPlayer *p,const s8* pbyUri,const u32 start,const u32 end,const BOOL32 bIsLoop);

        void DestroyPlayer(IAudioPlayer *&p);

    private:
        // engine interfaces
        SLObjectItf engineObject = NULL;
        SLEngineItf engineEngine;

// output mix interfaces
        SLObjectItf outputMixObject = NULL;
        SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

// aux effect on the output mix, used by the buffer queue player
        const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;

        void createEngine();

        void destory();
    };

}
#endif //_PAOMIANTV_JNIAUDIOPLAYER_H
