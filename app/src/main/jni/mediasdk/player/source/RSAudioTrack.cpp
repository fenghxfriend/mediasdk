#include "RSAudioTrack.h"
#include <thread>
#include <math.h>
#include <unistd.h>
#include <android/log.h>
#include "audiofilter.h"
#include "../../../common/RSSemaphore.h"
#include "../../../common/version.h"

///////////////////////////////////////////////////////////
/// jni macros and global variables
#define DELETE_LOCAL_REF(env, obj)  if(obj!=NULL){env->DeleteLocalRef(obj);obj=NULL;}  
#define DELETE_GLOBAL_REF(env, obj) if(obj!=NULL){env->DeleteGlobalRef(obj);obj=NULL;}  
#define DELETE_WEAK_GLOBAL_REF(env, obj) if(obj!=NULL){env->DeleteWeakGlobalRef(obj);obj=NULL;}  
#define JNI_FIELDID(name, field) fieldID_##name_##field
#define AUDIOTRACK_FIELD JNI_FIELDID(RSAudioTrack, _nativeHandle)

static jfieldID AUDIOTRACK_FIELD = NULL;
static const int MAX_WAIT_TIME = 1000; ///< 1000 milliseconds.
static const int POOL_TRUNK_COUNT = 16;
static const int PLAYING_BUFFER_COUNT = 2;
static std::string s_fullClassName = "";

///////////////////////////////////////////////////////////
/// jni methods

#if defined(__cpluscplus)
extern "C"
{
#endif

static inline RSAudioTrack * _getAudioTrack(JNIEnv* env, jobject thiz)
{
    if (NULL == env){
        return nullptr;
    }
    jlong nativeHandle = env->GetLongField(thiz, AUDIOTRACK_FIELD);
    return (RSAudioTrack*)nativeHandle;
}

static jboolean jni_initialize(JNIEnv *env, jobject thiz, jint sampleRate, jint channelCount, jint bytesPerSample, jfloat bufferTime)
{
    RSAudioTrack *pAudioTrack = _getAudioTrack(env, thiz);
    if (pAudioTrack) {
        return pAudioTrack->initialize((int)sampleRate, (int)channelCount, (int)bytesPerSample, (float)bufferTime);
    }
    return false;
}

static jint jni_getSampleRate(JNIEnv* env, jobject thiz)
{
    RSAudioTrack *pAudioTrack = _getAudioTrack(env, thiz);
    if (pAudioTrack) {
        return pAudioTrack->getSampleRate();
    }
    return 0;
}

static jint jni_getChannelCount(JNIEnv* env, jobject thiz)
{
    RSAudioTrack *pAudioTrack = _getAudioTrack(env, thiz);
    if (pAudioTrack) {
        return pAudioTrack->getChannelCount();
    }
    return 0;
}

static jint jni_getBytesPerSample(JNIEnv* env, jobject thiz)
{
    RSAudioTrack *pAudioTrack = _getAudioTrack(env, thiz);
    if (pAudioTrack) {
        return pAudioTrack->getBytesPerSample();
    }
    return 0;
}

static jfloat jni_getBufferTime(JNIEnv* env, jobject thiz)
{
    RSAudioTrack *pAudioTrack = _getAudioTrack(env, thiz);
    if (pAudioTrack) {
        return pAudioTrack->getBufferTime();
    }
    return 0.0f;
}

static void jni_setVolume(JNIEnv* env, jobject thiz, jfloat vol)
{
    RSAudioTrack *pAudioTrack = _getAudioTrack(env, thiz);
    if (pAudioTrack) {
        pAudioTrack->setVolume((float)vol);
    }
}

static jfloat jni_getVolume(JNIEnv* env, jobject thiz)
{
    RSAudioTrack *pAudioTrack = _getAudioTrack(env, thiz);
    if (pAudioTrack) {
        return pAudioTrack->getVolume();
    }
    return 0.0f;
}

static void jni_play(JNIEnv* env, jobject thiz)
{
    RSAudioTrack *pAudioTrack = _getAudioTrack(env, thiz);
    if (pAudioTrack) {
        pAudioTrack->play();
    }
}

static void jni_pause(JNIEnv* env, jobject thiz)
{
    RSAudioTrack *pAudioTrack = _getAudioTrack(env, thiz);
    if (pAudioTrack) {
        pAudioTrack->pause();
    }
}

static void jni_stop(JNIEnv* env, jobject thiz)
{
    RSAudioTrack *pAudioTrack = _getAudioTrack(env, thiz);
    if (pAudioTrack) {
        pAudioTrack->stop();
    }
}

static void jni_release(JNIEnv* env, jobject thiz)
{
    RSAudioTrack *pAudioTrack = _getAudioTrack(env, thiz);
    if (pAudioTrack) {
        pAudioTrack->release();
    }
}

static void jni_write(JNIEnv* env, jobject thiz, jbyteArray audioData, jint offset, jint dataSize)
{
    RSAudioTrack *pAudioTrack = _getAudioTrack(env, thiz);
    if (pAudioTrack) {
        if (0 > offset) {
            offset = 0;
        }

        jbyte *nativeBuffer = env->GetByteArrayElements(audioData, 0);
        jint fullLen = (jint)env->GetArrayLength(audioData);
        if ((offset + dataSize) <= fullLen){
            pAudioTrack->write((unsigned char *)nativeBuffer, (int)offset, (int)dataSize);
        }
        env->ReleaseByteArrayElements(audioData, nativeBuffer, 0);
    }
}

static void jni_flush(JNIEnv* env, jobject thiz)
{
    RSAudioTrack *pAudioTrack = _getAudioTrack(env, thiz);
    if (pAudioTrack) {
        pAudioTrack->flush();
    }
}

static void jni_setupNativeObject(JNIEnv* env, jobject thiz)
{
    if (NULL == env || NULL == thiz) {
        //Log error.
        return;
    }
    if (_getAudioTrack(env, thiz)) {
        return;
    }
    RSAudioTrack *pAudioTrack = new RSAudioTrack();
    pAudioTrack->bindJni(env, thiz);
}

static void jni_releaseNativeObject(JNIEnv* env, jobject thiz)
{
    RSAudioTrack *pAudioTrack = _getAudioTrack(env, thiz);
    if (pAudioTrack) {
        pAudioTrack->unbindJni(env, thiz);
        delete pAudioTrack;
    }
}

#if defined(__cpluscplus)
}
#endif

