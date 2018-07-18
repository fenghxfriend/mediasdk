/*
 * Android MediaCodec public API functions
 *
 * Copyright (c) 2016 Matthieu Bouron <matthieu.bouron stupeflix.com>
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
#include "mediacodec.h"

#include <jni.h>
#include <malloc.h>
#include <autolog.h>
#include <inttypes.h>
#include <string.h>

#include "ffjni.h"
#include "mediacodecdec_common.h"
#include "codec.h"

GLSurfaceContext *av_surface_alloc_context(void) {
    size_t n = sizeof(GLSurfaceContext);
    void *p = realloc(NULL, n);
    if (p) {
        memset(p, 0, n);
    }
    return p;
}

int av_surface_default_init(AVCodecContext *avctx, GLSurfaceContext *ctx, void *surface) {
    int ret = 0;
    JNIEnv *env = NULL;
    if (avctx->codec_type != AVMEDIA_TYPE_VIDEO) {
        return ret;
    }

    env = ff_jni_get_env();
    if (!env) {
        return AV_FLAG_EXTERNAL;
    }

    ctx->surface = (*env)->NewGlobalRef(env, surface);
    if (ctx->surface) {
        avctx->hwaccel_context = ctx;
    } else {
        LOGE("Could not create new global reference\n");
        ret = AV_FLAG_EXTERNAL;
    }

    return ret;
}

void av_surface_default_free(AVCodecContext *avctx) {
    JNIEnv *env = NULL;

    GLSurfaceContext *ctx = avctx->hwaccel_context;

    if (!ctx) {
        return;
    }

    env = ff_jni_get_env();
    if (!env) {
        return;
    }

    if (ctx->surface) {
        (*env)->DeleteGlobalRef(env, ctx->surface);
        ctx->surface = NULL;
    }

    free(&avctx->hwaccel_context);
    avctx->hwaccel_context = NULL;
}

int av_mediacodec_release_buffer(AVMediaCodecBuffer *buffer, int render) {
    MediaCodecContext *ctx = buffer->ctx;
    int released = atomic_fetch_add(&buffer->released, 1);

    if (!released && (ctx->delay_flush || buffer->serial == atomic_load(&ctx->serial))) {
        LOGD("Releasing output buffer %zd ts=%"
                     PRId64
                     " render=%d\n", buffer->index, buffer->pts, render);
        return ff_AMediaCodec_releaseOutputBuffer(ctx->codec, buffer->index, render);
    }

    return 0;
}
