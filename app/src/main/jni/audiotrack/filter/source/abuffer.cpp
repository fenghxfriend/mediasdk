//
// Created by ASUS on 2018/4/17.
//

#include <abuffer.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#ifdef __cplusplus
}
#endif

namespace paomiantv {

    CABuffer::CABuffer() :
            m_wWeight(1),
            m_eSrcSampleRate(EM_SAMPLE_RATE_START),
            m_eSrcFormat(AV_SAMPLE_FMT_NONE),
            m_ullSrcChannelLayout(0) {
        USE_LOG;
        m_eType = EM_A_FILTER_SOURCE;
        /* Create the abuffer filter;
                * it will be used for feeding the task into the graph. */
        m_ptFilter = avfilter_get_by_name("abuffer");
        if (!m_ptFilter) {
            LOGE("Could not find the abuffer filter.\n");
        }
    }

    CABuffer::~CABuffer() {
        USE_LOG;
    }

    s32 CABuffer::init(const CGraph *pGraph) {
        CAutoLock autoLock(m_pLock);
        if (m_ptFilter == NULL) {
            LOGE("Could not find the abuffer filter.\n");
            return AVERROR_FILTER_NOT_FOUND;
        }
        snprintf(m_achName, MAX_LEN_FILTER_NAME, "src%u", m_uId);
        m_ptContext = avfilter_graph_alloc_filter(pGraph->getGraph(), m_ptFilter, m_achName);
        if (!m_ptContext) {
            LOGE("Could not allocate the abuffer instance.\n");
            return AVERROR(ENOMEM);
        }
        return 0;
    }

    CABuffer *CABuffer::Create(const CGraph *pGraph) {
        CABuffer *p = new CABuffer;
        if (p->init(pGraph) != 0) {
            delete p;
            return NULL;
        }
        return p;
    }

    void CABuffer::destroy() {
        delete this;
    }

    s32
    CABuffer::setParams(EMSampleRate uSrcSampleRate, AVSampleFormat eSrcFormat,
                        u64 ullSrcChannelLayout, s16 wWeight) {
        CAutoLock autoLock(m_pLock);
        s32 err;
        s8 ch_layout[64];
        /* Set the filter options through the AVOptions API. */
        av_get_channel_layout_string(ch_layout, sizeof(ch_layout), 0, ullSrcChannelLayout);
        av_opt_set(m_ptContext, "channel_layout", ch_layout, AV_OPT_SEARCH_CHILDREN);
        av_opt_set(m_ptContext, "sample_fmt", av_get_sample_fmt_name(eSrcFormat),
                   AV_OPT_SEARCH_CHILDREN);
        av_opt_set_q(m_ptContext, "time_base", (AVRational) {1, uSrcSampleRate},
                     AV_OPT_SEARCH_CHILDREN);
        av_opt_set_int(m_ptContext, "sample_rate", uSrcSampleRate, AV_OPT_SEARCH_CHILDREN);

        /* Now initialize the filter; we pass NULL options, since we have already
         * set all the options above. */
        err = avfilter_init_str(m_ptContext, NULL);
        if (err < 0) {
            if (m_ptContext != NULL) {
                avfilter_free(m_ptContext);
                m_ptContext = NULL;
            }
            LOGE("Could not initialize the abuffer filter.\n");
            return err;
        }
        m_eSrcSampleRate = uSrcSampleRate;
        m_eSrcFormat = eSrcFormat;
        m_ullSrcChannelLayout = ullSrcChannelLayout;
        m_wWeight = wWeight;
        return 0;
    }

}