//
// Created by ASUS on 2018/1/4.
//

#include <jnimodulemuxer.h>
#include <jnicommon.h>
#include <jnimodulemanager.h>

namespace paomiantv {

    TJavaClazzParam *CJNIModuleMuxer::GetJavaClazzParam() {
        TJavaClazzParam *ptJavaClazzParam = new TJavaClazzParam;
        JNINativeMethod arrMethods[] =
                {
                        {"_init",              "(Ljava/lang/String;)Z",    (void *) jni_init},
                        {"_setTags",           "(Ljava/lang/String;)Z",    (void *) jni_setTags},
                        {"_uninit",            "()Z",                      (void *) jni_uninit},
                        {"_getDst",            "()Ljava/lang/String;",     (void *) jni_getDst},
                        {"_writeH264Frame",    "([BIIJZ)Z",                (void *) jni_writeH264Frame},
                        {"_writeAACFrame",     "([BIJZ)Z",                 (void *) jni_writeAACFrame},
                        {"_addH264VideoTrack", "(IIB[BS[BS)I",             (void *) jni_addH264VideoTrack},
                        {"_addAACAudioTrack",  "(IB[BS)I",                 (void *) jni_addAACAudioTrack},
//                        {"_copyFromSource",    "(Ljava/lang/String;IJJ)V", (void *) jni_copyFromSource},
                };
        ptJavaClazzParam->m_nMtdCount = NELEM(arrMethods);
        sprintf(ptJavaClazzParam->m_pchClazzName, "cn/paomiantv/mediasdk/PMMuxer%s", "");
        ptJavaClazzParam->m_ptMethods = (JNINativeMethod *) malloc(sizeof(arrMethods));
        memcpy(ptJavaClazzParam->m_ptMethods, arrMethods, sizeof(arrMethods));
        return ptJavaClazzParam;
    }

    CJNIModuleMuxer::CJNIModuleMuxer(JNIEnv *env, jobject jMuxer) {
        USE_LOG;

        if (env == NULL || jMuxer == NULL) {
            LOGE("invalid parameters");
            return;
        }

        m_jObject = env->NewGlobalRef(jMuxer);
        if (m_jObject == NULL) {
            LOGE("new global reference failed ,jvm stack table is full or unknown reason");
        }
        m_pMuxer = new CMuxer;
        if (m_pMuxer != NULL) {
            LOGD("Muxer instance allocated: %u", sizeof(CMuxer));
            // only register valid ones
            CJNIModuleManager::getInstance()->add(this);
        } else {
            LOGE("new muxer failed ,memory is not enough!");
        }
    }

    CJNIModuleMuxer::~CJNIModuleMuxer() {
        USE_LOG;

        if (m_pMuxer != NULL) {
            delete m_pMuxer;
            m_pMuxer = NULL;
            LOGD("Muxer instance freed: %u", sizeof(CMuxer));
        }

        JNIEnv *env = NULL;
        if (m_spJVM->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
            LOGE("get JNIEnv failed");
            return;
        }

        if (m_jObject != NULL) {
            env->DeleteGlobalRef(m_jObject);
            m_jObject = NULL;
        }

        // be sure unregister before killing
        CJNIModuleManager::getInstance()->remove(this);
    }

    jboolean
    CJNIModuleMuxer::jni_init(JNIEnv *env, jobject jmuxer, jstring jdst) {
        CJNIModuleMuxer *pJNIMuxer = CJNIModuleManager::getInstance()->createJniObject<CJNIModuleMuxer>(
                env, jmuxer);
        if (pJNIMuxer == NULL || pJNIMuxer->getMuxer() == NULL) {
            return FALSE;
        }
        s8 achDstPath[MAX_LEN_FILE_PATH] = {0};
        if ((jdst == NULL) ||
            (getStringBytes(env, jdst, achDstPath, MAX_LEN_FILE_PATH) <= 0)) {
            LOGW("file path is null or char array is not enough!");
        }
        return (jboolean) (pJNIMuxer->getMuxer()->init(achDstPath) ? JNI_TRUE : JNI_FALSE);
    }

