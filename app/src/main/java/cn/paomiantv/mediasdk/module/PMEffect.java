package cn.paomiantv.mediasdk.module;

import android.graphics.Bitmap;

import cn.paomiantv.mediasdk.common.PMConfig;

/**
 * Created by John on 2017/7/28.
 */

public class PMEffect extends PMModule {

    public PMEffect(Bitmap bitmap, PMConfig.EffectType type, long start, long duration) {
        _create(type.getIndex(), bitmap, start, duration);
    }

    public boolean destory() {
        return _destroy();
    }

    public PMConfig.EffectType getType() {
        return PMConfig.EffectType.getFilterType(_getType());
    }

    public void update(Bitmap bitmap) {
        _update(_getType(), bitmap);
    }

    public void update(Bitmap bitmap, PMConfig.EffectType type) {
        _update(type.getIndex(), bitmap);
    }

    public Bitmap getPicture() {
        return _getPicture();
    }

    public long getStart() {
        return _getStart();
    }

    public void setStart(long startTm) {
        _setStart(startTm);
    }

    public long getDuration() {
        return _getDuration();
    }

    public void setDuration(long duration) {
        _setDuration(duration);
    }

    private native boolean _create(int type, Bitmap bitmap, long start, long duration);

    private native boolean _destroy();

    private native void _update(int type, Bitmap bitmap);

    private native Bitmap _getPicture();

    private native int _getType();


    private native long _getStart();

    private native void _setStart(long start);

    private native long _getDuration();

    private native void _setDuration(long duration);

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        _destroy();
    }
}
