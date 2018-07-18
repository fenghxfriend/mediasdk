//
// Created by ASUS on 2018/2/8.
//
#include <android/native_window_jni.h>
#include <autolock.h>
#include <jnimodulereverser.h>
#include <jnimodulemanager.h>
#include <jnicommon.h>
#include <jnimodulestoryboard.h>
#include <jnimodulemuxer.h>

namespace paomiantv {

    void JNIModuleReverser_OnMessage(void *delegate, s32 nId, s8 *message) {
        if (delegate != NULL) {
            ((CJNIModuleReverser *) delegate)->onMessage(nId, message);
        }

    }

    void JNIModuleReverser_OnWritePCM(void *delegate, u64 uTimeStampUS, s32 nBufferSize,
                                      u8 *pBuffer, BOOL32 isEOS) {
        if (delegate != NULL) {
            ((CJNIModuleReverser *) delegate)->onWrite(uTimeStampUS, nBufferSize, pBuffer, isEOS);
        }

    }

    BOOL32
    JNIModuleReverser_OnStartVEncode(void *delegate,const s8 *mime, int width, int height, int bitRate,
                                     int framerate, int iInterval, int profile, int level) {
        BOOL32 re = FALSE;
        if (delegate != NULL) {
            re = ((CJNIModuleReverser *) delegate)->onStartVEncode(mime, width, height, bitRate,
                                                                   framerate, iInterval, profile,
                                                                   level);
        }
        return re;
    }

    BOOL32 JNIModuleReverser_OnStartAEncode(void *delegate,const s8 *mime, int sampleRate, int channels,
                                            int bitRate, int profile) {
        BOOL32 re = FALSE;
        if (delegate != NULL) {
            re = ((CJNIModuleReverser *) delegate)->onStartAEncode(mime, sampleRate, channels,
                                                                   bitRate, profile);
        }
        return re;
    }

    TJavaClazzParam *CJNIModuleReverser::GetJavaClazzParam() {
        TJavaClazzParam *ptJavaClazzParam = new TJavaClazzParam;
        JNINativeMethod arrMethods[] = {
                {"_create",     "()V",                                       (void *) jni_create},
                {"_setSurface", "(Landroid/view/Surface;II)V",               (void *) jni_setSurface},
                {"_reverse",    "(Ljava/lang/String;Ljava/lang/String;ZZ)V", (void *) jni_reverse},
                {"_release",    "()V",                                       (void *) jni_release}};

        ptJavaClazzParam->m_nMtdCount = NELEM(arrMethods);
        sprintf(ptJavaClazzParam->m_pchClazzName, "cn/paomiantv/mediasdk/PMReverser%s", "");
        ptJavaClazzParam->m_ptMethods = (JNINativeMethod *) malloc(sizeof(arrMethods));
        memcpy(ptJavaClazzParam->m_ptMethods, arrMethods, sizeof(arrMethods));
        return ptJavaClazzParam;
    }

    CJNIModuleReverser::CJNIModuleReverser(JNIEnv *env, jobject jPlayer) : m_jVEncoder(NULL),
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
        m_pReverser = new CReverser;
        if (m_pReverser != NULL) {
            LOGD("reverser instance allocated: %u", sizeof(CReverser));
            m_pReverser->setOnCB(this,
                                 JNIModuleReverser_OnMessage,
                                 JNIModuleReverser_OnWritePCM,
                                 JNIModuleReverser_OnStartVEncode,
                                 JNIModuleReverser_OnStartAEncode);
        }
        // only register valid ones
        CJNIModuleManager::getInstance()->add(this);
    }