///////////////////////////////////////////////////////////
/// RSAudioTrack Implementation
RSAudioTrack::RSAudioTrack()
    :_jniObject(NULL)
    ,_sampleRate(0)
    ,_channelCount(0)
    ,_bytesPerSample(0)
    ,_bufferTime(0.0f)
    ,_volume(1.0f)
    ,_trunkSize(0)
    ,_initialized(false)
    ,_playState(RSStateStoped)
    ,_poolBuffer(nullptr)
    ,_playingBuffer(nullptr)
    ,_playingIndex(0)
    ,_emptyTrunks()
    ,_readyTrunks()
    ,_emptyMutex()
    ,_readyMutex()
    ,_readySemaphore(nullptr)
    ,_emptySemaphore(nullptr)
    ,_slEngineObject(NULL)
    ,_slEngineInterface(NULL)
    ,_slMixObject(NULL)
    ,_slPlayerObject(NULL)
    ,_slPlayInterface(NULL)
    ,_slVolumeInterface(NULL)
    ,_slBufferQueue(NULL)
{

}

RSAudioTrack::~RSAudioTrack()
{
    this->release();
}

void RSAudioTrack::registerClass(JNIEnv *env, const char* fullCalssName)
{
    if (NULL == env) {
        env->FatalError("Fatal error:RSAudioTrack.registerClass, env is null!");
        return;
    }
    //
    jclass clazz = env->FindClass(fullCalssName);
    if (NULL == clazz) {
        return;
    }
    env->PushLocalFrame(20);
    JNINativeMethod objectMethods[] = {
        {"setupNativeObject", "()V", (void*)jni_setupNativeObject},
        {"releaseNativeObject", "()V", (void*)jni_releaseNativeObject},
        {"initialize", "(IIIF)Z", (void*)jni_initialize},
        {"getSampleRate", "()I", (void*)jni_getSampleRate},
        {"getChannelCount", "()I", (void*)jni_getChannelCount},
        {"getBytesPerSample", "()I", (void*)jni_getBytesPerSample},
        {"getBufferTime", "()F", (void*)jni_getBufferTime},
        {"setVolume", "(F)V", (void*)jni_setVolume},
        {"getVolume", "()F", (void*)jni_getVolume},
        {"play", "()V", (void*)jni_play},
        {"pause", "()V", (void*)jni_pause},
        {"stop", "()V", (void*)jni_stop},
        {"release", "()V", (void*)jni_release},
        {"write", "([BII)V", (void*)jni_write},
        {"flush", "()V", (void*)jni_flush}
    };
    if (env->RegisterNatives(clazz, objectMethods, sizeof(objectMethods) / sizeof(JNINativeMethod)) < 0) {
        env->FatalError("Fatal error:Failed to register methods of RSAudioTrack!");
    } else {
        s_fullClassName = fullCalssName;
    }
    env->PopLocalFrame(NULL);

    AUDIOTRACK_FIELD = env->GetFieldID(clazz, "_nativeHandle", "J");
}

