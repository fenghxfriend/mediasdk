package cn.paomiantv.mediasdk;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.TextureView;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import cn.paomiantv.mediasdk.common.PMConfig;
import cn.paomiantv.mediasdk.module.PMStoryboard;


/**
 * Created by ASUS on 2018/2/5.
 */

public class PMPlayer extends FrameLayout implements TextureView.SurfaceTextureListener {
    public static final String TAG = PMPlayer.class.getSimpleName();
    private TextureView mTextureView;

    private PMNativePlayer mNativePlayer;

    private PMConfig.PlayerStatus mStatus = PMConfig.PlayerStatus.UNKNOWN;

    public PMPlayer(@NonNull Context context) {
        super(context);
        init(context);
    }

    public PMPlayer(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    public PMPlayer(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(context);
    }

    private void init(Context context) {
        mNativePlayer = new PMNativePlayer();
        mTextureView = new TextureView(context);
        ViewGroup.LayoutParams layoutParams = mTextureView.getLayoutParams();
        if (layoutParams == null) {
            layoutParams = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
        }
        addView(mTextureView, layoutParams);
        mTextureView.setSurfaceTextureListener(this);
    }


    public void setMessageHandler(Handler handler) {
        if (mNativePlayer != null) {
            mNativePlayer.setMessageHandler(handler);
        }
    }

    public void setDataSource(PMStoryboard storyboard) {
        if (mNativePlayer == null) {
            mNativePlayer = new PMNativePlayer();
            if (mTextureView != null && mTextureView.isAvailable()) {
                Surface s = new Surface(mTextureView.getSurfaceTexture());
                mNativePlayer.bindSurface(s, mTextureView.getWidth(), mTextureView.getHeight());
                s.release();
            }

            mNativePlayer.setDataSource(storyboard);

        } else {
            mNativePlayer.setDataSource(storyboard);
        }
        mStatus = PMConfig.PlayerStatus.INITIALIZE;
    }

    public void prepare() {
        if (mNativePlayer != null) {
            mNativePlayer.prepare();
        }
        mStatus = PMConfig.PlayerStatus.PREPARED;
    }

    public void play() {
        if (mNativePlayer != null) {
            mNativePlayer.play();
        }
        mStatus = PMConfig.PlayerStatus.PLAYING;
    }

    public void pause() {
        if (mNativePlayer != null) {
            mNativePlayer.pause();
        }
        mStatus = PMConfig.PlayerStatus.PAUSED;
    }

    public void seekTo(long microsecond) {

        if (mNativePlayer != null) {
            mNativePlayer.seekTo(microsecond);
        }
    }

    public void locPreview(long microsecond) {

        if (mNativePlayer != null) {
            mNativePlayer.locPreview(microsecond);
        }
    }

    public void resume() {
        if (mNativePlayer != null) {
            mNativePlayer.resume();
        }
        mStatus = PMConfig.PlayerStatus.PLAYING;
    }

    public void stop() {
        if (mNativePlayer != null) {
            mNativePlayer.stop();
        }
        mStatus = PMConfig.PlayerStatus.STOPPED;
    }

    public void release() {
        if (mNativePlayer != null) {
            mNativePlayer.destroy();
            mNativePlayer = null;
        }
        mStatus = PMConfig.PlayerStatus.UNKNOWN;
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        if (mNativePlayer != null) {
            Surface s = new Surface(surface);
            mNativePlayer.bindSurface(s, width, height);
            s.release();
            mNativePlayer.surfaceSizeChanged(width, height);
        }
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        if (mNativePlayer != null) {
            mNativePlayer.surfaceSizeChanged(width, height);
        }
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        if (mNativePlayer != null) {
            mNativePlayer.unbindSurface();
        }
        return false;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {

    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        mTextureView = null;
        if (mNativePlayer != null) {
            mNativePlayer.destroy();
            mNativePlayer = null;
        }
    }

    public PMConfig.PlayerStatus getStatus() {
        return mStatus;
    }
}
