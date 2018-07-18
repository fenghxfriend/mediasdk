package cn.paomiantv.mediasdk.common;

import android.media.MediaCodecInfo;
import android.media.MediaFormat;

/**
 * Created by John on 2017/9/5.
 */

public class PMConfig {


    /*
     * thumbnail
     */

    public static final int THUMB_WIDTH = 40; // thumbnail width
    public static final int THUMB_HEIGHT = 40; // thumbnail height

    /*
     * parameters for the video encoder
     */
    public static final String OUTPUT_VIDEO_MIME_TYPE = MediaFormat.MIMETYPE_VIDEO_AVC; // H.264 Advanced Video Coding
    public static final int OUTPUT_VIDEO_BIT_RATE = 2 * 1024 * 1024; // 2Mbps
    public static final int OUTPUT_VIDEO_FRAME_RATE = 25; // 25fps
    public static final int OUTPUT_VIDEO_IFRAME_INTERVAL = 1; // 1 seconds between I-frames
    public static final int OUTPUT_VIDEO_COLOR_FORMAT = MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface;
    public static final int OUTPUT_VIDEO_PROFILE = MediaCodecInfo.CodecProfileLevel.AVCProfileMain;
    public static final int OUTPUT_VIDEO_LEVEL = MediaCodecInfo.CodecProfileLevel.AVCLevel31;
    public static final int OUTPUT_VIDEO_WIDTH = 854; // Video output picture width
    public static final int OUTPUT_VIDEO_HEIGHT = 480; // Video output picture height
    public static final long VIDEO_ONE_FRAME_DURATION = 1000 * 1000 / 25; // microsecond per frame


    /*
     * parameters for the audio encoder
     */
    public static final String OUTPUT_AUDIO_MIME_TYPE = MediaFormat.MIMETYPE_AUDIO_AAC; // Advanced Audio Coding
    public static final int OUTPUT_AUDIO_CHANNEL_COUNT = 2; // Must match the input stream.
    public static final int OUTPUT_AUDIO_BIT_RATE = 128 * 1024;
    public static final int OUTPUT_AUDIO_AAC_PROFILE = MediaCodecInfo.CodecProfileLevel.AACObjectLC;
    public static final int OUTPUT_AUDIO_SAMPLE_RATE_HZ = 44100; // Must match the input stream.

    public static final int AUDIO_ONE_FRAME_LENGTH = 1024 * 2 * 2;
    public static final long AUDIO_ONE_FRAME_DURATION = 1000 * 1000 * 1024 / 44100; // microsecond per aac frame

    public enum TrackType {
        EM_TRACK_VIDEO("EM_TRACK_VIDEO", 1), EM_TRACK_AUDIO("EM_TRACK_AUDIO", 2), EM_TRACK_SUBTILTLE("EM_TRACK_SUBTILTLE", 3);
        private String name;
        private int index;

        // 构造方法
        private TrackType(String name, int index) {
            this.name = name;
            this.index = index;
        }

        public static TrackType getType(int index) {
            if (index == EM_TRACK_VIDEO.index) {
                return EM_TRACK_VIDEO;
            } else if (index == EM_TRACK_AUDIO.index) {
                return EM_TRACK_AUDIO;
            } else if (index == EM_TRACK_SUBTILTLE.index) {
                return EM_TRACK_SUBTILTLE;
            }
            return null;
        }

        public String getName() {
            return name;
        }

        public int getIndex() {
            return index;
        }

        // 覆盖方法
        @Override
        public String toString() {
            return this.index + "_" + this.name;
        }
    }

    /*
     * 查色表是对视频图像进行查色，可同时添加animation进行旋转等变换
     * 前景层必须在图像层上，可同时进行旋转等变换
     * 背景层必须在图像层下，否则会覆盖图像
     * 几何变换层是仅对视频图像进行旋转缩放等变换，不可同时与查色表使用，如同时使用则上面层会覆盖底下一层，如查色时需要进行几何变换，请在查色时添加animation
     * */
    public enum EffectType {
        EM_EFFECT_TRANSFORM_COLOR("EM_EFFECT_TRANSFORM_COLOR", 1),EM_EFFECT_MASK("EM_EFFECT_MASK", 2), EM_EFFECT_BLUR("EM_EFFECT_BLUR", 3), EM_EFFECT_NON_LINEAR("EM_EFFECT_NON_LINEAR", 4);
        private String name;
        private int index;

