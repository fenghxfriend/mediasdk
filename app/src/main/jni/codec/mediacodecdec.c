/*
 * Android MediaCodec MPEG-2 / H.264 / H.265 / MPEG-4 / VP8 / VP9 decoders
 *
 * Copyright (c) 2015-2016 Matthieu Bouron <matthieu.bouron stupeflix.com>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdint.h>
#include <string.h>
#include <autolog.h>

#include "mediacodec_wrapper.h"
#include "mediacodecdec_common.h"
#include "ffjni.h"
#include "codec.h"

typedef struct MediaCodecH264DecContext {
    MediaCodecContext *ctx;

    AVPacket buffered_pkt;

    int delay_flush;

} MediaCodecH264DecContext;

static int mediacodec_close(AVCodecContext *avctx) {
    MediaCodecH264DecContext *s = avctx->priv_data;

    ff_mediacodec_dec_close(avctx, s->ctx);
    s->ctx = NULL;

    memset(&s->buffered_pkt, 0, sizeof(AVPacket));

    return 0;
}

static int mediacodec_init(AVCodecContext *avctx) {
    int ret;

    FFAMediaFormat *format = NULL;
    MediaCodecH264DecContext *s = avctx->priv_data;

    format = ff_AMediaFormat_new();
    if (!format) {
        LOGE("Failed to create media format\n");
        ret = AV_FLAG_EXTERNAL;
        goto done;
    }

    ff_AMediaFormat_setString(format, "mime", avctx->mime);
    ff_AMediaFormat_setInt32(format, "width", avctx->width);
    ff_AMediaFormat_setInt32(format, "height", avctx->height);

    size_t n = sizeof(*s->ctx);
    s->ctx = realloc(NULL, n);
    if (!s->ctx) {
        LOGE("Failed to allocate MediaCodecDecContext\n");
        ret = AV_FLAG_NOMEM;
        goto done;
    } else {
        memset(s->ctx, 0, n);
    }

    s->ctx->delay_flush = s->delay_flush;

    if ((ret = ff_mediacodec_dec_init(avctx, s->ctx, avctx->mime, format)) < 0) {
        s->ctx = NULL;
        goto done;
    }

    LOGI("MediaCodec started successfully, ret = %d\n", ret);

    done:
    if (format) {
        ff_AMediaFormat_delete(format);
    }

    if (ret < 0) {
        mediacodec_close(avctx);
    }

    return ret;
}

static int mediacodec_send_receive(AVCodecContext *avctx,
                                   MediaCodecH264DecContext *s,
                                   AVFrame *frame, bool wait) {
    int ret;

    /* send any pending data from buffered packet */
    while (s->buffered_pkt.size) {
        ret = ff_mediacodec_dec_send(avctx, s->ctx, &s->buffered_pkt);
        if (ret == AV_FLAG_AGAIN)
            break;
        else if (ret < 0)
            return ret;
        s->buffered_pkt.size -= ret;
        s->buffered_pkt.data += ret;
        if (s->buffered_pkt.size <= 0)
            memset(&s->buffered_pkt, 0, sizeof(AVPacket));
    }

    /* check for new frame */
    return ff_mediacodec_dec_receive(avctx, s->ctx, frame, wait);
}

static int mediacodec_receive_frame(AVCodecContext *avctx, AVFrame *frame) {
    MediaCodecH264DecContext *s = avctx->priv_data;
    int ret;

    /*
     * MediaCodec.flush() discards both input and output  buffers, thus we
     * need to delay the call to this function until the user has released or
     * renderered the frames he retains.
     *
     * After we have buffered an input packet, check if the codec is in the
     * flushing state. If it is, we need to call ff_mediacodec_dec_flush.
     *
     * ff_mediacodec_dec_flush returns 0 if the flush cannot be performed on
     * the codec (because the user retains frames). The codec stays in the
     * flushing state.
     *
     * ff_mediacodec_dec_flush returns 1 if the flush can actually be
     * performed on the codec. The codec leaves the flushing state and can
     * process again packets.
     *
     * ff_mediacodec_dec_flush returns a negative value if an error has
     * occurred.
     *
     */
    if (ff_mediacodec_dec_is_flushing(avctx, s->ctx)) {
        if (!ff_mediacodec_dec_flush(avctx, s->ctx)) {
            return AV_FLAG_AGAIN;
        }
    }

    /* flush buffered packet and check for new frame */
    ret = mediacodec_send_receive(avctx, s, frame, false);
    if (ret != AV_FLAG_AGAIN)
        return ret;

    /* skip fetching new packet if we still have one buffered */
    if (s->buffered_pkt.size > 0)
        return AV_FLAG_AGAIN;

    /* fetch new packet or eof */
    ret = ff_decode_get_packet(avctx, &s->buffered_pkt);
    if (ret == AV_FLAG_EOF) {
        AVPacket null_pkt = {0};
        ret = ff_mediacodec_dec_send(avctx, s->ctx, &null_pkt);
        if (ret < 0)
            return ret;
    } else if (ret < 0)
        return ret;

    /* crank decoder with new packet */
    return mediacodec_send_receive(avctx, s, frame, true);
}

static void mediacodec_flush(AVCodecContext *avctx) {
    MediaCodecH264DecContext *s = avctx->priv_data;

    memset(&s->buffered_pkt, 0, sizeof(AVPacket));

    ff_mediacodec_dec_flush(avctx, s->ctx);
}

