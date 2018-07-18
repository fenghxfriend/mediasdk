//
// Created by ASUS on 2017/12/28.
//

#include <malloc.h>
#include "texture.h"

namespace paomiantv {
    CTexture::CTexture(EMPixelFormat emPixelFormat) : m_puIds(NULL), m_eFormat(emPixelFormat) {
        USE_LOG;
        m_pcLock = new CLock;
        initTexture();
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    CTexture::~CTexture() {
        USE_LOG;
        clearTexture();
        glBindTexture(GL_TEXTURE_2D, 0);
        if (m_pcLock != NULL) {
            delete m_pcLock;
            m_pcLock = NULL;
        }
    }

    BOOL32
    CTexture::fillTexture(u8 *pData, u32 uStride, u32 uStrideHeight) {
        if (uStride == 0 || uStrideHeight == 0) {
            LOGE("image width or height is invalid!");
            return FALSE;
        }
        CAutoLock autoLock(m_pcLock);
        u32 size = uStride * uStrideHeight;
        switch (m_eFormat) {
            case EM_PIXEL_FORMAT_I420: {
                if (m_puIds[EM_TEXTURE_Y_OR_RGBA] == 0) {
                    addTexture(m_puIds[EM_TEXTURE_Y_OR_RGBA]);
                } else {
                    glBindTexture(GL_TEXTURE_2D, m_puIds[EM_TEXTURE_Y_OR_RGBA]);
                }
                glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, uStride, uStrideHeight,
                             0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pData);

                if (m_puIds[EM_TEXTURE_U] == 0) {
                    addTexture(m_puIds[EM_TEXTURE_U]);
                } else {
                    glBindTexture(GL_TEXTURE_2D, m_puIds[EM_TEXTURE_U]);
                }
                glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, uStride >> 1, uStrideHeight >> 1,
                             0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                             pData != NULL ? &pData[size] : NULL);

                if (m_puIds[EM_TEXTURE_V] == 0) {
                    addTexture(m_puIds[EM_TEXTURE_V]);
                } else {
                    glBindTexture(GL_TEXTURE_2D, m_puIds[EM_TEXTURE_V]);
                }
                glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, uStride >> 1, uStrideHeight >> 1,
                             0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                             pData != NULL ? &pData[size + (size >> 2)] : NULL);
            }
                break;
//            case EM_PIXEL_FORMAT_RGB: {
//                if (m_puIds[EM_TEXTURE_Y_OR_RGBA] == 0) {
//                    addTexture(m_puIds[EM_TEXTURE_Y_OR_RGBA]);
//                } else {
//                    glBindTexture(GL_TEXTURE_2D, m_puIds[EM_TEXTURE_Y_OR_RGBA]);
//                }
//                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, uStride, uStrideHeight,
//                             0, GL_RGB, GL_UNSIGNED_BYTE, pData);
//            }
            default: {
                if (m_puIds[EM_TEXTURE_Y_OR_RGBA] == 0) {
                    addTexture(m_puIds[EM_TEXTURE_Y_OR_RGBA]);
                } else {
                    glBindTexture(GL_TEXTURE_2D, m_puIds[EM_TEXTURE_Y_OR_RGBA]);
                }
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, uStride, uStrideHeight,
                             0, GL_RGBA, GL_UNSIGNED_BYTE, pData);
            }
                break;
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        return TRUE;
    }

    GLuint CTexture::getTextureId(EMTexture emTexture) {
        CAutoLock autoLock(m_pcLock);
        if (m_puIds != NULL) {
            if (m_eFormat == EM_PIXEL_FORMAT_I420 && emTexture >= EM_TEXTURE_Y_OR_RGBA &&
                emTexture <= EM_TEXTURE_V) {
                return m_puIds[emTexture];
            } else {
                return m_puIds[EM_TEXTURE_Y_OR_RGBA];
            }
        } else {
            return 0;
        }
    }

    void CTexture::addTexture(u32 &uId) {
        CAutoLock autoLock(m_pcLock);
        glGenTextures(1, &(uId));
        glBindTexture(GL_TEXTURE_2D, uId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }


    void CTexture::deleteTexture(u32 &uId) {
        CAutoLock autoLock(m_pcLock);
        glDeleteTextures(1, &(uId));
        uId = 0;
    }

    void CTexture::clearTexture() {
        CAutoLock autoLock(m_pcLock);
        if (m_puIds != NULL) {
            if (m_eFormat == EM_PIXEL_FORMAT_I420) {
                deleteTexture(m_puIds[EM_TEXTURE_Y_OR_RGBA]);
                deleteTexture(m_puIds[EM_TEXTURE_U]);
                deleteTexture(m_puIds[EM_TEXTURE_V]);
            } else {
                deleteTexture(m_puIds[EM_TEXTURE_Y_OR_RGBA]);
            }
            delete[] m_puIds;
            m_puIds = NULL;
        }

    }

    void CTexture::initTexture() {
        CAutoLock autoLock(m_pcLock);
        if (m_puIds != NULL) {
            clearTexture();
        }
        if (m_eFormat == EM_PIXEL_FORMAT_I420) {
            m_puIds = new GLuint[EM_TEXTURE_END];
            addTexture(m_puIds[EM_TEXTURE_Y_OR_RGBA]);
            addTexture(m_puIds[EM_TEXTURE_U]);
            addTexture(m_puIds[EM_TEXTURE_V]);
        } else {
            m_puIds = new GLuint[1];
            addTexture(m_puIds[EM_TEXTURE_Y_OR_RGBA]);
        }
    }
}