/*******************************************************************************
 *        Module: paomiantv
 *          File: jnimoduleclip.cpp
 * Functionality: clip jni.
 *       Related: mediasdk
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-20  v1.0        huangxuefeng  created
 ******************************************************************************/



#include <autolock.h>
#include <jnimodulemanager.h>
#include <jnimodulemultitrack.h>

namespace paomiantv {

    TJavaClazzParam *CJNIModuleMultiTrack::GetJavaClazzParam() {
        TJavaClazzParam *ptJavaClazzParam = new TJavaClazzParam;
        JNINativeMethod arrMethods[] =
                {
                        {"_create",           "(I)Z",                                            (void *) jni_create},
                        {"_destroy",          "()Z",                                             (void *) jni_destroy},
                        {"_getId",            "()I",                                             (void *) jni_getId},
                        {"_getType",          "()I",                                             (void *) jni_getType},

                        {"_setPlayStart",     "(J)V",                                            (void *) jni_setPlayStart},
                        {"_getPlayStart",     "()J",                                             (void *) jni_getPlayStart},

                        {"_setPlayDuration",  "(J)V",                                            (void *) jni_setPlayDuration},
                        {"_getPlayDuration",  "()J",                                             (void *) jni_getPlayDuration},

                        {"_setWeight",        "(S)V",                                            (void *) jni_setWeight},
                        {"_getWeight",        "()S",                                             (void *) jni_getWeight},

                        {"_setZIndex",        "(S)V",                                            (void *) jni_setZIndex},
                        {"_getZIndex",        "()S",                                             (void *) jni_getZIndex},

                        {"_getTrack",         "(I)Lcn/paomiantv/mediasdk/module/PMTrack;",       (void *) jni_getTrack},

                        {"_pushTrack",        "(Lcn/paomiantv/mediasdk/module/PMTrack;)Z",       (void *) jni_pushTrack},
                        {"_popTrack",         "(Z)Lcn/paomiantv/mediasdk/module/PMTrack;",       (void *) jni_popTrack},

                        {"_insertTrack",      "(Lcn/paomiantv/mediasdk/module/PMTrack;I)Z",      (void *) jni_insertTrack},
                        {"_removeTrack",      "(I)Lcn/paomiantv/mediasdk/module/PMTrack;",       (void *) jni_removeTrack},

                        {"_getTrackCount",    "()I",                                             (void *) jni_getTrackCount},

                        {"_addTransition",    "(Lcn/paomiantv/mediasdk/module/PMTransition;I)Z", (void *) jni_addTransition},
                        {"_getTransition",    "(I)Lcn/paomiantv/mediasdk/module/PMTransition;",  (void *) jni_getTransition},
                        {"_removeTransition", "(I)Lcn/paomiantv/mediasdk/module/PMTransition;",  (void *) jni_removeTransition},
                        {"_getDataDuration",  "()J",                                             (void *) jni_getDataDuration},
                };
        ptJavaClazzParam->m_nMtdCount = NELEM(arrMethods);
        sprintf(ptJavaClazzParam->m_pchClazzName, "cn/paomiantv/mediasdk/module/PMMultiTrack%s",
                "");
        ptJavaClazzParam->m_ptMethods = (JNINativeMethod *) malloc(sizeof(arrMethods));
        memcpy(ptJavaClazzParam->m_ptMethods, arrMethods, sizeof(arrMethods));
        return ptJavaClazzParam;
    }

    CJNIModuleMultiTrack::CJNIModuleMultiTrack(JNIEnv *env, jobject jClip) : m_pTrack(NULL) {
        USE_LOG;
        if (env == NULL || jClip == NULL) {
            LOGE("invalid parameters");
            return;
        }

        m_jObject = env->NewGlobalRef(jClip);
        if (m_jObject == NULL) {
            LOGE("new global reference failed ,jvm stack table is full or unknown reason");
        }
        m_pTrack = new CMultiTrack();
        if (m_pTrack != NULL) {
            LOGD("CMultiTrack instance allocated: %u", sizeof(CMultiTrack));
            // only register valid ones
            CJNIModuleManager::getInstance()->add(this);
        } else {
            LOGE("new CMultiTrack failed ,memory is not enough!");
        }
    }

