//
// Created by ASUS on 2018/1/18.
//

#include "typedef.h"
#include "autolog.h"
#include "vfilter.h"

namespace paomiantv {

    const float CVFilter::CUBE[DEFAULT_GRAPH_SIZE] = {-1.0f, -1.0f, 0.0f,
                                                      1.0f, -1.0f, 0.0f,
                                                      -1.0f, 1.0f, 0.0f,
                                                      1.0f, 1.0f, 0.0f};

    const float CVFilter::NORMALUV[DEFAULT_GRAPH_SIZE] = {0.0f, 1.0f, 0.0f,
                                                          1.0f, 1.0f, 0.0f,
                                                          0.0f, 0.0f, 0.0f,
                                                          1.0f, 0.0f, 0.0f};
    const float CVFilter::VFLIPUV[DEFAULT_GRAPH_SIZE] = {0.0f, 0.0f, 0.0f,
                                                         1.0f, 0.0f, 0.0f,
                                                         0.0f, 1.0f, 0.0f,
                                                         1.0f, 1.0f, 0.0f};
    const float CVFilter::HFLIPUV[DEFAULT_GRAPH_SIZE] = {1.0f, 1.0f, 0.0f,
                                                         0.0f, 1.0f, 0.0f,
                                                         1.0f, 0.0f, 0.0f,
                                                         0.0f, 0.0f, 0.0f};

    CVFilter::CVFilter() : m_uOutputWidth(0),
                           m_uOutputHeight(0),
                           m_uCoordinateVBO(0),
                           m_uPositionVBO(0),
                           m_fAlpha(1.0f),
                           m_eFilterType(EM_V_FILTER_START) {
        m_pProgram = new CProgram;
        m_pLock = new CLock;
    }

    CVFilter::~CVFilter() {
        if (m_pProgram != NULL) {
            delete m_pProgram;
            m_pProgram = NULL;
        }
        if (m_pLock != NULL) {
            delete m_pLock;
            m_pLock = NULL;
        }
        if (m_uCoordinateVBO != 0) {
            glDeleteBuffers(1, &m_uCoordinateVBO);
            m_uCoordinateVBO = 0;
        }
        if (m_uPositionVBO != 0) {
            glDeleteBuffers(1, &m_uPositionVBO);
            m_uPositionVBO = 0;
        }
    }

    BOOL32 CVFilter::init(EMPixelFormat eSourceFMT) {
        BOOL32 re = FALSE;
        CAutoLock autoLock(m_pLock);
        switch (eSourceFMT) {
            case EM_PIXEL_FORMAT_RGBA_8888: {
                if (configure(DEFAULT_RGBA_FRAGMENT_SHADER, DEFAULT_VERTEXT_SHADER)) {
                    m_pProgram->enable();
                    glUniform1i(m_pProgram->getUniform("u_texture"), 0);
                    m_pProgram->disable();
                    re = TRUE;
                }
            }
                break;
            case EM_PIXEL_FORMAT_I420: {
                if (configure(DEFAULT_YUV_FRAGMENT_SHADER, DEFAULT_VERTEXT_SHADER)) {
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

    BOOL32 CVFilter::configure(const char *pchFragmentShader, const char *pchVertexShader) {
        BOOL32 re = FALSE;
        CAutoLock autoLock(m_pLock);
        CShader vertexShader(GL_VERTEX_SHADER), fragmentShader(GL_FRAGMENT_SHADER);
        if (!vertexShader.loadShader(pchVertexShader) ||
            !fragmentShader.loadShader(pchFragmentShader)) {
            return re;
        }

        if (!m_pProgram->compileProgram(&vertexShader, &fragmentShader)) {
            return re;
        }
        glGenBuffers(1, &m_uPositionVBO);
        glGenBuffers(1, &m_uCoordinateVBO);
        return TRUE;
    }

    void CVFilter::changeOutputSize(u32 uWidth, u32 uHeight) {
        CAutoLock autoLock(m_pLock);
        m_uOutputWidth = uWidth;
        m_uOutputHeight = uHeight;
    }

    void CVFilter::draw(CTexture *pTexture, const float *pPositionBuffer, s32 uPPointSize,
                        const float *pUVBuffer,
                        s32 uUVPointSize) {
        CAutoLock autoLock(m_pLock);
        m_pProgram->enable();
        if (pTexture->getFormat() == EM_PIXEL_FORMAT_I420) {
            if (pTexture->getTextureId(EM_TEXTURE_Y_OR_RGBA) != 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, pTexture->getTextureId(EM_TEXTURE_Y_OR_RGBA));
            }
            if (pTexture->getTextureId(EM_TEXTURE_U) != 0) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, pTexture->getTextureId(EM_TEXTURE_U));
            }
            if (pTexture->getTextureId(EM_TEXTURE_V) != 0) {
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, pTexture->getTextureId(EM_TEXTURE_V));
            }
        } else {
            if (pTexture->getTextureId(EM_TEXTURE_Y_OR_RGBA) != 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, pTexture->getTextureId(EM_TEXTURE_Y_OR_RGBA));
            }
        }

