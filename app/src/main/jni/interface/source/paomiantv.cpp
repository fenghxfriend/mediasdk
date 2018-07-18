/*******************************************************************************
 *        Module: interface
 *          File: paomiantv.cpp
 * Functionality: load all jni modules.
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

#include <jniloader.h>
#include <autolog.h>
#include <version.h>

#if HW_MEDIACODEC_ENABLE
extern "C" {
#include <libavcodec/jni.h>
}
#endif

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGE("version: %u,build: %s %s", LIB_PAOMIANTV_VERSION, __DATE__, __TIME__);
#if HW_MEDIACODEC_ENABLE
    av_jni_set_java_vm(vm, NULL);//初始化ffmpeg的jni代码
#endif
    return paomiantv::CJNILoader::getInstance()->loadModules(vm);
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {
    paomiantv::CJNILoader::getInstance()->unloadModules(vm);
    return;
}