static void setCodecTypeFormMime(AVCodecContext *context, int type) {
    for (int i = 0; i < NUMELEM(mime_type_dic); i++) {
        if (mime_type_dic[i].type == type) {
            context->mime = (char *) mime_type_dic->disc;
            if (type > MIMETYPE_VIDEO_START && type < MIMETYPE_AUDIO_START) {
                context->codec_type = AVMEDIA_TYPE_VIDEO;
            } else if (type > MIMETYPE_AUDIO_START) {
                context->codec_type = AVMEDIA_TYPE_AUDIO;
            }
        }
    }
}

AVCodecHandle open(const AVDictionary *options, void *surface, const bool isEncoder) {
    int ret = 0;
    AVCodecContext *context = NULL;
    size_t n = sizeof(AVCodecContext);
    context = realloc(NULL, n);
    if (!context) {
        return NULL;
    }
    memset(context, 0, n);
    if (surface) {
        GLSurfaceContext *surfaceContext = NULL;
        n = sizeof(GLSurfaceContext);
        surfaceContext = realloc(NULL, n);
        if (!surfaceContext) {
            return NULL;
        }
        surfaceContext->surface = surface;
        context->hwaccel_context = surfaceContext;
    }
    int mime = 0;
    int width = 0;
    int height = 0;
    int bit_rate = 0;
    int color_format = 0;
    int frame_rate = 0;
    int i_interval = 0;
    int profile = 0;
    int level = 0;
    int bit_mode = 0;
    int sample_rate = 0;
    int channel_count = 0;
    int pcm_encoding = 0;
    int aac_profile = 0;
    for (int i = 0; i < options->count; ++i) {
        switch (options->elems[i].key) {
            case KEY_MIME: {
                mime = options->elems[i].value.i;
                setCodecTypeFormMime(context, mime);
            }
                break;
            case KEY_WIDTH: {
                width = options->elems[i].value.i;
                context->width = width;
            }
                break;
            case KEY_HEIGHT: {
                height = options->elems[i].value.i;
                context->height = height;
            }
                break;
            case KEY_BIT_RATE: {
                bit_rate = options->elems[i].value.i;
            }
                break;
            case KEY_COLOR_FORMAT: {
                color_format = options->elems[i].value.i;
            }
                break;
            case KEY_FRAME_RATE: {
                frame_rate = options->elems[i].value.i;
            }
                break;
            case KEY_I_FRAME_INTERVAL: {
                i_interval = options->elems[i].value.i;
            }
                break;
            case KEY_PROFILE: {
                profile = options->elems[i].value.i;
            }
                break;
            case KEY_LEVEL: {
                level = options->elems[i].value.i;
            }
                break;
            case KEY_BITRATE_MODE: {
                bit_mode = options->elems[i].value.i;
            }
                break;
            case KEY_SAMPLE_RATE: {
                sample_rate = options->elems[i].value.i;
            }
                break;
            case KEY_CHANNEL_COUNT: {
                channel_count = options->elems[i].value.i;
            }
                break;
            case KEY_PCM_ENCODING: {
                pcm_encoding = options->elems[i].value.i;
            }
                break;
            case KEY_AAC_PROFILE: {
                aac_profile = options->elems[i].value.i;
            }
                break;
            default:
                break;
        }
    }
    if (isEncoder) {
        FFAMediaFormat *format = ff_AMediaFormat_new();
        if (!format) {
            LOGE("Failed to create media format\n");
            ret = AV_FLAG_EXTERNAL;
            goto done;
        }
        ff_AMediaFormat_setString(format, "mime", context->mime);
        switch (context->codec_type){
            case AVMEDIA_TYPE_VIDEO:{
                //OMX.IMG.TOPAZ.VIDEO.Encoder 华为P9编码器花屏问题，必须设置 profile为main，level为31
                //OMX.hisi.video.encoder.avc 华为Mate9编码器崩溃问题，必须设置 width，height为8的倍数
                ff_AMediaFormat_setInt32("")

            }
                break;
            case AVMEDIA_TYPE_AUDIO:{

            }
                break;
            default:
                break;
        }
    } else {

    }
    done:
    if (ret < 0) {
        if (context->hwaccel_context != NULL) {
            free(context->hwaccel_context);
        }
        free(context);
        context = NULL;
    }

    return context;
}

int send_receive(AVCodecHandle codec, uint8_t *in, uint32_t in_size, int32_t flag, int64_t pts,
                 uint8_t **out, uint32_t *out_size, uint32_t *out_flag, int64_t *out_pts,
                 AVDictionary **out_options) {

}

void flush(AVCodecHandle codec) {

}

int close(AVCodecHandle codec) {

}


struct AVCodec {
    size_t priv_data_size;

    int (*init)(AVCodecContext *);

    int (*receive_frame)(AVCodecContext *avctx, AVFrame *frame);

    void (*flush)(AVCodecContext *);

    int (*close)(AVCodecContext *);
} codec = {
        .priv_data_size = sizeof(MediaCodecH264DecContext),
        .init           = mediacodec_init,
        .receive_frame  = mediacodec_receive_frame,
        .flush          = mediacodec_flush,
        .close          = mediacodec_close,
};
