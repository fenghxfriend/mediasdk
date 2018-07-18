//
// Created by John.Huang on 2017/9/2.
//

#include <assert.h>
#include "aplayerbf.h"
#include "../../../common/version.h"

namespace paomiantv {

    // this callback handler is called every time a buffer finishes playing
    void BQPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
        CAPlayerBQ *aplayer = (CAPlayerBQ *) context;
        assert(bq == aplayer->bqPlayerBufferQueue);
        BOOL32 played = FALSE;
        while (!played){
            aplayer->m_pLock->lock();
            if(aplayer->m_bIsStop){
                aplayer->m_pLock->unlock();
                break;
            }
            aplayer->m_pLock->unlock();
            if (aplayer->m_pFrame == NULL) {
                aplayer->popQueue((void *&) aplayer->m_pFrame);
                if (aplayer->m_pFrame == NULL || aplayer->m_pFrame->data == NULL ||
                    aplayer->m_pFrame->size <= 0) {
                    LOGE("frame is invalid! ");
                    if (aplayer->m_pFrame != NULL) {
                        LOGE("frame is invalid! ");
                        CPCMFrame::release(aplayer->m_pFrame);
                        aplayer->m_pFrame = NULL;
                    }
                    continue;
                }
            }

            // play audio
            SLresult result;
            if(aplayer->m_nSize>0){
                if(aplayer->m_nSize+aplayer->m_pFrame->size<g_bufferSize){
                    memcpy(aplayer->m_pbyBuffer+aplayer->m_nSize,aplayer->m_pFrame->data,aplayer->m_pFrame->size);
                    aplayer->m_nSize +=aplayer->m_pFrame->size;
                    CPCMFrame::release(aplayer->m_pFrame);
                    aplayer->m_pFrame = NULL;
                    aplayer->m_nOffset = 0;
                }else{
                    memcpy(aplayer->m_pbyBuffer+aplayer->m_nSize,aplayer->m_pFrame->data,g_bufferSize-aplayer->m_nSize);
                    aplayer->m_nSize =0;
                    if(aplayer->m_nSize+aplayer->m_pFrame->size==g_bufferSize){
                        CPCMFrame::release(aplayer->m_pFrame);
                        aplayer->m_pFrame = NULL;
                        aplayer->m_nOffset = 0;
                    }else{
                        aplayer->m_nOffset = g_bufferSize-aplayer->m_nSize;
                    }
                    result = (*(aplayer->bqPlayerBufferQueue))->Enqueue(aplayer->bqPlayerBufferQueue,
                                                                        aplayer->m_pbyBuffer,
                                                                        g_bufferSize);
                    // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
                    // which for this code example would indicate a programming error
                    if(SL_RESULT_SUCCESS == result){
                        played = TRUE;
                    }
                }
            }else{
                if(aplayer->m_pFrame->size - aplayer->m_nOffset < g_bufferSize){
                    memcpy(aplayer->m_pbyBuffer,aplayer->m_pFrame->data+aplayer->m_nOffset,aplayer->m_pFrame->size - aplayer->m_nOffset);
                    aplayer->m_nSize = aplayer->m_pFrame->size - aplayer->m_nOffset;
                    CPCMFrame::release(aplayer->m_pFrame);
                    aplayer->m_pFrame = NULL;
                    aplayer->m_nOffset = 0;
                }else{
                    s32 offset = aplayer->m_nOffset;
                    CPCMFrame* frame = aplayer->m_pFrame;
                    if(aplayer->m_pFrame->size - aplayer->m_nOffset == g_bufferSize){
                        CPCMFrame::release(aplayer->m_pFrame);
                        aplayer->m_pFrame = NULL;
                        aplayer->m_nOffset = 0;
                    }else{
                        aplayer->m_nOffset += g_bufferSize;
                    }
                    result = (*(aplayer->bqPlayerBufferQueue))->Enqueue(aplayer->bqPlayerBufferQueue,
                                                                        frame->data+offset,
                                                                        g_bufferSize);
                    // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
                    // which for this code example would indicate a programming error
                    if(SL_RESULT_SUCCESS == result){
                        played = TRUE;
                    }
                }
            }
        };
    }


    CAPlayerBQ::CAPlayerBQ() : m_bIsActived(FALSE), m_pFrame(NULL), m_nOffset(0),m_nSize(0),m_bIsStop(FALSE) {
        m_pQueue = new CSafeQueue<CPCMFrame>();
        m_pLock = new CLock;
        m_pThread = new CThread(ThreadWrapper, this);
        m_pbyBuffer = (u8*)malloc(g_bufferSize);
    }

    CAPlayerBQ::~CAPlayerBQ() {
        destory();
        if(m_pbyBuffer!=NULL){
            free(m_pbyBuffer);
            m_pbyBuffer = NULL;
        }
        if(m_pLock!=NULL){
            delete m_pLock;
            m_pLock = NULL;
        }
        if(m_pThread!=NULL){
            delete m_pThread;
            m_pThread = NULL;
        }
        if (m_pQueue != NULL) {
            delete m_pQueue;
            m_pQueue = NULL;
        }
    }

    void CAPlayerBQ::init(SLEngineItf engineEngine, SLObjectItf outputMixObject) {
        SLresult result;
        // configure audio source
        SLDataLocator_BufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
        SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
                                       SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                       SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
        format_pcm.numChannels = 2;
        format_pcm.samplesPerSec = g_sampleRate * 1000;

        format_pcm.bitsPerSample = 16;
        format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;

        SLDataSource audioSrc = {&loc_bufq, &format_pcm};

        // configure audio sink
        SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
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

        result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc,
                                                    &audioSnk,
                                                    3, ids, req);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // realize the player
        result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // get the play interface
        result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // get the buffer queue interface
        result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                                 &bqPlayerBufferQueue);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // register callback on the buffer queue
        result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, BQPlayerCallback,
                                                          this);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // get the effect send interface
        result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND,
                                                 &bqPlayerEffectSend);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // get the volume interface
        result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;

        // set the player's state to playing
        result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;
    }

    //static
    void *CAPlayerBQ::ThreadWrapper(void *pThis) {
        CAPlayerBQ *p = (CAPlayerBQ *) pThis;
        p->m_pThread->setName(typeid(*p).name());
        int nErr = p->ThreadEntry();
        return (void *) nErr;
    }

    int CAPlayerBQ::ThreadEntry() {
        int nErr = 0;
        CAutoLock autoLock(m_pLock);
        while (!m_bIsActived){
            if(m_bIsStop){
                break;
            }
            popQueue((void *&) m_pFrame);
            if (m_pFrame == NULL || m_pFrame->data == NULL || m_pFrame->size <= 0) {
                LOGE("frame is invalid! ");
                if (m_pFrame != NULL) {
                    LOGE("frame is invalid! ");
                    CPCMFrame::release(m_pFrame);
                    m_pFrame = NULL;
                }
                continue;
            }
            // play audio
            SLresult result;
            if (m_nOffset+ g_bufferSize > m_pFrame->size) {
                memcpy(m_pbyBuffer,m_pFrame->data,m_pFrame->size);
                CPCMFrame::release(m_pFrame);
                m_pFrame = NULL;
                m_nOffset = 0;
                m_nSize = m_pFrame->size;

            } else {
                result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue,
                                                         m_pFrame->data,
                                                         g_bufferSize);
                // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
                // which for this code example would indicate a programming error
                if(SL_RESULT_SUCCESS == result){
                    m_bIsActived = TRUE;
                }
                if(m_pFrame->size==m_nOffset){
                    CPCMFrame::release(m_pFrame);
                    m_pFrame = NULL;
                    m_nOffset = 0;
                    m_nSize = 0;
                }else{
                    m_nOffset += g_bufferSize;
                    m_nSize = 0;
                }
            }
        }
        return 0;
    }

    void CAPlayerBQ::addToQueue(void *pframe) {
        if (pframe == NULL || ((CPCMFrame *)pframe)->data == NULL) {
            LOGE("frame not invalid! ");
            return;
        }
        if (!m_pQueue->push((CPCMFrame *)pframe)) {
            CPCMFrame::release((CPCMFrame *)pframe);
            return;
        }
    }
    void CAPlayerBQ::start() {
        CAutoLock autoLock(m_pLock);
        if (!m_bIsActived) {
            m_pThread->start();
        }
    }
    void CAPlayerBQ::stop() {
        m_pLock->lock();
        m_bIsStop = TRUE;
        m_pLock->unlock();
        CPCMFrame *frame = NULL;
        while (!m_pQueue->empty()) {
            m_pQueue->pop(frame);
            if (frame != NULL) {
                CPCMFrame::release(frame);
                frame = NULL;
            }
        }
    }

    void CAPlayerBQ::popQueue(void *&pframe) {
        m_pQueue->pop((CPCMFrame *&) pframe);
    }

    BOOL32 CAPlayerBQ::isQueueEmpty() {
        return m_pQueue->empty();
    }


// set the playing state for the asset audio player
    void CAPlayerBQ::setPlaying(BOOL32 isPlaying) {
        SLresult result;

        // make sure the asset audio player was created
        if (NULL != bqPlayerPlay) {
            // set the player's state
            result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, isPlaying ? SL_PLAYSTATE_PLAYING
                                                                           : SL_PLAYSTATE_PAUSED);
            assert(SL_RESULT_SUCCESS == result);
            (void) result;
        }

    }

// shut down the native audio system
    void CAPlayerBQ::destory() {

        // destroy buffer queue audio player object, and invalidate all associated interfaces
        if (bqPlayerObject != NULL) {
            (*bqPlayerObject)->Destroy(bqPlayerObject);
            bqPlayerObject = NULL;
            bqPlayerPlay = NULL;
            bqPlayerBufferQueue = NULL;
            bqPlayerEffectSend = NULL;
            bqPlayerMuteSolo = NULL;
            bqPlayerVolume = NULL;
        }
    }
}