void RSAudioTrack::unregisterClass(JNIEnv *env)
{
    if (s_fullClassName.empty()) {
        return;
    }
    jclass clazz = env->FindClass(s_fullClassName.c_str());
    if (NULL == clazz) {
        return;
    }
    env->UnregisterNatives(clazz);
    s_fullClassName.clear();
}

void RSAudioTrack::bindJni(JNIEnv *env, jobject thiz)
{
    if (NULL == env || NULL == thiz) {
        return;
    }
    jclass clazz = env->GetObjectClass(thiz);
    _jniObject = env->NewWeakGlobalRef(thiz);
    DELETE_LOCAL_REF(env, clazz);
    env->SetLongField(_jniObject, AUDIOTRACK_FIELD, (jlong)this);
}

void RSAudioTrack::unbindJni(JNIEnv *env, jobject thiz)
{
    if (NULL == env || NULL == thiz) {
        return;
    }
    env->SetLongField(_jniObject, AUDIOTRACK_FIELD, 0);
}

static bool _validateSampleRate(int sampleRate)
{
    switch (sampleRate) {
        case 8000:
        case 11025:
        case 12000:
        case 16000:
        case 22050:
        case 24000:
        case 32000:
        case 44100:
        case 48000:
        case 64000:
        case 88200:
        case 96000:
            return true;
        default:
            return false;
    }
    return false;
}

static bool _validateChannelCount(int channelCount)
{
    if (1 == channelCount || 2 == channelCount) {
        return true;
    }
    return false;
}

static bool _validateBytesPerSample(int bytesPerSample)
{
    if (2 == bytesPerSample) {
        return true;
    }
    return false;
}

bool RSAudioTrack::initialize(int sampleRate, int channelCount, int bytesPerSample, float bufferSecs)
{
    if (!_validateSampleRate(sampleRate) || !_validateChannelCount(channelCount) || !_validateBytesPerSample(bytesPerSample)) {
        __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "RSAudioTrack.initialize:Error, invalid parameters!");
        return false;
    }

    _sampleRate = sampleRate;
    _channelCount = channelCount;
    _bytesPerSample = bytesPerSample;
    _bufferTime = bufferSecs;

    if (!_initAudioTrunks()) {
        this->release();
        __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "RSAudioTrack.initialize:Error, failed to init audio trunks!");
        return false;
    }
    if (!_initSoundPlayer()) {
        this->release();
        __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "RSAudioTrack.initialize:Error, failed to init sound player!");
        return false;
    }

    _readySemaphore = new RSSemaphore((int)_readyTrunks.size());
    _emptySemaphore = new RSSemaphore((int)_emptyTrunks.size());
    _initialized = true;
    return true;
}

int RSAudioTrack::getSampleRate()
{
    return _sampleRate;
}

int RSAudioTrack::getChannelCount()
{
    return _channelCount;
}

int RSAudioTrack::getBytesPerSample()
{
    return _bytesPerSample;
}

float RSAudioTrack::getBufferTime()
{
    return _bufferTime;
}

void RSAudioTrack::setVolume(float vol)
{
    _volume = vol > 1.0f ? 1.0f : (vol < 0.0f ? 0.0f : vol);
    if (_slVolumeInterface) {
        float attenuation = 1.0f / 1024.0f + _volume * 1023.0f / 1024.0f;
        float db = 3 * log10(attenuation) / log10(2);
        SLmillibel volLevel = (SLmillibel)(db * 167);
        (*_slVolumeInterface)->SetVolumeLevel(_slVolumeInterface, volLevel);
    }
}

float RSAudioTrack::getVolume()
{
    return _volume;
}

void RSAudioTrack::play()
{
    switch(_playState) {
        case RSStateStoped:
            {
                _playState = RSStatePlaying;
                if (_slPlayInterface) {
                    (*_slPlayInterface)->SetPlayState(_slPlayInterface, SL_PLAYSTATE_PLAYING);
                }
            }
            break;
        case RSStatePaused:
            _playState = RSStatePlaying;
            break;
        case RSStatePlaying:
        default:
            break;
    }
}

