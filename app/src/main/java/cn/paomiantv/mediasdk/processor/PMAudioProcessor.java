package cn.paomiantv.mediasdk.processor;

import android.media.AudioFormat;

import cn.paomiantv.mediasdk.common.PMConfig;

/**
 * Created by John on 2017/10/13.
 */

public class PMAudioProcessor {

    private int input_sample_rate = 44100;
    private int input_encoding = AudioFormat.ENCODING_PCM_16BIT;
    private int input_channel_layout = 2;
    private float out_volume = 1.0f;

    public void init(int input_sample_rate, int input_encoding, int input_channel_layout, float out_volume) {
        this.input_sample_rate = input_sample_rate;
        this.input_encoding = input_encoding;
        this.input_channel_layout = input_channel_layout;
        _setup(input_sample_rate, shiftEncoding(input_encoding), shiftChannelLayout(input_channel_layout), out_volume);
    }

    public void destroy() {
        _teardown();
    }

    public byte[] process(byte[] input, int samplesize) {
        return _process(input, samplesize);
    }

    private synchronized native final boolean _setup(int input_sample_rate, int input_format, long input_channel_layout, float out_volume);

    private synchronized native final void _teardown();

    private synchronized native final byte[] _process(byte[] input, int samplesize);

    private int shiftEncoding(int input_encoding) {
        int encoding = PMConfig.AV_SAMPLE_FMT_S16;
        switch (input_encoding) {
            case AudioFormat.ENCODING_PCM_8BIT:
                encoding = PMConfig.AV_SAMPLE_FMT_U8;
                break;
            case AudioFormat.ENCODING_PCM_16BIT:
                encoding = PMConfig.AV_SAMPLE_FMT_S16;
                break;
            case AudioFormat.ENCODING_PCM_FLOAT:
                encoding = PMConfig.AV_SAMPLE_FMT_FLT;
                break;
            default:
                break;
        }
        return encoding;
    }

    private long shiftChannelLayout(int input_channel_layout) {
        long channel_layout = 3;
        switch (input_channel_layout) {
            case AudioFormat.CHANNEL_OUT_MONO:
                channel_layout = PMConfig.AV_CH_LAYOUT_MONO;
                break;
            case AudioFormat.CHANNEL_OUT_STEREO:
                channel_layout = PMConfig.AV_CH_LAYOUT_STEREO;
                break;
            case AudioFormat.CHANNEL_OUT_5POINT1:
                channel_layout = PMConfig.AV_CH_LAYOUT_5POINT1;
                break;
            case AudioFormat.CHANNEL_OUT_7POINT1_SURROUND:
                channel_layout = PMConfig.AV_CH_LAYOUT_7POINT1;
                break;
            case AudioFormat.CHANNEL_OUT_BACK_CENTER:
                channel_layout = PMConfig.AV_CH_BACK_CENTER;
                break;
            case AudioFormat.CHANNEL_OUT_BACK_LEFT:
                channel_layout = PMConfig.AV_CH_BACK_LEFT;
                break;
            case AudioFormat.CHANNEL_OUT_BACK_RIGHT:
                channel_layout = PMConfig.AV_CH_BACK_RIGHT;
                break;
            case AudioFormat.CHANNEL_OUT_FRONT_CENTER:
                channel_layout = PMConfig.AV_CH_FRONT_CENTER;
                break;
            case AudioFormat.CHANNEL_OUT_SURROUND:
                channel_layout = PMConfig.AV_CH_LAYOUT_SURROUND;
                break;
            case AudioFormat.CHANNEL_OUT_QUAD:
                channel_layout = PMConfig.AV_CH_LAYOUT_QUAD;
                break;
            case AudioFormat.CHANNEL_OUT_SIDE_LEFT:
                channel_layout = PMConfig.AV_CH_SIDE_LEFT;
                break;
            case AudioFormat.CHANNEL_OUT_SIDE_RIGHT:
                channel_layout = PMConfig.AV_CH_SIDE_RIGHT;
                break;
            case AudioFormat.CHANNEL_OUT_LOW_FREQUENCY:
                channel_layout = PMConfig.AV_CH_LOW_FREQUENCY;
                break;
            default:
                break;
        }
        return channel_layout;
    }

//    public static void volume_control(byte[] buf, ByteOrder order, float in_vol) {
//        int i;
//        short tmp;
//        byte[] out;
//        byte[] in = new byte[2];
//
//        for (i = 0; i < (buf.length / 2); i++) {
//            in[0] = buf[i * 2];
//            in[1] = buf[i * 2 + 1];
//            if (order == ByteOrder.LITTLE_ENDIAN) {
//                tmp = (short) (PMByteTrans.getInstance().getShort(in, false) * in_vol);
//            } else {
//                tmp = (short) (PMByteTrans.getInstance().getShort(in, true) * in_vol);
//            }
//            if (tmp > 32767) {
//                tmp = 32767;
//            } else if (tmp < -32768) {
//                tmp = -32768;
//            }
//            if (order == ByteOrder.LITTLE_ENDIAN) {
//                out = PMByteTrans.getInstance().getBytes(tmp, false);
//            } else {
//                out = PMByteTrans.getInstance().getBytes(tmp, true);
//            }
//            buf[i * 2] = out[0];
//            buf[i * 2 + 1] = out[1];
//        }
//    }
}
