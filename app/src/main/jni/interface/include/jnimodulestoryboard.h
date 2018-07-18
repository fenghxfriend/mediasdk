/*******************************************************************************
 *        Module: paomiantv
 *          File: 
 * Functionality: define jni storyboard module
 *       Related: 
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomaintv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-20  v1.0        huangxuefeng  created
 ******************************************************************************/
#ifndef _PAOMIANTV_JNIMODULESTORYBOARD_H_
#define _PAOMIANTV_JNIMODULESTORYBOARD_H_

#include "jnimoduletrack.h"
#include "storyboard.h"
#include "jnimodulemultitrack.h"

#define ERROR_SOURCE 10000
#define ERROR_SOURCE_DIST "Soure file is null or not existed!"

namespace paomiantv {

    class CJNIModuleStoryboard : public CJNIModule {
    private:
        CStoryboard *m_pStoryboard;
        std::unordered_map<CJNIModule *, ITrack **> m_mapJNITracks;
    public:
        static TJavaClazzParam *GetJavaClazzParam();

        CJNIModuleStoryboard(JNIEnv *env, jobject jStoryboard);

        virtual ~CJNIModuleStoryboard();

        inline CStoryboard **getCStoryboard();

    private:


        static jboolean jni_create(JNIEnv *env, jobject jstoryboard);

        static jboolean jni_destroy(JNIEnv *env, jobject jstoryboard);

        static jlong jni_getDuration(JNIEnv *env, jobject jstoryboard);

        static jboolean
        jni_addTrack(JNIEnv *env, jobject jstoryboard, jobject jtrack, jshort jzindex,
                     jboolean jiszindexenable);

        static jboolean
        jni_addMultiTrack(JNIEnv *env, jobject jstoryboard, jobject jmultitrack, jshort jzindex,
                          jboolean jiszindexenable);

        static jobject jni_removeTrack(JNIEnv *env, jobject jstoryboard, jint jid);

        static jobject jni_removeMultiTrack(JNIEnv *env, jobject jstoryboard, jint jid);

        static jobject jni_getTrack(JNIEnv *env, jobject jstoryboard, jint jid);

        static jobject jni_getMultiTrack(JNIEnv *env, jobject jstoryboard, jint jid);

        static jint jni_getTrackCount(JNIEnv *env, jobject jstoryboard, jint jtype);

        static jintArray jni_getAllTrackIds(JNIEnv *env, jobject jstoryboard);

        static jintArray jni_getTrackIds(JNIEnv *env, jobject jstoryboard, jint jtype);

        static jboolean jni_isMultiTrack(JNIEnv *env, jobject jstoryboard, jint jid);

        BOOL32 addTrack(CJNIModuleTrack *track, s16 wZIndex, BOOL32 bIsZIndexEnable);

        BOOL32 addMultiTrack(CJNIModuleMultiTrack *track, s16 wZIndex, BOOL32 bIsZIndexEnable);

        jobject removeTrack(s32 nid);

        jobject removeMultiTrack(s32 nid);

        BOOL32 getTrack(s32 id, CJNIModule *&pJNIModule, ITrack **&ppTrack);

        jintArray getTrackIds(EMTrack type);

        jintArray getAllTrackIds();
    };

    inline CStoryboard **CJNIModuleStoryboard::getCStoryboard() {
        return &m_pStoryboard;
    }
}

#endif /* _PAOMIANTV_JNIMODULESTORYBOARD_H_ */