void RSAudioTrack::pause()
{
    if (RSStatePlaying == _playState) {
        _playState = RSStatePaused;
    }
    if (_emptySemaphore) {
        _emptySemaphore->post();
    }
}

void RSAudioTrack::stop()
{
    if (RSStateStoped == _playState) {
        return;
    }
    _playState = RSStateStoped;
    if (_emptySemaphore) {
        _emptySemaphore->post();
    }
    if (_readySemaphore) {
        _readySemaphore->post();
    }
}

void RSAudioTrack::release()
{
    if (_playState != RSStateStoped) {
        __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "RSAudioTrack.release:Error, please call stop first!");
        return;
    }
    _destroySoundPlayer();
    _destroyAudioTrunks();
    if (_readySemaphore) {
        delete _readySemaphore;
        _readySemaphore = nullptr;
    }
    if (_emptySemaphore) {
        delete _emptySemaphore;
        _emptySemaphore = nullptr;
    }
    _initialized = false;
}

void RSAudioTrack::write(unsigned char *audioData, int offset, int dataSize)
{
    if (_playState != RSStatePlaying || !audioData || 0 >= dataSize) {
        return;
    }

#if 0
    static uint8_t *pNewBuffer = new uint8_t[128 * 1024];
    int newSize = 0;
    memset(pNewBuffer, 0, 128 * 1024);
    s_audioFilter->process(audioData + offset, dataSize / 4, pNewBuffer, newSize);
    audioData = pNewBuffer;
    dataSize = newSize;
#endif

    while(dataSize > 0) {
        _emptySemaphore->wait();
        AudioTrunk *audioTrunk = _popFrontTrunks(_emptyTrunks, _emptyMutex);
        if (!audioTrunk || RSStateStoped == _playState) {
            break;
        }
        int freeSize = audioTrunk->capacity - audioTrunk->size;
        if (dataSize >= freeSize) {
            memcpy(audioTrunk->data + audioTrunk->size, audioData + offset, freeSize);
            audioTrunk->size += freeSize;
            offset += freeSize;
            dataSize -= freeSize;
            this->_pushBackTrunks(_readyTrunks, _readyMutex, audioTrunk);
            _readySemaphore->post();
        } else {
            memcpy(audioTrunk->data + audioTrunk->size, audioData + offset, dataSize);
            audioTrunk->size += dataSize;
            offset += dataSize;
            dataSize = 0;
            this->_pushFronTrunks(_emptyTrunks, _emptyMutex, audioTrunk);
            _emptySemaphore->post();
            break;
        }
    }
}

void RSAudioTrack::flush()
{
    if (!_initialized) {
        return;
    }
    if (!_emptyTrunks.empty()) {
        _emptyTrunks.front()->size = 0;
    }
    while(_readySemaphore->waitForTime(0)) {
        AudioTrunk *audioTrunk = _popFrontTrunks(_readyTrunks, _readyMutex);
        if (!audioTrunk) {
            break;
        }
        audioTrunk->size = 0;
        _pushBackTrunks(_emptyTrunks, _emptyMutex, audioTrunk);
        _emptySemaphore->post();
    }
}

static inline void _bufferQueueCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context)
{
    if (NULL == context) {
        return;
    }
    RSAudioTrack *pAudioTrack = (RSAudioTrack*)context;
    pAudioTrack->playTrunk();
}

