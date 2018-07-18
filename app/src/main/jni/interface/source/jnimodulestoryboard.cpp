/*******************************************************************************
 *        Module: paomiantv
 *          File: jnimodulestoryboard.cpp
 * Functionality: storyboard jni.
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

#include <jnimodulestoryboard.h>
#include <jnimodulemanager.h>

namespace paomiantv {

    TJavaClazzParam *CJNIModuleStoryboard::GetJavaClazzParam() {
        TJavaClazzParam *ptJavaClazzParam = new TJavaClazzParam;
        JNINativeMethod arrMethods[] =
                {
                        {"_create",           "()Z",                                              (void *) jni_create},
                        {"_destroy",          "()Z",                                              (void *) jni_destroy},
                        {"_getDuration",      "()J",                                              (void *) jni_getDuration},
                        {"_addTrack",         "(Lcn/paomiantv/mediasdk/module/PMTrack;SZ)Z",      (void *) jni_addTrack},
                        {"_addMultiTrack",    "(Lcn/paomiantv/mediasdk/module/PMMultiTrack;SZ)Z", (void *) jni_addMultiTrack},
                        {"_removeTrack",      "(I)Lcn/paomiantv/mediasdk/module/PMTrack;",        (void *) jni_removeTrack},
                        {"_removeMultiTrack", "(I)Lcn/paomiantv/mediasdk/module/PMMultiTrack;",   (void *) jni_removeMultiTrack},
                        {"_getTrack",         "(I)Lcn/paomiantv/mediasdk/module/PMTrack;",        (void *) jni_getTrack},
                        {"_getMultiTrack",    "(I)Lcn/paomiantv/mediasdk/module/PMMultiTrack;",   (void *) jni_getMultiTrack},
                        {"_getAllTrackIds",   "()[I",                                             (void *) jni_getAllTrackIds},
                        {"_getTrackIds",      "(I)[I",                                            (void *) jni_getTrackIds},
                        {"_isMultiTrack",     "(I)Z",                                             (void *) jni_isMultiTrack},
                        {"_getTrackCount",    "(I)I",                                             (void *) jni_getTrackCount},
                };
        ptJavaClazzParam->m_nMtdCount = NELEM(arrMethods);
        sprintf(ptJavaClazzParam->m_pchClazzName, "cn/paomiantv/mediasdk/module/PMStoryboard%s",
                "");
        ptJavaClazzParam->m_ptMethods = (JNINativeMethod *) malloc(sizeof(arrMethods));
        memcpy(ptJavaClazzParam->m_ptMethods, arrMethods, sizeof(arrMethods));
        return ptJavaClazzParam;
    }

    CJNIModuleStoryboard::CJNIModuleStoryboard(JNIEnv *env, jobject jStoryboard) {
        USE_LOG;
        m_mapJNITracks.clear();
        if (env == NULL || jStoryboard == NULL) {
            LOGE("invalid parameters");
            return;
        }

        m_jObject = env->NewGlobalRef(jStoryboard);
        if (m_jObject == NULL) {
            LOGE("new global reference failed ,jvm stack table is full or unknown reason");
        }
        m_pStoryboard = new CStoryboard;
        if (m_pStoryboard != NULL) {
            LOGD("Storyboard instance allocated: %u", sizeof(CStoryboard));
            // only register valid ones
            CJNIModuleManager::getInstance()->add(this);
        } else {
            LOGE("new storyboard failed ,memory is not enough!");
        }

    }

    CJNIModuleStoryboard::~CJNIModuleStoryboard() {
        USE_LOG;

        for (std::unordered_map<CJNIModule *, ITrack **>::iterator iter = m_mapJNITracks.begin();
             iter != m_mapJNITracks.end(); ++iter) {
            CJNIModule *pJNITrack = iter->first;
            ITrack **track = iter->second;
            if (track != NULL && *track != NULL) {
                ITrack *t = NULL;
                m_pStoryboard->removeTrack((*track)->getId(), t);
            }
            CJNIModuleManager::getInstance()->destroyJniObject(pJNITrack);
        }
        m_mapJNITracks.clear();

        if (m_pStoryboard != NULL) {
            delete m_pStoryboard;
            m_pStoryboard = NULL;
            LOGD("Storyboard instance freed: %u", sizeof(CStoryboard));
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

    jboolean CJNIModuleStoryboard::jni_create(JNIEnv *env, jobject jstoryboard) {
        CJNIModuleStoryboard *pJNIStoryboard = CJNIModuleManager::getInstance()->createJniObject<CJNIModuleStoryboard>(
                env,
                jstoryboard);
        if (pJNIStoryboard == NULL || pJNIStoryboard->getCStoryboard() == NULL) {
            return FALSE;
        }
        return TRUE;
    }

    jboolean CJNIModuleStoryboard::jni_destroy(JNIEnv *env, jobject jstoryboard) {
        CJNIModuleStoryboard *pJNIStoryboard = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleStoryboard>(
                env,
                jstoryboard);
        if (pJNIStoryboard == NULL) {
            return FALSE;
        } else {
            CJNIModuleManager::getInstance()->destroyJniObject(pJNIStoryboard);
        }
        return TRUE;
    }

    jlong CJNIModuleStoryboard::jni_getDuration(JNIEnv *env, jobject jstoryboard) {
        CJNIModuleStoryboard *pJNIStoryboard = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleStoryboard>(
                env,
                jstoryboard);
        if (pJNIStoryboard == NULL || pJNIStoryboard->getCStoryboard() == NULL) {
            return FALSE;
        }
        return (*pJNIStoryboard->getCStoryboard())->getDuration() / 1000 + 1;
    }

    jboolean CJNIModuleStoryboard::jni_addTrack(JNIEnv *env, jobject jstoryboard, jobject jtrack,
                                                jshort jzindex, jboolean jiszindexenable) {
        CJNIModuleStoryboard *pJNIStoryboard = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleStoryboard>(
                env,
                jstoryboard);
        if (pJNIStoryboard == NULL || pJNIStoryboard->getCStoryboard() == NULL) {
            return FALSE;
        }
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL) {
            return FALSE;
        }
        return pJNIStoryboard->addTrack(pJNITrack, jzindex, jiszindexenable) ? TRUE : FALSE;
    }

    jboolean
    CJNIModuleStoryboard::jni_addMultiTrack(JNIEnv *env, jobject jstoryboard, jobject jmultitrack,
                                            jshort jzindex, jboolean jiszindexenable) {
        CJNIModuleStoryboard *pJNIStoryboard = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleStoryboard>(
                env,
                jstoryboard);
        if (pJNIStoryboard == NULL || pJNIStoryboard->getCStoryboard() == NULL) {
            return FALSE;
        }
        CJNIModuleMultiTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMultiTrack>(
                env, jmultitrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL) {
            return FALSE;
        }
        return pJNIStoryboard->addMultiTrack(pJNITrack, jzindex, jiszindexenable) ? TRUE : FALSE;
    }

    jobject CJNIModuleStoryboard::jni_removeTrack(JNIEnv *env, jobject jstoryboard, jint jid) {
        CJNIModuleStoryboard *pJNIStoryboard = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleStoryboard>(
                env,
                jstoryboard);
        if (pJNIStoryboard == NULL || pJNIStoryboard->getCStoryboard() == NULL) {
            return NULL;
        }

        return pJNIStoryboard->removeTrack(jid);
    }

    jobject CJNIModuleStoryboard::jni_removeMultiTrack(JNIEnv *env, jobject jstoryboard, jint jid) {
        CJNIModuleStoryboard *pJNIStoryboard = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleStoryboard>(
                env,
                jstoryboard);
        if (pJNIStoryboard == NULL || pJNIStoryboard->getCStoryboard() == NULL) {
            return NULL;
        }

        return pJNIStoryboard->removeMultiTrack(jid);
    }

    jobject CJNIModuleStoryboard::jni_getTrack(JNIEnv *env, jobject jstoryboard, jint jid) {
        CJNIModuleStoryboard *pJNIStoryboard = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleStoryboard>(
                env,
                jstoryboard);
        if (pJNIStoryboard == NULL || pJNIStoryboard->getCStoryboard() == NULL) {
            return NULL;
        }

        CJNIModule *pJNITrack = NULL;
        ITrack **pTrack = NULL;
        if (!pJNIStoryboard->getTrack(jid, pJNITrack, pTrack) || pJNITrack == NULL ||
            pTrack == NULL || *pTrack == NULL) {
            return NULL;
        }
//        jclass clz = env->GetObjectClass(pJNITrack->getObject());
//        jmethodID jmtd = env->GetMethodID(clz, "getName", "()Ljava/lang/String;");
//        if (jmtd == NULL) {
//            LOGE("java method 'void %s()' is not defined", "getClassName");
//            return NULL;
//        }
//        char name[128];
//        jobject jname = env->CallObjectMethod(pJNITrack->getObject(), jmtd);
//
//        getStringBytes(env, (jstring) jname, name, 128);
//        LOGE("the return object class is {%s}", name);

        return pJNITrack->getObject();
    }

    jobject CJNIModuleStoryboard::jni_getMultiTrack(JNIEnv *env, jobject jstoryboard, jint jid) {
        CJNIModuleStoryboard *pJNIStoryboard = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleStoryboard>(
                env,
                jstoryboard);
        if (pJNIStoryboard == NULL || pJNIStoryboard->getCStoryboard() == NULL) {
            return NULL;
        }
        CJNIModule *pJNITrack = NULL;
        ITrack **pTrack = NULL;
        if (!pJNIStoryboard->getTrack(jid, pJNITrack, pTrack) || pJNITrack == NULL ||
            pTrack == NULL || *pTrack == NULL) {
            return NULL;
        }
        return pJNITrack->getObject();
    }


    jintArray CJNIModuleStoryboard::jni_getTrackIds(JNIEnv *env, jobject jstoryboard, jint jtype) {
        CJNIModuleStoryboard *pJNIStoryboard = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleStoryboard>(
                env,
                jstoryboard);
        if (pJNIStoryboard == NULL || pJNIStoryboard->getCStoryboard() == NULL) {
            return 0;
        }

        return pJNIStoryboard->getTrackIds(static_cast<EMTrack>(jtype));
    }

    jintArray CJNIModuleStoryboard::jni_getAllTrackIds(JNIEnv *env, jobject jstoryboard) {
        CJNIModuleStoryboard *pJNIStoryboard = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleStoryboard>(
                env,
                jstoryboard);
        if (pJNIStoryboard == NULL || pJNIStoryboard->getCStoryboard() == NULL) {
            return 0;
        }

        return pJNIStoryboard->getAllTrackIds();
    }

    jint CJNIModuleStoryboard::jni_getTrackCount(JNIEnv *env, jobject jstoryboard, jint jtype) {
        if (jtype <= EM_TRACK_START || jtype >= EM_TRACK_END) {
            LOGE("invalid params!");
            return 0;
        }
        CJNIModuleStoryboard *pJNIStoryboard = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleStoryboard>(
                env,
                jstoryboard);
        if (pJNIStoryboard == NULL || pJNIStoryboard->getCStoryboard() == NULL) {
            return 0;
        }
        return (*pJNIStoryboard->getCStoryboard())->getTrackByType((EMTrack) jtype)->size();
    }

    jboolean CJNIModuleStoryboard::jni_isMultiTrack(JNIEnv *env, jobject jstoryboard, jint jid) {
        CJNIModuleStoryboard *pJNIStoryboard = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleStoryboard>(
                env,
                jstoryboard);
        if (pJNIStoryboard == NULL || pJNIStoryboard->getCStoryboard() == NULL) {
            return FALSE;
        }
        CJNIModule *pJNITrack = NULL;
        ITrack **pTrack = NULL;
        if (!pJNIStoryboard->getTrack(jid, pJNITrack, pTrack) || pJNITrack == NULL ||
            pTrack == NULL || *pTrack == NULL) {
            return FALSE;
        }
        return (*pTrack)->getSourceType() == EM_SOURCE_MULTI ? TRUE : FALSE;
    }


    BOOL32
    CJNIModuleStoryboard::addTrack(CJNIModuleTrack *track, s16 wZIndex, BOOL32 bIsZIndexEnable) {
        if (m_pStoryboard == NULL) {
            return FALSE;
        }
        m_mapJNITracks[track] = track->getTrack();
        return m_pStoryboard->addTrack(*track->getTrack(), wZIndex, bIsZIndexEnable);
    }

    BOOL32 CJNIModuleStoryboard::addMultiTrack(CJNIModuleMultiTrack *track, s16 wZIndex,
                                               BOOL32 bIsZIndexEnable) {
        if (m_pStoryboard == NULL) {
            return FALSE;
        }
        m_mapJNITracks[track] = track->getTrack();
        return m_pStoryboard->addTrack(*track->getTrack(), wZIndex, bIsZIndexEnable);
    }

    jobject CJNIModuleStoryboard::removeTrack(s32 nId) {
        if (m_pStoryboard == NULL) {
            return NULL;
        }
        ITrack *pTrack = NULL;
        m_pStoryboard->removeTrack(nId, pTrack);
        if (pTrack == NULL) {
            return NULL;
        }
        for (std::unordered_map<CJNIModule *, ITrack **>::iterator iter = m_mapJNITracks.begin();
             iter != m_mapJNITracks.end();) {
            if (iter->first != NULL && iter->second != NULL && (*iter->second) == pTrack) {
                CJNIModuleManager::getInstance()->destroyJniObject(iter->first);
                iter = m_mapJNITracks.erase(iter);
            } else {
                ++iter;
            }
        }
        return NULL;
    }

    jobject CJNIModuleStoryboard::removeMultiTrack(s32 nId) {
        if (m_pStoryboard == NULL) {
            return NULL;
        }
        ITrack *pTrack = NULL;
        m_pStoryboard->removeTrack(nId, pTrack);
        if (pTrack == NULL) {
            return NULL;
        }
        for (std::unordered_map<CJNIModule *, ITrack **>::iterator iter = m_mapJNITracks.begin();
             iter != m_mapJNITracks.end();) {
            if (iter->first != NULL && iter->second != NULL && (*iter->second) == pTrack) {
                CJNIModuleManager::getInstance()->destroyJniObject(iter->first);
                iter = m_mapJNITracks.erase(iter);
            } else {
                ++iter;
            }
        }
        return NULL;
    }

    BOOL32 CJNIModuleStoryboard::getTrack(s32 id, CJNIModule *&pJNIModule, ITrack **&ppTrack) {
        if (m_pStoryboard == NULL) {
            return FALSE;
        }
        for (std::unordered_map<CJNIModule *, ITrack **>::iterator iter = m_mapJNITracks.begin();
             iter != m_mapJNITracks.end(); ++iter) {
            if (iter->first != NULL && iter->second != NULL && (*iter->second) != NULL &&
                (*iter->second)->getId() == id) {
                pJNIModule = iter->first;
                ppTrack = iter->second;
                return TRUE;
            }
        }
        return FALSE;
    }

    jintArray CJNIModuleStoryboard::getTrackIds(EMTrack type) {
        JNIEnv *env = NULL;
        if (m_spJVM->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
            LOGE("get JNIEnv failed");
            return NULL;
        }
        jintArray array = NULL;
        env->PushLocalFrame(10);
        do {
            jclass clsIntArray = env->FindClass("[I");
            if (clsIntArray == NULL) {
                LOGE("can not find class [I")
                break;
            }
            int i = 0;
            jint *tmp = new jint[m_mapJNITracks.size()];
            for (std::unordered_map<CJNIModule *, ITrack **>::iterator iter = m_mapJNITracks.begin();
                 iter != m_mapJNITracks.end(); ++iter) {
                if (iter->first != NULL && iter->second != NULL && (*iter->second) != NULL &&
                    (*iter->second)->getType() == type) {
                    tmp[i] = ((*iter->second))->getId();
                    i++;
                }
            }
            array = env->NewIntArray(i);
            env->SetIntArrayRegion(array, 0, i, tmp);
            delete[]tmp;
        } while (0);
        return (jintArray) env->PopLocalFrame(array);
    }

    jintArray CJNIModuleStoryboard::getAllTrackIds() {
        JNIEnv *env = NULL;
        if (m_spJVM->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
            LOGE("get JNIEnv failed");
            return NULL;
        }
        jintArray array = NULL;
        env->PushLocalFrame(10);
        do {
            jclass clsIntArray = env->FindClass("[I");
            if (clsIntArray == NULL) {
                LOGE("can not find class [I")
                break;
            }
            int i = 0;
            jint *tmp = new jint[m_mapJNITracks.size()];
            for (std::unordered_map<CJNIModule *, ITrack **>::iterator iter = m_mapJNITracks.begin();
                 iter != m_mapJNITracks.end(); ++iter) {
                if (iter->first != NULL && iter->second != NULL && (*iter->second) != NULL) {
                    tmp[i] = (*iter->second)->getId();
                    i++;
                }
            }
            array = env->NewIntArray(i);
            env->SetIntArrayRegion(array, 0, i, tmp);
            delete[]tmp;
        } while (0);
        return (jintArray) env->PopLocalFrame(array);
    }
}
