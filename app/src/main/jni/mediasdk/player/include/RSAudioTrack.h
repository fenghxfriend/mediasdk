#ifndef __RS_AUDIO_TRACK_H__
#define __RS_AUDIO_TRACK_H__

#include <jni.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <deque>
#include <mutex>

namespace std
{
    class thread;
}

class SoundTouch;
class RSSemaphore;
class RSAudioTrack
{
protected:
    typedef enum _RSPlayState {
        RSStateStoped = 0,
        RSStatePaused = 1,
        RSStatePlaying = 2
    }RSPlayState;

    class AudioTrunk
    {
    public:
        unsigned char *data;
        int size;
        int capacity;
        AudioTrunk()
            :data(nullptr)
            ,size(0)
            ,capacity(0)
        {
        }
    };

public:
    RSAudioTrack();
    ~RSAudioTrack();

public:
    static void registerClass(JNIEnv *env, const char* fullClassName);

    static void unregisterClass(JNIEnv *env);

public:

    void bindJni(JNIEnv *env, jobject obj);

    void unbindJni(JNIEnv *env, jobject obj);

    /// 初始化当前实例，此方法需要在其他所有非构造方法之前调用一次
    /// @param sampleRate 播放采样率
    /// @param channelCount 通道个数
    /// @param bytesPerSample 每个单采样数据的字节数
    /// @param bufferSecs 播放缓冲时间，单位为秒，此时间会导致缓冲延迟
    /// @return 是否初始化成功
    bool initialize(int sampleRate, int channelCount, int bytesPerSample, float bufferSecs);

    /// 获得播放采样率
    /// @return 采样率
    int getSampleRate();

    /// 获得通道个数
    /// @return 通道个数
    int getChannelCount();

    /// 获得每个采样数据的字节数
    /// @return 样本字节数
    int getBytesPerSample();

    /// 获得播放缓冲时间
    /// @return 播放缓冲时间，单位为秒
    float getBufferTime();

    /// 设置音量
    /// @param 音量，取值范围为0.0 ~ 1.0
    void setVolume(float vol);

    /// 获得音量
    /// @return 音量
    float getVolume();

    /// 开始播放
    void play();

    /// 暂停播放
    void pause();

    /// 停止播放
    /// @see 调用release方法前必须调用此接口停止播放
    void stop();

    /// 释放所有资源数据
    /// @see 调用此方法之后当前实例不再可用
    void release();

    /// 写入音频数据
    void write(unsigned char *audioData, int offset, int dataSize);

    /// 丢弃未播放的缓冲数据
    void flush();

    /// 内部调用方法
public:
    void playTrunk();

protected:
    bool _initAudioTrunks();

    void _destroyAudioTrunks();

    bool _initSoundPlayer();

    void _destroySoundPlayer();

    void _pushBackTrunks(
        std::deque<RSAudioTrack::AudioTrunk*>& trunkList,
        std::mutex& trunkMutex,
        RSAudioTrack::AudioTrunk *audioTrunk);

    void _pushFronTrunks(
        std::deque<RSAudioTrack::AudioTrunk*>& trunkList,
        std::mutex& trunkMutex,
        RSAudioTrack::AudioTrunk *audioTrunk);

    RSAudioTrack::AudioTrunk *_popBackTrunks(
        std::deque<RSAudioTrack::AudioTrunk*>& trunkList,
        std::mutex& trunkMutex);

    RSAudioTrack::AudioTrunk *_popFrontTrunks(
        std::deque<RSAudioTrack::AudioTrunk*>& trunkList,
        std::mutex& trunkMutex);

protected:
    jweak _jniObject;
    int _sampleRate;
    int _channelCount;
    int _bytesPerSample;
    float _bufferTime;
    float _volume;

    bool _initialized;
    int _playState;

    int _trunkSize;
    unsigned char *_poolBuffer;
    unsigned char *_playingBuffer;
    int _playingIndex;

    std::deque<AudioTrunk*> _emptyTrunks;
    std::deque<AudioTrunk*> _readyTrunks;
    std::mutex _emptyMutex;
    std::mutex _readyMutex;
    RSSemaphore *_emptySemaphore;
    RSSemaphore *_readySemaphore;

    SLObjectItf _slEngineObject;
    SLEngineItf _slEngineInterface;
    SLObjectItf _slMixObject;
    SLObjectItf _slPlayerObject;
    SLPlayItf _slPlayInterface;
    SLVolumeItf _slVolumeInterface;
    SLAndroidSimpleBufferQueueItf _slBufferQueue;
};

#endif ///__RS_AUDIO_TRACK_H__