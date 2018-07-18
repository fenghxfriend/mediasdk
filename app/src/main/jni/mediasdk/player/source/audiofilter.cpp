/*******************************************************************************
 *        Module: paomiantv
 *          File: aprocessor.cpp
 * Functionality: handle video data.
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-08-03  v1.0        huangxuefeng  created
 ******************************************************************************/
#include <inttypes.h>
#include "audiofilter.h"
#include "autolog.h"

#define MAX_OUTPUT_BUFFER_SIZE (128*AUDIO_SAMPLE_COUNT_PER_FRAME)

namespace paomiantv {

    CAudioFilter::CAudioFilter() :
            m_ullInputChannelLayout(AV_CH_LAYOUT_STEREO),
            m_eInputFormat(AV_SAMPLE_FMT_S16),
            m_nInputSampleRate(AUDIO_SAMPLE_FREQUENCY),
            m_fOutVolume(1.0f),
            m_ullFramePTS(0l),
            m_pGraph(NULL),
            m_pSrc(NULL),
            m_pSink(NULL),
            m_pFrame(NULL),
            m_pOutBuffer(NULL) {
        USE_LOG;
    }

    CAudioFilter::~CAudioFilter() {
        USE_LOG;
        uninit();
    }

    BOOL32 CAudioFilter::init(s32 nInputSampleRate, s32 nInputFormat, u64 nInputChannelLayout,
                              float fOutVolume) {
        m_eInputFormat = shiftfmt(nInputFormat);
        m_fOutVolume = fOutVolume;
        m_pOutBuffer = (u8 *) av_malloc(MAX_OUTPUT_BUFFER_SIZE);
        m_nInputSampleRate = nInputSampleRate;
        m_ullInputChannelLayout = shiftchnlyt(nInputChannelLayout);
        avfilter_register_all();
        m_pFrame = av_frame_alloc();
        if (!m_pFrame) {
            LOGE("Error allocating the frame\n");
            return FALSE;
        }
        s32 re = init_filter_graph(&m_pGraph, &m_pSrc, &m_pSink);
        if (re != 0) {
            return FALSE;
        }
        return TRUE;

    }

