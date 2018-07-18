/*******************************************************************************
 *        Module: interface
 *          File: jniloader.cpp
 * Functionality: define jni loader.
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

#ifndef _PAOMIANTV_JNILOADER_H
#define _PAOMIANTV_JNILOADER_H

#include <jni.h>
#include <jnimodule.h>

namespace paomiantv {

    class CJNILoader {
    private:
        CJNILoader();

        CJNILoader(const CJNILoader &);

        CJNILoader &operator=(const CJNILoader &);

        ~CJNILoader();

        static CJNILoader *m_pInstance;

        class Garbo {
        public:
            ~Garbo() {
                if (CJNILoader::m_pInstance) {
                    delete CJNILoader::m_pInstance;
                }
            }
        };

        void loadModule(JNIEnv *env, TJavaClazzParam *param);

        void registerJavaClass(JNIEnv *env, void (*func)(void *));

        void unloadModule(JNIEnv *env, TJavaClazzParam *param);

        void unregisterJavaClass(JNIEnv *env, void (*func)(void *));

        static Garbo garbo;

    public:
        static CJNILoader *getInstance();

        s32 loadModules(JavaVM *vm);

        void unloadModules(JavaVM *vm);
    };


}

#endif //_PAOMIANTV_JNILOADER_H
