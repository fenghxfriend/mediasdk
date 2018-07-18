/*******************************************************************************
 *        Module: mediasdk
 *          File: storyboard.cpp
 * Functionality: storyboard entity.
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
#include <frame.h>
#include <storyboard.h>
#include <constant.h>

namespace paomiantv {
    u32 CStoryboard::m_sCount = 1;

    CStoryboard::CStoryboard() :
            m_uId(m_sCount),
            m_wZIndex(1) {
        USE_LOG;
        m_pLock = new CLock;
        for (int i = 0; i < EM_TRACK_END; i++) {
            m_pTrackLists[i] = new CSafeList<ITrack>;
            m_pTrackLists[i]->clear();
        }
        if (m_sCount == UNSIGNED_INTEGER32_MAX_VALUE) {
            LOGE("the id is overflowed! please restart app!");
        } else {
            m_sCount++;
        }
    }

    CStoryboard::~CStoryboard() {
        USE_LOG;
        for (int i = 0; i < EM_TRACK_END; i++) {
            if (m_pTrackLists[i] != NULL) {
                while (!m_pTrackLists[i]->empty()) {
                    ITrack *t = m_pTrackLists[i]->pop();
                    if (t != NULL) {
                        delete t;
                    }
                }
                delete m_pTrackLists[i];
            }
        }
        if (m_pLock != NULL) {
            delete m_pLock;
        }
    }

    BOOL32 CStoryboard::addTrack(ITrack *pTrack, s16 wZIndex, BOOL32 bIsIndexEnable) {
        CAutoLock autoLock(m_pLock);
        if (pTrack != NULL &&
            m_pTrackLists[pTrack->getType()] != NULL &&
            !m_pTrackLists[pTrack->getType()]->contain(pTrack)) {
            m_pTrackLists[pTrack->getType()]->push(pTrack);
            if (bIsIndexEnable) {
                pTrack->setZIndex(wZIndex);
                m_pTrackLists[pTrack->getType()]->sort(ITrack::compareByZindexCB);
            } else {
                pTrack->setZIndex(m_wZIndex);
                m_wZIndex++;
            }

            return TRUE;
        } else {
            LOGE("track is invalid or has contained the track!!!")
        }
        return FALSE;
    }

    s32 CStoryboard::getTrackCount() {
        s32 re = 0;
        CAutoLock autoLock(m_pLock);
        for (int i = 0; i < EM_TRACK_END; i++) {
            re += m_pTrackLists[i]->size();
        }
        return re;
    }

    BOOL32 CStoryboard::removeTrack(s32 id, ITrack *&pTrack) {
        ITrack *tmp = NULL;
        CAutoLock autoLock(m_pLock);
        for (int i = 0; i < EM_TRACK_END; i++) {
            if ((tmp = m_pTrackLists[i]->remove(id, ITrack::compareByIdCB)) != NULL) {
                pTrack = tmp;
            }
        }
        return TRUE;
    }

    BOOL32 CStoryboard::getTrack(const s32 id, ITrack *&pTrack) {
        ITrack *tmp = NULL;
        CAutoLock autoLock(m_pLock);
        for (int i = 0; i < EM_TRACK_END; i++) {
            if ((tmp = m_pTrackLists[i]->get(id, ITrack::compareByIdCB)) != NULL) {
                pTrack = tmp;
            }
        }
        return TRUE;
    }

    CSafeList<ITrack> *CStoryboard::getTrackByType(EMTrack type) {
        CAutoLock autoLock(m_pLock);
        return m_pTrackLists[type];
    }

    static ITrack *getValidTrack(CSafeList<ITrack> *list) {
        ITrack *ret = NULL;
        u32 index = 0;
        s64 duration = 0;
        while (index < list->size()) {
            ITrack *temp = list->get(index);
            if (temp != NULL && temp->isSourceValid()) {
                switch (temp->getSourceType()) {
                    case EM_SOURCE_BITMAP:
                    case EM_SOURCE_SILENCE: {
                        if (temp->getDataDuration() != INTEGER32_MAX_VALUE &&
                            (temp->getPlayStart() * 1000 + temp->getDataDuration() > duration)) {
                            duration = temp->getPlayStart() * 1000 + temp->getDataDuration();
                            ret = temp;
                        }
                    }
                        break;
                    case EM_SOURCE_FILE:
                    case EM_SOURCE_WEBP:
                    case EM_SOURCE_MULTI: {
                        if (temp->getPlayStart() * 1000 + temp->getDataDuration() > duration) {
                            duration = temp->getPlayStart() * 1000 + temp->getDataDuration();
                            ret = temp;
                        }
                    }
                        break;
                    default:
                        LOGE("track%u is invalid!!!!!!", temp->getId());
                        break;
                }
            }
            index++;
        }
        return ret;
    }

    s64 CStoryboard::getDuration() {
        s64 duration = 0;
        ITrack *track = NULL;
        for (int i = 0; i < EM_TRACK_END; i++) {
            track = getValidTrack(m_pTrackLists[i]);
            if (track != NULL) {
                s64 pd = track->getPlayDuration();
                if (pd < 0) {
                    duration = MAX((track->getPlayStart() * 1000 + track->getDataDuration()),
                                   duration);
                } else {
                    duration = MAX((track->getPlayStart() * 1000 + track->getPlayDuration() * 1000),
                                   duration);
                }
            }
        }
        return duration;
    }
} // namespace paomiantv