    s32 CAudioFilter::init_filter_graph(AVFilterGraph **graph, AVFilterContext **src,
                                        AVFilterContext **sink) {
        AVFilterGraph *filter_graph;
        AVFilterContext *abuffer_ctx;
        AVFilter *abuffer;
        AVFilterContext *volume_ctx;
        AVFilter *volume;
        AVFilterContext *aformat_ctx;
        AVFilter *aformat;
        AVFilterContext *abuffersink_ctx;
        AVFilter *abuffersink;

        AVDictionary *options_dict = NULL;
        s8 options_str[1024];
        s8 ch_layout[64];

        int err;

        /* Create a new filtergraph, which will contain all the filters. */
        filter_graph = avfilter_graph_alloc();
        if (!filter_graph) {
            LOGE("Unable to create filter graph.\n");
            return AVERROR(ENOMEM);
        }

        /* Create the abuffer filter;
         * it will be used for feeding the data into the graph. */
        abuffer = avfilter_get_by_name("abuffer");
        if (!abuffer) {
            LOGE("Could not find the abuffer filter.\n");
            return AVERROR_FILTER_NOT_FOUND;
        }

        abuffer_ctx = avfilter_graph_alloc_filter(filter_graph, abuffer, "src");
        if (!abuffer_ctx) {
            LOGE("Could not allocate the abuffer instance.\n");
            return AVERROR(ENOMEM);
        }

        /* Set the filter options through the AVOptions API. */
        av_get_channel_layout_string(ch_layout, sizeof(ch_layout), 0, m_ullInputChannelLayout);
        av_opt_set(abuffer_ctx, "channel_layout", ch_layout, AV_OPT_SEARCH_CHILDREN);
        av_opt_set(abuffer_ctx, "sample_fmt", av_get_sample_fmt_name(m_eInputFormat),
                   AV_OPT_SEARCH_CHILDREN);
        av_opt_set_q(abuffer_ctx, "time_base", (AVRational) {1, m_nInputSampleRate},
                     AV_OPT_SEARCH_CHILDREN);
        av_opt_set_int(abuffer_ctx, "sample_rate", m_nInputSampleRate, AV_OPT_SEARCH_CHILDREN);

        /* Now initialize the filter; we pass NULL options, since we have already
         * set all the options above. */
        err = avfilter_init_str(abuffer_ctx, NULL);
        if (err < 0) {
            LOGE("Could not initialize the abuffer filter.\n");
            return err;
        }

        /* Create volume filter. */
        volume = avfilter_get_by_name("volume");
        if (!volume) {
            LOGE("Could not find the volume filter.\n");
            return AVERROR_FILTER_NOT_FOUND;
        }

        volume_ctx = avfilter_graph_alloc_filter(filter_graph, volume, "volume");
        if (!volume_ctx) {
            LOGE("Could not allocate the volume instance.\n");
            return AVERROR(ENOMEM);
        }

        /* A different way of passing the options is as key/value pairs in a
         * dictionary. */
        char tmp[6];
        sprintf(tmp, "%.4f", m_fOutVolume);
        av_dict_set(&options_dict, "volume", tmp, 0);
        err = avfilter_init_dict(volume_ctx, &options_dict);
        av_dict_free(&options_dict);
        if (err < 0) {
            LOGE("Could not initialize the volume filter.\n");
            return err;
        }

        /* Create the aformat filter;
         * it ensures that the output is of the format we want. */
        aformat = avfilter_get_by_name("aformat");
        if (!aformat) {
            LOGE("Could not find the aformat filter.\n");
            return AVERROR_FILTER_NOT_FOUND;
        }

        aformat_ctx = avfilter_graph_alloc_filter(filter_graph, aformat, "aformat");
        if (!aformat_ctx) {
            LOGE("Could not allocate the aformat instance.\n");
            return AVERROR(ENOMEM);
        }

        /* A third way of passing the options is in a string of the form
         * key1=value1:key2=value2.... */
        snprintf(options_str, sizeof(options_str),
                 "sample_fmts=%s:sample_rates=%d:channel_layouts=0x%llx",
                 av_get_sample_fmt_name(AV_SAMPLE_FMT_S16), AUDIO_SAMPLE_FREQUENCY,
                 (uint64_t) AV_CH_LAYOUT_STEREO);
        err = avfilter_init_str(aformat_ctx, options_str);
        if (err < 0) {
            LOGE("Could not initialize the aformat filter.\n");
            return err;
        }

        /* Finally create the abuffersink filter;
         * it will be used to get the filtered data out of the graph. */
        abuffersink = avfilter_get_by_name("abuffersink");
        if (!abuffersink) {
            LOGE("Could not find the abuffersink filter.\n");
            return AVERROR_FILTER_NOT_FOUND;
        }

        abuffersink_ctx = avfilter_graph_alloc_filter(filter_graph, abuffersink, "sink");
        if (!abuffersink_ctx) {
            LOGE("Could not allocate the abuffersink instance.\n");
            return AVERROR(ENOMEM);
        }

        /* This filter takes no options. */
        err = avfilter_init_str(abuffersink_ctx, NULL);
        if (err < 0) {
            LOGE("Could not initialize the abuffersink instance.\n");
            return err;
        }

        /* Connect the filters;
         * in this simple case the filters just form a linear chain. */
        err = avfilter_link(abuffer_ctx, 0, volume_ctx, 0);
        if (err >= 0)
            err = avfilter_link(volume_ctx, 0, aformat_ctx, 0);
        if (err >= 0)
            err = avfilter_link(aformat_ctx, 0, abuffersink_ctx, 0);
        if (err < 0) {
            LOGE("Error connecting filters\n");
            return err;
        }

        /* Configure the graph. */
        err = avfilter_graph_config(filter_graph, NULL);
        if (err < 0) {
            LOGE("Error configuring the filter graph\n");
            return err;
        }

        *graph = filter_graph;
        *src = abuffer_ctx;
        *sink = abuffersink_ctx;

        char *str = avfilter_graph_dump(filter_graph, NULL);
        LOGE("the filter dumped:\n%s", str);
        av_free(str);
        return 0;
    }

