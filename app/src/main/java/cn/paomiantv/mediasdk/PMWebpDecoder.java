package cn.paomiantv.mediasdk;

import android.graphics.Bitmap;

import java.nio.ByteBuffer;

import cn.paomiantv.mediasdk.module.PMModule;

public class PMWebpDecoder extends PMModule {
    public PMWebpDecoder() {
        _create();
    }

    public boolean destory() {
        return _destroy();
    }

    public boolean setDataSource(String src) {
        return _setDataSource(src);
    }

    public String getSrc() {
        return _getSrc();
    }

    public int getFrameCount() {
        return _getFrameCount();
    }

    public boolean getFrame(int index, Bitmap bitmap) {
        return _getFrame(index, bitmap);
    }

    public int getCanvasWidth() {
        return _getCanvasWidth();
    }

    public int getCanvasHeight() {
        return _getCanvasHeight();
    }

    public long getFrameDuration(int index) {
        return _getFrameDuration(index);
    }

    public ByteBuffer getRGBAFrame(int index) {
        return _getRGBAFrame(index);
    }

    private native boolean _create();

    private native boolean _destroy();

    private native boolean _setDataSource(String dst);

    private native String _getSrc();

    private native int _getFrameCount();

    private native ByteBuffer _getRGBAFrame(int index);

    private native boolean _getFrame(int index, Bitmap bitmap);

    private native int _getCanvasWidth();

    private native int _getCanvasHeight();

    private native long _getFrameDuration(int index);
}
