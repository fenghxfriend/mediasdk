package cn.paomiantv.mediasdk;

import android.annotation.SuppressLint;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.opengl.EGL14;
import android.opengl.EGLExt;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.Surface;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.nio.ByteBuffer;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import cn.paomiantv.mediasdk.common.PMMessage;
import cn.paomiantv.mediasdk.module.PMModule;
import cn.paomiantv.mediasdk.module.PMStoryboard;

/**
 * Created by ASUS on 2018/2/5.
 */

public class PMExporter extends PMModule {
    public static final String TAG = PMExporter.class.getSimpleName();
    private PMMuxer mPMMuxer = null;
    private MediaCodec mvEncoder = null;
    private MediaCodec maEncoder = null;
    private Thread mvEncoderThread;
    private Thread maEncoderThread;
    private boolean isStop = false;
    private WeakReference<Handler> mHandlerNativeMessage;
    private boolean vComplete = false;
    private boolean aComplete = false;
    private String mDescription = "";
    private Lock lock = new ReentrantLock();

    public PMExporter(String outputFile) {
        _create();
        this.mvEncoderThread = new Thread(mvRunnable, "VExportEncode");
        this.maEncoderThread = new Thread(maRunnable, "AExportEncode");
        this.mPMMuxer = new PMMuxer(outputFile);
    }

    public boolean setVideoEncoderParams(String mime, int width, int height, int bitRate, int framerate, int iInterval, int profile, int level) {
        mvEncoder = getVideoEncoder(mime, width, height, bitRate, framerate, iInterval, profile, level);
        if (mvEncoder != null && this.mvEncoderThread != null) {
            this.mvEncoderThread.start();
            return true;
        }
        return false;
    }

    public boolean setAudioEncoderParams(String mime, int sampleRate, int channels, int bitRate, int profile) {
        maEncoder = getAudioEncoder(mime, sampleRate, channels, bitRate, profile);
        if (maEncoder != null && this.maEncoderThread != null) {
            this.maEncoderThread.start();
            return true;
        }
        return false;
    }

    public void setDataSource(PMStoryboard storyboard) {
        _setDataSourceSTB(storyboard);
    }

    private void onMessage(int id, String message) {
        // TODO Auto-generated method stub
        switch (id) {
            case PMMessage.MESSAGE_ID_V_SAVE_RENDER_PROGRESS: {
                try {
                    JSONObject json = new JSONObject(message);
                    long ts = json.optLong("timestamp", 0);
//                    Log.e(TAG, "draw the frame: " + ts);
                    EGLExt.eglPresentationTimeANDROID(EGL14.eglGetCurrentDisplay(), EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW), ts * 1000);
                } catch (JSONException e) {
                    e.printStackTrace();
                }
            }
            break;
            case PMMessage.MESSAGE_ID_V_RENDER_COMPLETE: {
                Log.e(TAG, "video steam play complete!!!!!!!!!!!!!!");
                if (mvEncoder != null) {
                    mvEncoder.signalEndOfInputStream();
                }
            }
            break;
            case PMMessage.MESSAGE_ID_A_SAVE_PLAY_PROGRESS: {
                try {
                    JSONObject json = new JSONObject(message);
                    long ts = json.optLong("timestamp", 0);
//                    Log.e(TAG, "play the frame: " + ts);
                } catch (JSONException e) {
                    e.printStackTrace();
                }

            }
            break;
            case PMMessage.MESSAGE_ID_A_PLAY_COMPLETE: {
                Log.e(TAG, "audio steam play complete!!!!!!!!!!!!!!");
                if (maEncoder != null) {
                    int index = -1;
                    while (index < 0) {
                        index = maEncoder.dequeueInputBuffer(-1);
                    }
                    maEncoder.queueInputBuffer(
                            index,
                            0,
                            0,
                            0,
                            MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                }
            }
            break;
            case PMMessage.MESSAGE_ID_V_DECODE_COMPLETE: {
                Log.e(TAG, "video steam decode complete!!!!!!!!!!!!!!");
            }
            break;
            case PMMessage.MESSAGE_ID_A_DECODE_COMPLETE: {
                Log.e(TAG, "audio steam decode complete!!!!!!!!!!!!!!");
            }
            break;
            default:
                break;
        }
        if (mHandlerNativeMessage != null && mHandlerNativeMessage.get() != null) {
            Message msg = mHandlerNativeMessage.get().obtainMessage();
            msg.what = id;
            msg.obj = message;
            mHandlerNativeMessage.get().sendMessage(msg);
        }
        if (id > 10000) {
            Message msg = mHandler.obtainMessage();
            msg.what = FAILED;
            mHandler.sendMessage(msg);
        }
    }

