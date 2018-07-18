//
// Created by ASUS on 2018/4/16.
//

#include <autolog.h>
#include "amix.h"


namespace paomiantv {

    CAMix *CAMix::Create(const CGraph *pGraph) {
        CAMix *ab = new CAMix;
        if (ab->init(pGraph) != 0) {
            delete ab;
            return NULL;
        }
        return ab;
    }

    void CAMix::destroy() {
        delete this;
    }

    CAMix::CAMix() : m_uInputCount(MIX_SOURCE_NUM) {
        USE_LOG;
        m_eType = EM_A_FILTER_MIX;
        /* Create mix filter. */
        m_ptFilter = avfilter_get_by_name("amix");
        if (!m_ptFilter) {
            LOGE("Could not find the mix filter.\n");
        }
    }

    CAMix::~CAMix() {
        USE_LOG;
    }

    s32 CAMix::init(const CGraph *pGraph) {
        CAutoLock autoLock(m_pLock);
        if (m_ptFilter == NULL) {
            LOGE("Could not find the mix filter.\n");
            return AVERROR_FILTER_NOT_FOUND;
        }
        snprintf(m_achName, MAX_LEN_FILTER_NAME, "amix%u", m_uId);
        m_ptContext = avfilter_graph_alloc_filter(pGraph->getGraph(), m_ptFilter, m_achName);
        if (!m_ptContext) {
            LOGE("Could not allocate the mix instance.\n");
            return AVERROR(ENOMEM);
        }
        return 0;
    }

#define ENABLE_WEIGHT 0
    s32
    CAMix::setParams(const u32 uInputCount, const s16 *wWeight) {
        CAutoLock autoLock(m_pLock);
        s32 err;
#if ENABLE_WEIGHT
        u32 length = uInputCount * sizeof(s16) + 32;
        s8 *inputs = new s8[length];
        snprintf(inputs, length, "inputs=%u:weights=", uInputCount);
        u32 tmpSize = strlen(inputs);
        s8 *tmp = inputs + tmpSize;
        for (int i = 0; i < uInputCount; i++) {
            snprintf(tmp, length - tmpSize, "%d ", wWeight[i]);
            tmpSize = strlen(tmp);
            tmp += tmpSize;
        }
        tmpSize = strlen(inputs);
        inputs[tmpSize - 1] = '\0';
        err = avfilter_init_str(m_ptContext, inputs);
        delete[] inputs;
#else
        s8 inputs[32];
        snprintf(inputs, sizeof(inputs), "inputs=%u", uInputCount);
        err = avfilter_init_str(m_ptContext, inputs);
#endif
        if (err < 0) {
            if (m_ptContext != NULL) {
                avfilter_free(m_ptContext);
                m_ptContext = NULL;
            }
            LOGE("Could not initialize the mix filter.\n");
            return err;
        }
        m_uInputCount = uInputCount;
        return 0;
    }

}

