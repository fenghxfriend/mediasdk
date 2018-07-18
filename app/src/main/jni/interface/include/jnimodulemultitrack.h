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
#ifndef _PAOMIANTV_JNIMODULEMULTITRACK_H
#define _PAOMIANTV_JNIMODULEMULTITRACK_H


#include <multitrack.h>
#include <jnimoduletrack.h>
#include <jnimoduletransition.h>

namespace paomiantv {

    class CJNIModuleMultiTrack : public CJNIModule {
    private:
        std::vector<CJNIModuleTrack *> m_vJNITracks;
        std::vector<CJNIModuleTransition *> m_vJNITransitions;

    public:
        static TJavaClazzParam *GetJavaClazzParam();

        CJNIModuleMultiTrack(JNIEnv *env, jobject jmultitrack);

        virtual ~CJNIModuleMultiTrack();

        virtual ITrack **getTrack();

    private:


        static jboolean jni_create(JNIEnv *env, jobject jmultitrack, jint jtype);

        static jboolean jni_destroy(JNIEnv *env, jobject jmultitrack);

        static jint jni_getType(JNIEnv *env, jobject jmultitrack);

        static jint jni_getId(JNIEnv *env, jobject jmultitrack);

        static jlong jni_getPlayStart(JNIEnv *env, jobject jmultitrack);

        static void jni_setPlayStart(JNIEnv *env, jobject jmultitrack, jlong jstart);


        static void jni_setPlayDuration(JNIEnv *env, jobject jmultitrack, jlong jduration);

        static jlong jni_getPlayDuration(JNIEnv *env, jobject jmultitrack);

        static jshort jni_getWeight(JNIEnv *env, jobject jmultitrack);

        static void jni_setWeight(JNIEnv *env, jobject jmultitrack, jshort jweight);

        static jshort jni_getZIndex(JNIEnv *env, jobject jmultitrack);

        static void jni_setZIndex(JNIEnv *env, jobject jmultitrack, jshort jZindex);

        static jobject jni_getTrack(JNIEnv *env, jobject jmultitrack, jint jindex);

        static jboolean jni_pushTrack(JNIEnv *env, jobject jmultitrack, jobject jtrack);

        static jobject jni_popTrack(JNIEnv *env, jobject jmultitrack, jboolean jisdestory);

        static jobject jni_removeTrack(JNIEnv *env, jobject jmultitrack, jint jindex);

        static jboolean
        jni_insertTrack(JNIEnv *env, jobject jmultitrack, jobject jtrack, jint jindex);

        static jint jni_getTrackCount(JNIEnv *env, jobject jmultitrack);

        static jboolean jni_addTransition(JNIEnv *env, jobject jmultitrack, jobject jtransition,
                                          jint jindex);

        static jobject jni_getTransition(JNIEnv *env, jobject jmultitrack, jint jindex);

        static jobject jni_removeTransition(JNIEnv *env, jobject jmultitrack, jint jindex);

        static jlong jni_getDataDuration(JNIEnv *env, jobject jmultitrack);

    private:
        jboolean pushTrack(CJNIModuleTrack *track);

        jboolean insertTrack(CJNIModuleTrack *pJNITrack, s32 index);

        jobject removeTrack(s32 nIndex);

        CJNIModuleTrack *getTrack(s32 position);

        jboolean addTransition(CJNIModuleTransition *transition, s32 nIndex);

        jobject removeTransition(s32 nIndex);

        CJNIModuleTransition *getTransition(s32 position);

        ITrack *m_pTrack;

        jobject popTrack(BOOL32 isShouldDestroy);
    };
}

#endif /* _PAOMIANTV_JNIMODULEMULTITRACK_H */