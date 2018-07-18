

#include <assert.h>
#include "frame.h"
#include "pmanager.h"
#include "aplayerbf.h"
#include "aplayeruri.h"
#ifdef __cplusplus
extern "C"{
#endif
int g_sdkVersion = 21;
int g_sampleRate = 48000;
int g_bufferSize = 4096;
#ifdef __cplusplus
}
#endif
namespace paomiantv {
    CPManager::CPManager() {
        createEngine();
    }

    CPManager::~CPManager() {
        destory();
    }


    CPManager::Garbo CPManager::garbo; // 一定要初始化，不然程序结束时不会析构garbo

    CPManager *CPManager::m_pInstance = NULL;

    CPManager *CPManager::getInstance() {
        if (m_pInstance == NULL)
            m_pInstance = new CPManager();
        return m_pInstance;
    }

    IBQAudioPlayer *CPManager::CreateBQPlayer(EMPlayer type) {
        IBQAudioPlayer *re = NULL;
        switch (type) {
            case kAudio: {
                CAPlayerBQ* player = new CAPlayerBQ();
                player->init(engineEngine,outputMixObject);
                re = player;
            }
                break;
            case kVideo:
                re = NULL;
                break;
            default:
                re = NULL;
                break;
        }
        return re;
    }

    IURIAudioPlayer *CPManager::CreateUriPlayer(EMPlayer type,const s8* pbyUri,const u32 start,const u32 end,const BOOL32 bIsLoop) {
        IURIAudioPlayer *re = NULL;
        switch (type) {
            case kAudio: {
                CAPlayerUri* player = new CAPlayerUri(pbyUri,start,end,bIsLoop);
                player->init(engineEngine,outputMixObject);
                re = player;
            }
                break;
            case kVideo:
                re = NULL;
                break;
            default:
                re = NULL;
                break;
        }
        return re;
    }

    IURIAudioPlayer *CPManager::CreateUriPlayer(EMPlayer type) {
        IURIAudioPlayer *re = NULL;
        switch (type) {
            case kAudio: {
                CAPlayerUri* player = new CAPlayerUri();
                re = player;
            }
                break;
            case kVideo:
                re = NULL;
                break;
            default:
                re = NULL;
                break;
        }
        return re;
    }


    void CPManager::initUriPlayer(IURIAudioPlayer *p,const s8* pbyUri,const u32 start,const u32 end,const BOOL32 bIsLoop) {
        if (p != NULL) {
            p->setParams(pbyUri,start, end, bIsLoop);
            p->init(engineEngine,outputMixObject);
        }
    }

    void CPManager::DestroyPlayer(IAudioPlayer *&p) {
        if (p != NULL) {
            delete p;
            p = NULL;
        }
    }

// create the engine and output mix objects
    void CPManager::createEngine() {
        SLresult result;

        // create engine
        result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // realize the engine
        result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // get the engine interface, which is needed in order to create other objects
        result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // create output mix, with environmental reverb specified as a non-required interface
        const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
        const SLboolean req[1] = {SL_BOOLEAN_FALSE};
        result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // realize the output mix
        result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // get the environmental reverb interface
        // this could fail if the environmental reverb effect is not available,
        // either because the feature is not present, excessive CPU load, or
        // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
        result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                                  &outputMixEnvironmentalReverb);
        if (SL_RESULT_SUCCESS == result) {
            result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                    outputMixEnvironmentalReverb, &reverbSettings);
            (void) result;
        }
        // ignore unsuccessful result codes for environmental reverb, as it is optional for this example

    }

// shut down the native audio system
    void CPManager::destory() {
        // destroy output mix object, and invalidate all associated interfaces
        if (outputMixObject != NULL) {
            (*outputMixObject)->Destroy(outputMixObject);
            outputMixObject = NULL;
            outputMixEnvironmentalReverb = NULL;
        }

        // destroy engine object, and invalidate all associated interfaces
        if (engineObject != NULL) {
            (*engineObject)->Destroy(engineObject);
            engineObject = NULL;
            engineEngine = NULL;
        }
    }
}