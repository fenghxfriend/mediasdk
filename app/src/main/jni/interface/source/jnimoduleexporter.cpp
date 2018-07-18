//
// Created by ASUS on 2018/2/8.
//
#include <android/native_window_jni.h>
#include <autolock.h>
#include <jnimoduleexporter.h>
#include <jnimodulemanager.h>
#include <jnicommon.h>
#include <jnimodulestoryboard.h>

namespace paomiantv {

    void JNIModuleExporter_OnMessage(void *delegate, s32 nId, s8 *message) {
        if (delegate != NULL) {
            ((CJNIModuleExporter *) delegate)->onMessage(nId, message);
        }

    }

    void JNIModuleExporter_OnWritePCM(void *delegate, u64 uTimeStampUS, s32 nBufferSize,
                                      u8 *pBuffer, BOOL32 isEOS) {
        if (delegate != NULL) {
            ((CJNIModuleExporter *) delegate)->onWrite(uTimeStampUS, nBufferSize, pBuffer, isEOS);
        }

    }

    TJavaClazzParam *CJNIModuleExporter::GetJavaClazzParam() {
        TJavaClazzParam *ptJavaClazzParam = new TJavaClazzParam;
        JNINativeMethod arrMethods[] = {
                {"_create",           "()V",                                            (void *) jni_create},
                {"_setSurface",       "(Landroid/view/Surface;II)V",                    (void *) jni_setSurface},
                {"_setDataSourceSTB", "(Lcn/paomiantv/mediasdk/module/PMStoryboard;)V", (void *) jni_setDSStoryboard},
                {"_start",            "()V",                                            (void *) jni_start},
                {"_stop",             "()V",                                            (void *) jni_stop},
                {"_release",          "()V",                                            (void *) jni_release}};

        ptJavaClazzParam->m_nMtdCount = NELEM(arrMethods);
        sprintf(ptJavaClazzParam->m_pchClazzName, "cn/paomiantv/mediasdk/PMExporter%s", "");
        ptJavaClazzParam->m_ptMethods = (JNINativeMethod *) malloc(sizeof(arrMethods));
        memcpy(ptJavaClazzParam->m_ptMethods, arrMethods, sizeof(arrMethods));
        return ptJavaClazzParam;
    }

