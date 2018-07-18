/*******************************************************************************
 *        Module: interface
 *          File:
 * Functionality: define jni transition modules.
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-31  v1.0        huangxuefeng  created
 ******************************************************************************/

#ifndef _PAOMIANTV_JNIMODULETRANSITION_H
#define _PAOMIANTV_JNIMODULETRANSITION_H

#include "jnimodule.h"
#include "transition.h"

namespace paomiantv {
    class CJNIModuleTransition : public CJNIModule {

    private:
        CTransition *m_pTransition;

    public:

        static TJavaClazzParam *GetJavaClazzParam();

        CJNIModuleTransition(JNIEnv *env, jobject jtransition);

        virtual ~CJNIModuleTransition();

        inline CTransition *getTransition();

    private:

        static jboolean jni_init(JNIEnv *env, jobject jtransition, jint jtype);

        static jboolean jni_uninit(JNIEnv *env, jobject jtransition);

        static jlong jni_getStart(JNIEnv *env, jobject jtransition);

        static void jni_setStart(JNIEnv *env, jobject jtransition, jlong jstart);

        static jlong jni_getDuration(JNIEnv *env, jobject jtransition);

        static void jni_setDuration(JNIEnv *env, jobject jtransition, jlong jduration);
    };

    inline CTransition *CJNIModuleTransition::getTransition() {
        return m_pTransition;
    }
}

#endif //_PAOMIANTV_JNIMODULETRANSITION_H
