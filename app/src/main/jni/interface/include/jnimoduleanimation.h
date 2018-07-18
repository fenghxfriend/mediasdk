/*******************************************************************************
 *        Module: interface
 *          File:
 * Functionality: define jni animation modules.
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

#ifndef _PAOMIANTV_JNIMODULEANIMATION_H
#define _PAOMIANTV_JNIMODULEANIMATION_H

#include <animation.h>
#include "jnimodule.h"

namespace paomiantv {
    class CJNIModuleAnimation : public CJNIModule {

    private:
        CAnimation *m_pAnimation;

    public:

        static TJavaClazzParam *GetJavaClazzParam();

        CJNIModuleAnimation(JNIEnv *env, jobject jAnimation);

        virtual ~CJNIModuleAnimation();

        inline CAnimation *getAnimation();


    private:

        static jboolean jni_init(JNIEnv *env, jobject jAnimation);

        static jboolean jni_uninit(JNIEnv *env, jobject jAnimation);

        static jlong jni_get_start(JNIEnv *env, jobject jAnimation);

        static void jni_set_start(JNIEnv *env, jobject jAnimation, jlong jstart);

        static jlong jni_get_duration(JNIEnv *env, jobject jAnimation);

        static void jni_set_duration(JNIEnv *env, jobject jAnimation, jlong jdurantion);

        static jfloat jni_get_start_X(JNIEnv *env, jobject jAnimation);

        static void jni_set_start_X(JNIEnv *env, jobject jAnimation, jfloat jstartX);

        static jfloat jni_get_start_Y(JNIEnv *env, jobject jAnimation);

        static void jni_set_start_Y(JNIEnv *env, jobject jAnimation, jfloat jstartY);

        static jfloat jni_get_start_degree_Z(JNIEnv *env, jobject jAnimation);

        static void jni_set_start_degree_Z(JNIEnv *env, jobject jAnimation, jfloat jstartDegree);

        static jfloat jni_get_start_scale_X(JNIEnv *env, jobject jAnimation);

        static void jni_set_start_scale_X(JNIEnv *env, jobject jAnimation, jfloat jstart);

        static jfloat jni_get_start_scale_Y(JNIEnv *env, jobject jAnimation);

        static void jni_set_start_scale_Y(JNIEnv *env, jobject jAnimation, jfloat jstart);

        static jfloat jni_get_start_alpha(JNIEnv *env, jobject jAnimation);

        static void jni_set_start_alpha(JNIEnv *env, jobject jAnimation, jfloat jstart);


        static jfloat jni_get_end_X(JNIEnv *env, jobject jAnimation);

        static void jni_set_end_X(JNIEnv *env, jobject jAnimation, jfloat jstart);

        static jfloat jni_get_end_Y(JNIEnv *env, jobject jAnimation);

        static void jni_set_end_Y(JNIEnv *env, jobject jAnimation, jfloat jstart);

        static jfloat jni_get_end_degree_Z(JNIEnv *env, jobject jAnimation);

        static void jni_set_end_degree_Z(JNIEnv *env, jobject jAnimation, jfloat jstart);

        static jfloat jni_get_end_scale_X(JNIEnv *env, jobject jAnimation);

        static void jni_set_end_scale_X(JNIEnv *env, jobject jAnimation, jfloat jstart);

        static jfloat jni_get_end_scale_Y(JNIEnv *env, jobject jAnimation);

        static void jni_set_end_scale_Y(JNIEnv *env, jobject jAnimation, jfloat jstart);

        static jfloat jni_get_end_alpha(JNIEnv *env, jobject jAnimation);

        static void jni_set_end_alpha(JNIEnv *env, jobject jAnimation, jfloat jstart);

        static jfloat jni_get_crop_start_X(JNIEnv *env, jobject jAnimation);

        static void jni_set_crop_start_X(JNIEnv *env, jobject jAnimation, jfloat jcropstartX);

        static jfloat jni_get_crop_start_Y(JNIEnv *env, jobject jAnimation);

        static void jni_set_crop_start_Y(JNIEnv *env, jobject jAnimation, jfloat jcropstartY);

        static jfloat jni_get_crop_start_degree_Z(JNIEnv *env, jobject jAnimation);

        static void
        jni_set_crop_start_degree_Z(JNIEnv *env, jobject jAnimation, jfloat jcropstartDegree);

        static jfloat jni_get_crop_start_scale_X(JNIEnv *env, jobject jAnimation);

        static void
        jni_set_crop_start_scale_X(JNIEnv *env, jobject jAnimation, jfloat jcropstartScaleX);

        static jfloat jni_get_crop_start_scale_Y(JNIEnv *env, jobject jAnimation);

        static void
        jni_set_crop_start_scale_Y(JNIEnv *env, jobject jAnimation, jfloat jcropstartScaleY);

        static jfloat jni_get_crop_end_X(JNIEnv *env, jobject jAnimation);

        static void jni_set_crop_end_X(JNIEnv *env, jobject jAnimation, jfloat jcropendX);

        static jfloat jni_get_crop_end_Y(JNIEnv *env, jobject jAnimation);

        static void jni_set_crop_end_Y(JNIEnv *env, jobject jAnimation, jfloat jcropendY);

        static jfloat jni_get_crop_end_degree_Z(JNIEnv *env, jobject jAnimation);

        static void
        jni_set_crop_end_degree_Z(JNIEnv *env, jobject jAnimation, jfloat jcropendDegree);

        static jfloat jni_get_crop_end_scale_X(JNIEnv *env, jobject jAnimation);

        static void
        jni_set_crop_end_scale_X(JNIEnv *env, jobject jAnimation, jfloat jcropendScaleX);

        static jfloat jni_get_crop_end_scale_Y(JNIEnv *env, jobject jAnimation);

        static void
        jni_set_crop_end_scale_Y(JNIEnv *env, jobject jAnimation, jfloat jcropendScaleY);
    };

    inline CAnimation *CJNIModuleAnimation::getAnimation() {
        return m_pAnimation;
    }
}

#endif //_PAOMIANTV_JNIMODULEANIMATION_H