bool RSAudioTrack::_initSoundPlayer()
{
    SLresult res = SL_BOOLEAN_TRUE;
    do {
        res = slCreateEngine(&_slEngineObject, 0, NULL, 0, NULL, NULL);
        if (SL_RESULT_SUCCESS != res) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "RSAudioTrack._initSoundPlayer.Error:Failed to create engine!");
            break;
        }
        res = (*_slEngineObject)->Realize(_slEngineObject, SL_BOOLEAN_FALSE);
        if (SL_RESULT_SUCCESS != res) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "RSAudioTrack._initSoundPlayer.Error:Failed to realize engine!");
            break;
        }
        res = (*_slEngineObject)->GetInterface(_slEngineObject, SL_IID_ENGINE, &_slEngineInterface);
        if (SL_RESULT_SUCCESS != res) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "RSAudioTrack._initSoundPlayer.Error:Failed to get engine interface!");
            break;
        }
        const SLInterfaceID mids[] = {SL_IID_ENVIRONMENTALREVERB};
        const SLboolean mreq[] = {SL_BOOLEAN_FALSE};
        res = (*_slEngineInterface)->CreateOutputMix(_slEngineInterface, &_slMixObject, 0, mids, mreq);
        if (SL_RESULT_SUCCESS != res) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "RSAudioTrack._initSoundPlayer.Error:Failed to create output mix!");
            break;
        }
        res = (*_slMixObject)->Realize(_slMixObject, SL_BOOLEAN_FALSE);
        if (SL_RESULT_SUCCESS != res) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "RSAudioTrack._initSoundPlayer.Error:Failed to realize output mix!");
            break;
        }
        SLDataLocator_AndroidSimpleBufferQueue locatorBufferQueue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, PLAYING_BUFFER_COUNT
        };
        SLuint32 slChannel = 0;
        switch(_channelCount) {
            case 2:
                slChannel = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
                break;
            case 1:
                slChannel = SL_SPEAKER_FRONT_CENTER;
                break;
            default:
                break;
        }
        SLDataFormat_PCM pcmFormat = {
            SL_DATAFORMAT_PCM,
            (SLuint32)_channelCount,
            (SLuint32)_sampleRate * 1000,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            slChannel,
            SL_BYTEORDER_LITTLEENDIAN
        };
        SLDataSource audioSouce = {&locatorBufferQueue, &pcmFormat};
        SLDataLocator_OutputMix locatorMix = {SL_DATALOCATOR_OUTPUTMIX, _slMixObject};
        SLDataSink audioSink = {&locatorMix, NULL};
        const SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_EFFECTSEND};
        const SLboolean req[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
        res = (*_slEngineInterface)->CreateAudioPlayer(
            _slEngineInterface, &_slPlayerObject,
            &audioSouce, &audioSink,
            2, ids, req);
        if (SL_RESULT_SUCCESS != res) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "RSAudioTrack._initSoundPlayer.Error:Failed to create audio player!");
            break;
        }
        res = (*_slPlayerObject)->Realize(_slPlayerObject, SL_BOOLEAN_FALSE);
        if (SL_RESULT_SUCCESS != res) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "RSAudioTrack._initSoundPlayer.Error:Failed to realize audio player!");
            break;
        }
        res = (*_slPlayerObject)->GetInterface(_slPlayerObject, SL_IID_PLAY, &_slPlayInterface);
        if (SL_RESULT_SUCCESS != res) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "RSAudioTrack._initSoundPlayer.Error:Failed to get play interafce!");
            break;
        }
        res = (*_slPlayerObject)->GetInterface(_slPlayerObject, SL_IID_BUFFERQUEUE, &_slBufferQueue);
        if (SL_RESULT_SUCCESS != res) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "RSAudioTrack._initSoundPlayer.Error:Failed to get bufferQueue interface!");
            break;
        }
        res = (*_slBufferQueue)->RegisterCallback(_slBufferQueue, _bufferQueueCallback, this);
        if (SL_RESULT_SUCCESS != res) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "RSAudioTrack._initSoundPlayer.Error:Failed to register buffer callback!");
            break;
        }
        res = (*_slPlayerObject)->GetInterface(_slPlayerObject, SL_IID_VOLUME, &_slVolumeInterface);
        if (SL_RESULT_SUCCESS != res) {
            __android_log_print(ANDROID_LOG_ERROR, "PaomianTV", "RSAudioTrack._initSoundPlayer.Error:Failed to get volume interface!");
            break;
        }
        for (int i=0; i<PLAYING_BUFFER_COUNT; ++i) {
            (*_slBufferQueue)->Enqueue(_slBufferQueue, _playingBuffer + _trunkSize * i, _trunkSize);
        }
        _playingIndex = 0;
    } while(false);

    return res == SL_RESULT_SUCCESS ? true : false;
}

void RSAudioTrack::_destroySoundPlayer()
{
    _slVolumeInterface = NULL;
    _slPlayInterface = NULL;
    _slEngineInterface = NULL;
    _slBufferQueue = NULL;

    if (_slPlayerObject) {
        (*_slPlayerObject)->Destroy(_slPlayerObject);
        _slPlayerObject = NULL;
    }
    if (_slMixObject) {
        (*_slMixObject)->Destroy(_slMixObject);
        _slMixObject = NULL;
    }
    if (_slEngineObject) {
        (*_slEngineObject)->Destroy(_slEngineObject);
        _slEngineObject = NULL;
    }   
}