    void CAudioFilter::uninit() {
        if (&m_pGraph != NULL) {
            avfilter_graph_free(&m_pGraph);
        }
        if (&m_pFrame) {
            av_frame_free(&m_pFrame);
        }
        if (m_pOutBuffer != NULL) {
            av_free(m_pOutBuffer);
        }
    }

/* Construct a frame of audio data to be filtered;
 * this simple example just synthesizes a sine wave. */
    int CAudioFilter::process(u8 *in_buffer, int in_frame_size, u8 *&out_buffer, int &out_size) {
        int err;
        out_buffer = m_pOutBuffer;
        if (in_buffer == NULL || in_frame_size <= 0) {
            return -1;
        }
        err = process_input(in_buffer, in_frame_size);
        if (err < 0) {
            LOGE("Error generating input frame:");
            return err;
        }
        while ((err = av_buffersink_get_frame(m_pSink, m_pFrame)) >= 0) {
            /* now do something with our filtered frame */
            err = process_output(out_size, m_pFrame);
            if (err < 0) {
                LOGE("Error processing the filtered frame:");
                return err;
            }
            av_frame_unref(m_pFrame);
        }

        if (err == AVERROR(EAGAIN)) {
            /* Need to feed more frames in. */
//            LOGW("Need to feed more frames in.");
        } else if (err == AVERROR_EOF) {
            /* Nothing more to do, finish. */
//            LOGW("Nothing more to do, finish.");
        } else if (err < 0) {
            /* An error occurred. */
            LOGE("Error filtering the data:");
        }
        return err;
    }


    /* Construct a frame of audio data to be filtered;
     * this simple example just synthesizes a sine wave.
     */
    int CAudioFilter::process_input(u8 *buffer, int nframeSize) {
        int err;
        m_ullFramePTS += nframeSize;

        /* Set up the frame properties and allocate the buffer for the data. */
        m_pFrame->sample_rate = m_nInputSampleRate;
        m_pFrame->format = m_eInputFormat;
        m_pFrame->channel_layout = m_ullInputChannelLayout;
        m_pFrame->nb_samples = nframeSize;
        m_pFrame->pts = m_ullFramePTS;

        err = av_frame_get_buffer(m_pFrame, 0);
        if (err < 0) {
            m_ullFramePTS -= nframeSize;
            LOGE("Allocate new buffer(s) for audio failed.")
            return err;
        }
        int src_nb_channels = av_get_channel_layout_nb_channels(m_ullInputChannelLayout);
        int src_is_planar = av_sample_fmt_is_planar(m_eInputFormat);
        int nb_planes = src_is_planar ? src_nb_channels : 1;
        int src_bytes_per_sample = av_get_bytes_per_sample(m_eInputFormat);
        int src_bytes_per_planar =
                m_pFrame->nb_samples * src_bytes_per_sample * (src_is_planar ? 1 : src_nb_channels);

        /* Fill the data for each channel. */
        for (int i = 0; i < nb_planes; i++) {
            memcpy(m_pFrame->extended_data[i], buffer + (i * src_bytes_per_planar),
                   src_bytes_per_planar);
        }

        /* Send the frame to the input of the filtergraph. */
        err = av_buffersrc_add_frame(m_pSrc, m_pFrame);
        if (err < 0) {
            av_frame_unref(m_pFrame);
            LOGE("Error submitting the frame to the filtergraph:");
            return err;
        }
    }

    /* Do something useful with the filtered data: this simple
     * example just prints the MD5 checksum of each plane to stdout.
     */
    int CAudioFilter::process_output(s32 &size, AVFrame *frame) {
        int planar = av_sample_fmt_is_planar((AVSampleFormat) frame->format);
        int channels = av_get_channel_layout_nb_channels(frame->channel_layout);
        int planes = planar ? channels : 1;
        int bps = av_get_bytes_per_sample((AVSampleFormat) frame->format);
        int plane_size = bps * frame->nb_samples * (planar ? 1 : channels);
        for (int i = 0; i < planes; i++) {
            if (size + plane_size > MAX_OUTPUT_BUFFER_SIZE) {
                LOGE("Error the output buffer is too large.There is not enough memory");
                return -10000;
            }
            memcpy(m_pOutBuffer + (i * plane_size), frame->extended_data[i], plane_size);
            size += plane_size;
        }
        return 0;
    }

    AVSampleFormat CAudioFilter::shiftfmt(s32 format) {
        return (AVSampleFormat) format;
    }

    u64 CAudioFilter::shiftchnlyt(u64 nInputChannelLayout) {
        return nInputChannelLayout;
    }


}