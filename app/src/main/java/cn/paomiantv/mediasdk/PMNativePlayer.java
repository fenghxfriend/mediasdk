package cn.paomiantv.mediasdk;

import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Surface;

import java.lang.ref.WeakReference;

import cn.paomiantv.mediasdk.common.PMMessage;
import cn.paomiantv.mediasdk.module.PMModule;
import cn.paomiantv.mediasdk.module.PMStoryboard;

public class PMNativePlayer extends PMModule {
    public static final String TAG = PMNativePlayer.class.getSimpleName();

    public PMNativePlayer() {
        _create();
    }

    public void setViewPort(int x, int y, int width, int height) {
        _setViewPort(x, y, width, height);
    }

    public void setDataSource(PMStoryboard storyboard) {
        _setDataSourceSTB(storyboard);
    }

    public void prepare() {
        _prepare();
    }

    public void play() {
        _play();
    }

    public void pause() {
        _pause();
    }

    public void seekTo(long microsecond) {
        _seekTo(microsecond);
    }

    public void locPreview(long microsecond) {
        _locPreview(microsecond);
    }

    public void resume() {
        _resume();
    }

    public void stop() {
        _stop();
    }

    public void destroy() {
        _release();
    }

    public void bindSurface(Surface surface, int width, int height) {
        _bindSurface(surface, width, height);
    }

    public void surfaceSizeChanged(int width, int height) {
        _surfaceSizeChanged(width, height);
    }

    public void unbindSurface() {
        _unbindSurface();
    }

    private native void _create();

    private native void _setViewPort(int x, int y, int width, int height);

    private native void _setDataSourceSTB(PMStoryboard storyboard);

    private native void _prepare();

    private native void _bindSurface(Surface surface, int width, int height);

    private native void _surfaceSizeChanged(int width, int height);

    private native void _unbindSurface();

    private native void _play();

    private native void _pause();

    private native void _seekTo(long microsecond);

    private native void _locPreview(long microsecond);

    private native void _resume();

    private native void _stop();

    private native void _release();


    private WeakReference<Handler> mHandler;

    private void onMessage(int id, String message) {
        // TODO Auto-generated method stub
        if (mHandler != null && mHandler.get() != null) {
            Message msg = mHandler.get().obtainMessage();
            msg.what = id;
            msg.obj = message;
            mHandler.get().sendMessage(msg);
        }
        switch (id) {
            case PMMessage.MESSAGE_ID_V_DECODE_COMPLETE: {
                Log.e(TAG, "video steam decode complete!!!!!!!!!!!!!!");
            }
            break;
            case PMMessage.MESSAGE_ID_A_DECODE_COMPLETE: {
                Log.e(TAG, "audio steam decode complete!!!!!!!!!!!!!!");
            }
            break;
            case PMMessage.MESSAGE_ID_V_RENDER_COMPLETE: {
                Log.e(TAG, "video steam play complete!!!!!!!!!!!!!!");
            }
            break;
            case PMMessage.MESSAGE_ID_A_PLAY_COMPLETE: {
                Log.e(TAG, "audio steam play complete!!!!!!!!!!!!!!");
            }
            break;
            default:
                break;
        }

    }

    public void setMessageHandler(Handler handler) {
        if (handler != null) {
            mHandler = new WeakReference<>(handler);
        }
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        _release();
        mHandler = null;
    }
}
