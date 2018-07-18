/*******************************************************************************
 *        Module: interface
 *          File:
 * Functionality: define jni filter modules.
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

#ifndef _PAOMIANTV_JNIMODULEFILTER_H
#define _PAOMIANTV_JNIMODULEFILTER_H

//field name
#include <effect.h>
#include "jnimodule.h"

namespace paomiantv {
    class CJNIModuleEffect : public CJNIModule {

    private:
        CEffect *m_pEffect;

    public:
        static TJavaClazzParam *GetJavaClazzParam();

        CJNIModuleEffect(JNIEnv *env, jobject jeffect);

        virtual ~CJNIModuleEffect();

        inline CEffect *getEffect();

    private:

        static jboolean jni_init(JNIEnv *env, jobject jeffect, jint jtype, jobject jbitmap, jlong jstart, jlong jduration);

        static jboolean jni_uninit(JNIEnv *env, jobject jeffect);

        static jint jni_getType(JNIEnv *env, jobject jeffect);

        static void jni_update(JNIEnv *env, jobject jEffect, jint jtype, jbyteArray jbitmap);

        static jlong jni_getStart(JNIEnv *env, jobject jeffect);

        static void jni_setStart(JNIEnv *env, jobject jeffect, jlong jstart);

        static jlong jni_getDuration(JNIEnv *env, jobject jeffect);

        static void jni_setDuration(JNIEnv *env, jobject jeffect, jlong jduration);

        static jobject jni_getPicture(JNIEnv *env, jobject jEffect);
    };

    inline CEffect *CJNIModuleEffect::getEffect() {
        return m_pEffect;
    }

}

#endif //_PAOMIANTV_JNIMODULEFILTER_H
