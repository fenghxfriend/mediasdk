//
// Created by ASUS on 2018/1/18.
//

#include "blurfilter.h"

namespace paomiantv {

    CBlurFilter::CBlurFilter() : m_uItNumber(DEFAULT_ITERATE_TIME),
                                 m_fBlurOffset(DEFAULT_BLUR_OFFSET) {
        m_eFilterType = EM_V_FILTER_BLUR;
        memset(m_auFrameBuffers, 0, sizeof(m_auFrameBuffers));
        memset(m_pcFrameBufferTextures, 0, sizeof(m_pcFrameBufferTextures));
    }

    CBlurFilter::~CBlurFilter() {
        destroyFrameBuffers();
    }

    BOOL32 CBlurFilter::init(EMPixelFormat eSourceFMT) {
        BOOL32 re = FALSE;
        CAutoLock autoLock(m_pLock);
        switch (eSourceFMT) {
            case EM_PIXEL_FORMAT_RGBA_8888: {
                if (configure(GAUSSIAN_BLUR_RGBA_FRAGMENT_SHADER, GAUSSIAN_BLUR_VERTEX_SHADER)) {
                    m_pProgram->enable();
                    glUniform1i(m_pProgram->getUniform("u_texture"), 0);
                    m_pProgram->disable();
                    re = TRUE;
                }
            }
                break;
            case EM_PIXEL_FORMAT_I420: {
                if (configure(GAUSSIAN_BLUR_YUV_FRAGMENT_SHADER, GAUSSIAN_BLUR_VERTEX_SHADER)) {
                    m_pProgram->enable();
                    glUniform1i(m_pProgram->getUniform("u_yTexture"), 0);
                    glUniform1i(m_pProgram->getUniform("u_uTexture"), 1);
                    glUniform1i(m_pProgram->getUniform("u_vTexture"), 2);
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

    void CBlurFilter::addFrameBuffers(GLuint &uFrameBuffer, CTexture *&pcFrameBufferTexture) {
        if (m_uOutputHeight == 0 || m_uOutputWidth == 0) {
            return;
        }
        deleteFrameBuffers(uFrameBuffer, pcFrameBufferTexture);
        pcFrameBufferTexture = new CTexture;
        pcFrameBufferTexture->fillTexture(NULL, m_uOutputWidth, m_uOutputHeight);
        glGenFramebuffers(1, &uFrameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, uFrameBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, pcFrameBufferTexture->getTextureId(), 0);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            LOGE("add frame buffer failed!");
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void CBlurFilter::deleteFrameBuffers(GLuint &uFrameBuffer, CTexture *&pcFrameBufferTexture) {
        if (pcFrameBufferTexture != NULL) {
            delete pcFrameBufferTexture;
            pcFrameBufferTexture = NULL;
        }
        if (uFrameBuffer != 0) {
            glDeleteFramebuffers(1, &uFrameBuffer);
            uFrameBuffer = 0;
        }
    }


    void CBlurFilter::setBlurOffset(const float fBlurOffset) {
        CAutoLock autoLock(m_pLock);
        m_fBlurOffset = fBlurOffset;
    }

    BOOL32 CBlurFilter::setItNumber(u32 uItNumber) {
        CAutoLock autoLock(m_pLock);
        if (uItNumber > MAX_ITERATE_TIME || uItNumber == m_uItNumber ||
            m_uOutputWidth == 0 || m_uOutputHeight == 0) {
            return FALSE;
        }
        int count = 2 * (m_uItNumber - uItNumber);
        int offset = 0;
        if (count > 0) {
            offset = 2 * uItNumber - 1;
            for (int i = offset; i < 2 * m_uItNumber - 1; i++) {
                deleteFrameBuffers(m_auFrameBuffers[i], m_pcFrameBufferTextures[i]);
            }
        } else {
            offset = 2 * m_uItNumber - 1;
            for (int i = offset; i < 2 * uItNumber - 1; i++) {
                addFrameBuffers(m_auFrameBuffers[i], m_pcFrameBufferTextures[i]);
            }
        }

        m_uItNumber = uItNumber;
        return TRUE;
    }

    void CBlurFilter::changeOutputSize(u32 nWidth, u32 nHeight) {
        CAutoLock autoLock(m_pLock);
        if (m_uOutputWidth != nWidth || m_uOutputWidth != nHeight) {
            CVFilter::changeOutputSize(nWidth, nHeight);
            if (m_uItNumber > 0) {
                int size = 2 * m_uItNumber;
                for (u32 i = 0; i < size - 1; i++) {
                    if (m_pcFrameBufferTextures[i] == NULL || m_auFrameBuffers[i] == 0) {
                        addFrameBuffers(m_auFrameBuffers[i], m_pcFrameBufferTextures[i]);
                    }
                    if (m_pcFrameBufferTextures[i] != NULL) {
                        m_pcFrameBufferTextures[i]->fillTexture(NULL, nWidth, nHeight);
                    }
                }
            }
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void
    CBlurFilter::draw(CTexture *pTexture, const float *pPositionBuffer, s32 uPPointSize,
                      const float *pUVBuffer, s32 uUVPointSize) {
        CAutoLock autoLock(m_pLock);
        GLuint uDefaultFrameBuffer = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &uDefaultFrameBuffer);
        CTexture *previousTexture = pTexture;
        int size = 2 * m_uItNumber;
        bool bFlip = (pUVBuffer == VFLIPUV);
        for (int i = 0; i < size; i++) {
            bool isNotLast = i < size - 1;

            if (isNotLast) {
                glBindFramebuffer(GL_FRAMEBUFFER, m_auFrameBuffers[i]);
                glClearColor(0, 0, 0, 0);
                glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                drawBlur(previousTexture, CUBE, DEFAULT_GRAPH_POINT_SIZE, NORMALUV, DEFAULT_GRAPH_POINT_SIZE, (EMDirection) i % 2);
            } else if (uDefaultFrameBuffer != 0) {
                glBindFramebuffer(GL_FRAMEBUFFER, uDefaultFrameBuffer);
                glClearColor(0, 0, 0, 0);
                glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                drawBlur(previousTexture, CUBE, DEFAULT_GRAPH_POINT_SIZE, NORMALUV, DEFAULT_GRAPH_POINT_SIZE, (EMDirection) i % 2);
            }else{
                drawBlur(previousTexture, CUBE, DEFAULT_GRAPH_POINT_SIZE, bFlip ? NORMALUV:VFLIPUV, DEFAULT_GRAPH_POINT_SIZE, (EMDirection) i % 2);
            }

            if (isNotLast) {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                previousTexture = m_pcFrameBufferTextures[i];
            }
        }
    }

    void CBlurFilter::destroyFrameBuffers() {
        int size = 2 * m_uItNumber - 1;
        for (int i = 0; i < size; i++) {
            deleteFrameBuffers(m_auFrameBuffers[i], m_pcFrameBufferTextures[i]);
        }
    }

    void
    CBlurFilter::drawBlur(CTexture *pTexture, const float *pPositionBuffer, s32 uPPointSize,
                          const float *pUVBuffer,
                          s32 uUVPointSize, EMDirection direction) {
        GLfloat fOffset[2] = {0.0f, 0.0f};
        m_pProgram->enable();
        switch (direction) {
            case EM_DIRECT_Y: {
                fOffset[0] = m_fBlurOffset / m_uOutputWidth;
                glUniform2fv(m_pProgram->getUniform("u_texelOffset"), 1, fOffset);
            }
                break;
            case EM_DIRECT_Z: {
            }
                break;
            case EM_DIRECT_X:
            default: {
                fOffset[1] = m_fBlurOffset / m_uOutputHeight;
                glUniform2fv(m_pProgram->getUniform("u_texelOffset"), 1, fOffset);
            }
                break;
        }
        m_pProgram->disable();
        CVFilter::draw(pTexture, pPositionBuffer, uPPointSize, pUVBuffer, uUVPointSize);

    }
}