// FIXME: when exit the application, it will sometimes prints this error log:
// E/libEGL(19049): call to OpenGL ES API with no current context (logged once per thread)
// -- noted by huangxuefeng, 2013-07-29
    CJNIModuleReverser::~CJNIModuleReverser() {
        USE_LOG;

        if (m_pReverser != NULL) {
            m_pReverser->setOnCB(NULL, NULL, NULL);
            delete m_pReverser;
            m_pReverser = NULL;
            LOGD("reverser instance freed: %u", sizeof(CRenderer));
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

    void CJNIModuleReverser::onMessage(s32 nId, s8 *msg) {
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

    void CJNIModuleReverser::onWrite(u64 uTimeStampUS, s32 nBufferSize,
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


    BOOL32 CJNIModuleReverser::onStartVEncode(const s8 *mime,
                                              int width, int height,
                                              int bitRate,
                                              int framerate, int iInterval,
                                              int profile,
                                              int level) {
        if (!m_spJVM || !m_jObject)
            return FALSE;
        JNIEnv *env = NULL;
        bool isAttacked = false;
        int status = (m_spJVM)->GetEnv((void **) &env, JNI_VERSION_1_6);
        if (status != JNI_OK) {

            status = m_spJVM->AttachCurrentThread(&env, NULL);
            if (status != JNI_OK) {
                LOGE("failed to attach current thread");
                return FALSE;
            }
            isAttacked = true;
        }
        jboolean re = FALSE;
        env->PushLocalFrame(10);
        do {
            jclass jcls = env->GetObjectClass(m_jObject);
            if (jcls == NULL) {
                LOGE("get java class failed");
                break;
            }
            jmethodID jmtd = env->GetMethodID(jcls, EXPORTER_METHOD_ON_START_VIDEO_ENCODE_NAME,
                                              EXPORTER_METHOD_ON_START_VIDEO_ENCODE_SIG);
            if (jmtd == NULL) {
                LOGE("java method 'void %s()' is not defined",
                     EXPORTER_METHOD_ON_START_VIDEO_ENCODE_NAME);
                break;
            }
            re = env->CallBooleanMethod(m_jObject, jmtd, stringToJString(env, mime),
                                        width, height, bitRate,
                                        framerate, iInterval, profile,
                                        level);
        } while (0);

        env->PopLocalFrame(NULL);

        if (isAttacked) {
            m_spJVM->DetachCurrentThread();
        }
        return re;
    }

    BOOL32 CJNIModuleReverser::onStartAEncode(const s8 *mime,
                                              int sampleRate, int channels,
                                              int bitRate,
                                              int profile) {
        if (!m_spJVM || !m_jObject)
            return FALSE;
        JNIEnv *env = NULL;
        bool isAttacked = false;
        int status = (m_spJVM)->GetEnv((void **) &env, JNI_VERSION_1_6);
        if (status != JNI_OK) {

            status = m_spJVM->AttachCurrentThread(&env, NULL);
            if (status != JNI_OK) {
                LOGE("failed to attach current thread");
                return FALSE;
            }
            isAttacked = true;
        }
        jboolean re = FALSE;
        env->PushLocalFrame(10);
        do {
            jclass jcls = env->GetObjectClass(m_jObject);
            if (jcls == NULL) {
                LOGE("get java class failed");
                break;
            }
            jmethodID jmtd = env->GetMethodID(jcls, EXPORTER_METHOD_ON_START_AUDIO_ENCODE_NAME,
                                              EXPORTER_METHOD_ON_START_AUDIO_ENCODE_SIG);
            if (jmtd == NULL) {
                LOGE("java method 'void %s()' is not defined",
                     EXPORTER_METHOD_ON_START_AUDIO_ENCODE_NAME);
                break;
            }
            re = env->CallBooleanMethod(m_jObject, jmtd, stringToJString(env, mime), sampleRate,
                                        channels,
                                        bitRate, profile);
        } while (0);

        env->PopLocalFrame(NULL);

        if (isAttacked) {
            m_spJVM->DetachCurrentThread();
        }
        return re;
    }

    void CJNIModuleReverser::jni_create(JNIEnv *env, jobject thiz) {
        CJNIModuleReverser *pJNIReverser = CJNIModuleManager::getInstance()->createJniObject<CJNIModuleReverser>(
                env, thiz);
        if (pJNIReverser == NULL || pJNIReverser->getReverser() == NULL) {
            LOGE("create reverser failed!");
        }
    }

    void
    CJNIModuleReverser::jni_setSurface(JNIEnv *env, jobject thiz, jobject jsurface, jint jwidth,
                                       jint jheight) {
        ANativeWindow *win = ANativeWindow_fromSurface(env, jsurface);
        if (win == NULL) {
            LOGE("get native window failed!");
            return;
        }
        CJNIModuleReverser *pJNIReverser = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleReverser>(
                env, thiz);
        if (pJNIReverser == NULL || pJNIReverser->getReverser() == NULL) {
            ANativeWindow_release(win);
            LOGE("get reverser failed, reverser is invalid or released!");
        } else {
            pJNIReverser->getReverser()->bindSurface(win, jwidth, jheight);
        }
    }

    void
    CJNIModuleReverser::jni_reverse(JNIEnv *env, jobject thiz, jstring jtempPath, jstring jsrc,
                                    jboolean jisVideo,
                                    jboolean jisAudio) {
        CJNIModuleReverser *pJNIReverser = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleReverser>(
                env, thiz);
        if (pJNIReverser == NULL || pJNIReverser->getReverser() == NULL) {
            LOGE("get reverser failed, reverser is invalid or released!");
            return;
        }
        s8 achSrc[MAX_LEN_FILE_PATH] = {0};
        if ((jsrc == NULL) ||
            getStringBytes(env, jsrc, achSrc, MAX_LEN_FILE_PATH) <= 0) {
            LOGE("file path is null or char array is not enough!");
            return;
        }
        s8 achTemp[MAX_LEN_FILE_PATH] = {0};
        if ((jtempPath == NULL) ||
            getStringBytes(env, jtempPath, achTemp, MAX_LEN_FILE_PATH) <= 0) {
            LOGE("file path is null or char array is not enough!");
            return;
        }

        jobject value = NULL;
        if (!getObjectField(env, thiz, MODULE_FIELD_MUXER_NAME, MODULE_FIELD_MUXER_SIGNITURE,
                            &value) || value == NULL) {
            LOGE("java muxer is null");
            return;
        }
        CJNIModuleMuxer *pJNIMuxer = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleMuxer>(
                env, value);
        if (pJNIMuxer == NULL || pJNIMuxer->getMuxer() == NULL) {
            LOGE("native muxer is null");
            return;
        }
        pJNIReverser->getReverser()->reverse(pJNIMuxer->getMuxer(),achTemp, achSrc, jisVideo, jisAudio);
    }

    void CJNIModuleReverser::jni_release(JNIEnv *env, jobject thiz) {
        CJNIModuleReverser *pJNIReverser = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleReverser>(
                env, thiz);
        if (pJNIReverser != NULL) {
            CJNIModuleManager::getInstance()->destroyJniObject(pJNIReverser);
        }
    }
}