        // 构造方法
        private EffectType(String name, int index) {
            this.name = name;
            this.index = index;
        }

        public static EffectType getFilterType(int index) {
            if (index == EM_EFFECT_TRANSFORM_COLOR.index) {
                return EM_EFFECT_TRANSFORM_COLOR;
            } else if (index == EM_EFFECT_MASK.index) {
                return EM_EFFECT_MASK;
            } else if (index == EM_EFFECT_BLUR.index) {
                return EM_EFFECT_BLUR;
            } else if (index == EM_EFFECT_NON_LINEAR.index) {
                return EM_EFFECT_NON_LINEAR;
            }
            return null;
        }

        public String getName() {
            return name;
        }

        public int getIndex() {
            return index;
        }

        // 覆盖方法
        @Override
        public String toString() {
            return this.index + "_" + this.name;
        }
    }

    public enum SourceType {
        EM_SOURCE_FILE("EM_SOURCE_FILE", 1), EM_SOURCE_STREAM("EM_SOURCE_STREAM", 2), EM_SOURCE_OPAQUE("EM_SOURCE_OPAQUE", 3), EM_SOURCE_MULTI("EM_SOURCE_MULTI", 4);
        private String name;
        private int index;

        // 构造方法
        private SourceType(String name, int index) {
            this.name = name;
            this.index = index;
        }

        public static SourceType getSourceType(int index) {
            if (index == EM_SOURCE_FILE.index) {
                return EM_SOURCE_FILE;
            } else if (index == EM_SOURCE_STREAM.index) {
                return EM_SOURCE_STREAM;
            } else if (index == EM_SOURCE_OPAQUE.index) {
                return EM_SOURCE_OPAQUE;
            } else if (index == EM_SOURCE_MULTI.index) {
                return EM_SOURCE_MULTI;
            }
            return null;
        }

        public String getName() {
            return name;
        }

        public int getIndex() {
            return index;
        }

        // 覆盖方法
        @Override
        public String toString() {
            return this.index + "_" + this.name;
        }
    }

    public enum DirectionType {
        VERTICAL("vertical", 1), HORIZONTAL("horizontal", 2);
        private String name;
        private int index;

        // 构造方法
        private DirectionType(String name, int index) {
            this.name = name;
            this.index = index;
        }

        public static DirectionType getDirectionType(int index) {
            if (index == VERTICAL.index) {
                return VERTICAL;
            } else if (index == HORIZONTAL.index) {
                return HORIZONTAL;
            }
            return null;
        }

        public String getName() {
            return name;
        }

        public int getIndex() {
            return index;
        }

        // 覆盖方法
        @Override
        public String toString() {
            return this.index + "_" + this.name;
        }
    }

    public enum PlayerStatus {
        UNKNOWN("unknown", 0), INITIALIZE("initialize", 1), PREPARED("prepared", 3),PLAYING("playing", 4), PAUSED("paused", 5), STOPPED("stopped", 6);
        private String name;
        private int index;

        // 构造方法
        private PlayerStatus(String name, int index) {
            this.name = name;
            this.index = index;
        }

        public static PlayerStatus get(int index) {
            if (index == PLAYING.index) {
                return PLAYING;
            } else if (index == PAUSED.index) {
                return PAUSED;
            } else if (index == STOPPED.index) {
                return STOPPED;
            } else {
                return UNKNOWN;
            }
        }

        public String getName() {
            return name;
        }

        public int getIndex() {
            return index;
        }

        // 覆盖方法
        @Override
        public String toString() {
            return this.index + "_" + this.name;
        }
    }

    //pcm encoding
    public static final int AV_SAMPLE_FMT_U8 = 0;         ///< unsigned 8 bits
    public static final int AV_SAMPLE_FMT_S16 = 1;         ///< signed 16 bits
    public static final int AV_SAMPLE_FMT_S32 = 2;         ///< signed 32 bits
    public static final int AV_SAMPLE_FMT_FLT = 3;         ///< float
    public static final int AV_SAMPLE_FMT_DBL = 4;         ///< double