    jboolean CJNIModuleMuxer::jni_setTags(JNIEnv *env, jobject jmuxer, jstring jdescription) {
        CJNIModuleMuxer *pJNIMuxer = CJNIModuleManager::getInstance()->createJniObject<CJNIModuleMuxer>(
                env, jmuxer);
        if (pJNIMuxer == NULL || pJNIMuxer->getMuxer() == NULL) {
            return FALSE;
        }
        s8 achDsp[MAX_LEN_MP4_DESCRIPTION] = {0};
        if ((jdescription == NULL) ||
            (getStringBytes(env, jdescription, achDsp, MAX_LEN_MP4_DESCRIPTION) <= 0)) {
            LOGW("description is null or char array is not enough!");
        }
        return pJNIMuxer->getMuxer()->setDescription(achDsp);
    }

    jboolean CJNIModuleMuxer::jni_uninit(JNIEnv *env, jobject jmuxer) {
        CJNIModuleMuxer *pJNIMuxer = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMuxer>(
                env, jmuxer);
        if (pJNIMuxer == NULL) {
            return FALSE;
        } else {
            CJNIModuleManager::getInstance()->destroyJniObject(pJNIMuxer);
        }
        return TRUE;
    }

    jstring CJNIModuleMuxer::jni_getDst(JNIEnv *env, jobject jmuxer) {
        CJNIModuleMuxer *pJNIMuxer = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMuxer>(
                env, jmuxer);
        if (pJNIMuxer == NULL || pJNIMuxer->getMuxer() == NULL) {
            return NULL;
        }
        return stringToJString(env, pJNIMuxer->getMuxer()->getDst());
    }


    jboolean CJNIModuleMuxer::jni_writeH264Frame(JNIEnv *env, jobject jmuxer, jbyteArray jbuffer,
                                                 jint jbuferSize, jint jtype, jlong jpts,
                                                 jboolean jisEos) {
        CJNIModuleMuxer *pJNIMuxer = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMuxer>(
                env, jmuxer);
        if (pJNIMuxer == NULL || pJNIMuxer->getMuxer() == NULL || (jbuffer == NULL && !jisEos)) {
            return FALSE;
        }

        CH264Frame *pFrame = NULL;
        if (jbuffer != NULL) {
            jbyte *buffer = env->GetByteArrayElements(jbuffer, JNI_FALSE);
            if (env->GetArrayLength(jbuffer) >= jbuferSize && jbuferSize >= 0) {
                pFrame = CH264Frame::create();
                pFrame->reset(jbuferSize + 4);
                pFrame->offset = 4;
                pFrame->startTm = jpts;
                pFrame->isEos = jisEos;
                pFrame->size = jbuferSize;
                pFrame->type = (jtype <= EM_TYPE_KNOWN || jtype >= EM_TYPE_END) ? EM_TYPE_KNOWN
                                                                                : (EMH264FrameType) jtype;
                memcpy(pFrame->data + pFrame->offset, buffer, jbuferSize);
            }
            env->ReleaseByteArrayElements(jbuffer, buffer, 0);
        } else {
            pFrame = CH264Frame::create();
            pFrame->offset = 0;
            pFrame->startTm = jpts;
            pFrame->isEos = jisEos;
            pFrame->size = 0;
            pFrame->type = (jtype <= EM_TYPE_KNOWN || jtype >= EM_TYPE_END) ? EM_TYPE_KNOWN
                                                                            : (EMH264FrameType) jtype;
        }

        return pJNIMuxer->getMuxer()->writeH264Frame(pFrame);
    }

