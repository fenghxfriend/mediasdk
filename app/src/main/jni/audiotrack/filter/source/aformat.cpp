//
// Created by ASUS on 2018/4/16.
//

#include <enum.h>
#include "aformat.h"

namespace paomiantv {

    CAFormat::CAFormat() {
        USE_LOG;
        m_eType = EM_A_FILTER_FORMAT;
        /* Create the aformat filter;
         * it ensures that the output is of the format we want. */
        m_ptFilter = avfilter_get_by_name("aformat");
        if (!m_ptFilter) {
            LOGE("Could not find the aformat filter.\n");
        }
    }

    CAFormat::~CAFormat() {
        USE_LOG;
    }

    s32 CAFormat::init(const CGraph *pGraph) {
        CAutoLock autoLock(m_pLock);
        if (m_ptFilter == NULL) {
            LOGE("Could not find the aformat filter.\n");
            return AVERROR_FILTER_NOT_FOUND;
        }
        snprintf(m_achName, MAX_LEN_FILTER_NAME, "aformat%u", m_uId);
        m_ptContext = avfilter_graph_alloc_filter(pGraph->getGraph(), m_ptFilter, m_achName);
        if (!m_ptContext) {
            LOGE("Could not allocate the aformat instance.\n");
            return AVERROR(ENOMEM);
        }
        return 0;
    }

    CAFormat *CAFormat::Create(const CGraph *pGraph) {
        CAFormat *ab = new CAFormat;
        if (ab->init(pGraph) != 0) {
            delete ab;
            return NULL;
        }
        return ab;
    }

    void CAFormat::destroy() {
        delete this;
    }

    s32
    CAFormat::setParams(EMSampleRate eDstSampleRate, AVSampleFormat eDstFormat,
                        u64 ullDstChannelLayout) {
        CAutoLock autoLock(m_pLock);
        s32 err;
        s8 options_str[256];
        /* A third way of passing the options is in a string of the form
         * key1=value1:key2=value2.... */
        snprintf(options_str, sizeof(options_str),
                 "sample_fmts=%s:sample_rates=%d:channel_layouts=0x%llx",
                 av_get_sample_fmt_name(eDstFormat), eDstSampleRate,
                 ullDstChannelLayout);
        err = avfilter_init_str(m_ptContext, options_str);
        if (err < 0) {
            if (m_ptContext != NULL) {
                avfilter_free(m_ptContext);
                m_ptContext = NULL;
            }
            LOGE("Could not initialize the aformat filter.\n");
            return err;
        }
        return 0;
    }


}