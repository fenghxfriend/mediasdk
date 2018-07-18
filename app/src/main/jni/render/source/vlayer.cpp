//
// Created by ASUS on 2017/12/28.
//

#include <algorithm>
#include "vlayer.h"

namespace paomiantv {

    CVLayer::CVLayer() : m_ptHead(NULL), m_ptTail(NULL), m_uFilterSize(0), m_nHeight(0),
                         m_nWidth(0) {
        m_pLock = new CLock;
        reset();
    }

    CVLayer::~CVLayer() {
        reset();
        if (m_pLock != NULL) {
            delete m_pLock;
            m_pLock = NULL;
        }
    }

    void CVLayer::clearFilters() {
        CAutoLock autoLock(m_pLock);
        TVFilterNode *head = m_ptHead;
        while (head != NULL) {
            TVFilterNode *tmp = head;
            head = tmp->m_ptNext;

            deleteFrameBuffers(tmp->m_uFrameBuffer, tmp->m_pcFrameBufferTexture);
            if (tmp->m_pcFilter != NULL) {
                delete tmp->m_pcFilter;
            }
            delete tmp;
        }
        m_ptHead = NULL;
        m_ptTail = NULL;
        m_uFilterSize = 0;
    }

    void CVLayer::reset() {
        CAutoLock autoLock(m_pLock);
        clearFilters();
        m_nWidth = 0;
        m_nHeight = 0;
    }

    void CVLayer::updateLayerSize(u32 uGLWidth, u32 uGLHeight) {
        CAutoLock autoLock(m_pLock);
        m_nWidth = uGLWidth;
        m_nHeight = uGLHeight;
        TVFilterNode *tmp = m_ptHead;
        while (tmp != NULL && tmp != m_ptTail) {
            if (tmp->m_pcFilter != NULL) {
                tmp->m_pcFilter->changeOutputSize(uGLWidth, uGLHeight);
            }
            if (tmp->m_pcFrameBufferTexture != NULL) {
                tmp->m_pcFrameBufferTexture->fillTexture(NULL, uGLWidth, uGLHeight);
            }
            tmp = tmp->m_ptNext;
        }


    }

    BOOL32 CVLayer::pushFilter(CVFilter *pcFilter) {
        CAutoLock autoLock(m_pLock);
        if (pcFilter == NULL || pcFilter->getOutputHeight() == 0 ||
            pcFilter->getOutputWidth() == 0) {
            LOGW("filter do not initialize");
            return FALSE;
        }
        TVFilterNode *node = new TVFilterNode;
        node->m_pcFilter = pcFilter;
        if (m_ptHead == NULL) {
            m_ptHead = node;
            m_ptTail = node;
        } else {
            addFrameBuffers(m_ptTail->m_uFrameBuffer, m_ptTail->m_pcFrameBufferTexture);
            m_ptTail->m_ptNext = node;
            node->m_ptPre = m_ptTail;
            m_ptTail = node;
        }
        m_uFilterSize++;
        return TRUE;
    }

    BOOL32 CVLayer::popFilter(CVFilter *&pcFilter) {
        CAutoLock autoLock(m_pLock);
        if (m_uFilterSize == 0 || m_ptTail == NULL) {
//            LOGW("list size is 0");
            pcFilter = NULL;
            return FALSE;
        } else {
            pcFilter = m_ptTail->m_pcFilter;
            if (m_uFilterSize == 1) {
                deleteFrameBuffers(m_ptTail->m_uFrameBuffer, m_ptTail->m_pcFrameBufferTexture);
                delete m_ptTail;
                m_ptTail = NULL;
                m_ptHead = NULL;
            } else {
                TVFilterNode *tmp = m_ptTail;
                m_ptTail = tmp->m_ptPre;
                m_ptTail->m_ptNext = NULL;
                deleteFrameBuffers(m_ptTail->m_uFrameBuffer, m_ptTail->m_pcFrameBufferTexture);
                deleteFrameBuffers(tmp->m_uFrameBuffer, tmp->m_pcFrameBufferTexture);
                pcFilter = tmp->m_pcFilter;
                delete tmp;
            }
        }
        m_uFilterSize--;
        return TRUE;
    }

