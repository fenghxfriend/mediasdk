//
// Created by ASUS on 2018/4/17.
//

#include "afilter.h"

namespace paomiantv {
    u32 IAFilter::m_sCount = 1;

    IAFilter::IAFilter() : m_uId(m_sCount),
                           m_ptFilter(NULL),
                           m_ptContext(NULL),
                           m_eType(EM_A_FILTER_START) {
        USE_LOG;
        m_pLock = new CLock();
        if (m_sCount == UNSIGNED_INTEGER32_MAX_VALUE) {
            LOGE("the id is overflowed! please restart app!");
        } else {
            m_sCount++;
        }
        memset(m_achName, 0, MAX_LEN_FILTER_NAME);
    }

    IAFilter::~IAFilter() {
        USE_LOG;
        uninit();
        if (m_pLock != NULL) {
            delete m_pLock;
        }
    }

    void IAFilter::uninit() {
        CAutoLock autoLock(m_pLock);
        if (m_ptContext != NULL) {
            avfilter_free(m_ptContext);
            m_ptContext = NULL;
        }
    }

    void IAFilter::destroy() {
        delete this;
    }
}
