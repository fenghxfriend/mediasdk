//
// Created by ASUS on 2018/1/4.
//

#ifndef _PAOMIANTV_CJNIMODULEMUXER_H
#define _PAOMIANTV_CJNIMODULEMUXER_H


//field name
#include "../../common/autolock.h"
#include "jnimodule.h"
#include "../../mediasdk/io/include/muxer.h"

namespace paomiantv {
    class CJNIModuleMuxer : public CJNIModule {
    private:
        CMuxer *m_pMuxer;

    public:
        static TJavaClazzParam *GetJavaClazzParam();

        CJNIModuleMuxer(JNIEnv *env, jobject jMuxer);

        virtual ~CJNIModuleMuxer();

        inline CMuxer *getMuxer();

    private:


        static jboolean jni_init(JNIEnv *env, jobject jmuxer, jstring jsrc);

        static jboolean
        jni_setTags(JNIEnv *env, jobject jmuxer, jstring jdescription);

        static jboolean jni_uninit(JNIEnv *env, jobject jmuxer);

        static jstring jni_getDst(JNIEnv *env, jobject jmuxer);

        static jboolean jni_writeH264Frame(JNIEnv *env, jobject jmuxer, jbyteArray jbuffer,
                                           jint jbuferSize, jint jtype, jlong jpts,
                                           jboolean jisEos);

        static jboolean jni_writeAACFrame(JNIEnv *env, jobject jmuxer,jbyteArray jbuffer,
                                          jint jbuferSize, jlong jpts,
                                          jboolean jisEos);

        static jint jni_addH264VideoTrack(JNIEnv *env, jobject jmuxer, jint jwidth, jint jheight,
                                          jbyte jlevel, jbyteArray jsps, jshort jspsLen,
                                          jbyteArray jpps, jshort jppsLen);

        static jint jni_addAACAudioTrack(JNIEnv *env, jobject jmuxer, jint jSampleHZ, jbyte jlevel,
                                         jbyteArray jesds, jshort jesdsLen);
//
//        static void jni_copyFromSource(JNIEnv *env, jobject jmuxer, jstring jsource, jint jtype, jlong jstarttimeus,
//                       jlong jdurationus);
    };

    inline CMuxer *CJNIModuleMuxer::getMuxer() {
        return m_pMuxer;
    }
}


#endif //_PAOMIANTV_CJNIMODULEMUXER_H
