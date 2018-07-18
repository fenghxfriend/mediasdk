//
// Created by ASUS on 2018/4/17.
//

#include <enum.h>
#include "asink.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavutil/opt.h>
#ifdef __cplusplus
}
#endif

namespace paomiantv {
    CASink::CASink() {
        USE_LOG;
        m_eType = EM_A_FILTER_SINK;
        /* Finally create the abuffersink filter;
         * it will be used to get the filtered task out of the graph. */
        m_ptFilter = avfilter_get_by_name("abuffersink");
        if (!m_ptFilter) {
            LOGE("Could not find the abuffersink filter.\n");
        }
    }

    CASink::~CASink() {
        USE_LOG;
    }

    s32 CASink::init(const CGraph *pGraph) {
        CAutoLock autoLock(m_pLock);
        if (m_ptFilter == NULL) {
            LOGE("Could not find the abuffersink filter.\n");
            return AVERROR_FILTER_NOT_FOUND;
        }

        snprintf(m_achName, MAX_LEN_FILTER_NAME, "sink%u", m_uId);
        m_ptContext = avfilter_graph_alloc_filter(pGraph->getGraph(), m_ptFilter, m_achName);
        if (!m_ptContext) {
            LOGE("Could not allocate the abuffersink instance.\n");
            return AVERROR(ENOMEM);
        }
        return 0;
    }

    CASink *CASink::Create(const CGraph *pGraph) {
        CASink *ab = new CASink;
        if (ab->init(pGraph) != 0) {
            delete ab;
            return NULL;
        }
        return ab;
    }

    s32 CASink::setParams(EMSampleRate eDstSampleRate, AVSampleFormat eDstFormat,
                          u64 ullDstChannelLayout) {
        CAutoLock autoLock(m_pLock);
        s32 err;
        const enum AVSampleFormat out_sample_fmts[] = {eDstFormat, AV_SAMPLE_FMT_NONE};
        const s64 out_channel_layouts[] = {(s64) ullDstChannelLayout, -1};
        const int out_sample_rates[] = {eDstSampleRate, -1};
        err = av_opt_set_int_list(m_ptContext, "sample_fmts", out_sample_fmts, -1,
                                  AV_OPT_SEARCH_CHILDREN);
        if (err < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output sample format\n");
        }

        err = av_opt_set_int_list(m_ptContext, "channel_layouts", out_channel_layouts, -1,
                                  AV_OPT_SEARCH_CHILDREN);
        if (err < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output channel layout\n");
        }

        err = av_opt_set_int_list(m_ptContext, "sample_rates", out_sample_rates, -1,
                                  AV_OPT_SEARCH_CHILDREN);
        if (err < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output sample rate\n");
        }
        if (err < 0) {
            if (m_ptContext != NULL) {
                avfilter_free(m_ptContext);
                m_ptContext = NULL;
            }
            LOGE("Could not initialize the abuffersink filter.\n");
            return err;
        }
        return 0;
    }

    void CASink::destroy() {
        delete this;
    }
}