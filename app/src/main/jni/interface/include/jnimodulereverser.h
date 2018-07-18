//
// Created by ASUS on 2018/2/8.
//

#ifndef MEDIAENGINE_JNIMODULEREVERSER_H
#define MEDIAENGINE_JNIMODULEREVERSER_H

//method name
#include <reverser.h>
#include "jnimodule.h"

#define EXPORTER_METHOD_ON_MESSAGE_NAME "onMessage"
//method signiture
#define EXPORTER_METHOD_ON_MESSAGE_SIG "(ILjava/lang/String;)V"


#define EXPORTER_METHOD_ON_WRITE_PCM_NAME "onWritePCM"
//method signiture
#define EXPORTER_METHOD_ON_WRITE_PCM_SIG "(J[BZ)V"

#define EXPORTER_METHOD_ON_START_VIDEO_ENCODE_NAME "onStartVEncode"
//method signiture
#define EXPORTER_METHOD_ON_START_VIDEO_ENCODE_SIG "(Ljava/lang/String;IIIIIII)Z"

#define EXPORTER_METHOD_ON_START_AUDIO_ENCODE_NAME "onStartAEncode"
//method signiture
#define EXPORTER_METHOD_ON_START_AUDIO_ENCODE_SIG "(Ljava/lang/String;IIII)Z"

//feild
#define MODULE_FIELD_MUXER_NAME "mPMMuxer"
#define MODULE_FIELD_MUXER_SIGNITURE "Lcn/paomiantv/mediasdk/PMMuxer;"

namespace paomiantv {
    class CJNIModuleReverser : public CJNIModule {

    private:

        CReverser *m_pReverser;
        jobject m_jVEncoder;
        jobject m_jAEncoder;

    public:
        static TJavaClazzParam *GetJavaClazzParam();

        CJNIModuleReverser(JNIEnv *env, jobject jPlayer);

        virtual ~CJNIModuleReverser();

        inline CReverser *getReverser();

    private:


        friend void JNIModuleReverser_OnMessage(void *delegate, s32 nId, s8 *message);

        friend void JNIModuleReverser_OnWritePCM(void *delegate, u64 uTimeStampUS, s32 nBufferSize,
                                                 u8 *pBuffer, BOOL32 isEOS);

        friend BOOL32
        JNIModuleReverser_OnStartVEncode(void *delegate,const s8 *mime, int width, int height,
                                         int bitRate, int framerate, int iInterval, int profile,
                                         int level);

        friend BOOL32 JNIModuleReverser_OnStartAEncode(void *delegate,const s8 *mime, int sampleRate,
                                                       int channels,
                                                       int bitRate, int profile);

        static void jni_create(JNIEnv *env, jobject thiz);


        static void
        jni_setSurface(JNIEnv *env, jobject thiz, jobject jsurface, jint jwidth, jint jheight);

        static void
        jni_reverse(JNIEnv *env, jobject thiz, jstring jtempPath, jstring jsrc,jboolean jisVideo, jboolean jisAudio);

        static void jni_release(JNIEnv *env, jobject thiz);

        void onMessage(s32 sllTimeStamp, s8 *msg);

        void onWrite(u64 uTimeStampUS, s32 nBufferSize, u8 *pBuffer, BOOL32 isEOS);

        BOOL32 onStartVEncode(const s8 *mime, int width, int height, int bitRate, int framerate, int iInterval,
                       int profile, int level);

        BOOL32 onStartAEncode(const s8 *mime, int sampleRate, int channels, int bitRate, int profile);
    };


/*
 * inline member functions implementation
 */
    inline CReverser *CJNIModuleReverser::getReverser() {
        return m_pReverser;
    }
}
#endif //MEDIAENGINE_JNIMODULEREVERSER_H
