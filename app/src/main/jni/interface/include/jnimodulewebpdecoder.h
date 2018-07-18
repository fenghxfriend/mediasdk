//
// Created by ASUS on 2018/1/4.
//

#ifndef _PAOMIANTV_CJNIMODULEWEBPDECODER_H
#define _PAOMIANTV_CJNIMODULEWEBPDECODER_H


//field name
#include "jnimodule.h"
#include <webpdec.h>
#include <autolock.h>

namespace paomiantv {
    class CJNIModuleWebpDecoder : public CJNIModule {
    private:
        CWebpDec *m_pDecoder;

    public:
        static TJavaClazzParam *GetJavaClazzParam();

        CJNIModuleWebpDecoder(JNIEnv *env, jobject jDecoder);

        virtual ~CJNIModuleWebpDecoder();

        inline CWebpDec *getDecoder();

    private:
        static jboolean jni_create(JNIEnv *env, jobject jdecoder);

        static jboolean jni_destroy(JNIEnv *env, jobject jdecoder);

        static jboolean jni_setDataSource(JNIEnv *env, jobject jdecoder, jstring jsrc);

        static jstring jni_getSrc(JNIEnv *env, jobject jdecoder);

        static jint jni_getFrameCount(JNIEnv *env, jobject jdecoder);

        static jobject jni_getRGBAFrame(JNIEnv *env, jobject jdecoder, jint jindex);

        static jboolean jni_getFrame(JNIEnv *env, jobject jdecoder, jint jindex, jobject jbitmap);

        static jint jni_getCanvasWidth(JNIEnv *env, jobject jdecoder);

        static jint jni_getCanvasHeight(JNIEnv *env, jobject jdecoder);

        static jlong jni_getFrameDuration(JNIEnv *env, jobject jdecoder, jint jindex);
    };

    inline CWebpDec *CJNIModuleWebpDecoder::getDecoder() {
        return m_pDecoder;
    }
}


#endif //_PAOMIANTV_CJNIMODULEWEBPDECODER_H
