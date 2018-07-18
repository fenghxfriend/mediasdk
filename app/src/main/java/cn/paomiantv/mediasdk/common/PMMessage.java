package cn.paomiantv.mediasdk.common;

/**
 * Created by John on 2017/8/22.
 */

public class PMMessage {
    public static final int MESSAGE_ID_V_DECODE_COMPLETE = 1;
    public static final int MESSAGE_ID_A_DECODE_COMPLETE = 2;
    public static final int MESSAGE_ID_V_RENDER_PROGRESS = 3;
    public static final int MESSAGE_ID_A_PLAY_PROGRESS = 4;
    public static final int MESSAGE_ID_V_RENDER_COMPLETE = 5;
    public static final int MESSAGE_ID_A_PLAY_COMPLETE = 6;
    public static final int MESSAGE_ID_V_SAVE_RENDER_PROGRESS = 7;
    public static final int MESSAGE_ID_A_SAVE_PLAY_PROGRESS = 8;

    public static final int MESSAGE_ID_V_CUT_RENDER_COMPLETE = 9;
    public static final int MESSAGE_ID_A_CUT_PLAY_COMPLETE = 10;

    public static final int MESSAGE_ID_SAVE_COMPLETE = 999;
    public static final int MESSAGE_ID_SAVE_CANCELED = 1000;


    public static final int MESSAGE_ID_REVERSE_COMPLETE = 1001;
    public static final int MESSAGE_ID_REVERSE_CANCELED = 1002;


    public static final int MESSAGE_ID_ERROR_PREPARE_AUDIO_CTRL_FAILED = 10001;
    public static final int MESSAGE_ID_ERROR_PREPARE_AUDIO_DECODER_FAILED = 10002;
    public static final int MESSAGE_ID_ERROR_PREPARE_AUDIO_TRACK_FAILED = 10003;
    public static final int MESSAGE_ID_ERROR_PREPARE_VIDEO_CTRL_FAILED = 10004;
    public static final int MESSAGE_ID_ERROR_PREPARE_VIDEO_DECODER_FAILED = 10005;

}
