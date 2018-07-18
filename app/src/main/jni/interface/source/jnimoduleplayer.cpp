//
// Created by ASUS on 2018/2/8.
//
#include <android/native_window_jni.h>
#include <autolock.h>
#include <jnimoduleplayer.h>
#include <jnimodulemanager.h>
#include <jnicommon.h>
#include <jnimodulestoryboard.h>

namespace paomiantv {

    void JNIModulePlayer_OnMessage(void *delegate, s32 nId, s8 *message) {
        if (delegate != NULL) {
            ((CJNIModulePlayer *) delegate)->onMessage(nId, message);
        }

    }

    TJavaClazzParam *CJNIModulePlayer::GetJavaClazzParam() {
        TJavaClazzParam *ptJavaClazzParam = new TJavaClazzParam;
        JNINativeMethod arrMethods[] = {
                {"_create",             "()V",                                            (void *) jni_create},
                {"_setDataSourceSTB",   "(Lcn/paomiantv/mediasdk/module/PMStoryboard;)V", (void *) jni_setDSStoryboard},
                {"_prepare",            "()V",                                            (void *) jni_prepare},
                {"_bindSurface",        "(Landroid/view/Surface;II)V",                    (void *) jni_bindSurface},
                {"_surfaceSizeChanged", "(II)V",                                          (void *) jni_surfaceSizeChanged},
                {"_unbindSurface",      "()V",                                            (void *) jni_unbindSurface},
                {"_play",               "()V",                                            (void *) jni_play},
                {"_pause",              "()V",                                            (void *) jni_pause},
                {"_seekTo",             "(J)V",                                           (void *) jni_seekTo},
                {"_locPreview",         "(J)V",                                           (void *) jni_locPreview},
                {"_resume",             "()V",                                            (void *) jni_resume},
                {"_stop",               "()V",                                            (void *) jni_stop},
                {"_release",            "()V",                                            (void *) jni_release}};

        ptJavaClazzParam->m_nMtdCount = NELEM(arrMethods);
        sprintf(ptJavaClazzParam->m_pchClazzName, "cn/paomiantv/mediasdk/PMNativePlayer%s", "");
        ptJavaClazzParam->m_ptMethods = (JNINativeMethod *) malloc(sizeof(arrMethods));
        memcpy(ptJavaClazzParam->m_ptMethods, arrMethods, sizeof(arrMethods));
        return ptJavaClazzParam;
    }

