/*******************************************************************************
 *        Module: paomiantv
 *          File: 
 * Functionality: define jni engine module
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

#ifndef _PAOMIANTV_JNIMODULEENGINE_H_
#define _PAOMIANTV_JNIMODULEENGINE_H_

#include "jnimodule.h"
#include "engine.h"

namespace paomiantv {

    class CJNIModuleEngine : public CJNIModule {
//    private:
//        CEngine *m_pEngine;
    public:

        static TJavaClazzParam *GetJavaClazzParam();

        CJNIModuleEngine(JNIEnv *env, jobject jEngine);

        virtual ~CJNIModuleEngine();

//        inline CEngine *getEngine();

    private:


        static jboolean
        jni_init(JNIEnv *env, jobject jengine, jint jversion, jint jsampleRate, jint jbufferSize);

        static void jni_uninit(JNIEnv *env, jobject jengine);
    };

//    inline CEngine *CJNIModuleEngine::getEngine() {
//        return m_pEngine;
//    }
}

#endif /* _PAOMIANTV_JNIMODULEENGINE_H_ */