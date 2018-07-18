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
#include <constant.h>
#include "animation.h"

namespace paomiantv {

    u32 CAnimation::m_sCount = 1;

    CAnimation::CAnimation() :
            m_uId(m_sCount) {
        USE_LOG;
        m_fStartTransX = 0.0f;
        m_fStartTransY = 0.0f;
        m_fStartTransZ = 0.0f;
        m_fStartDegreeX = 0.0f;
        m_fStartDegreeY = 0.0f;
        m_fStartDegreeZ = 0.0f;
        m_fStartScaleX = 1.0f;
        m_fStartScaleY = 1.0f;
        m_fStartScaleZ = 1.0f;

        m_fEndTransX = 0.0f;
        m_fEndTransY = 0.0f;
        m_fEndTransZ = 0.0f;
        m_fEndDegreeX = 0.0f;
        m_fEndDegreeY = 0.0f;
        m_fEndDegreeZ = 0.0f;
        m_fEndScaleX = 1.0f;
        m_fEndScaleY = 1.0f;
        m_fEndScaleZ = 1.0f;


        m_fCropStartTransX = 0.0f;
        m_fCropStartTransY = 0.0f;
        m_fCropStartTransZ = 0.0f;
        m_fCropStartDegreeX = 0.0f;
        m_fCropStartDegreeY = 0.0f;
        m_fCropStartDegreeZ = 0.0f;
        m_fCropStartScaleX = 1.0f;
        m_fCropStartScaleY = 1.0f;
        m_fCropStartScaleZ = 1.0f;

        m_fCropEndTransX = 0.0f;
        m_fCropEndTransY = 0.0f;
        m_fCropEndTransZ = 0.0f;
        m_fCropEndDegreeX = 0.0f;
        m_fCropEndDegreeY = 0.0f;
        m_fCropEndDegreeZ = 0.0f;
        m_fCropEndScaleX = 1.0f;
        m_fCropEndScaleY = 1.0f;
        m_fCropEndScaleZ = 1.0f;


        m_fStartAlpha = 1.0f;
        m_fEndAlpha = 1.0f;

        m_sllStart = 0;
        m_sllDuration = -1;
        if (m_sCount == UNSIGNED_INTEGER32_MAX_VALUE) {
            LOGE("the id is overflowed! please restart app!");
        } else {
            m_sCount++;
        }
    }

    CAnimation::~CAnimation() {
        USE_LOG;
    }

    float CAnimation::getVecX() {
        return m_sllDuration > 0 ? (m_fEndTransX - m_fStartTransX) / m_sllDuration : 0;
    }

    float CAnimation::getVecY() {
        return m_sllDuration > 0 ? (m_fEndTransY - m_fStartTransY) / m_sllDuration : 0;
    }

    float CAnimation::getVecZ() {
        return m_sllDuration > 0 ? (m_fEndTransZ - m_fStartTransZ) / m_sllDuration : 0;
    }

    float CAnimation::getVecScaleX() {
        return m_sllDuration > 0 ? (m_fEndScaleX - m_fStartScaleX) / m_sllDuration : 0;
    }

    float CAnimation::getVecScaleY() {
        return m_sllDuration > 0 ? (m_fEndScaleY - m_fStartScaleY) / m_sllDuration : 0;
    }

    float CAnimation::getVecScaleZ() {
        return m_sllDuration > 0 ? (m_fEndScaleZ - m_fStartScaleZ) / m_sllDuration : 0;
    }

    float CAnimation::getVecDegreeX() {
        return m_sllDuration > 0 ? (m_fEndDegreeX - m_fStartDegreeX) / m_sllDuration : 0;
    }

    float CAnimation::getVecDegreeY() {
        return m_sllDuration > 0 ? (m_fEndDegreeY - m_fStartDegreeY) / m_sllDuration : 0;
    }

    float CAnimation::getVecDegreeZ() {
        return m_sllDuration > 0 ? (m_fEndDegreeZ - m_fStartDegreeZ) / m_sllDuration : 0;
    }


    float CAnimation::getCropVecX() {
        return m_sllDuration > 0 ? (m_fCropEndTransX - m_fCropStartTransX) / m_sllDuration : 0;
    }

    float CAnimation::getCropVecY() {
        return m_sllDuration > 0 ? (m_fCropEndTransY - m_fCropStartTransY) / m_sllDuration : 0;
    }

    float CAnimation::getCropVecZ() {
        return m_sllDuration > 0 ? (m_fCropEndTransZ - m_fCropStartTransZ) / m_sllDuration : 0;
    }

    float CAnimation::getCropVecScaleX() {
        return m_sllDuration > 0 ? (m_fCropEndScaleX - m_fCropStartScaleX) / m_sllDuration : 0;
    }

    float CAnimation::getCropVecScaleY() {
        return m_sllDuration > 0 ? (m_fCropEndScaleY - m_fCropStartScaleY) / m_sllDuration : 0;
    }

    float CAnimation::getCropVecScaleZ() {
        return m_sllDuration > 0 ? (m_fCropEndScaleZ - m_fCropStartScaleZ) / m_sllDuration : 0;
    }

    float CAnimation::getCropVecDegreeX() {
        return m_sllDuration > 0 ? (m_fCropEndDegreeX - m_fCropStartDegreeX) / m_sllDuration : 0;
    }

    float CAnimation::getCropVecDegreeY() {
        return m_sllDuration > 0 ? (m_fCropEndDegreeY - m_fCropStartDegreeY) / m_sllDuration : 0;
    }

    float CAnimation::getCropVecDegreeZ() {
        return m_sllDuration > 0 ? (m_fCropEndDegreeZ - m_fCropStartDegreeZ) / m_sllDuration : 0;
    }


    float CAnimation::getVecAlpha() {
        return m_sllDuration > 0 ? (m_fEndAlpha - m_fStartAlpha) / m_sllDuration : 0;
    }


} // namespace paomiantv
