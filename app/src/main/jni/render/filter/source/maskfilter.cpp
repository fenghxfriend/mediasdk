//
// Created by ASUS on 2018/6/21.
//

#include <cmath>
#include "maskfilter.h"

namespace paomiantv {

    CMaskFilter::CMaskFilter() : m_pMaskTexture(NULL) {
        m_eFilterType = EM_V_FILTER_MASK;
    }

    CMaskFilter::~CMaskFilter() {
        if (m_pMaskTexture != NULL) {
            delete m_pMaskTexture;
            m_pMaskTexture = NULL;
        }
    }

    BOOL32 CMaskFilter::init(EMPixelFormat eSourceFMT) {

        BOOL32 re = FALSE;
        CAutoLock autoLock(m_pLock);
        switch (eSourceFMT) {
            case EM_PIXEL_FORMAT_RGBA_8888: {
                if (configure(MASK_RGBA_FRAGMENT_SHADER, DEFAULT_VERTEXT_SHADER)) {
                    m_pProgram->enable();
                    glUniform1i(m_pProgram->getUniform("u_texture"), 0);
                    glUniform1i(m_pProgram->getUniform("u_maskMap"), 1);
                    m_pProgram->disable();
                    re = TRUE;
                }
            }
                break;
            case EM_PIXEL_FORMAT_I420: {
                if (configure(MASK_YUV_FRAGMENT_SHADER, DEFAULT_VERTEXT_SHADER)) {
                    m_pProgram->enable();
                    glUniform1i(m_pProgram->getUniform("u_yTexture"), 0);
                    glUniform1i(m_pProgram->getUniform("u_uTexture"), 1);
                    glUniform1i(m_pProgram->getUniform("u_vTexture"), 2);
                    glUniform1i(m_pProgram->getUniform("u_maskMap"), 3);
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

    void CMaskFilter::changeOutputSize(u32 uWidth, u32 uHeight) {
        CAutoLock autoLock(m_pLock);
        CVFilter::changeOutputSize(uWidth, uHeight);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    BOOL32
    CMaskFilter::fillMaskTexture(u8 *pbyMask, u32 uWidth, u32 uHeight, EMPixelFormat format) {
        BOOL32 re = TRUE;
        CAutoLock autoLock(m_pLock);
        switch (format) {
//            case EM_PIXEL_FORMAT_RGB: {
//                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, uWidth, uHeight,
//                             0, GL_RGB, GL_UNSIGNED_BYTE, pbyMap);
//            }
//                break;
            case EM_PIXEL_FORMAT_RGBA_8888: {
                if (m_pMaskTexture == NULL) {
                    m_pMaskTexture = new CTexture;
                }
                glBindTexture(GL_TEXTURE_2D, m_pMaskTexture->getTextureId());
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, uWidth, uHeight,
                             0, GL_RGBA, GL_UNSIGNED_BYTE, pbyMask);
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
    CMaskFilter::draw(CTexture *pTexture, const float *pPositionBuffer, s32 uPPointSize,
                      const float *pUVBuffer, s32 uUVPointSize) {
        CAutoLock autoLock(m_pLock);
        GLuint uDefaultFrameBuffer = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &uDefaultFrameBuffer);
        if (pUVBuffer == CVFilter::VFLIPUV) {
            setFlipAnchor(0.5f * getUVCropScale(EM_DIRECT_X) *
                          ((float) cos(getUVCropRotation(EM_DIRECT_X))) +
                          getUVCropTranslate(EM_DIRECT_X), 0.0f);
            if (uDefaultFrameBuffer != 0) {
                pUVBuffer = CVFilter::NORMALUV;
            }
        } else {
            setFlipAnchor(0.0f, 0.0f);
        }


        if (m_pMaskTexture != NULL) {
            switch (pTexture->getFormat()) {
                case EM_PIXEL_FORMAT_RGBA_8888: {
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, m_pMaskTexture->getTextureId());
                }
                    break;
                case EM_PIXEL_FORMAT_I420: {
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, m_pMaskTexture->getTextureId());
                }
                    break;
                default:
                    break;
            }
            m_pProgram->enable();
            glUniform2fv(m_pProgram->getUniform("u_maskFlipAnchor"), 1, m_afAnchorVec);
            glUniform2fv(m_pProgram->getUniform("u_maskFlip"), 1, m_afFilpVec);
            m_pProgram->disable();
            CVFilter::draw(pTexture, pPositionBuffer, uPPointSize, pUVBuffer, uUVPointSize);
        }
    }

    void CMaskFilter::setFlipAnchor(const float fAnchorX, const float fAnchorY) {
        CAutoLock autoLock(m_pLock);
        if (floor(fabs(fAnchorX * 100)) > 0) {
            m_afAnchorVec[1] = fAnchorX;
            m_afFilpVec[1] = 1.0f;
        } else {
            m_afAnchorVec[1] = 0.0f;
            m_afFilpVec[1] = -1.0f;
        }
        if (floor(fabs(fAnchorY * 100)) > 0) {
            m_afAnchorVec[0] = fAnchorY;
            m_afFilpVec[0] = 1.0f;
        } else {
            m_afAnchorVec[0] = 0.0f;
            m_afFilpVec[0] = -1.0f;
        }

    }
}