    CJNIModuleMultiTrack::~CJNIModuleMultiTrack() {
        USE_LOG;

        std::vector<CJNIModuleTrack *>::iterator iterTrack = m_vJNITracks.begin();

        while (iterTrack != m_vJNITracks.end()) {
            if (*iterTrack != NULL) {
                CJNIModuleTrack *pJNIMultiTrack = *iterTrack;
                ITrack **track = pJNIMultiTrack->getTrack();
                if (track != NULL && *track != NULL) {
                    ITrack *tc = NULL;
                    CTransition *tt = NULL;
                    ((CMultiTrack *) m_pTrack)->removeTrackById((*track)->getId(), tc, tt);
                }
                CJNIModuleManager::getInstance()->destroyJniObject(pJNIMultiTrack);
            }
            ++iterTrack;
        }
        m_vJNITracks.clear();

        std::vector<CJNIModuleTransition *>::iterator iterTrans = m_vJNITransitions.begin();

        while (iterTrans != m_vJNITransitions.end()) {
            if (*iterTrans != NULL) {
                CJNIModuleTransition *pJNITrans = *iterTrans;
                CTransition *transition = pJNITrans->getTransition();
                if (transition != NULL) {
                    CTransition *tt = NULL;
                    ((CMultiTrack *) m_pTrack)->replaceTransitionById(transition->getId(), tt);
                }
                CJNIModuleManager::getInstance()->destroyJniObject(pJNITrans);
            }
            ++iterTrans;
        }

        m_vJNITransitions.clear();
        if (m_pTrack != NULL) {
            delete m_pTrack;
            m_pTrack = NULL;
            LOGD("Track instance freed: %u", sizeof(CTrack));
        }

        JNIEnv *env = NULL;
        if (m_spJVM->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
            LOGE("get JNIEnv failed");
            return;
        }

        if (m_jObject != NULL) {
            env->DeleteGlobalRef(m_jObject);
            m_jObject = NULL;
        }

        // be sure unregister before killing
        CJNIModuleManager::getInstance()->remove(this);
    }

