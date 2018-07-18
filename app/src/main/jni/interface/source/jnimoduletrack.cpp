/*******************************************************************************
 *        Module: paomiantv
 *          File: jnimoduleclip.cpp
 * Functionality: clip jni.
 *       Related: mediasdk
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-20  v1.0        huangxuefeng  created
 ******************************************************************************/



#include <autolock.h>
#include <jnimoduletrack.h>
#include <jnimodulemanager.h>
#include <jnicommon.h>
#include <track.h>
#include <android/bitmap.h>

namespace paomiantv {

    TJavaClazzParam *CJNIModuleTrack::GetJavaClazzParam() {
        TJavaClazzParam *ptJavaClazzParam = new TJavaClazzParam;
        JNINativeMethod arrMethods[] =
                {
                        {"_create",                "(I)Z",                                          (void *) jni_create},
                        {"_destroy",               "()Z",                                           (void *) jni_destroy},
                        {"_getId",                 "()I",                                           (void *) jni_getId},
                        {"_getType",               "()I",                                           (void *) jni_getType},
                        {"_setDataSource_path",    "(Ljava/lang/String;)Z",                         (void *) jni_setDataSource_path},
                        {"_setDataSource_webp",    "(Ljava/lang/String;)Z",                         (void *) jni_setDataSource_webp},
                        {"_setDataSource_bitmap",  "(Landroid/graphics/Bitmap;)Z",                  (void *) jni_setDataSource_bitmap},
                        {"_setDataSource_silence", "()Z",                                           (void *) jni_setDataSource_silence},
                        {"_getSourceType",         "()I",                                           (void *) jni_getSourceType},
                        {"_getWidth",              "()I",                                           (void *) jni_getWidth},
                        {"_getHeight",             "()I",                                           (void *) jni_getHeight},
                        {"_getSrc",                "()[B",                                          (void *) jni_getSrc},

                        {"_setCutStart",           "(J)V",                                          (void *) jni_setCutStart},
                        {"_getCutStart",           "()J",                                           (void *) jni_getCutStart},

                        {"_setCutDuration",        "(J)V",                                          (void *) jni_setCutDuration},
                        {"_getCutDuration",        "()J",                                           (void *) jni_getCutDuration},

                        {"_setPlayStart",          "(J)V",                                          (void *) jni_setPlayStart},
                        {"_getPlayStart",          "()J",                                           (void *) jni_getPlayStart},

                        {"_setPlayDuration",       "(J)V",                                          (void *) jni_setPlayDuration},
                        {"_getPlayDuration",       "()J",                                           (void *) jni_getPlayDuration},

                        {"_setWeight",             "(S)V",                                          (void *) jni_setWeight},
                        {"_getWeight",             "()S",                                           (void *) jni_getWeight},

                        {"_setLoop",               "(Z)V",                                          (void *) jni_setLoop},
                        {"_isLoop",                "()Z",                                           (void *) jni_isLoop},

                        {"_setZIndex",             "(S)V",                                          (void *) jni_setZIndex},
                        {"_getZIndex",             "()S",                                           (void *) jni_getZIndex},

                        {"_setVolume",             "(F)V",                                          (void *) jni_setVolume},
                        {"_getVolume",             "()F",                                           (void *) jni_getVolume},

                        {"_setPlaybackRate",       "(F)V",                                          (void *) jni_setPlaybackRate},
                        {"_getPlaybackRate",       "()F",                                           (void *) jni_getPlaybackRate},

                        {"_setShowFirstFrame",     "(Z)V",                                          (void *) jni_setShowFirstFrame},
                        {"_isShowFirstFrame",      "()Z",                                           (void *) jni_isShowFirstFrame},

                        {"_setShowLastFrame",      "(Z)V",                                          (void *) jni_setShowLastFrame},
                        {"_isShowLastFrame",       "()Z",                                           (void *) jni_isShowLastFrame},

                        {"_isIndependent",         "()Z",                                           (void *) jni_isIndependent},

                        {"_getEffect",             "(I)Lcn/paomiantv/mediasdk/module/PMEffect;",    (void *) jni_getEffect},
                        {"_addEffect",             "(Lcn/paomiantv/mediasdk/module/PMEffect;)Z",    (void *) jni_addEffect},
                        {"_removeEffect",          "(I)Lcn/paomiantv/mediasdk/module/PMEffect;",    (void *) jni_removeEffect},
                        {"_getEffectCount",        "()I",                                           (void *) jni_getEffectCount},

                        {"_getAnimation",          "(I)Lcn/paomiantv/mediasdk/module/PMAnimation;", (void *) jni_getAnimation},
                        {"_addAnimation",          "(Lcn/paomiantv/mediasdk/module/PMAnimation;)Z", (void *) jni_addAnimation},
                        {"_removeAnimation",       "(I)Lcn/paomiantv/mediasdk/module/PMAnimation;", (void *) jni_removeAnimation},
                        {"_getAnimationCount",     "()I",                                           (void *) jni_getAnimationCount},

                        {"_getDataDuration",       "()J",                                           (void *) jni_getDataDuration},
                };
        ptJavaClazzParam->m_nMtdCount = NELEM(arrMethods);
        sprintf(ptJavaClazzParam->m_pchClazzName, "cn/paomiantv/mediasdk/module/PMTrack%s", "");
        ptJavaClazzParam->m_ptMethods = (JNINativeMethod *) malloc(sizeof(arrMethods));
        memcpy(ptJavaClazzParam->m_ptMethods, arrMethods, sizeof(arrMethods));
        return ptJavaClazzParam;
    }