    jboolean CJNIModuleMuxer::jni_writeAACFrame(JNIEnv *env, jobject jmuxer, jbyteArray jbuffer,
                                                jint jbuferSize, jlong jpts,
                                                jboolean jisEos) {
        CJNIModuleMuxer *pJNIMuxer = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMuxer>(
                env, jmuxer);
        if (pJNIMuxer == NULL || pJNIMuxer->getMuxer() == NULL) {
            return FALSE;
        }
        CAACFrame *pFrame = NULL;
        jbyte *buffer = env->GetByteArrayElements(jbuffer, JNI_FALSE);
        if (env->GetArrayLength(jbuffer) >= jbuferSize && jbuferSize > 0) {
            pFrame = CAACFrame::create();
            pFrame->reset(jbuferSize);
            pFrame->offset = 0;
            pFrame->startTm = jpts;
            pFrame->isEos = jisEos;
            pFrame->size = jbuferSize;
            memcpy(pFrame->data + pFrame->offset, buffer, jbuferSize);
        }
        env->ReleaseByteArrayElements(jbuffer, buffer, 0);

        return pJNIMuxer->getMuxer()->writeAACFrame(pFrame);
    }

    jint
    CJNIModuleMuxer::jni_addH264VideoTrack(JNIEnv *env, jobject jmuxer, jint jwidth, jint jheight,
                                           jbyte jlevel, jbyteArray jsps, jshort jspsLen,
                                           jbyteArray jpps, jshort jppsLen) {
        CJNIModuleMuxer *pJNIMuxer = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMuxer>(
                env, jmuxer);
        if (pJNIMuxer == NULL || pJNIMuxer->getMuxer() == NULL || jpps == NULL || jsps == NULL) {
            return -1;
        }

        TH264Metadata th264Metadata;
        jbyte *sps = env->GetByteArrayElements(jsps, JNI_FALSE);
        if (env->GetArrayLength(jsps) >= jspsLen && jspsLen > 0) {
            memcpy(th264Metadata.m_abySps, sps, jspsLen);
            th264Metadata.m_wSpsSize = jspsLen;
        }
        env->ReleaseByteArrayElements(jsps, sps, 0);

        jbyte *pps = env->GetByteArrayElements(jpps, JNI_FALSE);
        if (env->GetArrayLength(jpps) >= jppsLen && jppsLen > 0) {
            memcpy(th264Metadata.m_abyPps, pps, jppsLen);
            th264Metadata.m_wPpsSize = jppsLen;
        }
        env->ReleaseByteArrayElements(jpps, pps, 0);

        return pJNIMuxer->getMuxer()->addH264VideoTrack(jwidth, jheight, jlevel, th264Metadata);
    }

    jint
    CJNIModuleMuxer::jni_addAACAudioTrack(JNIEnv *env, jobject jmuxer, jint jSampleHZ, jbyte jlevel,
                                          jbyteArray jesds, jshort jesdsLen) {
        CJNIModuleMuxer *pJNIMuxer = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMuxer>(
                env, jmuxer);
        if (pJNIMuxer == NULL || pJNIMuxer->getMuxer() == NULL || jesds == NULL) {
            return -1;
        }

        u8 abyESDS[MAX_ESDS_SIZE] = {0};
        jbyte *esds = env->GetByteArrayElements(jesds, JNI_FALSE);
        if (env->GetArrayLength(jesds) >= jesdsLen && jesdsLen > 0) {
            memcpy(abyESDS, esds, jesdsLen);
        }
        env->ReleaseByteArrayElements(jesds, esds, 0);

        return pJNIMuxer->getMuxer()->addAACAudioTrack(jSampleHZ, jlevel, abyESDS, jesdsLen);
    }

//    void
//    CJNIModuleMuxer::jni_copyFromSource(JNIEnv *env, jobject jmuxer, jstring jsource, jint jtype,
//                                        jlong jstarttimeus, jlong jdurationus) {
//        CJNIModuleMuxer *pJNIMuxer = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMuxer>(
//                env, jmuxer);
//        if (pJNIMuxer == NULL || pJNIMuxer->getMuxer() == NULL || jsource == NULL) {
//            return;
//        }
//        s8 achSource[MAX_LEN_FILE_PATH] = {0};
//        if ((getStringBytes(env, jsource, achSource, MAX_LEN_MP4_DESCRIPTION) <= 0)) {
//            LOGW("description is null or char array is not enough!");
//            return;
//        }
//        return pJNIMuxer->getMuxer()->copyFromSource(achSource, (EMTrack) jtype, jstarttimeus,
//                                                     jdurationus);
//    }
}