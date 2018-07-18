//
// Created by ASUS on 2018/4/13.
//

#include <track.h>
#include "multitrack.h"

namespace paomiantv {


    CMultiTrack::CMultiTrack() :
            m_ptTail(NULL),
            m_ptHead(NULL),
            m_uTrackSize(0) {
        m_ptTrackSource->m_eSourceType = EM_SOURCE_MULTI;
        m_ptTrackSource->m_bIsValid = TRUE;
    }

    CMultiTrack::~CMultiTrack() {

    }


    BOOL32 CMultiTrack::pushTrack(ITrack *pcTrack) {
        CAutoLock autoLock(m_pLock);
        if (pcTrack == NULL || pcTrack->getType() != m_eType || !pcTrack->isSourceValid()) {
            LOGW("track is invalid");
            return FALSE;
        }
        TTrackNode *node = new TTrackNode;
        node->m_pcTrack = pcTrack;
        if (m_ptHead == NULL) {
            m_ptHead = node;
            m_ptTail = node;
        } else {
            m_ptTail->m_ptNext = node;
            node->m_ptPre = m_ptTail;
            m_ptTail = node;
        }
        m_uTrackSize++;
        pcTrack->setIndependent(FALSE);
        pcTrack->setLoop(FALSE);
        return TRUE;
    }

    BOOL32 CMultiTrack::popTrack(ITrack *&pcTrack, CTransition *&pcTransition) {
        CAutoLock autoLock(m_pLock);
        if (m_uTrackSize == 0 || m_ptTail == NULL) {
            LOGW("list size is 0");
            pcTrack = NULL;
            return FALSE;
        } else {
            pcTrack = m_ptTail->m_pcTrack;
            pcTransition = m_ptTail->m_pTransition;
            if (m_uTrackSize == 1) {
                delete m_ptTail;
                m_ptTail = NULL;
                m_ptHead = NULL;
            } else {
                TTrackNode *tmp = m_ptTail;
                m_ptTail = tmp->m_ptPre;
                m_ptTail->m_ptNext = NULL;
                pcTransition = tmp->m_pTransition;
                pcTrack = tmp->m_pcTrack;
                delete tmp;
            }
        }
        m_uTrackSize--;
        pcTrack->setIndependent(TRUE);
        pcTrack->setLoop(FALSE);
        return TRUE;
    }

