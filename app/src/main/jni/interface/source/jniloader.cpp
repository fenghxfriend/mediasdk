/*******************************************************************************
 *        Module: interface
 *          File: jniloader.cpp
 * Functionality: load all jni modules.
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-08-01  v1.0        huangxuefeng  created
 ******************************************************************************/
#include <jnimodulewebpdecoder.h>
#include <jnimoduletransition.h>
#include <jnimoduleengine.h>
#include <jnimodulestoryboard.h>
#include <jnimodulemuxer.h>
#include <jnimoduleplayer.h>
#include <jnimoduleexporter.h>
#include <jnimodulereverser.h>
#include "jniloader.h"
#include "RSAudioTrack.h"

namespace paomiantv {

    JavaVM *CJNIModule::m_spJVM = NULL;

    CJNILoader::Garbo CJNILoader::garbo; // 一定要初始化，不然程序结束时不会析构garbo

    CJNILoader *CJNILoader::m_pInstance = NULL;

    CJNILoader *CJNILoader::getInstance() {
        if (m_pInstance == NULL)
            m_pInstance = new CJNILoader();
        return m_pInstance;
    }

    CJNILoader::CJNILoader() {

    }

    CJNILoader::~CJNILoader() {

    }

    void CJNILoader::loadModule(JNIEnv *env, TJavaClazzParam *param) {
        if (param == NULL) {
            LOGE("load module: %s error!", param->m_pchClazzName);
            return;
        }
        env->PushLocalFrame(10);
        LOGI("load module: %s \n", param->m_pchClazzName);
        jclass clazz = env->FindClass(param->m_pchClazzName);
        if (clazz == NULL) {
            s8 *msg;
            asprintf(&msg, "Native registration unable to find class '%s', aborting",
                     param->m_pchClazzName);
            env->FatalError(msg);
            free(msg);
        }

        if (env->RegisterNatives(clazz, param->m_ptMethods, param->m_nMtdCount) < 0) {
            s8 *msg;
            asprintf(&msg, "RegisterNatives failed for '%s', aborting", param->m_pchClazzName);
            env->FatalError(msg);
            free(msg);
        }
        env->PopLocalFrame(NULL);
        free(param->m_ptMethods);
        delete param;
    }

    void CJNILoader::registerJavaClass(JNIEnv *env, void (*func)(void *)) {
        func(env);
    }

    s32 CJNILoader::loadModules(JavaVM *vm) {
        s32 result = -1;
        JNIEnv *env = NULL;
        if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) == JNI_OK) {
            CJNIModule::m_spJVM = vm;
            loadModule(env, CJNIModuleEngine::GetJavaClazzParam());
            loadModule(env, CJNIModuleStoryboard::GetJavaClazzParam());
            loadModule(env, CJNIModuleMultiTrack::GetJavaClazzParam());
            loadModule(env, CJNIModuleTrack::GetJavaClazzParam());
            loadModule(env, CJNIModuleEffect::GetJavaClazzParam());
            loadModule(env, CJNIModuleAnimation::GetJavaClazzParam());
            loadModule(env, CJNIModuleTransition::GetJavaClazzParam());
            loadModule(env, CJNIModuleMuxer::GetJavaClazzParam());
            loadModule(env, CJNIModulePlayer::GetJavaClazzParam());
            loadModule(env, CJNIModuleExporter::GetJavaClazzParam());
            loadModule(env, CJNIModuleWebpDecoder::GetJavaClazzParam());
            loadModule(env, CJNIModuleReverser::GetJavaClazzParam());
            RSAudioTrack::registerClass(env, "cn/paomiantv/mediasdk/media/audio/RSAudioTrack");
            result = JNI_VERSION_1_6;
        }
        return result;
    }

    void CJNILoader::unloadModule(JNIEnv *env, TJavaClazzParam *param) {
        if (param == NULL) {
            LOGE("unload module: %s error!", param->m_pchClazzName);
            return;
        }
        LOGI("unload module: %s", param->m_pchClazzName);
        jclass clazz = env->FindClass(param->m_pchClazzName);
        if (clazz == NULL) {
            s8 *msg;
            asprintf(&msg, "Native unregistration unable to find class '%s', aborting",
                     param->m_pchClazzName);
            env->FatalError(msg);
            free(msg);
        }

        if (env->UnregisterNatives(clazz) < 0) {
            s8 *msg;
            asprintf(&msg, "UnregisterNatives failed for '%s', aborting", param->m_pchClazzName);
            env->FatalError(msg);
            free(msg);
        }
        free(param->m_ptMethods);
        delete param;
    }

    void CJNILoader::unregisterJavaClass(JNIEnv *env, void (*func)(void *)) {
        func(env);
    }

    void CJNILoader::unloadModules(JavaVM *vm) {
        JNIEnv *env = NULL;
        if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
            return;
        }
        unloadModule(env, CJNIModuleEngine::GetJavaClazzParam());
        unloadModule(env, CJNIModuleStoryboard::GetJavaClazzParam());
        unloadModule(env, CJNIModuleMultiTrack::GetJavaClazzParam());
        unloadModule(env, CJNIModuleTrack::GetJavaClazzParam());
        unloadModule(env, CJNIModuleEffect::GetJavaClazzParam());
        unloadModule(env, CJNIModuleAnimation::GetJavaClazzParam());
        unloadModule(env, CJNIModuleTransition::GetJavaClazzParam());
        unloadModule(env, CJNIModuleMuxer::GetJavaClazzParam());
        unloadModule(env, CJNIModulePlayer::GetJavaClazzParam());
        unloadModule(env, CJNIModuleExporter::GetJavaClazzParam());
        unloadModule(env, CJNIModuleWebpDecoder::GetJavaClazzParam());
        unloadModule(env, CJNIModuleReverser::GetJavaClazzParam());
    }


}