    CJNIModuleTrack::CJNIModuleTrack(JNIEnv *env, jobject jtrack) : m_pTrack(NULL) {
        USE_LOG;
        if (env == NULL || jtrack == NULL) {
            LOGE("invalid parameters");
            return;
        }

        m_jObject = env->NewGlobalRef(jtrack);
        if (m_jObject == NULL) {
            LOGE("new global reference failed ,jvm stack table is full or unknown reason");
        }
        m_pTrack = new CTrack();
        if (m_pTrack != NULL) {
            LOGD("CTrack instance allocated: %u", sizeof(CMultiTrack));
            // only register valid ones
            CJNIModuleManager::getInstance()->add(this);
        } else {
            LOGE("new CTrack failed ,memory is not enough!");
        }
    }

    CJNIModuleTrack::~CJNIModuleTrack() {
        USE_LOG;

        std::vector<CJNIModuleEffect *>::iterator iterEffect = m_vJNIEffects.begin();

        while (iterEffect != m_vJNIEffects.end()) {
            if (*iterEffect != NULL) {
                CJNIModuleEffect *pJNIEffect = *iterEffect;
                CEffect *effect = pJNIEffect->getEffect();
                if (effect != NULL) {
                    ((CTrack *) m_pTrack)->removeEffectById(effect->getId());
                }
                CJNIModuleManager::getInstance()->destroyJniObject(*iterEffect);
            }
            ++iterEffect;
        }
        m_vJNIEffects.clear();

        std::vector<CJNIModuleAnimation *>::iterator iterAni = m_vJNIAnimations.begin();

        while (iterAni != m_vJNIAnimations.end()) {
            if (*iterAni != NULL) {
                CJNIModuleAnimation *pJNIAni = *iterAni;
                CAnimation *animation = pJNIAni->getAnimation();
                if (animation != NULL) {
                    ((CTrack *) m_pTrack)->removeAnimationById(animation->getId());
                }
                CJNIModuleManager::getInstance()->destroyJniObject(*iterAni);
            }
            ++iterAni;
        }
        m_vJNIAnimations.clear();

        if (m_pTrack != NULL) {
            delete m_pTrack;
            m_pTrack = NULL;
            LOGD("Track instance freed: %u", sizeof(CTrack));
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
    CJNIModuleTrack::jni_create(JNIEnv *env, jobject jtrack, jint jtype) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->createJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return FALSE;
        }
        (*pJNITrack->getTrack())->setType((EMTrack) jtype);
        return TRUE;
    }