    BOOL32 CMultiTrack::insertTrack(ITrack *pcTrack, u32 index) {
        CAutoLock autoLock(m_pLock);
        if (pcTrack == NULL || pcTrack->getType() != m_eType || !pcTrack->isSourceValid()) {
            LOGW("track is invalid");
            return FALSE;
        }
        TTrackNode *node = new TTrackNode;
        node->m_pcTrack = pcTrack;
        if (index >= m_uTrackSize) {
            if (m_ptHead == NULL) {
                m_ptHead = node;
                m_ptTail = node;
            } else {
                m_ptTail->m_ptNext = node;
                node->m_ptPre = m_ptTail;
                m_ptTail = node;
            }
        } else {
            TTrackNode *curr = m_ptHead;
            TTrackNode *pre = curr == NULL ? NULL : curr->m_ptPre;
            TTrackNode *next = curr == NULL ? NULL : curr->m_ptNext;
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
        m_uTrackSize++;
        pcTrack->setIndependent(FALSE);
        pcTrack->setLoop(FALSE);
        return TRUE;
    }

    BOOL32
    CMultiTrack::removeTrackByIndex(ITrack *&pcTrack, CTransition *&pcTransition, u32 index) {
        CAutoLock autoLock(m_pLock);
        if (m_uTrackSize == 0 || index >= m_uTrackSize || m_ptHead == NULL) {
            LOGW("list size is 0");
            pcTrack = NULL;
            return FALSE;
        } else {
            TTrackNode *curr = m_ptHead;
            TTrackNode *pre = curr == NULL ? NULL : curr->m_ptPre;
            TTrackNode *next = curr == NULL ? NULL : curr->m_ptNext;
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
            pcTrack = NULL;
            if (curr != NULL) {
                pcTrack = curr->m_pcTrack;
                pcTransition = curr->m_pTransition;
                delete curr;
                pcTrack->setIndependent(TRUE);
                pcTrack->setLoop(FALSE);
                --m_uTrackSize;
                return TRUE;
            }
        }
        return FALSE;
    }

    BOOL32 CMultiTrack::removeTrackById(u32 id, ITrack *&pcTrack, CTransition *&pcTransition) {
        CAutoLock autoLock(m_pLock);
        if (m_uTrackSize == 0 || m_ptHead == NULL) {
            LOGW("list size is 0");
            pcTrack = NULL;
            return FALSE;
        } else {
            TTrackNode *curr = m_ptHead;
            TTrackNode *pre = curr == NULL ? NULL : curr->m_ptPre;
            TTrackNode *next = curr == NULL ? NULL : curr->m_ptNext;
            while (curr != NULL) {
                if (curr->m_pcTrack != NULL && curr->m_pcTrack->getId() == id) {
                    break;
                }
                curr = curr->m_ptNext;
                pre = curr == NULL ? NULL : curr->m_ptPre;
                next = curr == NULL ? NULL : curr->m_ptNext;
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
            pcTrack = NULL;
            if (curr != NULL) {
                pcTrack = curr->m_pcTrack;
                pcTransition = curr->m_pTransition;
                delete curr;
                pcTrack->setIndependent(TRUE);
                pcTrack->setLoop(FALSE);
                --m_uTrackSize;
                return TRUE;
            }
        }
        return FALSE;
    }

    BOOL32 CMultiTrack::replaceTrack(ITrack *&pcTrack, u32 index) {
        CAutoLock autoLock(m_pLock);
        ITrack *tmp = pcTrack;
        if (index >= m_uTrackSize) {
            if (m_ptHead == NULL) {
                return FALSE;
            } else {
                if (m_ptTail != NULL) {
                    pcTrack = m_ptTail->m_pcTrack;
                    pcTrack->setIndependent(TRUE);
                    pcTrack->setLoop(FALSE);
                    m_ptTail->m_pcTrack = tmp;
                    m_ptTail->m_pcTrack->setIndependent(FALSE);
                    m_ptTail->m_pcTrack->setLoop(FALSE);
                    return TRUE;
                }
            }
        } else {
            TTrackNode *curr = m_ptHead;
            TTrackNode *next = curr == NULL ? NULL : curr->m_ptNext;
            while (index != 0 && curr != NULL && next != NULL) {
                curr = curr->m_ptNext;
                next = curr == NULL ? NULL : curr->m_ptNext;
                index--;
            }
            if (curr != NULL) {
                pcTrack = curr->m_pcTrack;
                pcTrack->setIndependent(TRUE);
                pcTrack->setLoop(FALSE);
                curr->m_pcTrack = tmp;
                curr->m_pcTrack->setIndependent(FALSE);
                curr->m_pcTrack->setLoop(FALSE);
                return TRUE;
            }
        }
        return FALSE;
    }

    BOOL32 CMultiTrack::getTrack(ITrack *&pcTrack, u32 index) {
        CAutoLock autoLock(m_pLock);
        if (m_uTrackSize == 0 || index >= m_uTrackSize || m_ptHead == NULL) {
            pcTrack = NULL;
            return FALSE;
        }
        TTrackNode *curr = m_ptHead;
        TTrackNode *next = curr == NULL ? NULL : curr->m_ptNext;
        while (index != 0 && curr != NULL && next != NULL) {
            curr = curr->m_ptNext;
            next = curr == NULL ? NULL : curr->m_ptNext;
            index--;
        }
        pcTrack = NULL;
        if (curr != NULL) {
            pcTrack = curr->m_pcTrack;
            return TRUE;
        }
        return FALSE;
    }

    void CMultiTrack::clearTracks() {
        CAutoLock autoLock(m_pLock);
        TTrackNode *tmp = NULL;
        while (m_ptHead != NULL) {
            tmp = m_ptHead;
            m_ptHead = tmp->m_ptNext;
            if (tmp->m_pTransition != NULL) {
                delete tmp->m_pTransition;
            }
            if (tmp->m_pcTrack != NULL) {
                delete tmp->m_pcTrack;
            }
            delete tmp;
        }
        m_ptHead = NULL;
        m_ptTail = NULL;
        m_uTrackSize = 0;
    }

    BOOL32 CMultiTrack::replaceTransitionByIndex(CTransition *&pcTransition, u32 index) {
        CAutoLock autoLock(m_pLock);
        if (index >= m_uTrackSize) {
            if (m_ptHead == NULL) {
                return FALSE;
            } else {
                if (m_ptTail != NULL) {
                    CTransition *tmp = m_ptTail->m_pTransition;
                    m_ptTail->m_pTransition = pcTransition;
                    pcTransition = tmp;
                    return TRUE;
                }
            }
        } else {
            TTrackNode *curr = m_ptHead;
            TTrackNode *next = curr == NULL ? NULL : curr->m_ptNext;
            while (index != 0 && curr != NULL && next != NULL) {
                curr = curr->m_ptNext;
                next = curr == NULL ? NULL : curr->m_ptNext;
                index--;
            }
            if (curr != NULL) {
                CTransition *tmp = curr->m_pTransition;
                curr->m_pTransition = pcTransition;
                pcTransition = tmp;
                return TRUE;
            }
        }
        return FALSE;
    }

    BOOL32 CMultiTrack::replaceTransitionById(u32 id, CTransition *&pTransition) {
        CAutoLock autoLock(m_pLock);

        TTrackNode *curr = m_ptHead;
        while (curr != NULL) {
            if (curr->m_pTransition != NULL && curr->m_pTransition->getId() == id) {
                break;
            }
            curr = curr->m_ptNext;
        }
        if (curr != NULL) {
            CTransition *tmp = curr->m_pTransition;
            curr->m_pTransition = pTransition;
            pTransition = tmp;
            return TRUE;
        }

        return FALSE;
    }

    BOOL32 CMultiTrack::getTransition(CTransition *&pcTransition, u32 index) {
        CAutoLock autoLock(m_pLock);
        if (m_uTrackSize == 0 || index >= m_uTrackSize || m_ptHead == NULL) {
            pcTransition = NULL;
            return FALSE;
        }
        TTrackNode *curr = m_ptHead;
        TTrackNode *next = curr == NULL ? NULL : curr->m_ptNext;
        while (index != 0 && curr != NULL && next != NULL) {
            curr = curr->m_ptNext;
            next = curr == NULL ? NULL : curr->m_ptNext;
            index--;
        }
        pcTransition = NULL;
        if (curr != NULL) {
            pcTransition = curr->m_pTransition;
            return TRUE;
        }
        return FALSE;
    }

    inline u32 CMultiTrack::getTrackCount() {
        return m_uTrackSize;
    }

    s64 CMultiTrack::getDataDuration() {
        s64 d = 0;
        TTrackNode *curr = m_ptHead;
        while (curr != NULL) {
            if (curr->m_pcTrack != NULL && curr->m_pcTrack->getDataDuration() >= 0) {
                d += curr->m_pcTrack->getDataDuration();
            }
            curr = curr->m_ptNext;
        }
        return d;
    }
}


