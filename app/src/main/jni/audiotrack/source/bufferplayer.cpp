//
// Created by ASUS on 2018/4/20.
//

#include <bufferplayer.h>
#include <version.h>
#include <unistd.h>

namespace paomiantv {

    // this callback handler is called every time a buffer finishes playing
    void CBufferPlayer::BQPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
        CBufferPlayer *player = (CBufferPlayer *) context;
        if (player == NULL) {
            LOGE("audio track is invalid!")
            return;
        }
        player->play();
    }

    CBufferPlayer *
    CBufferPlayer::Create(EMSampleRate eSampleRate, u16 wEncoding, u32 uChannel,
                          u32 uChannelMask) {
        CBufferPlayer *p = new CBufferPlayer(eSampleRate, wEncoding, uChannel, uChannelMask);
        if (p->prepare()) {
            return p;
        } else {
            delete p;
            return NULL;
        }
    }

    CBufferPlayer::CBufferPlayer(EMSampleRate eSampleRate, u16 wEncoding, u32 uChannel,
                                 u32 uChannelMask) :
            m_pbyRemainderBuffer(NULL),
            m_uHwRemainderBufferSize(0),
            m_uPlayTimeStampUS(0),
            m_bIsStopped(FALSE),
            m_bIsPaused(FALSE),
            m_uHwBufferDurationUS(0),
            m_pLastFrame(NULL),
            m_eSampleRate(eSampleRate),
            m_uChannel(uChannel),
            m_wEncoding(wEncoding),
            m_uChannelMask(uChannelMask),
            m_cbDelegate(NULL),
            m_cbOnMessage(NULL) {
        USE_LOG;
        m_pAudios = new CSafeQueue<CAudio>(10);
        m_pAEngine = new CAEngine;
        m_pLock = new CLock;

        bqPlayerObject = nullptr;
        bqPlayerPlay = nullptr;
        bqPlayerBufferQueue = nullptr;
        bqPlayerEffectSend = nullptr;
        bqPlayerMuteSolo = nullptr;
        bqPlayerVolume = nullptr;

        m_uHwBufferCapacity = 0;
        m_uHwRemainderBufferSize = 0;
        m_uHwBufferDurationUS = 0;
        m_uPlayTimeStampUS = 0;
        m_pbyRemainderBuffer = nullptr;
        m_cbOnMessage = nullptr;
        m_cbDelegate = nullptr;
    }

    CBufferPlayer::~CBufferPlayer() {
        USE_LOG;
        m_pAudios->disable();
        m_bIsStopped = TRUE;
        m_bIsPaused = FALSE;
        release();
        if (m_pAEngine != NULL) {
            delete m_pAEngine;
            m_pAEngine = NULL;
        }
        if (m_pAudios != NULL) {
            delete m_pAudios;
            m_pAudios = NULL;
        }

        if (m_pLock != NULL) {
            delete m_pLock;
            m_pLock = NULL;
        }
    }

    void CBufferPlayer::destroy() {
        delete this;
    }

    static FILE *fp = NULL;

    static void savePCM(u8 *in, u32 size) {
        if (fp == NULL) {
            if ((fp = fopen("/storage/emulated/0/raw.pcm", "wb")) == NULL) {
                LOGE("open raw.pcm failed");
            }
        }
        if (fp != NULL) {
//            fwrite(in, 1, size, fp);
        }

    }

    void CBufferPlayer::play() {
        if (m_bIsStopped) {
            return;
        }
        while (m_bIsPaused) {
            usleep(3000);
            if (m_bIsStopped) {
                return;
            }
        }

        if (m_pLastFrame != NULL) {
            CAudio::release(m_pLastFrame);
        }

        CAudio *audio = NULL;
        if (!m_pAudios->pop(audio) && audio != NULL) {
            CAudio::release(audio);
            audio = NULL;
        }
        if (audio == NULL) {
            audio = CAudio::create();
            audio->resize(m_uHwBufferCapacity);
            audio->m_uSize = m_uHwBufferCapacity;
        }
        SLresult result;
        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue,
                                                 audio->m_pbyVoice + audio->m_uOffset,
                                                 audio->m_uSize);
        m_pLastFrame = audio;
        if (audio->isEOS) {
            onSendEOSMessage();
        }

        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
        // which for this code example would indicate a programming error
        if (SL_RESULT_SUCCESS == result) {
            // udpate audio timestamp
            m_uPlayTimeStampUS += m_uHwBufferDurationUS;
            onPlayedMessage(m_uPlayTimeStampUS);
        }

    }

    void CBufferPlayer::flush() {
        if (!m_pAudios) {
            return;
        }
        m_pAudios->disable();
        m_pAudios->clear();
        m_pAudios->enable();
    }

    void CBufferPlayer::pause() {
        m_bIsPaused = TRUE;
    }

    void CBufferPlayer::resume() {
        m_bIsPaused = FALSE;
    }

    void CBufferPlayer::release() {
        // destroy buffer queue audio player object, and invalidate all associated interfaces
        CAutoLock autoLock(m_pLock);
        if (bqPlayerObject != NULL) {
            (*bqPlayerObject)->Destroy(bqPlayerObject);
            bqPlayerObject = NULL;
            bqPlayerPlay = NULL;
            bqPlayerBufferQueue = NULL;
            bqPlayerEffectSend = NULL;
            bqPlayerMuteSolo = NULL;
            bqPlayerVolume = NULL;
        }
        if (m_pbyRemainderBuffer != NULL) {
            free(m_pbyRemainderBuffer);
            m_pbyRemainderBuffer = NULL;
        }
        m_uHwRemainderBufferSize = 0;
        if (m_pLastFrame != NULL) {
            CAudio::release(m_pLastFrame);
            m_pLastFrame = NULL;
        }
        m_uHwBufferDurationUS = 0;
        m_uPlayTimeStampUS = 0;
    }

    BOOL32
    CBufferPlayer::prepare() {
        m_pAudios->clear();
        m_pAudios->enable();
        if (m_pAEngine->getEngineObject() == NULL || m_pAEngine->getMixObject() == NULL) {
            LOGE("openSL engine is NULL, can not create player!");
            return FALSE;
        }
        SLresult result;
        do {
            // configure audio source
            SLDataLocator_BufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                  AUDIO_TRACK_HWAAC_BUFFER_SIZE};
            SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
                                           SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                           SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
            format_pcm.numChannels = m_uChannel;
            format_pcm.samplesPerSec = m_eSampleRate * 1000;
            format_pcm.bitsPerSample = (u32) (m_wEncoding * 8);
            format_pcm.channelMask = m_uChannelMask;

            SLDataSource audioSrc = {&loc_bufq, &format_pcm};

            // configure audio sink
            SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX,
                                                  m_pAEngine->getMixObject()};
            SLDataSink audioSnk = {&loc_outmix, NULL};

            /*
             * create audio player:
             *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
             *     for fast audio case
             */
            const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_EFFECTSEND,
                    /*SL_IID_MUTESOLO,*/};
            const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
                    /*SL_BOOLEAN_TRUE,*/ };

            result = (*m_pAEngine->getEngineObject())->CreateAudioPlayer(
                    m_pAEngine->getEngineObject(), &bqPlayerObject, &audioSrc,
                    &audioSnk,
                    3, ids, req);
            if (SL_RESULT_SUCCESS != result)break;

            // realize the player
            result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
            if (SL_RESULT_SUCCESS != result)break;

            // get the play interface
            result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
            if (SL_RESULT_SUCCESS != result)break;

            // get the buffer queue interface
            result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                                     &bqPlayerBufferQueue);
            if (SL_RESULT_SUCCESS != result)break;
            // register callback on the buffer queue
            result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, BQPlayerCallback,
                                                              this);
            if (SL_RESULT_SUCCESS != result)break;

            // get the effect send interface
            result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND,
                                                     &bqPlayerEffectSend);
            if (SL_RESULT_SUCCESS != result)break;

            // get the volume interface
            result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME,
                                                     &bqPlayerVolume);
            if (SL_RESULT_SUCCESS != result)break;

            // set the player's state to playing
            result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
            if (SL_RESULT_SUCCESS != result)break;

            m_uHwBufferCapacity =
                    g_bufferSize * format_pcm.bitsPerSample / 8 * m_uChannel;

            m_pbyRemainderBuffer = (u8 *) malloc(m_uHwBufferCapacity);

            if (m_pbyRemainderBuffer == NULL) {
                result = SL_RESULT_MEMORY_FAILURE;
                break;
            }

            memset(m_pbyRemainderBuffer, 0, m_uHwBufferCapacity);
            m_uHwBufferDurationUS = g_bufferSize * 1000 * 1000 / m_eSampleRate;

        } while (0);
        if (SL_RESULT_SUCCESS != result) {
            LOGE("create openSL player failed!")
            release();
            return FALSE;
        }
        return TRUE;
    }

    BOOL32 CBufferPlayer::sendBuffer(u8 *in, int size) {
        // handle the processed data
        if (m_bIsStopped) {
            return FALSE;
        }
        if (in == NULL || size == 0) {
            CAudio *audio = CAudio::create();
            audio->resize(m_uHwBufferCapacity);
            audio->m_uSize = m_uHwBufferCapacity;
            audio->isEOS = TRUE;
            if (m_uHwRemainderBufferSize > 0) {
                memcpy(audio->m_pbyVoice, m_pbyRemainderBuffer,
                       m_uHwRemainderBufferSize);
            }
            if (!m_pAudios->push(audio)) {
                CAudio::release(audio);
            }
            m_uHwRemainderBufferSize = 0;
            return TRUE;
        }
        u32 sum = size + m_uHwRemainderBufferSize;
        u32 count = sum / m_uHwBufferCapacity;
        u32 rmd = sum % m_uHwBufferCapacity;

        u32 outOffset = 0;
        for (u32 i = 0; i < count; i++) {
            CAudio *audio = CAudio::create();
            audio->resize(m_uHwBufferCapacity);
            audio->m_uSize = m_uHwBufferCapacity;
            if (m_uHwRemainderBufferSize > 0) {
                memcpy(audio->m_pbyVoice, m_pbyRemainderBuffer,
                       m_uHwRemainderBufferSize);
                memcpy(audio->m_pbyVoice + m_uHwRemainderBufferSize, in,
                       m_uHwBufferCapacity - m_uHwRemainderBufferSize);
                outOffset += (m_uHwBufferCapacity - m_uHwRemainderBufferSize);
                m_uHwRemainderBufferSize = 0;
            } else {
                memcpy(audio->m_pbyVoice, in + outOffset, m_uHwBufferCapacity);
                outOffset += m_uHwBufferCapacity;
            }
            if (!m_pAudios->push(audio)) {
                CAudio::release(audio);
            }
        }
#ifdef LOG_ENABLE
        if (rmd != size - outOffset + m_uHwRemainderBufferSize) {
            LOGE("this situation will never happend!");
        }
#endif
        memcpy(m_pbyRemainderBuffer + m_uHwRemainderBufferSize, in + outOffset,
               size - outOffset);
        m_uHwRemainderBufferSize += (size - outOffset);
        return TRUE;
    }

    void CBufferPlayer::onSendEOSMessage() {
        if (m_cbOnMessage != NULL && m_cbDelegate != NULL) {
            char message[128];
            sprintf(message, "{isEOS:%d}", 1);
            m_cbOnMessage(m_cbDelegate, EM_MESSAGE_ID_A_PLAY_COMPLETE, message);
        }
    }

    void CBufferPlayer::onPlayedMessage(s64 sllTimeStamp) {
        CAutoLock autoLock(m_pLock);
        if (m_cbOnMessage != NULL && m_cbDelegate != NULL) {
            char message[128];
            sprintf(message, "{timestamp:%lld}", sllTimeStamp / 1000 + 1);
            m_cbOnMessage(m_cbDelegate, EM_MESSAGE_ID_A_PLAY_PROGRESS, message);
        }
    }

    void CBufferPlayer::setOnMessageCB(OnMessageCB cbOnMessage, void *cbDelegate) {
        CAutoLock autoLock(m_pLock);
        m_cbOnMessage = cbOnMessage;
        m_cbDelegate = cbDelegate;
    }

    void CBufferPlayer::start() {
        m_pAudios->enable();
        CAutoLock autoLock(m_pLock);
        m_bIsStopped = FALSE;
        m_bIsPaused = FALSE;
        SLresult result = 0;
        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue,
                                                 m_pbyRemainderBuffer,
                                                 m_uHwBufferCapacity);
//            m_bIsPlayed = TRUE;
        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
        // which for this code example would indicate a programming error
        if (SL_RESULT_SUCCESS != result) {
            LOGE("active player with silence frame failed!");
        }
    }

    void CBufferPlayer::stop() {
        m_pAudios->disable();
        m_pAudios->clear();
        CAutoLock autoLock(m_pLock);
        m_bIsStopped = TRUE;
    }
}