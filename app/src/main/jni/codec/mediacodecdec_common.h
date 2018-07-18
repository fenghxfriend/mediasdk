/*
 * Android MediaCodec decoder
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

#ifndef AVCODEC_MEDIACODECDEC_COMMON_H
#define AVCODEC_MEDIACODECDEC_COMMON_H

#include <stdint.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <sys/types.h>
#include "mediacodec_wrapper.h"

typedef struct MediaCodecContext {

    AVCodecContext *avctx;
    atomic_int refcount;

    char *codec_name;

    FFAMediaCodec *codec;
    FFAMediaFormat *format;

    void *surface;

    int started;
    int draining;
    int flushing;
    int eos;

    int width;
    int height;
    int stride;
    int slice_height;
    int color_format;
    int crop_top;
    int crop_bottom;
    int crop_left;
    int crop_right;

    uint64_t output_buffer_count;

    int delay_flush;
    atomic_int serial;

} MediaCodecContext;


typedef struct AVPacket {
    int64_t pts;
    uint8_t *data;
    int size;
    int offset;
    int flags;
    int64_t duration;
} AVPacket;


typedef struct AVFrame {

    uint8_t *data[3];

    int linesize[3];

    uint8_t **extended_data;

    int width, height;

    int nb_samples;

    int format;

    int64_t pts;

    int quality;

    void *opaque;

    int sample_rate;

    uint64_t channel_layout;

    int flags;
    int channels;
} AVFrame;


void av_free(void *ptr);

void av_freep(void *arg);

int ff_mediacodec_dec_init(AVCodecContext *avctx,
                           MediaCodecContext *s,
                           const char *mime,
                           FFAMediaFormat *format);

int ff_mediacodec_dec_send(AVCodecContext *avctx,
                           MediaCodecContext *s,
                           AVPacket *pkt);

int ff_mediacodec_dec_receive(AVCodecContext *avctx,
                              MediaCodecContext *s,
                              AVFrame *frame,
                              bool wait);

int ff_mediacodec_dec_flush(AVCodecContext *avctx,
                            MediaCodecContext *s);

int ff_mediacodec_dec_close(AVCodecContext *avctx,
                            MediaCodecContext *s);

int ff_mediacodec_dec_is_flushing(AVCodecContext *avctx,
                                  MediaCodecContext *s);

typedef struct MediaCodecBuffer {

    MediaCodecContext *ctx;
    ssize_t index;
    int64_t pts;
    atomic_int released;
    int serial;

} MediaCodecBuffer;

#endif /* AVCODEC_MEDIACODECDEC_COMMON_H */
