//
// Created by ASUS on 2018/3/15.
//

#ifndef MEDIAENGINE_CODEC_H
#define MEDIAENGINE_CODEC_H

#include <stdint.h>
#include <stdbool.h>

#define NUMELEM(x) ((int)(sizeof(x) / sizeof((x)[0])))

typedef enum AVMimeType {
    MIMETYPE_UNKNOWN = -1,

    //mime type video
    MIMETYPE_VIDEO_START,
    MIMETYPE_VIDEO_VP8,
    MIMETYPE_VIDEO_VP9,
    MIMETYPE_VIDEO_AVC,
    MIMETYPE_VIDEO_HEVC,
    MIMETYPE_VIDEO_MPEG4,
    MIMETYPE_VIDEO_H263,
    MIMETYPE_VIDEO_MPEG2,
    MIMETYPE_VIDEO_RAW,
    MIMETYPE_VIDEO_DOLBY_VISION,
    MIMETYPE_VIDEO_SCRAMBLED,

    //mime type audio
    MIMETYPE_AUDIO_START = 100,
    MIMETYPE_AUDIO_AMR_NB,
    MIMETYPE_AUDIO_AMR_WB,
    MIMETYPE_AUDIO_MPEG,
    MIMETYPE_AUDIO_AAC,
    MIMETYPE_AUDIO_QCELP,
    MIMETYPE_AUDIO_VORBIS,
    MIMETYPE_AUDIO_OPUS,
    MIMETYPE_AUDIO_G711_ALAW,
    MIMETYPE_AUDIO_G711_MLAW,
    MIMETYPE_AUDIO_RAW,
    MIMETYPE_AUDIO_FLAC,
    MIMETYPE_AUDIO_MSGSM,
    MIMETYPE_AUDIO_AC3,
    MIMETYPE_AUDIO_EAC3,
    MIMETYPE_AUDIO_SCRAMBLED,

//    MIMETYPE_START_OTHER=200,
} AVMimeType;

typedef struct AVMimeTypeEntry {
    AVMimeType type;
    const char *disc;
} AVMimeTypeEntry;

AVMimeTypeEntry mime_type_dic[] = {
//mime type video
        {MIMETYPE_VIDEO_VP8,          "video/x-vnd.on2.vp8"},
        {MIMETYPE_VIDEO_VP9,          "video/x-vnd.on2.vp9"},
        {MIMETYPE_VIDEO_AVC,          "video/avc"},
        {MIMETYPE_VIDEO_HEVC,         "video/hevc"},
        {MIMETYPE_VIDEO_MPEG4,        "video/mp4v-es"},
        {MIMETYPE_VIDEO_H263,         "video/3gpp"},
        {MIMETYPE_VIDEO_MPEG2,        "video/mpeg2"},
        {MIMETYPE_VIDEO_RAW,          "video/raw"},
        {MIMETYPE_VIDEO_DOLBY_VISION, "video/dolby-vision"},
        {MIMETYPE_VIDEO_SCRAMBLED,    "video/scrambled"},

//mime type audio
        {MIMETYPE_AUDIO_AMR_NB,       "audio/3gpp"},
        {MIMETYPE_AUDIO_AMR_WB,       "audio/amr-wb"},
        {MIMETYPE_AUDIO_MPEG,         "audio/mpeg"},
        {MIMETYPE_AUDIO_AAC,          "audio/mp4a-latm"},
        {MIMETYPE_AUDIO_QCELP,        "audio/qcelp"},
        {MIMETYPE_AUDIO_VORBIS,       "audio/vorbis"},
        {MIMETYPE_AUDIO_OPUS,         "audio/opus"},
        {MIMETYPE_AUDIO_G711_ALAW,    "audio/g711-alaw"},
        {MIMETYPE_AUDIO_G711_MLAW,    "audio/g711-mlaw"},
        {MIMETYPE_AUDIO_RAW,          "audio/raw"},
        {MIMETYPE_AUDIO_FLAC,         "audio/flac"},
        {MIMETYPE_AUDIO_MSGSM,        "audio/gsm"},
        {MIMETYPE_AUDIO_AC3,          "audio/ac3"},
        {MIMETYPE_AUDIO_EAC3,         "audio/eac3"},
        {MIMETYPE_AUDIO_SCRAMBLED,    "audio/scrambled"}
};

typedef enum AVMediaType {
    AVMEDIA_TYPE_UNKNOWN = -1,  ///< Usually treated as AVMEDIA_TYPE_DATA
    AVMEDIA_TYPE_VIDEO,
    AVMEDIA_TYPE_AUDIO,
    AVMEDIA_TYPE_DATA,          ///< Opaque data information usually continuous
    AVMEDIA_TYPE_SUBTITLE,
    AVMEDIA_TYPE_ATTACHMENT,    ///< Opaque data information usually sparse
    AVMEDIA_TYPE_NB
} AVMediaType;
/*
 * key
 */
typedef enum AVDictKey {
    KEY_UNKNOWN = 0,
    KEY_MIME,
    KEY_CSD_0,
    KEY_CSD_1,
    KEY_WIDTH,
    KEY_HEIGHT,
    KEY_STRIDE,
    KEY_STRIDE_HEIGHT,
    KEY_BIT_RATE,
    KEY_COLOR_FORMAT,
    KEY_FRAME_RATE,
    KEY_I_FRAME_INTERVAL,
    KEY_PROFILE,
    KEY_LEVEL,
    KEY_BITRATE_MODE,
    KEY_SAMPLE_RATE,
    KEY_CHANNEL_COUNT,
    KEY_PCM_ENCODING,
    KEY_AAC_PROFILE,
} AVDictKey;

typedef struct AVDictionaryEntry {
    AVDictKey key;
    union Value {
        char *c;
        int i;
    } value;

} AVDictionaryEntry;

typedef struct AVDictionary {
    int count;
    AVDictionaryEntry *elems;
} AVDictionary;

typedef void *AVCodecHandle;

AVCodecHandle open(const AVDictionary *options, void *surface, const bool isEncoder);

int send_receive(AVCodecHandle codec, uint8_t *in, uint32_t in_size, int32_t flag, int64_t pts,
                 uint8_t **out, uint32_t *out_size, uint32_t *out_flag, int64_t *out_pts,
                 AVDictionary **out_options);

void flush(AVCodecHandle codec);

int close(AVCodecHandle codec);

#endif //MEDIAENGINE_CODEC_H