bool RSAudioTrack::_initAudioTrunks()
{
    _trunkSize = g_bufferSize * _channelCount * _bytesPerSample;
    if (0 >= _trunkSize) {
        return false;
    }

    _poolBuffer = new unsigned char[_trunkSize * POOL_TRUNK_COUNT];
    memset(_poolBuffer, 0, _trunkSize * POOL_TRUNK_COUNT);
    for (int i=0; i<POOL_TRUNK_COUNT; ++i) {
        AudioTrunk *audioTrunk = new AudioTrunk();
        audioTrunk->data = _poolBuffer + i * _trunkSize;
        audioTrunk->size = 0;
        audioTrunk->capacity = _trunkSize;
        _pushBackTrunks(_emptyTrunks, _emptyMutex, audioTrunk);
    }

    _playingBuffer = new unsigned char[_trunkSize * PLAYING_BUFFER_COUNT];
    memset(_playingBuffer, 0, _trunkSize * PLAYING_BUFFER_COUNT);
    return true;
}

void RSAudioTrack::_destroyAudioTrunks()
{
    while(!_emptyTrunks.empty()) {
        AudioTrunk *audioTrunk = this->_popFrontTrunks(_emptyTrunks, _emptyMutex);
        delete audioTrunk;
    }
    while(!_readyTrunks.empty()) {
        AudioTrunk *audioTrunk = this->_popFrontTrunks(_readyTrunks, _readyMutex);
        delete audioTrunk;
    }
    if (_poolBuffer) {
        delete[] _poolBuffer;
        _poolBuffer = nullptr;
    }
    if (_playingBuffer) {
        delete[] _playingBuffer;
        _playingBuffer = nullptr;
    }
}

void RSAudioTrack::playTrunk()
{
    _readySemaphore->waitForTime(100);
    AudioTrunk *audioTrunk = this->_popFrontTrunks(_readyTrunks, _readyMutex);

    while(RSStatePaused == _playState) {
        usleep(2000);
    }
    if (RSStateStoped == _playState || !_slBufferQueue) {
        if (audioTrunk) {
            _pushBackTrunks(_emptyTrunks, _emptyMutex, audioTrunk);
            _emptySemaphore->post();
            audioTrunk = nullptr;
        }
        return;
    }

    unsigned char *pBuffer = _playingBuffer + _playingIndex * _trunkSize;
    if (audioTrunk) {
        memcpy(pBuffer, audioTrunk->data, _trunkSize);
        audioTrunk->size = 0;
        this->_pushBackTrunks(_emptyTrunks, _emptyMutex, audioTrunk);
        _emptySemaphore->post();
    } else {
        memset(pBuffer, 0, _trunkSize);
    }

    (*_slBufferQueue)->Enqueue(_slBufferQueue, pBuffer, _trunkSize);
    ++_playingIndex;
    _playingIndex %= PLAYING_BUFFER_COUNT;
}

void RSAudioTrack::_pushBackTrunks(
    std::deque<RSAudioTrack::AudioTrunk*>& trunkList,
    std::mutex& trunkMutex,
    RSAudioTrack::AudioTrunk *audioTrunk)
{
    std::lock_guard<std::mutex> autoLock(trunkMutex);
    trunkList.push_back(audioTrunk);
}

void RSAudioTrack::_pushFronTrunks(
    std::deque<RSAudioTrack::AudioTrunk*>& trunkList,
    std::mutex& trunkMutex,
    RSAudioTrack::AudioTrunk *audioTrunk)
{
    std::lock_guard<std::mutex> autoLock(trunkMutex);
    trunkList.push_front(audioTrunk);
}

RSAudioTrack::AudioTrunk *RSAudioTrack::_popBackTrunks(
    std::deque<RSAudioTrack::AudioTrunk*>& trunkList,
    std::mutex& trunkMutex)
{
    std::lock_guard<std::mutex> autoLock(trunkMutex);
    AudioTrunk *audioTrunk = nullptr;
    if (!trunkList.empty()) {
        audioTrunk = trunkList.back();
        trunkList.pop_back();
    }
    return audioTrunk;
}

RSAudioTrack::AudioTrunk *RSAudioTrack::_popFrontTrunks(
    std::deque<RSAudioTrack::AudioTrunk*>& trunkList,
    std::mutex& trunkMutex)
{
    std::lock_guard<std::mutex> autoLock(trunkMutex);
    AudioTrunk *audioTrunk = nullptr;
    if (!trunkList.empty()) {
        audioTrunk = trunkList.front();
        trunkList.pop_front();
    }
    return audioTrunk;
}
