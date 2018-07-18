//
// Created by ASUS on 2018/2/8.
//

#ifndef MEDIAENGINE_JNIMODULEEXPORTER_H
#define MEDIAENGINE_JNIMODULEEXPORTER_H

//method name
#include <exporter.h>
#include "jnimodule.h"

#define EXPORTER_METHOD_ON_MESSAGE_NAME "onMessage"
//method signiture
#define EXPORTER_METHOD_ON_MESSAGE_SIG "(ILjava/lang/String;)V"


#define EXPORTER_METHOD_ON_WRITE_PCM_NAME "onWritePCM"
//method signiture
#define EXPORTER_METHOD_ON_WRITE_PCM_SIG "(J[BZ)V"

namespace paomiantv {
    class CJNIModuleExporter : public CJNIModule {

    private:

        CExporter *m_pExporter;
        jobject m_jVEncoder;
        jobject m_jAEncoder;

    public:
        static TJavaClazzParam *GetJavaClazzParam();

        CJNIModuleExporter(JNIEnv *env, jobject jPlayer);

        virtual ~CJNIModuleExporter();

        inline CExporter *getExporter();

    private:


        friend void JNIModuleExporter_OnMessage(void *delegate, s32 nId, s8 *message);

        friend void
        JNIModuleExporter_OnWritePCM(void *delegate, u64 uTimeStampUS, s32 nBufferSize,
                                     u8 *pBuffer, BOOL32 isEOS);

        static void jni_create(JNIEnv *env, jobject thiz);

//        static jboolean
//        jni_setVideoEncoderParams(JNIEnv *env, jobject thiz, jstring mime, jint width, jint height,
//                                  jint bitRate, jint framerate, jint iInterval, jint profile,
//                                  jint level);
//
//        static jboolean
//        jni_setAudioEncoderParams(JNIEnv *env, jobject thiz, jstring mime, jint sampleRate,
//                                  jint channels,
//                                  jint bitRate, jint profile);

        static void jni_setSurface(JNIEnv *env, jobject thiz, jobject jsurface, jint jwidth, jint jheight);

        static void jni_setDSStoryboard(JNIEnv *env, jobject thiz, jobject jstoryboard);

        static void jni_start(JNIEnv *env, jobject thiz);

        static void jni_stop(JNIEnv *env, jobject thiz);

        static void jni_release(JNIEnv *env, jobject thiz);

        void onMessage(s32 sllTimeStamp, s8 *msg);

        void onWrite(u64 uTimeStampUS, s32 nBufferSize, u8 *pBuffer, BOOL32 isEOS);
    };


/*
 * inline member functions implementation
 */
    inline CExporter *CJNIModuleExporter::getExporter() {
        return m_pExporter;
    }
}
#endif //MEDIAENGINE_JNIMODULEEXPORTER_H