    jboolean
    CJNIModuleMultiTrack::jni_create(JNIEnv *env, jobject jmultitrack, jint jtype) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->createJniObject<CJNIModuleMultiTrack>(
                env, jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return FALSE;
        }
        (*(pJNIMultiTrack->getTrack()))->setType((EMTrack) jtype);
        return TRUE;
    }

    jboolean CJNIModuleMultiTrack::jni_destroy(JNIEnv *env, jobject jmultitrack) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return FALSE;
        } else {
            CJNIModuleManager::getInstance()->destroyJniObject(pJNIMultiTrack);
        }
        return TRUE;
    }

    jint CJNIModuleMultiTrack::jni_getType(JNIEnv *env, jobject jmultitrack) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return 0;
        }
        return (*(pJNIMultiTrack->getTrack()))->getType();
    }

    jint CJNIModuleMultiTrack::jni_getId(JNIEnv *env, jobject jmultitrack) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return 0;
        }
        return (*(pJNIMultiTrack->getTrack()))->getId();
    }


    jlong CJNIModuleMultiTrack::jni_getPlayStart(JNIEnv *env, jobject jmultitrack) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return 0;
        }

        return (*(pJNIMultiTrack->getTrack()))->getPlayStart();
    }

    void CJNIModuleMultiTrack::jni_setPlayStart(JNIEnv *env, jobject jmultitrack, jlong jstart) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return;
        }
        (*(pJNIMultiTrack->getTrack()))->setPlayStart(jstart);
    }

    jlong CJNIModuleMultiTrack::jni_getPlayDuration(JNIEnv *env, jobject jmultitrack) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return 0;
        }
        return (*(pJNIMultiTrack->getTrack()))->getPlayDuration();
    }

    void
    CJNIModuleMultiTrack::jni_setPlayDuration(JNIEnv *env, jobject jmultitrack, jlong jduration) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return;
        }
        (*(pJNIMultiTrack->getTrack()))->setPlayDuration(jduration);
    }

    jshort CJNIModuleMultiTrack::jni_getWeight(JNIEnv *env, jobject jmultitrack) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return -1;
        }
        return (*(pJNIMultiTrack->getTrack()))->getWeight();
    }

    void CJNIModuleMultiTrack::jni_setWeight(JNIEnv *env, jobject jmultitrack, jshort jweight) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return;
        }
        (*(pJNIMultiTrack->getTrack()))->setWeight(jweight);
    }

    jshort CJNIModuleMultiTrack::jni_getZIndex(JNIEnv *env, jobject jmultitrack) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return -1;
        }
        return (*(pJNIMultiTrack->getTrack()))->getZIndex();
    }

    void CJNIModuleMultiTrack::jni_setZIndex(JNIEnv *env, jobject jmultitrack, jshort jZindex) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return;
        }
        (*(pJNIMultiTrack->getTrack()))->setZIndex(jZindex);
    }


    jobject CJNIModuleMultiTrack::jni_getTrack(JNIEnv *env, jobject jmultitrack, jint jindex) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return NULL;
        }
        CJNIModuleTrack *pJNITrack = pJNIMultiTrack->getTrack(jindex);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL ||
            *(pJNITrack->getTrack()) == NULL) {
            return NULL;
        }
        return pJNITrack->getObject();
    }

    jboolean CJNIModuleMultiTrack::jni_pushTrack(JNIEnv *env, jobject jmultitrack, jobject jtrack) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return FALSE;
        }

        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL ||
            *(pJNITrack->getTrack()) == NULL) {
            return FALSE;
        }
        return pJNIMultiTrack->pushTrack(pJNITrack) ? TRUE : FALSE;
    }

    jobject
    CJNIModuleMultiTrack::jni_popTrack(JNIEnv *env, jobject jmultitrack, jboolean jisdestory) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return NULL;
        }
        return pJNIMultiTrack->popTrack(jisdestory);
    }

    jboolean
    CJNIModuleMultiTrack::jni_insertTrack(JNIEnv *env, jobject jmultitrack, jobject jtrack,
                                          jint jindex) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return FALSE;
        }

        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL ||
            *(pJNITrack->getTrack()) == NULL) {
            return FALSE;
        }

        return pJNIMultiTrack->insertTrack(pJNITrack, jindex);
    }

    jobject
    CJNIModuleMultiTrack::jni_removeTrack(JNIEnv *env, jobject jmultitrack, jint jindex) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return NULL;
        }
        return pJNIMultiTrack->removeTrack(jindex);
    }


    jint CJNIModuleMultiTrack::jni_getTrackCount(JNIEnv *env, jobject jmultitrack) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return 0;
        }
        return ((CMultiTrack *) (*(pJNIMultiTrack->getTrack())))->getTrackCount();
    }

    jboolean
    CJNIModuleMultiTrack::jni_addTransition(JNIEnv *env, jobject jmultitrack, jobject jtransition,
                                            jint jindex) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return FALSE;
        }
        CJNIModuleTransition *pJNITransition = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTransition>(
                env,
                jtransition);
        if (pJNITransition == NULL) {
            return FALSE;
        }
        return pJNIMultiTrack->addTransition(pJNITransition, jindex);
    }

    jobject
    CJNIModuleMultiTrack::jni_getTransition(JNIEnv *env, jobject jmultitrack, jint jposition) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return NULL;
        }
        CJNIModuleTransition *pJNITransition = pJNIMultiTrack->getTransition(jposition);
        if (pJNITransition == NULL) {
            return NULL;
        }
        return pJNITransition->getObject();
    }

    jobject
    CJNIModuleMultiTrack::jni_removeTransition(JNIEnv *env, jobject jmultitrack, jint jindex) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env,
                jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *(pJNIMultiTrack->getTrack()) == NULL) {
            return NULL;
        }
        return pJNIMultiTrack->removeTransition(jindex);
    }

    jlong CJNIModuleMultiTrack::jni_getDataDuration(JNIEnv *env, jobject jmultitrack) {
        CJNIModuleMultiTrack *pJNIMultiTrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env, jmultitrack);
        if (pJNIMultiTrack == NULL || pJNIMultiTrack->getTrack() == NULL ||
            *pJNIMultiTrack->getTrack() == NULL) {
            return 0;
        }
        return (*(pJNIMultiTrack->getTrack()))->getDataDuration() / 1000 + 1;
    }


    CJNIModuleTrack *CJNIModuleMultiTrack::getTrack(s32 position) {
        ITrack *pcTrack = NULL;
        ((CMultiTrack *) m_pTrack)->getTrack(pcTrack, position);
        if (pcTrack == NULL) {
            return NULL;
        }
        std::vector<CJNIModuleTrack *>::iterator iter;
        for (iter = m_vJNITracks.begin(); iter != m_vJNITracks.end();) {
            if (*iter != NULL && (*iter)->getTrack() != NULL && *((*iter)->getTrack()) == pcTrack) {
                return *iter;
            }
            ++iter;
        }
        return NULL;
    }

    jboolean CJNIModuleMultiTrack::pushTrack(CJNIModuleTrack *pJNITrack) {
        if (m_pTrack == NULL) {
            return FALSE;
        }
        m_vJNITracks.push_back(pJNITrack);


        return ((CMultiTrack *) m_pTrack)->pushTrack(*(pJNITrack->getTrack()));;
    }

    jobject CJNIModuleMultiTrack::popTrack(BOOL32 isShouldDestroy) {
        jobject re = NULL;
        if (m_pTrack == NULL) {
            return re;
        }

        ITrack *pcTrack = NULL;
        CTransition *pcTransition = NULL;
        ((CMultiTrack *) m_pTrack)->popTrack(pcTrack, pcTransition);
        if (pcTrack != NULL) {
            std::vector<CJNIModuleTrack *>::iterator iter;
            for (iter = m_vJNITracks.begin(); iter != m_vJNITracks.end();) {
                if (*iter != NULL && (*iter)->getTrack() != NULL &&
                    *((*iter)->getTrack()) == pcTrack) {
                    CJNIModuleTrack *pJNITrack = *iter;
                    iter = m_vJNITracks.erase(iter);
                    if (isShouldDestroy) {
                        CJNIModuleManager::getInstance()->destroyJniObject(pJNITrack);
                    } else {
                        re = pJNITrack->getObject();
                    }
                } else {
                    ++iter;
                }
            }
        }

        if (pcTransition != NULL) {
            std::vector<CJNIModuleTransition *>::iterator iter;
            for (iter = m_vJNITransitions.begin(); iter != m_vJNITransitions.end();) {
                if (*iter != NULL && (*iter)->getTransition() == pcTransition) {
                    CJNIModuleTransition *pJNITransition = *iter;
                    iter = m_vJNITransitions.erase(iter);
                    CJNIModuleManager::getInstance()->destroyJniObject(pJNITransition);
                } else {
                    ++iter;
                }
            }
        }

        return re;
    }

    jboolean CJNIModuleMultiTrack::insertTrack(CJNIModuleTrack *pJNITrack, s32 index) {
        if (m_pTrack == NULL) {
            return FALSE;
        }
        m_vJNITracks.push_back(pJNITrack);
        return ((CMultiTrack *) m_pTrack)->insertTrack(*(pJNITrack->getTrack()), index);;
    }


    jobject CJNIModuleMultiTrack::removeTrack(s32 nIndex) {
        if (m_pTrack == NULL) {
            return NULL;
        }

        ITrack *pcTrack = NULL;
        CTransition *pcTransition = NULL;
        ((CMultiTrack *) m_pTrack)->removeTrackByIndex(pcTrack, pcTransition, nIndex);
        if (pcTrack != NULL) {
            std::vector<CJNIModuleTrack *>::iterator iter;
            for (iter = m_vJNITracks.begin(); iter != m_vJNITracks.end();) {
                if (*iter != NULL && (*iter)->getTrack() != NULL &&
                    *((*iter)->getTrack()) == pcTrack) {
                    CJNIModuleTrack *pJNITrack = *iter;
                    iter = m_vJNITracks.erase(iter);
                    CJNIModuleManager::getInstance()->destroyJniObject(pJNITrack);
                } else {
                    ++iter;
                }
            }
        }

        if (pcTransition != NULL) {
            std::vector<CJNIModuleTransition *>::iterator iter;
            for (iter = m_vJNITransitions.begin(); iter != m_vJNITransitions.end();) {
                if (*iter != NULL && (*iter)->getTransition() == pcTransition) {
                    CJNIModuleTransition *pJNITransition = *iter;
                    iter = m_vJNITransitions.erase(iter);
                    CJNIModuleManager::getInstance()->destroyJniObject(pJNITransition);
                } else {
                    ++iter;
                }
            }
        }

        return NULL;
    }

    CJNIModuleTransition *CJNIModuleMultiTrack::getTransition(s32 position) {
        if (m_pTrack == NULL) {
            return NULL;
        }
        CTransition *pTransition = NULL;
        ((CMultiTrack *) m_pTrack)->getTransition(pTransition, position);
        if (pTransition == NULL) {
            return NULL;
        }
        std::vector<CJNIModuleTransition *>::iterator iter;
        for (iter = m_vJNITransitions.begin(); iter != m_vJNITransitions.end();) {
            if (*iter != NULL && (*iter)->getTransition() == pTransition) {
                return *iter;
            }
            ++iter;
        }
        return NULL;
    }


    jboolean CJNIModuleMultiTrack::addTransition(CJNIModuleTransition *transition, s32 nIndex) {
        if (m_pTrack == NULL) {
            return FALSE;
        }
        m_vJNITransitions.push_back(transition);
        CTransition *pTransition = transition->getTransition();
        ((CMultiTrack *) m_pTrack)->replaceTransitionByIndex(pTransition, nIndex);
        if (pTransition == NULL) {
            return FALSE;
        }
        std::vector<CJNIModuleTransition *>::iterator iter;
        for (iter = m_vJNITransitions.begin(); iter != m_vJNITransitions.end();) {
            if (*iter != NULL && (*iter)->getTransition() == pTransition) {
                CJNIModuleTransition *pJNITransition = *iter;
                iter = m_vJNITransitions.erase(iter);
                CJNIModuleManager::getInstance()->destroyJniObject(pJNITransition);
            } else {
                ++iter;
            }
        }
        return TRUE;
    }

    jobject CJNIModuleMultiTrack::removeTransition(s32 nIndex) {
        if (m_pTrack == NULL) {
            return NULL;
        }
        CTransition *pTransition = NULL;
        ((CMultiTrack *) m_pTrack)->replaceTransitionByIndex(pTransition, nIndex);
        if (pTransition == NULL) {
            return NULL;
        }
        std::vector<CJNIModuleTransition *>::iterator iter;
        for (iter = m_vJNITransitions.begin(); iter != m_vJNITransitions.end();) {
            if (*iter != NULL && (*iter)->getTransition() == pTransition) {
                CJNIModuleTransition *pJNITransition = *iter;
                iter = m_vJNITransitions.erase(iter);
                CJNIModuleManager::getInstance()->destroyJniObject(pJNITransition);
            } else {
                ++iter;
            }
        }
        return NULL;
    }

    ITrack **CJNIModuleMultiTrack::getTrack() {
        return &m_pTrack;
    }
}