    jboolean CJNIModuleTrack::jni_destroy(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL) {
            return FALSE;
        } else {
            CJNIModuleManager::getInstance()->destroyJniObject(pJNITrack);
        }
        return TRUE;
    }

    jint CJNIModuleTrack::jni_getType(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return 0;
        }
        return (*pJNITrack->getTrack())->getType();
    }

    jint CJNIModuleTrack::jni_getId(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return 0;
        }
        return (*pJNITrack->getTrack())->getId();
    }

    jboolean
    CJNIModuleTrack::jni_setDataSource_path(JNIEnv *env, jobject jtrack, jstring jsrc) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return FALSE;
        }
        s8 achSrc[MAX_LEN_FILE_PATH] = {0};
        if ((jsrc == NULL) ||
            getStringBytes(env, (jstring) jsrc, achSrc, MAX_LEN_FILE_PATH) <= 0) {
            LOGE("file path is null or char array is not enough!");
            return FALSE;
        }
        return ((CTrack *) (*pJNITrack->getTrack()))->setDataSource(EM_SOURCE_FILE,
                                                                    (u8 *) achSrc,
                                                                    strlen(achSrc)) ? TRUE : FALSE;
    }

    jboolean CJNIModuleTrack::jni_setDataSource_webp(JNIEnv *env, jobject jtrack, jstring jpath) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return FALSE;
        }
        s8 achSrc[MAX_LEN_FILE_PATH] = {0};
        if ((jpath == NULL) ||
            getStringBytes(env, (jstring) jpath, achSrc, MAX_LEN_FILE_PATH) <= 0) {
            LOGE("file path is null or char array is not enough!");
            return FALSE;
        }
        ((CTrack *) (*pJNITrack->getTrack()))->setDataSource(EM_SOURCE_WEBP,
                                                             (u8 *) achSrc,
                                                             strlen(achSrc));
        return TRUE;
    }

    jboolean
    CJNIModuleTrack::jni_setDataSource_bitmap(JNIEnv *env, jobject jtrack, jobject jbitmap) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return FALSE;
        }

        AndroidBitmapInfo infocolor;
        void *pixelscolor;
        int ret;
        if ((ret = AndroidBitmap_getInfo(env, jbitmap, &infocolor)) < 0) {
            LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
            return FALSE;
        }

        LOGI("color image :: width is %d; height is %d; stride is %d; format is %d;flags is %d",
             infocolor.width, infocolor.height, infocolor.stride, infocolor.format,
             infocolor.flags);
        if (infocolor.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
            LOGE("Bitmap format is not RGBA_8888 !");
            return FALSE;
        }

        if ((ret = AndroidBitmap_lockPixels(env, jbitmap, &pixelscolor)) < 0) {
            LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
            return FALSE;
        }

        ((CTrack *) (*pJNITrack->getTrack()))->setDataSource(EM_SOURCE_BITMAP, (u8 *) pixelscolor,
                                                             infocolor.height * infocolor.width * 4,
                                                             infocolor.width, infocolor.height);
        LOGI("unlocking pixels");
        AndroidBitmap_unlockPixels(env, jbitmap);
        return TRUE;
    }

    jboolean CJNIModuleTrack::jni_setDataSource_silence(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return FALSE;
        }
        return ((CTrack *) (*pJNITrack->getTrack()))->setDataSource(EM_SOURCE_SILENCE) ? TRUE
                                                                                       : FALSE;
    }

    jbyteArray CJNIModuleTrack::jni_getSrc(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return NULL;
        }
        uint8_t *data = nullptr;
        uint32_t size = 0;
        (*pJNITrack->getTrack())->getSourceData(data, size);
        return StringToJByteArr(env, data, size);
    }


    jint CJNIModuleTrack::jni_getSourceType(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return 0;
        }
        return ((CTrack *) (*pJNITrack->getTrack()))->getSourceType();
    }

    jint CJNIModuleTrack::jni_getWidth(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return 0;
        }
        return (*pJNITrack->getTrack())->getWidth();
    }

    jint CJNIModuleTrack::jni_getHeight(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return 0;
        }
        return (*pJNITrack->getTrack())->getHeight();
    }


    jlong CJNIModuleTrack::jni_getCutStart(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return 0;
        }

        return ((CTrack *) (*pJNITrack->getTrack()))->getCutStart();
    }

    void CJNIModuleTrack::jni_setCutStart(JNIEnv *env, jobject jtrack, jlong jstart) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return;
        }
        ((CTrack *) (*pJNITrack->getTrack()))->setCutStart(jstart);
    }

    jlong CJNIModuleTrack::jni_getCutDuration(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return 0;
        }
        return ((CTrack *) (*pJNITrack->getTrack()))->getCutDuration();
    }

    void CJNIModuleTrack::jni_setCutDuration(JNIEnv *env, jobject jtrack, jlong jduration) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return;
        }
        ((CTrack *) (*pJNITrack->getTrack()))->setCutDuration(jduration);
    }


    jlong CJNIModuleTrack::jni_getPlayStart(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return 0;
        }

        return ((CTrack *) (*pJNITrack->getTrack()))->getPlayStart();
    }

    void CJNIModuleTrack::jni_setPlayStart(JNIEnv *env, jobject jtrack, jlong jstart) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return;
        }
        ((CTrack *) (*pJNITrack->getTrack()))->setPlayStart(jstart);
    }

    jlong CJNIModuleTrack::jni_getPlayDuration(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return 0;
        }
        return ((CTrack *) (*pJNITrack->getTrack()))->getPlayDuration();
    }

    void CJNIModuleTrack::jni_setPlayDuration(JNIEnv *env, jobject jtrack, jlong jduration) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return;
        }
        ((CTrack *) (*pJNITrack->getTrack()))->setPlayDuration(jduration);
    }


    jfloat CJNIModuleTrack::jni_getVolume(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return 0.f;
        }
        return ((CTrack *) (*pJNITrack->getTrack()))->getVolume();
    }


    void CJNIModuleTrack::jni_setVolume(JNIEnv *env, jobject jtrack, jfloat jvolume) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return;
        }
        ((CTrack *) (*pJNITrack->getTrack()))->setVolume(jvolume);
    }

    jfloat CJNIModuleTrack::jni_getPlaybackRate(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return 1.0f;
        }
        return ((CTrack *) (*pJNITrack->getTrack()))->getPlayRate();
    }


    void CJNIModuleTrack::jni_setPlaybackRate(JNIEnv *env, jobject jtrack, jfloat jrate) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return;
        }
        ((CTrack *) (*pJNITrack->getTrack()))->setPlayRate(jrate);
    }


    void CJNIModuleTrack::jni_setLoop(JNIEnv *env, jobject jtrack, jboolean jloop) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return;
        }
        ((CTrack *) (*pJNITrack->getTrack()))->setLoop(jloop);
    }

    jboolean CJNIModuleTrack::jni_isLoop(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return FALSE;
        }
        return ((CTrack *) (*pJNITrack->getTrack()))->isLoop() ? TRUE : FALSE;
    }

    void CJNIModuleTrack::jni_setShowFirstFrame(JNIEnv *env, jobject jtrack, jboolean jloop) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return;
        }
        ((CTrack *) (*pJNITrack->getTrack()))->setShowFirstFrame(jloop);
    }

    jboolean CJNIModuleTrack::jni_isShowFirstFrame(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return FALSE;
        }
        return ((CTrack *) (*pJNITrack->getTrack()))->isShowFirstFrame() ? TRUE : FALSE;
    }

    void CJNIModuleTrack::jni_setShowLastFrame(JNIEnv *env, jobject jtrack, jboolean jloop) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return;
        }
        ((CTrack *) (*pJNITrack->getTrack()))->setShowLastFrame(jloop);
    }

    jboolean CJNIModuleTrack::jni_isShowLastFrame(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return FALSE;
        }
        return ((CTrack *) (*pJNITrack->getTrack()))->isShowLastFrame() ? TRUE : FALSE;
    }

    jboolean CJNIModuleTrack::jni_isIndependent(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return FALSE;
        }
        return ((CTrack *) (*pJNITrack->getTrack()))->isIndependent() ? TRUE : FALSE;
    }

    jshort CJNIModuleTrack::jni_getWeight(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return -1;
        }
        return ((CTrack *) (*pJNITrack->getTrack()))->getWeight();
    }

    void CJNIModuleTrack::jni_setWeight(JNIEnv *env, jobject jtrack, jshort jweight) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return;
        }
        ((CTrack *) (*pJNITrack->getTrack()))->setWeight(jweight);
    }

    jshort CJNIModuleTrack::jni_getZIndex(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return -1;
        }
        return ((CTrack *) (*pJNITrack->getTrack()))->getZIndex();
    }

    void CJNIModuleTrack::jni_setZIndex(JNIEnv *env, jobject jtrack, jshort jZindex) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return;
        }
        ((CTrack *) (*pJNITrack->getTrack()))->setZIndex(jZindex);
    }


    jobject CJNIModuleTrack::jni_getEffect(JNIEnv *env, jobject jtrack, jint jindex) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL) {
            return NULL;
        }
        CJNIModuleEffect *pJNIEffect = pJNITrack->getEffect(jindex);
        if (pJNIEffect == NULL) {
            return NULL;
        }
        return pJNIEffect->getObject();
    }

    jboolean CJNIModuleTrack::jni_addEffect(JNIEnv *env, jobject jtrack, jobject jeffect) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL) {
            return FALSE;
        }

        CJNIModuleEffect *pJNIEffect = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleEffect>(
                env,
                jeffect);
        if (pJNIEffect == NULL) {
            return FALSE;
        }
        return pJNITrack->addEffect(pJNIEffect) ? TRUE : FALSE;
    }

    jobject CJNIModuleTrack::jni_removeEffect(JNIEnv *env, jobject jtrack, jint jposition) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL) {
            return NULL;
        }
        return pJNITrack->removeEffect(jposition);
    }

    jint CJNIModuleTrack::jni_getEffectCount(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return 0;
        }
        return ((CTrack *) (*pJNITrack->getTrack()))->getEffectCount();
    }

    CJNIModuleEffect *CJNIModuleTrack::getEffect(s32 position) {
        CEffect *pFilter = ((CTrack *) m_pTrack)->getEffect(position);
        if (pFilter == NULL) {
            return NULL;
        }
        std::vector<CJNIModuleEffect *>::iterator iter;
        for (iter = m_vJNIEffects.begin(); iter != m_vJNIEffects.end();) {
            if (*iter != NULL && (*iter)->getEffect() == pFilter) {
                return *iter;
            }
            ++iter;
        }
        return NULL;
    }

    BOOL32 CJNIModuleTrack::addEffect(CJNIModuleEffect *effect) {
        if (m_pTrack == NULL) {
            return FALSE;
        }
        m_vJNIEffects.push_back(effect);

        ((CTrack *) m_pTrack)->addEffect(effect->getEffect());
        return TRUE;
    }

    jobject CJNIModuleTrack::removeEffect(s32 nIndex) {
        if (m_pTrack == NULL) {
            return NULL;
        }
        CEffect *effect = ((CTrack *) m_pTrack)->removeEffectByIndex(nIndex);
        if (effect == NULL) {
            return NULL;
        }
        std::vector<CJNIModuleEffect *>::iterator iter;
        for (iter = m_vJNIEffects.begin(); iter != m_vJNIEffects.end();) {
            if (*iter != NULL && (*iter)->getEffect() == effect) {
                CJNIModuleEffect *pJNIEffect = *iter;
                iter = m_vJNIEffects.erase(iter);
                CJNIModuleManager::getInstance()->destroyJniObject(pJNIEffect);
            } else {
                ++iter;
            }
        }
        return NULL;
    }


    jobject CJNIModuleTrack::jni_getAnimation(JNIEnv *env, jobject jEffect, jint jindex) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jEffect);
        if (pJNITrack == NULL) {
            return NULL;
        }
        CJNIModuleAnimation *pJNIAnimation = pJNITrack->getAnimation(jindex);
        if (pJNIAnimation == NULL) {
            return NULL;
        }
        return pJNIAnimation->getObject();
    }

    jboolean CJNIModuleTrack::jni_addAnimation(JNIEnv *env, jobject jEffect, jobject janimation) {
        CJNIModuleTrack *pJNIEffect = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jEffect);
        if (pJNIEffect == NULL) {
            return FALSE;
        }

        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env,
                janimation);
        if (pJNIAnimation == NULL) {
            return FALSE;
        }
        return pJNIEffect->addAnimation(pJNIAnimation) ? TRUE : FALSE;
    }

    jobject CJNIModuleTrack::jni_removeAnimation(JNIEnv *env, jobject jEffect, jint jposition) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jEffect);
        if (pJNITrack == NULL) {
            return FALSE;
        }
        return pJNITrack->removeAnimation(jposition);
    }

    jint CJNIModuleTrack::jni_getAnimationCount(JNIEnv *env, jobject jEffect) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jEffect);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return 0;
        }
        return ((CTrack *) (*pJNITrack->getTrack()))->getAnimationCount();
    }

    jlong CJNIModuleTrack::jni_getDataDuration(JNIEnv *env, jobject jtrack) {
        CJNIModuleTrack *pJNITrack = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleTrack>(
                env, jtrack);
        if (pJNITrack == NULL || pJNITrack->getTrack() == NULL || *pJNITrack->getTrack() == NULL) {
            return 0;
        }
        return ((CTrack *) (*pJNITrack->getTrack()))->getDataDuration() / 1000 + 1;
    }

    CJNIModuleAnimation *CJNIModuleTrack::getAnimation(s32 position) {
        CAnimation *pAnimation = ((CTrack *) m_pTrack)->getAnimation(position);
        if (pAnimation == NULL) {
            return NULL;
        }
        std::vector<CJNIModuleAnimation *>::iterator iter;
        for (iter = m_vJNIAnimations.begin(); iter != m_vJNIAnimations.end();) {
            if (*iter != NULL && (*iter)->getAnimation() == pAnimation) {
                return *iter;
            }
            ++iter;
        }
        return NULL;
    }


    BOOL32 CJNIModuleTrack::addAnimation(CJNIModuleAnimation *animation) {
        if (m_pTrack == NULL) {
            return FALSE;
        }
        m_vJNIAnimations.push_back(animation);
        ((CTrack *) m_pTrack)->addAnimation(animation->getAnimation());
        return TRUE;
    }

    jobject CJNIModuleTrack::removeAnimation(s32 nIndex) {
        if (m_pTrack == NULL) {
            return NULL;
        }
        CAnimation *pAnimation = ((CTrack *) m_pTrack)->removeAnimationByIndex(nIndex);
        if (pAnimation == NULL) {
            return NULL;
        }
        std::vector<CJNIModuleAnimation *>::iterator iter;
        for (iter = m_vJNIAnimations.begin(); iter != m_vJNIAnimations.end();) {
            if (*iter != NULL && (*iter)->getAnimation() == pAnimation) {
                CJNIModuleAnimation *pJNIAnimation = *iter;
                iter = m_vJNIAnimations.erase(iter);
                CJNIModuleManager::getInstance()->destroyJniObject(pJNIAnimation);
            } else {
                ++iter;
            }
        }
        return NULL;
    }

    ITrack **CJNIModuleTrack::getTrack() {
        return &m_pTrack;
    }
}