    CJNIModulePlayer::CJNIModulePlayer(JNIEnv *env, jobject jPlayer) {
        USE_LOG;
        if (env == NULL || jPlayer == NULL) {
            LOGE("invalid parameters");
            return;
        }

        m_jObject = env->NewGlobalRef(jPlayer);
        if (m_jObject == NULL) {
            LOGE("new global reference failed ,jvm stack table is full or unknown reason");
        }
        m_pPlayer = new CPlayer;
        if (m_pPlayer != NULL) {
            LOGD("player instance allocated: %u", sizeof(CPlayer));
            m_pPlayer->setOnMessageCB(JNIModulePlayer_OnMessage, this);
        }
        // only register valid ones
        CJNIModuleManager::getInstance()->add(this);
    }

// FIXME: when exit the application, it will sometimes prints this error log:
// E/libEGL(19049): call to OpenGL ES API with no current context (logged once per thread)
// -- noted by huangxuefeng, 2013-07-29
    CJNIModulePlayer::~CJNIModulePlayer() {
        USE_LOG;

        if (m_pPlayer != NULL) {
            m_pPlayer->setOnMessageCB(NULL, NULL);
            delete m_pPlayer;
            m_pPlayer = NULL;
            LOGD("player instance freed: %u", sizeof(CRenderer));
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

    void CJNIModulePlayer::onMessage(s32 nId, s8 *msg) {
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
            jmethodID jmtd = env->GetMethodID(jcls, PLAYER_METHOD_ON_MESSAGE_NAME,
                                              PLAYER_METHOD_ON_MESSAGE_SIG);
            if (jmtd == NULL) {
                LOGE("java method 'void %s()' is not defined", PLAYER_METHOD_ON_MESSAGE_NAME);
                break;
            }
            env->CallVoidMethod(m_jObject, jmtd, nId, stringToJString(env, msg));
        } while (0);

        env->PopLocalFrame(NULL);

        if (isAttacked) {
            m_spJVM->DetachCurrentThread();
        }
    }

    void CJNIModulePlayer::jni_create(JNIEnv *env, jobject thiz) {
        CJNIModulePlayer *pJNIPlayer = CJNIModuleManager::getInstance()->createJniObject<CJNIModulePlayer>(
                env, thiz);
        if (pJNIPlayer == NULL || pJNIPlayer->getPlayer() == NULL) {
            LOGE("create player failed!");
        }
    }


    void CJNIModulePlayer::jni_setDSStoryboard(JNIEnv *env, jobject thiz, jobject jstoryboard) {
        CJNIModulePlayer *pJNIPlayer = CJNIModuleManager::getInstance()->getJniObject<CJNIModulePlayer>(
                env, thiz);
        if (pJNIPlayer == NULL || pJNIPlayer->getPlayer() == NULL) {
            LOGE("get player failed, player is invalid or released!");
        } else {
            CJNIModuleStoryboard *pJNIStoryboard = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleStoryboard>(
                    env,
                    jstoryboard);
            if (pJNIStoryboard == NULL || pJNIStoryboard->getCStoryboard() == NULL) {
                LOGE("get storyboard failed, storyboard is invalid or released!");
                return;
            }
            pJNIPlayer->getPlayer()->setDataSource(pJNIStoryboard->getCStoryboard());
        }
    }

    void CJNIModulePlayer::jni_prepare(JNIEnv *env, jobject thiz) {
        CJNIModulePlayer *pJNIPlayer = CJNIModuleManager::getInstance()->getJniObject<CJNIModulePlayer>(
                env, thiz);
        if (pJNIPlayer == NULL || pJNIPlayer->getPlayer() == NULL) {
            LOGE("get player failed, player is invalid or released!");
            return;
        }
        pJNIPlayer->getPlayer()->prepare();
    }

    void CJNIModulePlayer::jni_bindSurface(JNIEnv *env, jobject thiz, jobject jsurface, jint jwidth,
                                           jint jheight) {
        ANativeWindow *win = ANativeWindow_fromSurface(env, jsurface);
        if (win == NULL) {
            LOGE("get native window failed!");
            return;
        }
        CJNIModulePlayer *pJNIPlayer = CJNIModuleManager::getInstance()->getJniObject<CJNIModulePlayer>(
                env, thiz);
        if (pJNIPlayer == NULL || pJNIPlayer->getPlayer() == NULL) {
            ANativeWindow_release(win);
            LOGE("get player failed, player is invalid or released!");
        } else {
            pJNIPlayer->getPlayer()->bindSurface(win, jwidth, jheight);
        }
    }

    void
    CJNIModulePlayer::jni_surfaceSizeChanged(JNIEnv *env, jobject thiz, jint jwidth, jint jheight) {
        CJNIModulePlayer *pJNIPlayer = CJNIModuleManager::getInstance()->getJniObject<CJNIModulePlayer>(
                env, thiz);
        if (pJNIPlayer == NULL || pJNIPlayer->getPlayer() == NULL) {
            LOGE("get player failed, player is invalid or released!");
        } else {
            pJNIPlayer->getPlayer()->surfaceSizeChanged((u32) jwidth, (u32) jheight);
        }
    }

    void CJNIModulePlayer::jni_unbindSurface(JNIEnv *env, jobject thiz) {
        CJNIModulePlayer *pJNIPlayer = CJNIModuleManager::getInstance()->getJniObject<CJNIModulePlayer>(
                env, thiz);
        if (pJNIPlayer == NULL || pJNIPlayer->getPlayer() == NULL) {
            LOGE("get player failed, player is invalid or released!");
        } else {
            pJNIPlayer->getPlayer()->unbindSurface(ANativeWindow_release);
        }
    }

    void CJNIModulePlayer::jni_play(JNIEnv *env, jobject thiz) {
        CJNIModulePlayer *pJNIPlayer = CJNIModuleManager::getInstance()->getJniObject<CJNIModulePlayer>(
                env, thiz);
        if (pJNIPlayer == NULL || pJNIPlayer->getPlayer() == NULL) {
            LOGE("get player failed, player is invalid or released!");
        } else {
            pJNIPlayer->getPlayer()->start();
        }
    }

    void CJNIModulePlayer::jni_pause(JNIEnv *env, jobject thiz) {
        CJNIModulePlayer *pJNIPlayer = CJNIModuleManager::getInstance()->getJniObject<CJNIModulePlayer>(
                env, thiz);
        if (pJNIPlayer == NULL || pJNIPlayer->getPlayer() == NULL) {
            LOGE("get player failed, player is invalid or released!");
        } else {
            pJNIPlayer->getPlayer()->pause();
        }
    }

    void CJNIModulePlayer::jni_seekTo(JNIEnv *env, jobject thiz, jlong jmicrosecond) {
        CJNIModulePlayer *pJNIPlayer = CJNIModuleManager::getInstance()->getJniObject<CJNIModulePlayer>(
                env, thiz);
        if (pJNIPlayer == NULL || pJNIPlayer->getPlayer() == NULL) {
            LOGE("get player failed, player is invalid or released!");
        } else {
            pJNIPlayer->getPlayer()->seekTo(jmicrosecond);
        }
    }

    void CJNIModulePlayer::jni_locPreview(JNIEnv *env, jobject thiz, jlong jmicrosecond) {
        CJNIModulePlayer *pJNIPlayer = CJNIModuleManager::getInstance()->getJniObject<CJNIModulePlayer>(
                env, thiz);
        if (pJNIPlayer == NULL || pJNIPlayer->getPlayer() == NULL) {
            LOGE("get player failed, player is invalid or released!");
        } else {
            pJNIPlayer->getPlayer()->locPreview(jmicrosecond);
        }
    }

    void CJNIModulePlayer::jni_resume(JNIEnv *env, jobject thiz) {
        CJNIModulePlayer *pJNIPlayer = CJNIModuleManager::getInstance()->getJniObject<CJNIModulePlayer>(
                env, thiz);
        if (pJNIPlayer == NULL || pJNIPlayer->getPlayer() == NULL) {
            LOGE("get player failed, player is invalid or released!");
        } else {
            pJNIPlayer->getPlayer()->resume();
        }
    }

    void CJNIModulePlayer::jni_stop(JNIEnv *env, jobject thiz) {
        CJNIModulePlayer *pJNIPlayer = CJNIModuleManager::getInstance()->getJniObject<CJNIModulePlayer>(
                env, thiz);
        if (pJNIPlayer == NULL || pJNIPlayer->getPlayer() == NULL) {
            LOGE("get player failed, player is invalid or released!");
        } else {
            pJNIPlayer->getPlayer()->stop();
        }
    }

    void CJNIModulePlayer::jni_release(JNIEnv *env, jobject thiz) {
        CJNIModulePlayer *pJNIPlayer = CJNIModuleManager::getInstance()->getJniObject<CJNIModulePlayer>(
                env, thiz);
        if (pJNIPlayer != NULL) {
            CJNIModuleManager::getInstance()->destroyJniObject(pJNIPlayer);
        }
    }
}