    private void onWritePCM(long timestampUS, byte[] buffer, boolean isEOS) {
        // TODO Auto-generated method stub
        if (maEncoder != null && buffer != null && buffer.length > 0) {
//            Log.e(TAG, "play the frame: " + timestampUS);
            int index = -1;
            while (index < 0) {
                index = maEncoder.dequeueInputBuffer(-1);
            }
            ByteBuffer inputBuffer = maEncoder.getInputBuffers()[index];
            inputBuffer.clear();
            inputBuffer.position(0);
            inputBuffer.put(buffer, 0, buffer.length);
            maEncoder.queueInputBuffer(index, 0, inputBuffer.position(), timestampUS, 0);
        }
    }

    public void setMessageHandler(Handler handler) {
        if (handler != null) {
            mHandlerNativeMessage = new WeakReference<>(handler);
        }

    }

    private MediaCodec getVideoEncoder(String mime, int width, int height, int bitRate, int framerate, int iInterval, int profile, int level) {
        MediaCodec codec = null;
        if (Build.MODEL.equalsIgnoreCase("MHA-AL00")) {
            width = (width % 8 == 0 ? width : ((width / 8) + 1) * 8);
        }

        //adjust the width && height by align 2
        width = (width % 2 == 0 ? width : width + 1);
        height = (height % 2 == 0 ? height : height + 1);

        MediaFormat format = MediaFormat.createVideoFormat(mime, width, height);

        // Set some properties. Failing to specify some of these can cause the MediaCodec
        // configure() call to throw an unhelpful exception.
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        format.setInteger(MediaFormat.KEY_BIT_RATE, bitRate);
        format.setInteger(MediaFormat.KEY_FRAME_RATE, framerate);
        format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, iInterval);
        if (Build.MODEL.equalsIgnoreCase("EVA-AL00") && Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            if (profile != 0) {
                format.setInteger(MediaFormat.KEY_PROFILE, MediaCodecInfo.CodecProfileLevel.AVCProfileMain);
            }
            if (level != 0) {
                format.setInteger(MediaFormat.KEY_LEVEL, MediaCodecInfo.CodecProfileLevel.AVCLevel31);
            }
            format.setInteger(MediaFormat.KEY_BITRATE_MODE, MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_VBR);
            format.setInteger(MediaFormat.KEY_COMPLEXITY, MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_CBR);
        }
        Log.d(TAG, "video encoder format: " + format);

        String name = PMHwCodecManager.instance().selectEncoder(format);
        if (name == null) {
            // Don't fail CTS if they don't have an AVC codec (not here, anyway).
            Log.e(TAG, "Unable to find an appropriate codec for " + mime);
            return null;
        }
        Log.d(TAG, "video found codec: " + name);

        try {
            codec = MediaCodec.createByCodecName(name);
            codec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            _setSurface(codec.createInputSurface(), width, height);
            codec.start();
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }

