package cn.paomiantv.mediasdk.module;

import android.graphics.Bitmap;

import cn.paomiantv.mediasdk.common.PMConfig;

/**
 * Created by John on 2017/7/21.
 */

public class PMTrack extends PMModule {
    public PMTrack(PMConfig.TrackType type) {
        _create(type.getIndex());
    }

    public boolean destory() {
        return _destroy();
    }

    public int getId() {
        return _getId();
    }

    public int getType() {
        return _getType();
    }

    public boolean setDataSource(String file) {
        return _setDataSource_path(file);
    }

    public boolean setDataSource(Bitmap bitmap) {
        return _setDataSource_bitmap(bitmap);
    }

    public boolean setDataSourceWebP(String webp) {
        return _setDataSource_webp(webp);
    }

    public boolean setDataSourceSilence() {
        return _setDataSource_silence();
    }

    public PMConfig.SourceType getSourceType() {
        return PMConfig.SourceType.getSourceType(_getSourceType());
    }

    public byte[] getSrc() {
        return _getSrc();
    }


    public int getWidth() {
        return _getWidth();
    }


    public int getHeight() {
        return _getHeight();
    }

    public void setCutStart(long startTm) {
        _setCutStart(startTm);
    }

    public long getCutStart() {
        return _getCutStart();
    }

    public void setCutDuration(long duration) {
        _setCutDuration(duration);
    }

    public long getCutDuration() {
        return _getCutDuration();
    }

    public void setPlayStart(long startPlayTm) {
        _setPlayStart(startPlayTm);
    }

    public long getPlayStart() {
        return _getPlayStart();
    }

    public void setPlayDuration(long duration) {
        _setPlayDuration(duration);
    }

    public long getPlayDuration() {
        return _getPlayDuration();
    }

    public void setWeight(short weight) {
        _setWeight(weight);
    }

    public short getWeight() {
        return _getWeight();
    }

//    public void setZindex(short Zindex) {
//        _setZIndex(Zindex);
//    }

    public short getZindex() {
        return _getZIndex();
    }

    public void setVolume(float volume) {
        _setVolume(volume);
    }

    public float getVolume() {
        return _getVolume();
    }

    public void setPlaybackRate(float rate) {
        _setPlaybackRate(rate);
    }

    public float getPlaybackRate() {
        return _getPlaybackRate();
    }

    public void setLoop(boolean isLoop) {
        _setLoop(isLoop);
    }

    public boolean isLoop() {
        return _isLoop();
    }

    public void setShowFirstFrame(boolean isShowFirstFrame) {
        _setShowFirstFrame(isShowFirstFrame);
    }

    public boolean isShowFirstFrame() {
        return _isShowFirstFrame();
    }

    public void setShowLastFrame(boolean isShowLastFrame) {
        _setShowLastFrame(isShowLastFrame);
    }

    public boolean isShowLastFrame() {
        return _isShowLastFrame();
    }

    public boolean isIndependent() {
        return _isIndependent();
    }


    public PMEffect getEffect(int position) {
        return _getEffect(position);
    }

    public boolean addEffect(PMEffect e) {
        return _addEffect(e);
    }

    public PMEffect removeEffect(int position) {
        return _removeEffect(position);
    }

    public int getEffectCount() {
        return _getEffectCount();
    }


    public PMAnimation getAnimation(int position) {
        return _getAnimation(position);
    }

    public boolean addAnimation(PMAnimation animation) {
        return _addAnimation(animation);
    }

    public PMAnimation removeAnimation(int position) {
        return _removeAnimation(position);
    }

    public int getAnimationCount() {
        return _getAnimationCount();
    }

    public long getDataDuration() {
        return _getDataDuration();
    }

    private native boolean _create(int type);

    private native boolean _destroy();

    private native int _getId();

    private native int _getType();

    private native boolean _setDataSource_path(String path);

    private native boolean _setDataSource_webp(String src);

    private native boolean _setDataSource_bitmap(Bitmap bitmap);

    private native boolean _setDataSource_silence();

    private native int _getWidth();

    private native int _getHeight();

    private native int _getSourceType();

    private native byte[] _getSrc();

    private native void _setCutStart(long start);

    private native long _getCutStart();

    private native void _setCutDuration(long duration);

    private native long _getCutDuration();

    private native void _setPlayStart(long playStart);

    private native long _getPlayStart();

    private native void _setPlayDuration(long playDuration);

    private native long _getPlayDuration();

    private native void _setWeight(short weight);

    private native short _getWeight();

    private native void _setShowFirstFrame(boolean isShowFirstFrame);

    private native boolean _isShowFirstFrame();

    private native void _setShowLastFrame(boolean isShowLastFrame);

    private native boolean _isShowLastFrame();

    private native boolean _isIndependent();

    private native void _setLoop(boolean isLoop);

    private native boolean _isLoop();

    private native void _setZIndex(short Zindex);

    private native short _getZIndex();

    private native void _setVolume(float volume);

    private native float _getVolume();

    private native void _setPlaybackRate(float rate);

    private native float _getPlaybackRate();


    private native PMEffect _getEffect(int position);

    private native boolean _addEffect(PMEffect effect);

    private native PMEffect _removeEffect(int position);

    private native int _getEffectCount();


    private native PMAnimation _getAnimation(int position);

    private native boolean _addAnimation(PMAnimation animation);

    private native PMAnimation _removeAnimation(int position);

    private native int _getAnimationCount();


    private native long _getDataDuration();

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        _destroy();
    }
}