    public static final int AV_SAMPLE_FMT_U8P = 5;         ///< unsigned 8 bits, planar
    public static final int AV_SAMPLE_FMT_S16P = 6;        ///< signed 16 bits, planar
    public static final int AV_SAMPLE_FMT_S32P = 7;        ///< signed 32 bits, planar
    public static final int AV_SAMPLE_FMT_FLTP = 8;        ///< float, planar
    public static final int AV_SAMPLE_FMT_DBLP = 9;        ///< double, planar
    public static final int AV_SAMPLE_FMT_S64 = 10;         ///< signed 64 bits
    public static final int AV_SAMPLE_FMT_S64P = 11;        ///< signed 64 bits, planar

    public static final int AV_SAMPLE_FMT_NB = 12;           ///< Number of sample formats. DO NOT USE if linking dynamically
    //channel layout
    public static final long AV_CH_FRONT_LEFT = 0x00000001;
    public static final long AV_CH_FRONT_RIGHT = 0x00000002;
    public static final long AV_CH_FRONT_CENTER = 0x00000004;
    public static final long AV_CH_LOW_FREQUENCY = 0x00000008;
    public static final long AV_CH_BACK_LEFT = 0x00000010;
    public static final long AV_CH_BACK_RIGHT = 0x00000020;
    public static final long AV_CH_FRONT_LEFT_OF_CENTER = 0x00000040;
    public static final long AV_CH_FRONT_RIGHT_OF_CENTER = 0x00000080;
    public static final long AV_CH_BACK_CENTER = 0x00000100;
    public static final long AV_CH_SIDE_LEFT = 0x00000200;
    public static final long AV_CH_SIDE_RIGHT = 0x00000400;
    public static final long AV_CH_TOP_CENTER = 0x00000800;
    public static final long AV_CH_TOP_FRONT_LEFT = 0x00001000;
    public static final long AV_CH_TOP_FRONT_CENTER = 0x00002000;
    public static final long AV_CH_TOP_FRONT_RIGHT = 0x00004000;
    public static final long AV_CH_TOP_BACK_LEFT = 0x00008000;
    public static final long AV_CH_TOP_BACK_CENTER = 0x00010000;
    public static final long AV_CH_TOP_BACK_RIGHT = 0x00020000;
    public static final long AV_CH_STEREO_LEFT = 0x20000000;  ///< Stereo downmix.
    public static final long AV_CH_STEREO_RIGHT = 0x40000000; ///< See AV_CH_STEREO_LEFT.
    public static final long AV_CH_WIDE_LEFT = 0x0000000080000000;
    public static final long AV_CH_WIDE_RIGHT = 0x0000000100000000l;
    public static final long AV_CH_SURROUND_DIRECT_LEFT = 0x0000000200000000l;
    public static final long AV_CH_SURROUND_DIRECT_RIGHT = 0x0000000400000000l;
    public static final long AV_CH_LOW_FREQUENCY_2 = 0x0000000800000000l;

    /**
     * Channel mask value used for AVCodecContext.request_channel_layout
     * to indicate that the user requests the channel order of the decoder output
     * to be the native codec channel order.
     */
    public static final long AV_CH_LAYOUT_NATIVE = 0x8000000000000000l;

