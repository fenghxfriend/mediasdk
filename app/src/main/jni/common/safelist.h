/*******************************************************************************
 *        Module: common
 *          File:
 * Functionality: safe queue defination
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 PAOMIANTV, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-08-17  v1.0        huangxuefeng  created
 ******************************************************************************/

#ifndef _PAOMIANTV_SAFELIST_H_
#define _PAOMIANTV_SAFELIST_H_

#include "autolock.h"
#include "constant.h"

namespace paomiantv {

    template<class T>
    class CSafeList {
    private:
        typedef struct tagNode {
            T *data;
            tagNode *pre;
            tagNode *next;

            tagNode() {
                memset(this, 0, sizeof(tagNode));
            }
        } TNode;
#ifndef SortCB

        typedef bool ( *SortCB    )(T *t1, T *t2);

        typedef bool ( *CompareCB    )(u64 params, T *t);

#endif

    public:
        CSafeList() :
                m_pHead(NULL),
                m_pTail(NULL),
                m_uSize(0) {
            m_pLock = new CLock;
        }

        ~CSafeList() {
            clear();
            delete m_pLock;
            m_pLock = NULL;
        }

        void push(T *t) {
            CAutoLock cAutoLock(m_pLock);
            TNode *node = new TNode;
            node->data = t;
            if (m_pHead == NULL) {
                m_pHead = node;
                m_pTail = node;
            } else {
                m_pTail->next = node;
                node->pre = m_pTail;
                m_pTail = node;
            }
            m_uSize++;
        }

        T *pop() {
            CAutoLock cAutoLock(m_pLock);
            T *t = NULL;
            if (m_pTail != NULL) {
                TNode *node = m_pTail;
                t = node->data;
                if (m_pTail == m_pHead) {
                    m_pHead = m_pTail = NULL;
                } else {
                    m_pTail = m_pTail->pre;
                    m_pTail->next = NULL;
                }
                delete node;
                m_uSize--;
            }
            return t;
        }

        T *get(u32 index) {
            CAutoLock cAutoLock(m_pLock);
            T *t = NULL;
            if (index < m_uSize && m_uSize != 0) {
                TNode *curr = m_pHead;
                while (curr != NULL && index != 0) {
                    curr = curr->next;
                    index--;
                }
                if (index == 0 && curr != NULL) {
                    t = curr->data;
                }
            }
            return t;
        }

        T *get(u64 params, CompareCB cb) {
            CAutoLock cAutoLock(m_pLock);
            T *t = NULL;
            TNode *curr = m_pHead;
            while (curr != NULL && curr->data != NULL && !cb(params, curr->data)) {
                curr = curr->next;
            }
            if (curr != NULL && curr->data != NULL && cb(params, curr->data)) {
                t = curr->data;
            }
            return t;
        }

        bool contain(T *t) {
            CAutoLock cAutoLock(m_pLock);
            if (m_uSize > 0 && m_pHead != NULL) {
                TNode *curr = m_pHead;
                while (curr != NULL) {
                    if (curr->data == t) {
                        return true;
                    }
                    curr = curr->next;
                }
            }
            return false;
        }

        bool contain(u64 params, CompareCB cb) {
            CAutoLock cAutoLock(m_pLock);
            if (m_uSize > 0 && m_pHead != NULL) {
                TNode *curr = m_pHead;
                while (curr != NULL && curr->data != NULL && !cb(params, curr->data)) {
                    curr = curr->next;
                }
                if (curr != NULL && curr->data != NULL && cb(params, curr->data)) {
                    return true;
                }
            }
            return false;
        }

        bool insert(T *t, u32 index) {
            CAutoLock cAutoLock(m_pLock);
            if (index <= m_uSize && t != NULL) {
                TNode *node = new TNode;
                node->data = t;

                TNode *curr = m_pHead;
                while (curr != NULL && index != 0) {
                    curr = curr->next;
                    index--;
                }

                if (curr == NULL) {
                    if (m_uSize > 0) {
                        node->pre = m_pTail;
                        m_pTail->next = node;
                        m_pTail = node;
                    } else {
                        m_pHead = m_pTail = node;
                    }
                } else {
                    if (curr == m_pHead) {
                        node->next = m_pHead;
                        m_pHead->pre = node;
                        m_pHead = node;
                    } else {
                        node->pre = curr->pre;
                        node->next = curr;
                        node->pre->next = node;
                        curr->pre = node;
                    }
                }
                m_uSize++;
                return true;
            }
            return false;
        }


