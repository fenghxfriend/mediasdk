/*******************************************************************************
 *        Module: mediasdk
 *          File: filter.cpp
 * Functionality: filter entity.
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
#include "effect.h"
#include <libyuv.h>
#include <constant.h>

namespace paomiantv {
    u32 CEffect::m_sCount = 1;

    CEffect::CEffect()
            : m_uId(m_sCount),
              m_eType(EM_EFFECT_START),
              m_sllStart(0),
              m_sllDuration(-1) {
        USE_LOG;
        m_pLock = new CLock;
        m_pPicture = CPicture::create();
        if (m_sCount == UNSIGNED_INTEGER32_MAX_VALUE) {
            LOGE("the id is overflowed! please restart app!");
        } else {
            m_sCount++;
        }
    }

    CEffect::~CEffect() {
        USE_LOG;
        uninit();
        if (m_pPicture != NULL) {
            CPicture::release(m_pPicture);
            m_pPicture = NULL;
        }
        if (m_pLock != NULL) {
            delete m_pLock;
            m_pLock = NULL;
        }
    }


    void
    CEffect::init(EMEffect nType, u8 *pbyPicture, EMPixelFormat emPixelFormat, u32 uWidth,
                  u32 uHeight, s64 sllStart,
                  s64 sllDuration) {
        CAutoLock autoLock(m_pLock);
        update(nType, pbyPicture, emPixelFormat, uWidth, uHeight);
        m_sllStart = sllStart;
        m_sllDuration = sllDuration;
    }

    void CEffect::uninit() {
        CAutoLock autoLock(m_pLock);
        m_sllStart = 0;
        m_sllDuration = -1;
    }

    void CEffect::update(EMEffect nType, u8 *pbyPicture, EMPixelFormat emPixelFormat, u32 uWidth,
                         u32 uHeight) {
        CAutoLock autoLock(m_pLock);
        if (nType > EM_EFFECT_START && nType < EM_EFFECT_END) {
            m_eType = (EMEffect) nType;
            if (pbyPicture != NULL && uHeight > 0 && uWidth > 0 &&
                emPixelFormat > EM_PIXEL_FORMAT_START && emPixelFormat < EM_PIXEL_FORMAT_END) {
                u32 size = uHeight * uWidth * 4;
                m_pPicture->resize(size);
                switch (emPixelFormat) {
                    case EM_PIXEL_FORMAT_RGBA_8888: {
                        memcpy(m_pPicture->m_pbyPicture, pbyPicture, size);
                    }
                        break;
                    case EM_PIXEL_FORMAT_I420: {
                        libyuv::I420ToRGBA(pbyPicture, uWidth,
                                           pbyPicture + uWidth * uHeight, (uWidth + 1) / 2,
                                           pbyPicture + uWidth * uHeight * 5 / 4, (uWidth + 1) / 2,
                                           m_pPicture->m_pbyPicture, 4 * uWidth, uWidth, uHeight);
                    }
                        break;
                    case EM_PIXEL_FORMAT_ARGB_8888: {
                        libyuv::ARGBToRGBA(pbyPicture, 4 * uWidth, m_pPicture->m_pbyPicture,
                                           4 * uWidth, uWidth,
                                           uHeight);
                    }
                        break;
                    case EM_PIXEL_FORMAT_ARGB_4444: {
                        u8 *tmp = new u8[size];
                        libyuv::ARGB4444ToARGB(pbyPicture, 2 * uWidth, tmp, 4 * uWidth, uWidth,
                                               uHeight);
                        libyuv::ARGBToRGBA(tmp, 4 * uWidth, m_pPicture->m_pbyPicture, 4 * uWidth,
                                           uWidth,
                                           uHeight);
                        delete[] tmp;
                    }
                        break;
                    case EM_PIXEL_FORMAT_RGB_565: {
                        u8 *tmp = new u8[size];
                        libyuv::RGB565ToARGB(pbyPicture, 2 * uWidth, tmp, 4 * uWidth, uWidth,
                                             uHeight);
                        libyuv::ARGBToRGBA(tmp, 4 * uWidth, m_pPicture->m_pbyPicture, 4 * uWidth,
                                           uWidth,
                                           uHeight);
                        delete[] tmp;
                    }
                        break;
                    default: {
                        LOGE("Bitmap format is invalid !");
                    }
                        break;
                }
                m_pPicture->m_uSize = size;
                m_pPicture->m_uHeight = uHeight;
                m_pPicture->m_uWidth = uWidth;
            }
        }
    }

    void CEffect::setStart(s64 sllStart) {
        CAutoLock autoLock(m_pLock);
        if (m_sllStart != sllStart) {
            m_sllStart = sllStart;
        }
    }

    void CEffect::setDuration(s64 sllDuration) {
        CAutoLock autoLock(m_pLock);
        if (m_sllDuration != sllDuration) {
            m_sllDuration = sllDuration;
        }
    }

    void CEffect::parse() {
    }

    EMEffect CEffect::getType() {
        CAutoLock autoLock(m_pLock);
        return m_eType;
    }

    s64 CEffect::getStart() {
        CAutoLock autoLock(m_pLock);
        return m_sllStart;
    }

    s64 CEffect::getDuration() {
        CAutoLock autoLock(m_pLock);
        return m_sllDuration;
    }

    void CEffect::getFilter(CPicture *&p, EMVFilter &eType) {
        p = NULL;
        eType = EM_V_FILTER_START;
        CAutoLock autoLock(m_pLock);
        switch (m_eType) {
            case EM_EFFECT_BLUR:
                eType = EM_V_FILTER_BLUR;
                p = m_pPicture->clone();
                break;
            case EM_EFFECT_TRANSFORM_COLOR:
                eType = EM_V_FILTER_LUT;
                p = m_pPicture->clone();
                break;
            case EM_EFFECT_MASK:
                eType = EM_V_FILTER_MASK;
                p = m_pPicture->clone();
                break;
            default:
                LOGE("effect is invalid");
                break;
        }
    }

} // namespace paomiantv
