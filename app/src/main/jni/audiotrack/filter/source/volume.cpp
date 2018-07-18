//
// Created by ASUS on 2018/4/16.
//

#include "volume.h"

namespace paomiantv {

    CVolume::CVolume() {
        USE_LOG;
        m_eType = EM_A_FILTER_VOLUME;
        /* Create volume filter. */
        m_ptFilter = avfilter_get_by_name("volume");
        if (!m_ptFilter) {
            LOGE("Could not find the volume filter.\n");
        }
    }

    CVolume::~CVolume() {
        USE_LOG;

    }

    s32 CVolume::init(const CGraph *pGraph) {
        CAutoLock autoLock(m_pLock);
        if (m_ptFilter == NULL) {
            LOGE("Could not find the volume filter.\n");
            return AVERROR_FILTER_NOT_FOUND;
        }
        snprintf(m_achName, MAX_LEN_FILTER_NAME, "volume%u", m_uId);
        m_ptContext = avfilter_graph_alloc_filter(pGraph->getGraph(), m_ptFilter, m_achName);
        if (!m_ptContext) {
            LOGE("Could not allocate the volume instance.\n");
            return AVERROR(ENOMEM);
        }
        return 0;
    }

    CVolume *CVolume::Create(const CGraph *pGraph) {
        CVolume *ab = new CVolume;
        if (ab->init(pGraph) != 0) {
            delete ab;
            return NULL;
        }
        return ab;
    }

    void CVolume::destroy() {
        delete this;
    }

    s32
    CVolume::setParams(float fVolume) {
        CAutoLock autoLock(m_pLock);
        s32 err;

        AVDictionary *options_dict = NULL;
        /* A different way of passing the options is as key/value pairs in a
         * dictionary. */

        char tmp[6];
        sprintf(tmp, "%.4f", fVolume);

        av_dict_set(&options_dict, "volume", tmp, 0);
        err = avfilter_init_dict(m_ptContext, &options_dict);
        av_dict_free(&options_dict);
        if (err < 0) {
            if (m_ptContext != NULL) {
                avfilter_free(m_ptContext);
                m_ptContext = NULL;
            }
            LOGE("Could not initialize the volume filter.\n");
            return err;
        }
        return 0;
    }

}