//
// Created by ASUS on 2018/2/8.
//

#ifndef MEDIAENGINE_JNIMODULEPLAYER_H
#define MEDIAENGINE_JNIMODULEPLAYER_H

//method name
#include <player.h>
#include "jnimodule.h"

#define PLAYER_METHOD_ON_MESSAGE_NAME "onMessage"
//method signiture
#define PLAYER_METHOD_ON_MESSAGE_SIG "(ILjava/lang/String;)V"

namespace paomiantv {
    class CJNIModulePlayer : public CJNIModule {

    private:
        CPlayer *m_pPlayer;

    public:
        static TJavaClazzParam *GetJavaClazzParam();

        CJNIModulePlayer(JNIEnv *env, jobject jPlayer);

        virtual ~CJNIModulePlayer();

        inline CPlayer *getPlayer();

    private:


        friend void JNIModulePlayer_OnMessage(void *delegate, s32 nId, s8 *message);

        static void jni_create(JNIEnv *env, jobject thiz);

        static void jni_setDSStoryboard(JNIEnv *env, jobject thiz, jobject jstoryboard);

        static void jni_prepare(JNIEnv *env, jobject thiz);

        static void
        jni_bindSurface(JNIEnv *env, jobject thiz, jobject jsurface, jint jwidth, jint jheight);

        static void jni_surfaceSizeChanged(JNIEnv *env, jobject thiz, jint jwidth, jint jheight);

        static void jni_unbindSurface(JNIEnv *env, jobject thiz);

        static void jni_play(JNIEnv *env, jobject thiz);

        static void jni_pause(JNIEnv *env, jobject thiz);

        static void jni_seekTo(JNIEnv *env, jobject thiz, jlong jmicrosecond);

        static void jni_locPreview(JNIEnv *env, jobject thiz, jlong jmicrosecond);

        static void jni_resume(JNIEnv *env, jobject thiz);

        static void jni_stop(JNIEnv *env, jobject thiz);

        static void jni_release(JNIEnv *env, jobject thiz);

        void onMessage(s32 sllTimeStamp, s8 *msg);
    };


/*
 * inline member functions implementation
 */
    inline CPlayer *CJNIModulePlayer::getPlayer() {
        return m_pPlayer;
    }
}
#endif //MEDIAENGINE_JNIMODULEPLAYER_H
