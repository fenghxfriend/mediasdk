/*******************************************************************************
 *        Module: paomiantv
 *          File: transition.cpp
 * Functionality: transition entity.
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-20  v1.0        huangxuefeng  created
 ******************************************************************************/
#include <stdlib.h>
#include <autolog.h>
#include "transition.h"

namespace paomiantv {

    u32 CTransition::m_sCount = 1;

    CTransition::CTransition() :
            m_uId(m_sCount),
            m_eTransType(),
            m_sllDuration(0),
            m_sllStart(0) {
        USE_LOG;
        m_pLock = new CLock;
        if (m_sCount == UNSIGNED_INTEGER32_MAX_VALUE) {
            LOGE("the id is overflowed! please restart app!");
        } else {
            m_sCount++;
        }
    }

    CTransition::~CTransition() {
        USE_LOG;
        if (m_pLock != NULL) {
            delete m_pLock;
        }
    }

    void CTransition::setType(EMTransition emTransition) {
        m_eTransType = emTransition;
    }

    void CTransition::setStart(s64 sllStart) {
        m_sllStart = sllStart;
    }

    void CTransition::setDuration(s64 sllDuration) {
        m_sllDuration = sllDuration;
    }

} // namespace paomiantv