    CJNIModuleExporter::CJNIModuleExporter(JNIEnv *env, jobject jPlayer) : m_jVEncoder(NULL),
                                                                           m_jAEncoder(NULL) {
        USE_LOG;
        if (env == NULL || jPlayer == NULL) {
            LOGE("invalid parameters");
            return;
        }

        m_jObject = env->NewGlobalRef(jPlayer);
        if (m_jObject == NULL) {
            LOGE("new global reference failed ,jvm stack table is full or unknown reason");
        }
        m_pExporter = new CExporter;
        if (m_pExporter != NULL) {
            LOGD("exporter instance allocated: %u", sizeof(CExporter));
            m_pExporter->setOnCB(this, JNIModuleExporter_OnMessage,
                                 JNIModuleExporter_OnWritePCM);
        }
        // only register valid ones
        CJNIModuleManager::getInstance()->add(this);
    }

// FIXME: when exit the application, it will sometimes prints this error log:
// E/libEGL(19049): call to OpenGL ES API with no current context (logged once per thread)
// -- noted by huangxuefeng, 2013-07-29
    CJNIModuleExporter::~CJNIModuleExporter() {
        USE_LOG;

        if (m_pExporter != NULL) {
            m_pExporter->setOnCB(NULL, NULL, NULL);
            delete m_pExporter;
            m_pExporter = NULL;
            LOGD("exporter instance freed: %u", sizeof(CRenderer));
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
        if (m_jAEncoder != NULL) {
            env->DeleteGlobalRef(m_jAEncoder);
            m_jAEncoder = NULL;
        }
        if (m_jVEncoder != NULL) {
            env->DeleteGlobalRef(m_jVEncoder);
            m_jVEncoder = NULL;
        }
        // be sure unregister before killing
        CJNIModuleManager::getInstance()->remove(this);
    }

    void CJNIModuleExporter::onMessage(s32 nId, s8 *msg) {
        if (!m_spJVM || !m_jObject)
            return;
        JNIEnv *env = NULL;
        bool isAttacked = false;
        int status = (m_spJVM)->GetEnv((void **) &env, JNI_VERSION_1_6);
        if (status != JNI_OK) {

            status = m_spJVM->AttachCurrentThread(&env, NULL);
            if (status != JNI_OK) {
                LOGE("failed to attach current thread");
                return;
            }
            isAttacked = true;
        }

        env->PushLocalFrame(10);
        do {
            jclass jcls = env->GetObjectClass(m_jObject);
            if (jcls == NULL) {
                LOGE("get java class failed");
                break;
            }
            jmethodID jmtd = env->GetMethodID(jcls, EXPORTER_METHOD_ON_MESSAGE_NAME,
                                              EXPORTER_METHOD_ON_MESSAGE_SIG);
            if (jmtd == NULL) {
                LOGE("java method 'void %s()' is not defined", EXPORTER_METHOD_ON_MESSAGE_NAME);
                break;
            }
            env->CallVoidMethod(m_jObject, jmtd, nId, stringToJString(env, msg));
        } while (0);

        env->PopLocalFrame(NULL);

        if (isAttacked) {
            m_spJVM->DetachCurrentThread();
        }
    }

    void CJNIModuleExporter::onWrite(u64 uTimeStampUS, s32 nBufferSize,
                                     u8 *pBuffer, BOOL32 isEOS) {
        if (!m_spJVM || !m_jObject)
            return;
        JNIEnv *env = NULL;
        bool isAttacked = false;
        int status = (m_spJVM)->GetEnv((void **) &env, JNI_VERSION_1_6);
        if (status != JNI_OK) {

            status = m_spJVM->AttachCurrentThread(&env, NULL);
            if (status != JNI_OK) {
                LOGE("failed to attach current thread");
                return;
            }
            isAttacked = true;
        }

        env->PushLocalFrame(10);
        do {
            jclass jcls = env->GetObjectClass(m_jObject);
            if (jcls == NULL) {
                LOGE("get java class failed");
                break;
            }
            jmethodID jmtd = env->GetMethodID(jcls, EXPORTER_METHOD_ON_WRITE_PCM_NAME,
                                              EXPORTER_METHOD_ON_WRITE_PCM_SIG);
            if (jmtd == NULL) {
                LOGE("java method 'void %s()' is not defined",
                     EXPORTER_METHOD_ON_WRITE_PCM_NAME);
                break;
            }
            env->CallVoidMethod(m_jObject, jmtd, uTimeStampUS,
                                StringToJByteArr(env, pBuffer, nBufferSize), isEOS);
        } while (0);

        env->PopLocalFrame(NULL);

        if (isAttacked) {
            m_spJVM->DetachCurrentThread();
        }
    }

    void CJNIModuleExporter::jni_create(JNIEnv *env, jobject thiz) {
        CJNIModuleExporter *pJNIExporter = CJNIModuleManager::getInstance()->createJniObject<CJNIModuleExporter>(
                env, thiz);
        if (pJNIExporter == NULL || pJNIExporter->getExporter() == NULL) {
            LOGE("create exporter failed!");
        }
    }
//
//    jboolean CJNIModuleExporter::jni_setVideoEncoderParams(JNIEnv *env, jobject thiz,
//                                                           jstring jmime,
//                                                           jint jwidth, jint jheight,
//                                                           jint jbitRate,
//                                                           jint jframerate, jint jiInterval,
//                                                           jint jprofile,
//                                                           jint jlevel) {
//        CJNIModuleExporter *pJNIExporter = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleExporter>(
//                env, thiz);
//        if (pJNIExporter == NULL || pJNIExporter->getExporter() == NULL) {
//            LOGE("get exporter failed, exporter is invalid or released!");
//            return FALSE;
//        } else {
//            s8 achMime[MAX_LEN_MIME] = {0};
//            if ((jmime == NULL) ||
//                getStringBytes(env, (jstring) jmime, achMime, MAX_LEN_FILE_PATH) <= 0) {
//                LOGE("file path is null or char array is not enough!");
//                return FALSE;
//            }
//            pJNIExporter->getExporter()->setVideoEncoderParams(
//                    achMime, jwidth, jheight, jbitRate, jframerate, jiInterval, jprofile, jlevel);
//
//        }
//    }
//
//    jboolean CJNIModuleExporter::jni_setAudioEncoderParams(JNIEnv *env, jobject thiz, jstring jmime,
//                                                           jint jsampleRate, jint jchannels,
//                                                           jint jbitRate,
//                                                           jint jprofile) {
//        CJNIModuleExporter *pJNIExporter = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleExporter>(
//                env, thiz);
//        if (pJNIExporter == NULL || pJNIExporter->getExporter() == NULL) {
//            LOGE("get exporter failed, exporter is invalid or released!");
//            return FALSE;
//        } else {
//            s8 achMime[MAX_LEN_MIME] = {0};
//            if ((jmime == NULL) ||
//                getStringBytes(env, (jstring) jmime, achMime, MAX_LEN_FILE_PATH) <= 0) {
//                LOGE("file path is null or char array is not enough!");
//                return FALSE;
//            }
//            return pJNIExporter->getExporter()->setAudioEncoderParams(achMime, jsampleRate,
//                                                                      jchannels,
//                                                                      jbitRate, jprofile);
//        }
//    }

    void
    CJNIModuleExporter::jni_setSurface(JNIEnv *env, jobject thiz, jobject jsurface, jint jwidth,
                                       jint jheight) {
        ANativeWindow *win = ANativeWindow_fromSurface(env, jsurface);
        if (win == NULL) {
            LOGE("get native window failed!");
            return;
        }
        CJNIModuleExporter *pJNIExporter = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleExporter>(
                env, thiz);
        if (pJNIExporter == NULL || pJNIExporter->getExporter() == NULL) {
            ANativeWindow_release(win);
            LOGE("get exporter failed, exporter is invalid or released!");
        } else {
            pJNIExporter->getExporter()->bindSurface(win, jwidth, jheight);
        }
    }


    void CJNIModuleExporter::jni_setDSStoryboard(JNIEnv *env, jobject thiz, jobject jstoryboard) {
        CJNIModuleExporter *pJNIExporter = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleExporter>(
                env, thiz);
        if (pJNIExporter == NULL || pJNIExporter->getExporter() == NULL) {
            LOGE("get exporter failed, exporter is invalid or released!");
        } else {
            CJNIModuleStoryboard *pJNIStoryboard = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleStoryboard>(
                    env,
                    jstoryboard);
            if (pJNIStoryboard == NULL || pJNIStoryboard->getCStoryboard() == NULL) {
                LOGE("get storyboard failed, storyboard is invalid or released!");
                return;
            }
            pJNIExporter->getExporter()->setDataSource(pJNIStoryboard->getCStoryboard());
        }
    }

    void CJNIModuleExporter::jni_start(JNIEnv *env, jobject thiz) {
        CJNIModuleExporter *pJNIExporter = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleExporter>(
                env, thiz);
        if (pJNIExporter == NULL || pJNIExporter->getExporter() == NULL) {
            LOGE("get exporter failed, exporter is invalid or released!");
        } else {
            pJNIExporter->getExporter()->start();
        }
    }

    void CJNIModuleExporter::jni_stop(JNIEnv *env, jobject thiz) {
        CJNIModuleExporter *pJNIExporter = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleExporter>(
                env, thiz);
        if (pJNIExporter == NULL || pJNIExporter->getExporter() == NULL) {
            LOGE("get exporter failed, exporter is invalid or released!");
        } else {
            pJNIExporter->getExporter()->stop();
        }
    }

    void CJNIModuleExporter::jni_release(JNIEnv *env, jobject thiz) {
        CJNIModuleExporter *pJNIExporter = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleExporter>(
                env, thiz);
        if (pJNIExporter != NULL) {
            CJNIModuleManager::getInstance()->destroyJniObject(pJNIExporter);
        }
    }
}
