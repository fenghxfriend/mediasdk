/*******************************************************************************
 *        Module: paomiantv
 *          File: 
 * Functionality: define jni clip module
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
#ifndef _PAOMIANTV_JNIMODULETRACK_H
#define _PAOMIANTV_JNIMODULETRACK_H

#include <multitrack.h>
#include "jnimoduleeffect.h"
#include "jnimoduleanimation.h"

namespace paomiantv {

    class CJNIModuleTrack : public CJNIModule {
    private:
        std::vector<CJNIModuleEffect *> m_vJNIEffects;
        std::vector<CJNIModuleAnimation *> m_vJNIAnimations;

    public:
        static TJavaClazzParam *GetJavaClazzParam();

        CJNIModuleTrack(JNIEnv *env, jobject jtrack);

        virtual ~CJNIModuleTrack();


        virtual ITrack **getTrack();

    private:

        static jboolean jni_create(JNIEnv *env, jobject jtrack, jint jstype);

        static jboolean jni_destroy(JNIEnv *env, jobject jtrack);

        static jint jni_getType(JNIEnv *env, jobject jtrack);

        static jint jni_getId(JNIEnv *env, jobject jtrack);

        static jboolean jni_setDataSource_path(JNIEnv *env, jobject jtrack, jstring jpath);

        static jboolean jni_setDataSource_bitmap(JNIEnv *env, jobject jtrack, jobject jbitmap);

        static jboolean jni_setDataSource_webp(JNIEnv *env, jobject jtrack, jstring jpath);

        static jboolean jni_setDataSource_silence(JNIEnv *env, jobject jtrack);

        static jbyteArray jni_getSrc(JNIEnv *env, jobject jtrack);

        static jint jni_getSourceType(JNIEnv *env, jobject jtrack);

        static jint jni_getWidth(JNIEnv *env, jobject jtrack);

        static jint jni_getHeight(JNIEnv *env, jobject jtrack);

        static jlong jni_getCutStart(JNIEnv *env, jobject jtrack);

        static void jni_setCutStart(JNIEnv *env, jobject jtrack, jlong jstart);


        static void jni_setCutDuration(JNIEnv *env, jobject jtrack, jlong jduration);

        static jlong jni_getCutDuration(JNIEnv *env, jobject jtrack);


        static jlong jni_getPlayStart(JNIEnv *env, jobject jtrack);

        static void jni_setPlayStart(JNIEnv *env, jobject jtrack, jlong jstart);


        static void jni_setPlayDuration(JNIEnv *env, jobject jtrack, jlong jduration);

        static jlong jni_getPlayDuration(JNIEnv *env, jobject jtrack);


        static void jni_setVolume(JNIEnv *env, jobject jtrack, jfloat jvolume);

        static jfloat jni_getVolume(JNIEnv *env, jobject jtrack);


        static void jni_setPlaybackRate(JNIEnv *env, jobject jtrack, jfloat jrate);

        static jfloat jni_getPlaybackRate(JNIEnv *env, jobject jtrack);


        static void jni_setLoop(JNIEnv *env, jobject jtrack, jboolean jloop);

        static jboolean jni_isLoop(JNIEnv *env, jobject jtrack);

        static void jni_setShowFirstFrame(JNIEnv *env, jobject jtrack, jboolean jloop);

        static jboolean jni_isShowFirstFrame(JNIEnv *env, jobject jtrack);

        static void jni_setShowLastFrame(JNIEnv *env, jobject jtrack, jboolean jloop);

        static jboolean jni_isShowLastFrame(JNIEnv *env, jobject jtrack);

        static jboolean jni_isIndependent(JNIEnv *env, jobject jtrack);

        static jshort jni_getWeight(JNIEnv *env, jobject jtrack);

        static void jni_setWeight(JNIEnv *env, jobject jtrack, jshort jweight);

        static jshort jni_getZIndex(JNIEnv *env, jobject jtrack);

        static void jni_setZIndex(JNIEnv *env, jobject jtrack, jshort jZindex);


        static jobject jni_getEffect(JNIEnv *env, jobject jtrack, jint jindex);

        static jboolean jni_addEffect(JNIEnv *env, jobject jtrack, jobject jeffect);

        static jobject jni_removeEffect(JNIEnv *env, jobject jtrack, jint jposition);

        static jint jni_getEffectCount(JNIEnv *env, jobject jtrack);

        static jobject jni_getAnimation(JNIEnv *env, jobject jtrack, jint jindex);

        static jboolean jni_addAnimation(JNIEnv *env, jobject jtrack, jobject janimation);

        static jobject jni_removeAnimation(JNIEnv *env, jobject jtrack, jint jposition);

        static jint jni_getAnimationCount(JNIEnv *env, jobject jtrack);

        static jlong jni_getDataDuration(JNIEnv *env, jobject jtrack);

        BOOL32 addEffect(CJNIModuleEffect *effect);

        jobject removeEffect(s32 nIndex);

        CJNIModuleEffect *getEffect(s32 position);

        BOOL32 addAnimation(CJNIModuleAnimation *animation);

        jobject removeAnimation(s32 nIndex);

        CJNIModuleAnimation *getAnimation(s32 position);

    private:
        ITrack *m_pTrack;
    };
}

#endif /* _PAOMIANTV_JNIMODULECLIP_H */