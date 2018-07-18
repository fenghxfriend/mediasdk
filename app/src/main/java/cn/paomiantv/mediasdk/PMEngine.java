package cn.paomiantv.mediasdk;

import android.content.Context;
import android.os.Build;
import android.os.Environment;
import android.util.Log;

import java.io.File;

import cn.paomiantv.mediasdk.media.audio.OpenSlParams;
import cn.paomiantv.mediasdk.module.PMModule;

/**
 * Created by John on 2017/7/20.
 */

public class PMEngine extends PMModule {
    private String sdk_temp_path;

    static {
        System.loadLibrary("yuv");
        System.loadLibrary("mp4v2");
        System.loadLibrary("soundtouch");
        System.loadLibrary("render");
        System.loadLibrary("mediasdk");
        System.loadLibrary("interface");
    }

    private PMEngine() {
    }

    // 变量定义
    private static class PMEngineHolder {
        private final static PMEngine mInstance = new PMEngine();
    }

    public static PMEngine getInstance() {
        return PMEngineHolder.mInstance;
    }

    public boolean init(Context context) {
        Log.e("PMEngine", "SDK_VERSION" + Build.VERSION.SDK_INT);
        createCacheStoreDirs(context);
        OpenSlParams openSlParams = OpenSlParams.createInstance(context);
        return _init(Build.VERSION.SDK_INT, openSlParams.getSampleRate(), openSlParams.getBufferSize());
    }

    private void createCacheStoreDirs(Context context) {
        File cacheFile = null;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.FROYO) {
            cacheFile = context.getExternalCacheDir();
        }
        if (cacheFile == null) {
            final String cacheDir = "/Android/data/" + context.getPackageName() + "/cache/";
            cacheFile = new File(cacheDir);
            if (!cacheFile.exists()) {
                cacheFile.mkdirs();
            }
        }
        File tempDir = new File(cacheFile, "temp_sdk");
        if (!tempDir.exists()) {
            tempDir.mkdirs();
        }
        sdk_temp_path = tempDir.getAbsolutePath();
    }

    public void uninit() {
        _uninit();
    }

    public String getSdk_temp_path() {
        return sdk_temp_path;
    }

    private native boolean _init(int version, int sampleRate, int bufferSize);

    private native void _uninit();

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        _uninit();
    }
}
