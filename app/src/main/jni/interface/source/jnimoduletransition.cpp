/*******************************************************************************
 *        Module: paomiantv
 *          File: jnimoduletransition.cpp
 * Functionality: transition jni.
 *       Related: mediasdk
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-31  v1.0        huangxuefeng  created
 ******************************************************************************/


#include <jnimodule.h>
#include <jnimoduletransition.h>
#include <jnimodulemanager.h>

namespace paomiantv {
    TJavaClazzParam *CJNIModuleTransition::GetJavaClazzParam() {
        TJavaClazzParam *ptJavaClazzParam = new TJavaClazzParam;
        JNINativeMethod arrMethods[] =
                {
                        {"_init",        "(Ljava/lang/String;JJ)Z", (void *) jni_init},
                        {"_uninit",      "()Z",                     (void *) jni_uninit},
                        {"_getStart",    "()J",                     (void *) jni_getStart},
                        {"_setStart",    "(J)V",                    (void *) jni_setStart},
                        {"_getDuration", "()J",                     (void *) jni_getDuration},
                        {"_setDuration", "(J)V",                    (void *) jni_setDuration}
                };
        ptJavaClazzParam->m_nMtdCount = NELEM(arrMethods);
        sprintf(ptJavaClazzParam->m_pchClazzName, "cn/paomiantv/mediasdk/module/PMTransition%s", "");
        ptJavaClazzParam->m_ptMethods = (JNINativeMethod *) malloc(sizeof(arrMethods));
        memcpy(ptJavaClazzParam->m_ptMethods, arrMethods, sizeof(arrMethods));
        return ptJavaClazzParam;
    }


    CJNIModuleTransition::CJNIModuleTransition(JNIEnv *env, jobject jTransition) {
        USE_LOG;

        if (env == NULL || jTransition == NULL) {
            LOGE("invalid parameters");
            return;
        }

        m_jObject = env->NewGlobalRef(jTransition);
        if (m_jObject == NULL) {
            LOGE("new global reference failed ,jvm stack table is full or unknown reason");
        }
        m_pTransition = new CTransition();
        if (m_pTransition != NULL) {
            LOGD("Transition instance allocated: %u", sizeof(CTransition));
            // only register valid ones
            CJNIModuleManager::getInstance()->add(this);
        } else {
            LOGE("new Transition failed ,memory is not enough!");
        }

    }

    CJNIModuleTransition::~CJNIModuleTransition() {
        USE_LOG;

        if (m_pTransition != NULL) {
            delete m_pTransition;
            m_pTransition = NULL;
            LOGD("Transition instance freed: %u", sizeof(CTransition));
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
    CJNIModuleTransition::jni_init(JNIEnv *env, jobject jTransition, jint jtype) {
        USE_LOG;
        CJNIModuleTransition *pJNITransition = CJNIModuleManager::getInstance()->createJniObject<CJNIModuleTransition>(
                env,
                jTransition);
        if (pJNITransition == NULL) {
            return FALSE;
        }
        pJNITransition->getTransition()->setType((EMTransition) jtype);
        return TRUE;
    }

    jboolean CJNIModuleTransition::jni_uninit(JNIEnv *env, jobject jTransition) {
        USE_LOG;
        CJNIModuleTransition *pJNITransition = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTransition>(env,
                                                                                              jTransition);
        if (pJNITransition == NULL) {
            return FALSE;
        } else {
            CJNIModuleManager::getInstance()->destroyJniObject(pJNITransition);
        }
        return TRUE;
    }

    jlong CJNIModuleTransition::jni_getStart(JNIEnv *env, jobject jTransition) {
        USE_LOG;
        CJNIModuleTransition *pJNITransition = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTransition>(env,
                                                                                      jTransition);
        if (pJNITransition == NULL) {
            return -1;
        }

        return pJNITransition->getTransition()->getStart();
    }

    void CJNIModuleTransition::jni_setStart(JNIEnv *env, jobject jTransition, jlong jstart) {
        USE_LOG;
        CJNIModuleTransition *pJNITransition = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTransition>(env,
                                                                                      jTransition);
        if (pJNITransition == NULL) {
            return;
        }
        pJNITransition->getTransition()->setStart(jstart);
    }

    jlong CJNIModuleTransition::jni_getDuration(JNIEnv *env, jobject jTransition) {
        USE_LOG;
        CJNIModuleTransition *pJNITransition = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTransition>(env,
                                                                                      jTransition);
        if (pJNITransition == NULL) {
            return -1;
        }
        return pJNITransition->getTransition()->getDuration();
    }

    void CJNIModuleTransition::jni_setDuration(JNIEnv *env, jobject jTransition, jlong jduration) {
        USE_LOG;
        CJNIModuleTransition *pJNITransition = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTransition>(env,
                                                                                      jTransition);
        if (pJNITransition == NULL) {
            return;
        }
        pJNITransition->getTransition()->setDuration(jduration);
    }
}