    BOOL32 CVLayer::insertFilter(CVFilter *pcFilter, u32 index) {
        CAutoLock autoLock(m_pLock);
        if (pcFilter == NULL || pcFilter->getOutputHeight() == 0 ||
            pcFilter->getOutputWidth() == 0) {
            LOGW("filter do not initialize");
            return FALSE;
        }
        TVFilterNode *node = new TVFilterNode;
        node->m_pcFilter = pcFilter;
        if (index >= m_uFilterSize) {
            if (m_ptHead == NULL) {
                m_ptHead = node;
                m_ptTail = node;
            } else {
                addFrameBuffers(m_ptTail->m_uFrameBuffer, m_ptTail->m_pcFrameBufferTexture);
                m_ptTail->m_ptNext = node;
                node->m_ptPre = m_ptTail;
                m_ptTail = node;
            }
        } else {
            TVFilterNode *curr = m_ptHead;
            TVFilterNode *pre = curr == NULL ? NULL : curr->m_ptPre;
            TVFilterNode *next = curr == NULL ? NULL : curr->m_ptNext;
            while (index != 0 && curr != NULL && next != NULL) {
                curr = curr->m_ptNext;
                pre = curr == NULL ? NULL : curr->m_ptPre;
                next = curr == NULL ? NULL : curr->m_ptNext;
                index--;
            }
            if (curr == NULL) {
                m_ptHead = node;
                m_ptTail = node;
            } else {
                addFrameBuffers(node->m_uFrameBuffer, node->m_pcFrameBufferTexture);
                if (pre == NULL) {
                    node->m_ptNext = curr;
                    curr->m_ptPre = node;
                    m_ptHead = node;
                } else {
                    node->m_ptNext = curr;
                    node->m_ptPre = pre;
                    curr->m_ptPre->m_ptNext = node;
                    curr->m_ptPre = node;
                }
            }
        }
        m_uFilterSize++;
        return TRUE;
    }

    BOOL32 CVLayer::removeFilter(CVFilter *&pcFilter, u32 index) {
        CAutoLock autoLock(m_pLock);
        if (m_uFilterSize == 0 || index >= m_uFilterSize || m_ptHead == NULL) {
            LOGW("list size is 0 or index is not less than array size");
            pcFilter = NULL;
            return FALSE;
        } else {
            TVFilterNode *curr = m_ptHead;
            TVFilterNode *pre = curr == NULL ? NULL : curr->m_ptPre;
            TVFilterNode *next = curr == NULL ? NULL : curr->m_ptNext;
            while (index != 0 && curr != NULL && next != NULL) {
                curr = curr->m_ptNext;
                pre = curr == NULL ? NULL : curr->m_ptPre;
                next = curr == NULL ? NULL : curr->m_ptNext;
                index--;
            }
            if (pre == NULL) {
                if (next == NULL) {
                    m_ptTail = NULL;
                    m_ptHead = NULL;
                } else {
                    if (curr != NULL) {
                        curr->m_ptNext->m_ptPre = NULL;
                    }
                    m_ptHead = next;
                }
            } else {
                if (next == NULL) {
                    if (curr != NULL) {
                        curr->m_ptPre->m_ptNext = NULL;
                    }
                    m_ptTail = pre;
                } else {
                    if (curr != NULL) {
                        curr->m_ptPre->m_ptNext = next;
                        curr->m_ptNext->m_ptPre = pre;
                    }
                }
            }
            pcFilter = NULL;
            if (curr != NULL) {
                pcFilter = curr->m_pcFilter;
                deleteFrameBuffers(curr->m_uFrameBuffer, curr->m_pcFrameBufferTexture);
                delete curr;
                m_uFilterSize--;
                return TRUE;
            }
        }
        return FALSE;
    }

