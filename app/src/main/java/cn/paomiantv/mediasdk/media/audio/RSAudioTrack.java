package cn.paomiantv.mediasdk.media.audio;

public class RSAudioTrack
{
    private long _nativeHandle = 0;

    public RSAudioTrack()
    {
        setupNativeObject();
    }

    public void finalize()
    {
        releaseNativeObject();
    }

    public native void setupNativeObject();

    public native void releaseNativeObject();

    public native boolean initialize(int sampleRate, int channelCount, int bytesPerSample, float bufferTime);

    public native int getSampleRate();

    public native int getChannelCount();

    public native int getBytesPerSample();

    public native float getBufferTime();

    public native void setVolume(float vol);

    public native float getVolume();

    public native void play();

    public native void pause();

    public native void stop();

    public native void release();

    public native void write(byte[] audioData, int offset, int dataSize);

    public native void flush();

}