        glUniform3fv(m_pProgram->getUniform("u_posRotate"), 1, m_afRotationVec);
        glUniform3fv(m_pProgram->getUniform("u_posTranslate"), 1, m_afTranslateVec);
        glUniform3fv(m_pProgram->getUniform("u_posScale"), 1, m_afScaleVec);

        glUniform3fv(m_pProgram->getUniform("u_uvRotate"), 1, m_afUVCropRotationVec);
        glUniform3fv(m_pProgram->getUniform("u_uvScale"), 1, m_afUVCropScaleVec);
        glUniform3fv(m_pProgram->getUniform("u_uvTranslate"), 1, m_afUVCropTranslateVec);
        glUniform1f(m_pProgram->getUniform("u_alpha"), m_fAlpha);

        glEnableVertexAttribArray(m_pProgram->getAttribute("a_position"));
        glBindBuffer(GL_ARRAY_BUFFER, m_uPositionVBO);
        glBufferData(GL_ARRAY_BUFFER, DEFAULT_SIZE_PER_POINT * uPPointSize * sizeof(float),
                     pPositionBuffer, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(m_pProgram->getAttribute("a_position"), DEFAULT_SIZE_PER_POINT,
                              GL_FLOAT, GL_FALSE,
                              DEFAULT_SIZE_PER_POINT * sizeof(GLfloat), (const GLvoid *) 0);

        glEnableVertexAttribArray(m_pProgram->getAttribute("a_texCoord"));
        glBindBuffer(GL_ARRAY_BUFFER, m_uCoordinateVBO);
        glBufferData(GL_ARRAY_BUFFER, DEFAULT_SIZE_PER_POINT * uUVPointSize * sizeof(float),
                     pUVBuffer, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(m_pProgram->getAttribute("a_texCoord"), DEFAULT_SIZE_PER_POINT,
                              GL_FLOAT, GL_FALSE,
                              DEFAULT_SIZE_PER_POINT * sizeof(GLfloat), (const GLvoid *) 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, uPPointSize);

        glDisableVertexAttribArray(m_pProgram->getAttribute("a_position"));
        glDisableVertexAttribArray(m_pProgram->getAttribute("a_texCoord"));
        m_pProgram->disable();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // "value" be a const value with float type
    void CVFilter::sendUniform1f(const char *uniformName, const float value) {
        m_pProgram->enable();
        if (m_pProgram->getUniform(uniformName) != 0) {
            glUniform1f(m_pProgram->getUniform(uniformName), value);
        }
        m_pProgram->disable();
    }


    // "value" must be a 4x4 Matrix width float elements
    void CVFilter::sendUniformMatrix4fv(const char *uniformName, const float *value) {
        m_pProgram->enable();
        if (m_pProgram->getUniform(uniformName) != 0) {
            glUniformMatrix4fv(m_pProgram->getUniform(uniformName), 1, GL_FALSE, value);
        }
        m_pProgram->disable();
    }

    void CVFilter::flipUV(float *pUVData, u32 uUVPointSize, EMDirection dir) {
        switch (dir) {
            case EM_DIRECT_X: {
                for (int i = 0; i < uUVPointSize; i++) {
                    pUVData[i * DEFAULT_SIZE_PER_POINT] =
                            1.0f - pUVData[i * DEFAULT_SIZE_PER_POINT];
                }
            }
                break;
            case EM_DIRECT_Y: {
                for (int i = 0; i < uUVPointSize; i++) {
                    pUVData[i * DEFAULT_SIZE_PER_POINT + 1] =
                            1.0f - pUVData[i * DEFAULT_SIZE_PER_POINT + 1];
                }
            }
                break;
            default:
                break;
        }

    }

    void CVFilter::setTranslate(GLfloat fTranslateX, GLfloat fTranslateY, GLfloat fTranslateZ) {
        CAutoLock autoLock(m_pLock);
        m_afTranslateVec[EM_DIRECT_X] = fTranslateX;
        m_afTranslateVec[EM_DIRECT_Y] = fTranslateY;
        m_afTranslateVec[EM_DIRECT_Z] = fTranslateZ;
    }

    void CVFilter::setTranslate(GLfloat fTranslate, EMDirection direction) {
        CAutoLock autoLock(m_pLock);
        m_afTranslateVec[direction] = fTranslate;
    }

    void CVFilter::setScale(GLfloat fScaleX, GLfloat fScaleY, GLfloat fScaleZ) {
        CAutoLock autoLock(m_pLock);
        m_afScaleVec[EM_DIRECT_X] = fScaleX;
        m_afScaleVec[EM_DIRECT_Y] = fScaleY;
        m_afScaleVec[EM_DIRECT_Z] = fScaleZ;
    }

    void CVFilter::setScale(GLfloat fScale, EMDirection direction) {
        CAutoLock autoLock(m_pLock);
        m_afScaleVec[direction] = fScale;
    }

    void CVFilter::setRotation(GLfloat fRotationX, GLfloat fRotationY, GLfloat fRotationZ) {
        CAutoLock autoLock(m_pLock);
        m_afRotationVec[EM_DIRECT_X] = fRotationX;
        m_afRotationVec[EM_DIRECT_Y] = fRotationY;
        m_afRotationVec[EM_DIRECT_Z] = fRotationZ;
    }

    void CVFilter::setRotation(GLfloat fRotation, EMDirection direction) {
        CAutoLock autoLock(m_pLock);
        m_afRotationVec[direction] = fRotation;
    }

    void
    CVFilter::setUVCropTranslate(GLfloat fTranslateX, GLfloat fTranslateY, GLfloat fTranslateZ) {
        CAutoLock autoLock(m_pLock);
        m_afUVCropTranslateVec[EM_DIRECT_X] = fTranslateX;
        m_afUVCropTranslateVec[EM_DIRECT_Y] = fTranslateY;
        m_afUVCropTranslateVec[EM_DIRECT_Z] = fTranslateZ;
    }

    void CVFilter::setUVCropScale(GLfloat fScaleX, GLfloat fScaleY, GLfloat fScaleZ) {
        CAutoLock autoLock(m_pLock);
        m_afUVCropScaleVec[EM_DIRECT_X] = fScaleX;
        m_afUVCropScaleVec[EM_DIRECT_Y] = fScaleY;
        m_afUVCropScaleVec[EM_DIRECT_Z] = fScaleZ;
    }

    void CVFilter::setUVCropTranslate(GLfloat fTranslate, EMDirection direction) {
        CAutoLock autoLock(m_pLock);
        m_afUVCropTranslateVec[direction] = fTranslate;
    }

    void CVFilter::setUVCropScale(GLfloat fScale, EMDirection direction) {
        CAutoLock autoLock(m_pLock);
        m_afUVCropScaleVec[direction] = fScale;
    }

    void CVFilter::setUVCropRotation(GLfloat fRotationX, GLfloat fRotationY, GLfloat fRotationZ) {
        CAutoLock autoLock(m_pLock);
        m_afUVCropRotationVec[EM_DIRECT_X] = fRotationX;
        m_afUVCropRotationVec[EM_DIRECT_Y] = fRotationY;
        m_afUVCropRotationVec[EM_DIRECT_Z] = fRotationZ;
    }

    void CVFilter::setUVCropRotation(GLfloat fRotation, EMDirection direction) {
        CAutoLock autoLock(m_pLock);
        m_afUVCropRotationVec[direction] = fRotation;
    }

    void CVFilter::setAlpha(GLfloat fAlpha) {
        CAutoLock autoLock(m_pLock);
        m_fAlpha = fAlpha;
    }

    float CVFilter::getUVCropTranslate(EMDirection direction) {
        CAutoLock autoLock(m_pLock);
        return m_afRotationVec[direction];
    }

    float CVFilter::getUVCropScale(EMDirection direction) {
        CAutoLock autoLock(m_pLock);
        return m_afUVCropScaleVec[direction];
    }

    float CVFilter::getUVCropRotation(EMDirection direction) {
        CAutoLock autoLock(m_pLock);
        return m_afUVCropRotationVec[direction];
    }
}