    BOOL32 CVLayer::getFilter(CVFilter *&pcFilter, u32 index) {
        CAutoLock autoLock(m_pLock);
        if (m_uFilterSize == 0 || index >= m_uFilterSize || m_ptHead == NULL) {
            pcFilter = NULL;
            return FALSE;
        }
        TVFilterNode *curr = m_ptHead;
        TVFilterNode *next = curr == NULL ? NULL : curr->m_ptNext;
        while (index != 0 && curr != NULL && next != NULL) {
            curr = curr->m_ptNext;
            next = curr == NULL ? NULL : curr->m_ptNext;
            index--;
        }
        pcFilter = NULL;
        if (curr != NULL) {
            pcFilter = curr->m_pcFilter;
            return TRUE;
        }
        return FALSE;
    }


    void CVLayer::addFrameBuffers(GLuint &uFrameBuffer, CTexture *&pcFrameBufferTexture) {
        if (m_nWidth == 0 || m_nHeight == 0) {
            return;
        }
        deleteFrameBuffers(uFrameBuffer, pcFrameBufferTexture);
        pcFrameBufferTexture = new CTexture;
        pcFrameBufferTexture->fillTexture(NULL, m_nWidth, m_nHeight);
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

    void CVLayer::deleteFrameBuffers(GLuint &uFrameBuffer, CTexture *&pcFrameBufferTexture) {
        if (pcFrameBufferTexture != NULL) {
            delete pcFrameBufferTexture;
            pcFrameBufferTexture = NULL;
        }
        if (uFrameBuffer != 0) {
            glDeleteFramebuffers(1, &uFrameBuffer);
            uFrameBuffer = 0;
        }
    }

    void CVLayer::draw(CTexture *pOriginTexture) {
        CAutoLock autoLock(m_pLock);
        CTexture *previousTexture = pOriginTexture;
        GLuint frameBuffer = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &frameBuffer);
        TVFilterNode *curr = m_ptHead;
        // if 1 filter, no frame buffer
        // if size of filter more than 1, the size of frame buffer is (size-1)
        bool bFlipped = false;
        for (int i = 0; i < m_uFilterSize && curr != NULL; i++) {
            BOOL32 isNotLast = i < m_uFilterSize - 1;
            if (isNotLast) {
                glBindFramebuffer(GL_FRAMEBUFFER, curr->m_uFrameBuffer);
                glClearColor(0, 0, 0, 0);
                glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            }


            if (isNotLast) {
                if (curr->m_pcFilter->getFilterType() == EM_V_FILTER_MASK && bFlipped) {
                    curr->m_pcFilter->draw(previousTexture, CVFilter::CUBE,
                                           DEFAULT_GRAPH_POINT_SIZE,
                                           CVFilter::VFLIPUV,
                                           DEFAULT_GRAPH_POINT_SIZE);
                } else {
                    curr->m_pcFilter->draw(previousTexture, CVFilter::CUBE,
                                           DEFAULT_GRAPH_POINT_SIZE,
                                           CVFilter::NORMALUV,
                                           DEFAULT_GRAPH_POINT_SIZE);
                }
            } else {
                curr->m_pcFilter->draw(previousTexture, CVFilter::CUBE,
                                       DEFAULT_GRAPH_POINT_SIZE,
                                       bFlipped ? CVFilter::VFLIPUV : CVFilter::NORMALUV,
                                       DEFAULT_GRAPH_POINT_SIZE);
            }


            //The picture rendered into texture width framebuffer by filter is fliped except blur filter!
            if (curr->m_pcFilter->getFilterType() != EM_V_FILTER_BLUR) {
                bFlipped = !bFlipped;
            }

            if (isNotLast) {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                previousTexture = curr->m_pcFrameBufferTexture;
            }

            curr = curr->m_ptNext;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    }
}