/*******************************************************************************
 *        Module: mediasdk
 *          File: lutfilter.cpp
 * Functionality: LUT filter entity. Color-Lookup-Table
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2018-01-18  v1.0        huangxuefeng  created
 ******************************************************************************/

#include "vfilter.h"
#include "lutfilter.h"

namespace paomiantv {

    CLUTFilter::CLUTFilter() : m_pMapTexture(NULL) {
        m_eFilterType = EM_V_FILTER_LUT;
    }

    CLUTFilter::~CLUTFilter() {
        if (m_pMapTexture != 0) {
            delete m_pMapTexture;
            m_pMapTexture = NULL;
        }
    }

    BOOL32 CLUTFilter::init(EMPixelFormat eSourceFMT) {

        BOOL32 re = FALSE;
        CAutoLock autoLock(m_pLock);
        switch (eSourceFMT) {
            case EM_PIXEL_FORMAT_RGBA_8888: {
                if (configure(LUT_RGBA_FRAGMENT_SHADER, DEFAULT_VERTEXT_SHADER)) {
                    m_pProgram->enable();
                    glUniform1i(m_pProgram->getUniform("u_texture"), 0);
                    glUniform1i(m_pProgram->getUniform("u_toneMap"), 1);
                    m_pProgram->disable();
                    re = TRUE;
                }
            }
                break;
            case EM_PIXEL_FORMAT_I420: {
                if (configure(LUT_YUV_FRAGMENT_SHADER, DEFAULT_VERTEXT_SHADER)) {
                    m_pProgram->enable();
                    glUniform1i(m_pProgram->getUniform("u_yTexture"), 0);
                    glUniform1i(m_pProgram->getUniform("u_uTexture"), 1);
                    glUniform1i(m_pProgram->getUniform("u_vTexture"), 2);
                    glUniform1i(m_pProgram->getUniform("u_toneMap"), 3);
                    m_pProgram->disable();
                    re = TRUE;
                }
            }
                break;
            default:
                LOGE("source picture format is invalid,neither rgba nor yuv420p!")
                break;
        }
        return re;
    }

    void CLUTFilter::changeOutputSize(u32 uWidth, u32 uHeight) {
        CAutoLock autoLock(m_pLock);
        CVFilter::changeOutputSize(uWidth, uHeight);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    BOOL32 CLUTFilter::fillMapTexture(u8 *pbyMap, u32 uWidth, u32 uHeight, EMPixelFormat format) {
        BOOL32 re = TRUE;
        CAutoLock autoLock(m_pLock);
        switch (format) {
//            case EM_PIXEL_FORMAT_RGB: {
//                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, uWidth, uHeight,
//                             0, GL_RGB, GL_UNSIGNED_BYTE, pbyMap);
//            }
//                break;
            case EM_PIXEL_FORMAT_RGBA_8888: {
                if (m_pMapTexture == NULL) {
                    m_pMapTexture = new CTexture;
                }
                glBindTexture(GL_TEXTURE_2D, m_pMapTexture->getTextureId());
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, uWidth, uHeight,
                             0, GL_RGBA, GL_UNSIGNED_BYTE, pbyMap);
            }
                break;
            default: {
                LOGE("mapper pixel format is wrong, pixel of mapper have to be RGBA or RGB!")
//                    u8 *pbyRGBA = (u8 *) malloc(uWidth * uHeight * 4);
//                    libyuv::ConvertFromI420(pbyMap, uWidth,
//                                            pbyMap + uWidth * uHeight,
//                                            (uWidth + 1) / 2,
//                                            pbyMap + uWidth * uHeight * 5 / 4,
//                                            (uWidth + 1) / 2, pbyRGBA, 0, uWidth, uHeight,
//                                            libyuv::FOURCC_RGBA);
//                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, uWidth, uHeight,
//                                 0, GL_RGBA, GL_UNSIGNED_BYTE, pbyRGBA);
//                    free(pbyRGBA);
                re = FALSE;
            }
                break;
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        return re;
    }

    void
    CLUTFilter::draw(CTexture *pTexture, const float *pPositionBuffer, s32 uPPointSize,
                     const float *pUVBuffer, s32 uUVPointSize) {
        CAutoLock autoLock(m_pLock);
        if (m_pMapTexture != NULL) {
            switch (pTexture->getFormat()) {
                case EM_PIXEL_FORMAT_RGBA_8888: {
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, m_pMapTexture->getTextureId());
                }
                    break;
                case EM_PIXEL_FORMAT_I420: {
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, m_pMapTexture->getTextureId());
                }
                    break;
                default:
                    break;
            }
            CVFilter::draw(pTexture, pPositionBuffer, uPPointSize, pUVBuffer, uUVPointSize);
        }
    }
}