        T *remove(T *t) {
            T *ret = NULL;
            CAutoLock cAutoLock(m_pLock);
            if (m_uSize != 0) {
                TNode *curr = m_pHead;
                while (curr != NULL && curr->data != t) {
                    curr = curr->next;
                }
                if (curr != NULL && curr->data == t) {
                    if (curr == m_pHead) {
                        if (curr == m_pTail) {
                            m_pHead = m_pTail = NULL;
                        } else {
                            m_pHead = m_pHead->next;
                            m_pHead->pre = NULL;
                        }
                    } else {
                        if (curr == m_pTail) {
                            m_pTail = m_pTail->pre;
                            m_pTail->next = NULL;
                        } else {
                            curr->pre->next = curr->next;
                            curr->next->pre = curr->pre;
                        }
                    }
                    ret = curr->data;
                    delete curr;
                    m_uSize--;
                    return ret;
                }
            }
            return ret;
        }

        T *remove(u32 index) {
            T *ret = NULL;
            CAutoLock cAutoLock(m_pLock);
            if (index < m_uSize && m_uSize != 0) {
                TNode *curr = m_pHead;
                while (curr != NULL && index != 0) {
                    curr = curr->next;
                    index--;
                }
                if (index == 0 && curr != NULL) {
                    if (curr == m_pHead) {
                        if (curr == m_pTail) {
                            m_pHead = m_pTail = NULL;
                        } else {
                            m_pHead = m_pHead->next;
                            m_pHead->pre = NULL;
                        }
                    } else {
                        if (curr == m_pTail) {
                            m_pTail = m_pTail->pre;
                            m_pTail->next = NULL;
                        } else {
                            curr->pre->next = curr->next;
                            curr->next->pre = curr->pre;
                        }
                    }
                    ret = curr->data;
                    delete curr;
                    m_uSize--;
                    return ret;
                }
            }
            return ret;
        }

        T *remove(u64 params, CompareCB cb) {
            T *ret = NULL;
            CAutoLock cAutoLock(m_pLock);
            if (m_uSize != 0) {
                TNode *curr = m_pHead;
                while (curr != NULL && curr->data != NULL && !cb(params, curr->data)) {
                    curr = curr->next;
                }
                if (curr != NULL && curr->data != NULL && cb(params, curr->data)) {
                    if (curr == m_pHead) {
                        if (curr == m_pTail) {
                            m_pHead = m_pTail = NULL;
                        } else {
                            m_pHead = m_pHead->next;
                            m_pHead->pre = NULL;
                        }
                    } else {
                        if (curr == m_pTail) {
                            m_pTail = m_pTail->pre;
                            m_pTail->next = NULL;
                        } else {
                            curr->pre->next = curr->next;
                            curr->next->pre = curr->pre;
                        }
                    }
                    ret = curr->data;
                    delete curr;
                    m_uSize--;
                    return ret;
                }
            }
            return ret;
        }

        void sort(SortCB cb) {
            CAutoLock cAutoLock(m_pLock);
            if (m_pHead == NULL || m_uSize < 2 || cb == NULL) {
                return;
            }
            int i, j;
            TNode *curr;
            T *temp;
            for (i = 0; i < m_uSize - 1; i++) {
                for (j = 0, curr = m_pHead; j < m_uSize - 1 - i; j++) {
                    if (curr->data != NULL && curr->next != NULL && curr->next->data != NULL) {
                        if (cb(curr->data, curr->next->data)) {
                            temp = curr->data;
                            curr->data = curr->next->data;
                            curr->next->data = temp;
                        }
                        curr = curr->next;
                    }
                }
            }
        }

        bool empty() {
            CAutoLock cAutoLock(m_pLock);
            return m_uSize == 0;
        }

        u32 size() {
            CAutoLock cAutoLock(m_pLock);
            return m_uSize;
        }

        void clear() {
            CAutoLock cAutoLock(m_pLock);
            TNode *node = m_pHead;
            TNode *temp = NULL;
            while (node != NULL) {
                temp = node->next;
                delete node;
                node = temp;
            }
            m_pHead = NULL;
            m_pTail = NULL;
            m_uSize = 0;
        }

    private:
        ILock *m_pLock;
        u32 m_uSize;
        TNode *m_pHead;
        TNode *m_pTail;
    };
}

#endif //_PAOMIANTV_SAFELIST_H_
