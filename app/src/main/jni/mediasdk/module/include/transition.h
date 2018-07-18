/*******************************************************************************
 *        Module: mediasdk
 *          File:
 * Functionality: transition entity.
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-08-01  v1.0        huangxuefeng  created
 ******************************************************************************/

#ifndef _PAOMIANTV_TRANSITION_H_
#define _PAOMIANTV_TRANSITION_H_

#include <autolock.h>
#include <enum.h>
#include <constant.h>
#include "typedef.h"

namespace paomiantv {

    class CTransition {
    public:
        CTransition();

        virtual ~CTransition();

    public:
        void setStart(s64 sllStart);

        void setType(EMTransition emTransition);

        void setDuration(s64 sllDuration);

        inline EMTransition getType() const;

        inline s64 getStart() const;

        inline s64 getDuration() const;

        inline const u32 getId() const;

    private:
        ILock *m_pLock;

        static u32 m_sCount;
        // transition id
        const u32 m_uId;

        EMTransition m_eTransType;

        s64 m_sllStart;
        s64 m_sllDuration;
    };

    inline EMTransition CTransition::getType() const {
        return m_eTransType;
    }

    inline s64 CTransition::getStart() const {
        return m_sllStart;
    }

    inline s64 CTransition::getDuration() const {
        return m_sllDuration;
    }

    inline const u32 CTransition::getId() const {
        return m_uId;
    }

} // namespace paomiantv

#endif // _PAOMIANTV_TRANSITION_H_