//
// Created by John.Huang on 2017/9/2.
//

#include <assert.h>
#include <jni.h>
#include <string.h>
#include <pthread.h>
#include<fcntl.h>
#include <cstdlib>
#include <math.h>
#include "../../../common/typedef.h"
#include "aplayeruri.h"

namespace paomiantv {
    CAPlayerUri::CAPlayerUri() : IURIAudioPlayer() {

    }

    CAPlayerUri::CAPlayerUri(const s8 *pbyBGM, const u32 uStart, const u32 uEnd,
                             const BOOL32 bIsLoop) : IURIAudioPlayer(pbyBGM, uStart, uEnd, bIsLoop) {
        setParams(pbyBGM, uStart, uEnd, bIsLoop);
    }

    CAPlayerUri::~CAPlayerUri() {
        destory();
    }

    void CAPlayerUri::setParams(const s8 *pbyUri, const u32 uStart, const u32 uEnd,
                                const BOOL32 bIsLoop) {
        strncpy(m_pbyUri, pbyUri, MAX_LEN_FILE_PATH);
        m_uStart = uStart;
        m_uEnd = uEnd;
        m_bIsLoop = bIsLoop;
    }

    void CAPlayerUri::init(SLEngineItf engineEngine, SLObjectItf outputMixObject) {
        SLresult result;

        // configure audio source
        SLDataLocator_URI loc_uri = {SL_DATALOCATOR_URI, (SLchar *) m_pbyUri};
        SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
        SLDataSource audioSrc = {&loc_uri, &format_mime};

        // configure audio sink
        SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
        SLDataSink audioSnk = {&loc_outmix, NULL};

        // create audio player
        const SLInterfaceID ids[3] = {SL_IID_SEEK, SL_IID_MUTESOLO, SL_IID_VOLUME};
        const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
        result = (*engineEngine)->CreateAudioPlayer(engineEngine, &uriPlayerObject, &audioSrc,
                                                    &audioSnk,
                                                    3, ids, req);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // realize the player
        result = (*uriPlayerObject)->Realize(uriPlayerObject, SL_BOOLEAN_FALSE);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // get the play interface
        result = (*uriPlayerObject)->GetInterface(uriPlayerObject, SL_IID_PLAY, &uriPlayerPlay);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // get the seek interface
        result = (*uriPlayerObject)->GetInterface(uriPlayerObject, SL_IID_SEEK, &uriPlayerSeek);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // get the mute/solo interface
        result = (*uriPlayerObject)->GetInterface(uriPlayerObject, SL_IID_MUTESOLO,
                                                  &uriPlayerMuteSolo);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // get the volume interface
        result = (*uriPlayerObject)->GetInterface(uriPlayerObject, SL_IID_VOLUME, &uriPlayerVolume);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // enable whole file looping

        result = (*uriPlayerSeek)->SetLoop(uriPlayerSeek, m_bIsLoop, m_uStart, SL_TIME_UNKNOWN);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;
    }

// set the playing state for the asset audio player
    void CAPlayerUri::setPlaying(BOOL32 isPlaying) {
        SLresult result;

        // make sure the asset audio player was created
        if (NULL != uriPlayerPlay) {

            // set the player's state
            result = (*uriPlayerPlay)->SetPlayState(uriPlayerPlay, isPlaying ?
                                                                   SL_PLAYSTATE_PLAYING
                                                                             : SL_PLAYSTATE_PAUSED);
            assert(SL_RESULT_SUCCESS == result);
            (void) result;
        }

    }


// set the playing volume for the audio player
    void CAPlayerUri::setVolume(float volume) {
        SLresult result;
        // make sure the asset audio player was created
        if (NULL != uriPlayerVolume) {
//            SLmillibel oldvolume = 0;
//            result = (*uriPlayerVolume)->GetVolumeLevel(uriPlayerVolume, &oldvolume);
//            assert(SL_RESULT_SUCCESS == result);
//            SLmillibel maxvolume = 0;
//            result = (*uriPlayerVolume)->GetMaxVolumeLevel(uriPlayerVolume, &maxvolume);
//            assert(SL_RESULT_SUCCESS == result);
            // set the player's state
            float attenuation = 1.0f / 1024.0f + volume * 1023.0f / 1024.0f;
            float db = 3 * log10(attenuation) / log10(2);
            SLmillibel setGain = (SLmillibel)(db * 167);
            result = (*uriPlayerVolume)->SetVolumeLevel(uriPlayerVolume, setGain);
            assert(SL_RESULT_SUCCESS == result);
            (void) result;
        }
    }

    void CAPlayerUri::seekTo(u32 time) {
        SLresult result;
        // make sure the asset audio player was created
        if (NULL != uriPlayerSeek) {
            // set the player's state
            result = (*uriPlayerSeek)->SetPosition(uriPlayerSeek, time, SL_SEEKMODE_FAST);
            assert(SL_RESULT_SUCCESS == result);
            (void) result;
        }

    }

// shut down the native audio system
    void CAPlayerUri::destory() {

        // destroy file descriptor audio player object, and invalidate all associated interfaces
        if (uriPlayerObject != NULL) {
            (*uriPlayerObject)->Destroy(uriPlayerObject);
            uriPlayerObject = NULL;
            uriPlayerPlay = NULL;
            uriPlayerSeek = NULL;
            uriPlayerMuteSolo = NULL;
            uriPlayerVolume = NULL;
        }
    }
}