    /**
     * @}
     * @defgroup channel_mask_c Audio channel layouts
     * @{
     */
    public static final long AV_CH_LAYOUT_MONO = (AV_CH_FRONT_CENTER);
    public static final long AV_CH_LAYOUT_STEREO = (AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT);
    public static final long AV_CH_LAYOUT_2POINT1 = (AV_CH_LAYOUT_STEREO | AV_CH_LOW_FREQUENCY);
    public static final long AV_CH_LAYOUT_2_1 = (AV_CH_LAYOUT_STEREO | AV_CH_BACK_CENTER);
    public static final long AV_CH_LAYOUT_SURROUND = (AV_CH_LAYOUT_STEREO | AV_CH_FRONT_CENTER);
    public static final long AV_CH_LAYOUT_3POINT1 = (AV_CH_LAYOUT_SURROUND | AV_CH_LOW_FREQUENCY);
    public static final long AV_CH_LAYOUT_4POINT0 = (AV_CH_LAYOUT_SURROUND | AV_CH_BACK_CENTER);
    public static final long AV_CH_LAYOUT_4POINT1 = (AV_CH_LAYOUT_4POINT0 | AV_CH_LOW_FREQUENCY);
    public static final long AV_CH_LAYOUT_2_2 = (AV_CH_LAYOUT_STEREO | AV_CH_SIDE_LEFT | AV_CH_SIDE_RIGHT);
    public static final long AV_CH_LAYOUT_QUAD = (AV_CH_LAYOUT_STEREO | AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT);
    public static final long AV_CH_LAYOUT_5POINT0 = (AV_CH_LAYOUT_SURROUND | AV_CH_SIDE_LEFT | AV_CH_SIDE_RIGHT);
    public static final long AV_CH_LAYOUT_5POINT1 = (AV_CH_LAYOUT_5POINT0 | AV_CH_LOW_FREQUENCY);
    public static final long AV_CH_LAYOUT_5POINT0_BACK = (AV_CH_LAYOUT_SURROUND | AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT);
    public static final long AV_CH_LAYOUT_5POINT1_BACK = (AV_CH_LAYOUT_5POINT0_BACK | AV_CH_LOW_FREQUENCY);
    public static final long AV_CH_LAYOUT_6POINT0 = (AV_CH_LAYOUT_5POINT0 | AV_CH_BACK_CENTER);
    public static final long AV_CH_LAYOUT_6POINT0_FRONT = (AV_CH_LAYOUT_2_2 | AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER);
    public static final long AV_CH_LAYOUT_HEXAGONAL = (AV_CH_LAYOUT_5POINT0_BACK | AV_CH_BACK_CENTER);
    public static final long AV_CH_LAYOUT_6POINT1 = (AV_CH_LAYOUT_5POINT1 | AV_CH_BACK_CENTER);
    public static final long AV_CH_LAYOUT_6POINT1_BACK = (AV_CH_LAYOUT_5POINT1_BACK | AV_CH_BACK_CENTER);
    public static final long AV_CH_LAYOUT_6POINT1_FRONT = (AV_CH_LAYOUT_6POINT0_FRONT | AV_CH_LOW_FREQUENCY);
    public static final long AV_CH_LAYOUT_7POINT0 = (AV_CH_LAYOUT_5POINT0 | AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT);
    public static final long AV_CH_LAYOUT_7POINT0_FRONT = (AV_CH_LAYOUT_5POINT0 | AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER);
    public static final long AV_CH_LAYOUT_7POINT1 = (AV_CH_LAYOUT_5POINT1 | AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT);
    public static final long AV_CH_LAYOUT_7POINT1_WIDE = (AV_CH_LAYOUT_5POINT1 | AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER);
    public static final long AV_CH_LAYOUT_7POINT1_WIDE_BACK = (AV_CH_LAYOUT_5POINT1_BACK | AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER);
    public static final long AV_CH_LAYOUT_OCTAGONAL = (AV_CH_LAYOUT_5POINT0 | AV_CH_BACK_LEFT | AV_CH_BACK_CENTER | AV_CH_BACK_RIGHT);
    public static final long AV_CH_LAYOUT_HEXADECAGONAL = (AV_CH_LAYOUT_OCTAGONAL | AV_CH_WIDE_LEFT | AV_CH_WIDE_RIGHT | AV_CH_TOP_BACK_LEFT | AV_CH_TOP_BACK_RIGHT | AV_CH_TOP_BACK_CENTER | AV_CH_TOP_FRONT_CENTER | AV_CH_TOP_FRONT_LEFT | AV_CH_TOP_FRONT_RIGHT);
    public static final long AV_CH_LAYOUT_STEREO_DOWNMIX = (AV_CH_STEREO_LEFT | AV_CH_STEREO_RIGHT);
}