        return codec;
    }

    private MediaCodec getAudioEncoder(String mime, int sampleRate, int channels, int bitRate, int profile) {
        MediaCodec codec = null;
        MediaFormat format = MediaFormat.createAudioFormat(mime, sampleRate, channels);
        format.setInteger(MediaFormat.KEY_BIT_RATE, bitRate);
        format.setInteger(MediaFormat.KEY_AAC_PROFILE, profile);
        Log.d(TAG, "audio encoder format: " + format);

        String name = PMHwCodecManager.instance().selectEncoder(format);
        if (name == null) {
            // Don't fail CTS if they don't have an AVC codec (not here, anyway).
            Log.e(TAG, "Unable to find an appropriate codec for " + mime);
            return null;
        }
        Log.d(TAG, "video found codec: " + name);
        try {
            codec = MediaCodec.createByCodecName(name);
            codec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            codec.start();
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
        return codec;
    }


    public void save(String description) {
        this.mDescription = description;
        _start();
    }

    public void cancel() {
        release();
        if (mHandlerNativeMessage != null && mHandlerNativeMessage.get() != null) {
            Message msg = mHandlerNativeMessage.get().obtainMessage();
            msg.what = PMMessage.MESSAGE_ID_SAVE_CANCELED;
            msg.obj = "cancel success";
            mHandlerNativeMessage.get().sendMessage(msg);
        }
    }

    private void release() {
        _release();
        if (lock.tryLock()) {
            isStop = true;
            try {
                if (this.maEncoderThread != null) {
                    this.maEncoderThread.join(5000);
                    this.maEncoderThread = null;
                }

            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            try {
                if (this.mvEncoderThread != null) {
                    this.mvEncoderThread.join(5000);
                    this.mvEncoderThread = null;
                }

            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            try {
                if (maEncoder != null) {
                    maEncoder.stop();
                    maEncoder.release();
                    maEncoder = null;
                }
            } catch (Exception e) {
                e.printStackTrace();
            }

            try {
                if (mvEncoder != null) {
                    mvEncoder.stop();
                    mvEncoder.release();
                    mvEncoder = null;
                }
            } catch (Exception e) {
                e.printStackTrace();
            }

            try {
                if (mPMMuxer != null) {
                    mPMMuxer.destory();
                    mPMMuxer = null;
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            lock.unlock();
        }
    }

    private Runnable mvRunnable = new Runnable() {


        @Override
        public void run() {
            while (!isStop) {

                MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
                int outIndex = mvEncoder.dequeueOutputBuffer(info, 10000);
                switch (outIndex) {
                    case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
                        Log.d(TAG, "INFO_OUTPUT_BUFFERS_CHANGED");
                        break;

                    case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                        MediaFormat format = mvEncoder.getOutputFormat();
                        int width = format.getInteger(MediaFormat.KEY_WIDTH);
                        int height = format.getInteger(MediaFormat.KEY_HEIGHT);
                        byte[] sps = format.getByteBuffer("csd-0").array();
                        byte[] pps = format.getByteBuffer("csd-1").array();
                        mPMMuxer.addH264VideoTrack(width, height, (byte) 0x01, sps, (short) sps.length, pps, (short) pps.length);
                        Log.d(TAG, "INFO_OUTPUT_FORMAT_CHANGED format : " + format);
                        break;

                    case MediaCodec.INFO_TRY_AGAIN_LATER:
//                        Log.d(TAG, "INFO_TRY_AGAIN_LATER");
                        break;

                    default:
//                        Log.e(TAG, "encoder presentation time: " + info.presentationTimeUs);

                        if (outIndex >= 0) {
                            try {
                                ByteBuffer outputBuffer = mvEncoder.getOutputBuffers()[outIndex];
                                if ((info.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
                                    Log.d(TAG, "video encoder: codec config buffer");
                                    // Simply ignore codec config buffers.
                                    info.size = 0;
                                }

                                if (info.size != 0) {
                                    byte[] h264 = new byte[info.size];
                                    outputBuffer.get(h264, info.offset, info.size);
                                    mPMMuxer.writeH264Frame(h264, h264.length, info.flags == MediaCodec.BUFFER_FLAG_KEY_FRAME ? 3 : 6, info.presentationTimeUs, false);
                                }
                                mvEncoder.releaseOutputBuffer(outIndex, false);
                                if ((info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                                    Log.d(TAG, "video encoder: EOS");
                                    mPMMuxer.writeH264Frame(null, 0, 0, 0, true);
                                    vComplete = true;
                                    if (aComplete) {
                                        Message msg = mHandler.obtainMessage();
                                        msg.what = SUCCESS;
                                        mHandler.sendMessage(msg);
                                    }
                                    return;
                                }
                            } catch (Exception e) {
                                e.printStackTrace();
                            }
                        }
                        break;
                }

            }
        }
    };


    private Runnable maRunnable = new Runnable() {
        @Override
        public void run() {
            while (!isStop) {
                MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
                int outIndex = maEncoder.dequeueOutputBuffer(info, 10000);
                switch (outIndex) {
                    case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
                        Log.d(TAG, "INFO_OUTPUT_BUFFERS_CHANGED");
                        break;

                    case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                        MediaFormat format = maEncoder.getOutputFormat();
                        int sampleHZ = format.getInteger(MediaFormat.KEY_SAMPLE_RATE);
                        byte[] esds = format.getByteBuffer("csd-0").array();
                        mPMMuxer.addAACAudioTrack(sampleHZ, (byte) 0x02, esds, (short) esds.length);
                        Log.d(TAG, "INFO_OUTPUT_FORMAT_CHANGED format : " + format);
                        break;

                    case MediaCodec.INFO_TRY_AGAIN_LATER:
//                        Log.d(TAG, "INFO_TRY_AGAIN_LATER");
                        break;

                    default:
                        if (outIndex >= 0) {
                            try {
                                ByteBuffer outputBuffer = maEncoder.getOutputBuffers()[outIndex];
                                if ((info.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
                                    Log.d(TAG, "audio encoder: codec config buffer");
                                    // Simply ignore codec config buffers.
                                    info.size = 0;
                                }
                                if (info.size != 0) {
                                    byte[] aac = new byte[info.size];
                                    outputBuffer.get(aac, info.offset, info.size);
                                    mPMMuxer.writeAACFrame(aac, aac.length, info.presentationTimeUs, false);
                                }
                                maEncoder.releaseOutputBuffer(outIndex, false);
                                if ((info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                                    Log.d(TAG, "audio encoder: EOS");
                                    aComplete = true;
                                    if (vComplete) {
                                        Message msg = mHandler.obtainMessage();
                                        msg.what = SUCCESS;
                                        mHandler.sendMessage(msg);
                                    }
                                    return;
                                }
                            } catch (Exception e) {
                                e.printStackTrace();
                            }
                        }
                        break;
                }
            }
        }
    };


    @SuppressLint("HandlerLeak")
    private Handler mHandler = new Handler(Looper.getMainLooper(), new Handler.Callback() {
        @Override
        public boolean handleMessage(Message msg) {
            switch (msg.what) {
                case SUCCESS: {
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            if (mPMMuxer != null) {
                                mPMMuxer.setDescription(mDescription);
                                mPMMuxer.destory();
                                mPMMuxer = null;
                                if (mHandlerNativeMessage != null && mHandlerNativeMessage.get() != null) {
                                    Message msg = mHandlerNativeMessage.get().obtainMessage();
                                    msg.what = PMMessage.MESSAGE_ID_SAVE_COMPLETE;
                                    mHandlerNativeMessage.get().sendMessage(msg);
                                }
                            }
                            release();
                        }
                    }).start();
                }

                break;
                case FAILED: {
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            release();
                        }
                    }).start();
                }
                break;
                default:
                    break;
            }
            return false;
        }
    });

    private native void _create();

    private native void _release();

    private native void _setDataSourceSTB(PMStoryboard storyboard);

//    private native boolean _setVideoEncoderParams(String mime, int width, int height, int bitRate, int framerate, int iInterval, int profile, int level);
//
//    private native boolean _setAudioEncoderParams(String mime, int sampleRate, int channels, int bitRate, int profile);

    private native void _setSurface(Surface surface, int width, int height);

    private native void _start();

    private native void _stop();


    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        release();
        mHandlerNativeMessage = null;
    }

    private static final int SUCCESS = PMMessage.MESSAGE_ID_SAVE_COMPLETE + 2000;
    private static final int FAILED = PMMessage.MESSAGE_ID_SAVE_CANCELED + 2000;

}