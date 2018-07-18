/*******************************************************************************
 *        Module: interface
 *          File: jnimoduleengine.cpp
 * Functionality: engine jni.
 *       Related: mediasdk
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2013 360ANTS, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-20  v1.0        huangxuefeng  created
 ******************************************************************************/

#include <autolock.h>
#include <jnimoduleengine.h>
#include <jnimodulemanager.h>

namespace paomiantv {

    TJavaClazzParam *CJNIModuleEngine::GetJavaClazzParam() {
        TJavaClazzParam *ptJavaClazzParam = new TJavaClazzParam;
        JNINativeMethod arrMethods[] =
                {
                        {"_init",   "(III)Z", (void *) jni_init},
                        {"_uninit", "()V",    (void *) jni_uninit}};
        ptJavaClazzParam->m_nMtdCount = NELEM(arrMethods);
        sprintf(ptJavaClazzParam->m_pchClazzName, "cn/paomiantv/mediasdk/PMEngine%s", "");
        ptJavaClazzParam->m_ptMethods = (JNINativeMethod *) malloc(sizeof(arrMethods));
        memcpy(ptJavaClazzParam->m_ptMethods, arrMethods, sizeof(arrMethods));
        return ptJavaClazzParam;
    }

    CJNIModuleEngine::CJNIModuleEngine(JNIEnv *env, jobject jEngine) {
        USE_LOG;

        if (env == NULL || jEngine == NULL) {
            LOGE("invalid parameters");
            return;
        }

        m_jObject = env->NewGlobalRef(jEngine);
        if (m_jObject == NULL) {
            LOGE("new global reference failed ,jvm stack table is full or unknown reason");
        }
//        m_pEngine = CEngine::getInstance();
//        if (m_pEngine != NULL) {
//            LOGD("Engine instance allocated: %u", sizeof(CEngine));
//            // only register valid ones
//            CJNIModuleManager::getInstance()->add(this);
//        } else {
//            LOGE("new filter failed ,memory is not enough!");
//        }
        CJNIModuleManager::getInstance()->add(this);
    }

    CJNIModuleEngine::~CJNIModuleEngine() {
        USE_LOG;

//        if (m_pEngine != NULL) {
//            delete m_pEngine;
//            m_pEngine = NULL;
//            LOGD("Engine instance freed: %u", sizeof(CEngine));
//        }

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
    CJNIModuleEngine::jni_init(JNIEnv *env, jobject jengine, jint jversion, jint jsampleRate,
                               jint jbufferSize) {
        USE_LOG;
        CJNIModuleEngine *pJNIEngine = CJNIModuleManager::getInstance()->createJniObject<CJNIModuleEngine>(
                env, jengine);
//        if (pJNIEngine == NULL || pJNIEngine->getEngine() == NULL) {
//            return FALSE;
//        }
//
//        return pJNIEngine->getEngine()->init(jversion, jsampleRate, jbufferSize) ? TRUE : FALSE;

        if (pJNIEngine == NULL) {
            return FALSE;
        }

        return CEngine::getInstance()->init(jversion, jsampleRate, jbufferSize) ? TRUE : FALSE;
    }

    void CJNIModuleEngine::jni_uninit(JNIEnv *env, jobject jengine) {
        USE_LOG;
        CJNIModuleEngine *pJNIEngine = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleEngine>(
                env, jengine);
//        if (pJNIEngine == NULL || pJNIEngine->getEngine() == NULL) {
//            return;
//        }
//        pJNIEngine->getEngine()->uninit();
        if (pJNIEngine == NULL) {
            return;
        }
        CEngine::getInstance()->uninit();
        CJNIModuleManager::getInstance()->destroyJniObject(pJNIEngine);
        CJNIModuleManager::getInstance()->clear();
    }
}
