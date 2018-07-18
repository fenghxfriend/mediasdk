/*******************************************************************************
*        Module: paomiantv
*          File: jnimoduleeffect.cpp
* Functionality: effect jni.
*       Related: mediasdk
*        System: android
*      Language: C++
*        Author: huangxuefeng
*       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
* -----------------------------------------------------------------------------
* Revisions:
* Date        Version     Reviser       Description
* 2017-07-30  v1.0        huangxuefeng  created
******************************************************************************/


#include <android/bitmap.h>
#include <libyuv.h>
#include <jnimoduleeffect.h>
#include <jnimodulemanager.h>
#include <jnicommon.h>

namespace paomiantv {

    TJavaClazzParam *CJNIModuleEffect::GetJavaClazzParam() {
        TJavaClazzParam *ptJavaClazzParam = new TJavaClazzParam;
        JNINativeMethod arrMethods[] =
                {
                        {"_create",      "(ILandroid/graphics/Bitmap;JJ)Z", (void *) jni_init},
                        {"_destroy",     "()Z",                             (void *) jni_uninit},

                        {"_update",      "(ILandroid/graphics/Bitmap;)V",   (void *) jni_update},

                        {"_getPicture",  "()Landroid/graphics/Bitmap;",     (void *) jni_getPicture},
                        {"_getType",     "()I",                             (void *) jni_getType},
                        {"_getStart",    "()J",                             (void *) jni_getStart},
                        {"_setStart",    "(J)V",                            (void *) jni_setStart},
                        {"_getDuration", "()J",                             (void *) jni_getDuration},
                        {"_setDuration", "(J)V",                            (void *) jni_setDuration}};
        ptJavaClazzParam->m_nMtdCount = NELEM(arrMethods);
        sprintf(ptJavaClazzParam->m_pchClazzName, "cn/paomiantv/mediasdk/module/PMEffect%s", "");
        ptJavaClazzParam->m_ptMethods = (JNINativeMethod *) malloc(sizeof(arrMethods));
        memcpy(ptJavaClazzParam->m_ptMethods, arrMethods, sizeof(arrMethods));
        return ptJavaClazzParam;
    }


    CJNIModuleEffect::CJNIModuleEffect(JNIEnv *env, jobject jeffect) {
        USE_LOG;

        if (env == NULL || jeffect == NULL) {
            LOGE("invalid parameters");
            return;
        }

        m_jObject = env->NewGlobalRef(jeffect);
        if (m_jObject == NULL) {
            LOGE("new global reference failed ,jvm stack table is full or unknown reason");
        }
        m_pEffect = new CEffect;
        if (m_pEffect != NULL) {
            LOGD("Filter instance allocated: %u", sizeof(CEffect));
            // only register valid ones
            CJNIModuleManager::getInstance()->add(this);
        } else {
            LOGE("new filter failed ,memory is not enough!");
        }

    }

    CJNIModuleEffect::~CJNIModuleEffect() {
        USE_LOG;

        if (m_pEffect != NULL) {
            delete m_pEffect;
            m_pEffect = NULL;
            LOGD("Filter instance freed: %u", sizeof(CEffect));
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
    CJNIModuleEffect::jni_init(JNIEnv *env, jobject jEffect, jint jtype, jobject jbitmap,
                               jlong jstart, jlong jduration) {
        CJNIModuleEffect *pJNIEffect = CJNIModuleManager::getInstance()->createJniObject<CJNIModuleEffect>(
                env, jEffect);
        if (pJNIEffect == NULL ||
            pJNIEffect->getEffect() == NULL ||
            jtype <= EM_EFFECT_START ||
            jtype >= EM_EFFECT_END) {
            LOGE("effect type is invalid!");
            return FALSE;
        }
        if (jbitmap == NULL) {
            pJNIEffect->getEffect()->init((EMEffect) jtype, NULL, EM_PIXEL_FORMAT_START,
                                          0, 0,
                                          jstart,
                                          jduration);
            return TRUE;
        }
        jboolean re = FALSE;
        env->PushLocalFrame(10);
        do {
            AndroidBitmapInfo bitmapInfo = {0};
            void *pixelscolor = NULL;
            int error;
            if ((error = AndroidBitmap_getInfo(env, jbitmap, &bitmapInfo)) < 0) {
                LOGE("AndroidBitmap_getInfo() failed ! error=%d", error);
                break;
            }
            LOGI("original image :: width is %d; height is %d; stride is %d; format is %d;flags is   %d,stride is %u",
                 bitmapInfo.width, bitmapInfo.height, bitmapInfo.stride, bitmapInfo.format,
                 bitmapInfo.flags, bitmapInfo.stride);
            if ((error = AndroidBitmap_lockPixels(env, jbitmap, &pixelscolor)) < 0) {
                LOGE("AndroidBitmap_lockPixels() failed ! error=%d", error);
                break;
            }
            EMPixelFormat pixelFormat = EM_PIXEL_FORMAT_START;
            switch (bitmapInfo.format) {
                case ANDROID_BITMAP_FORMAT_RGBA_8888: {
                    pixelFormat = EM_PIXEL_FORMAT_RGBA_8888;
                }
                    break;
                case ANDROID_BITMAP_FORMAT_RGB_565: {
                    pixelFormat = EM_PIXEL_FORMAT_RGB_565;
                }
                    break;
//                case ANDROID_BITMAP_FORMAT_RGBA_4444: {
//                    pixelFormat = EM_PIXEL_FORMAT_RGBA_8888;
//                }
//                    break;
//                case ANDROID_BITMAP_FORMAT_A_8:
                default: {
                    LOGE("Bitmap format is invalid !");
                }
                    break;
            }
            if (pixelFormat == EM_PIXEL_FORMAT_START) {
                AndroidBitmap_unlockPixels(env, jbitmap);
                break;
            } else {
                pJNIEffect->getEffect()->init((EMEffect) jtype, (u8 *) pixelscolor, pixelFormat,
                                              bitmapInfo.width, bitmapInfo.height,
                                              jstart,
                                              jduration);
                AndroidBitmap_unlockPixels(env, jbitmap);
            }

            re = TRUE;
        } while (0);

        env->PopLocalFrame(NULL);
        return re;
    }

    jboolean CJNIModuleEffect::jni_uninit(JNIEnv *env, jobject jEffect) {
        CJNIModuleEffect *pJNIEffect = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleEffect>(
                env, jEffect);
        if (pJNIEffect == NULL) {
            return FALSE;
        } else {
            CJNIModuleManager::getInstance()->destroyJniObject(pJNIEffect);
        }
        return TRUE;
    }

    jint CJNIModuleEffect::jni_getType(JNIEnv *env, jobject jEffect) {
        CJNIModuleEffect *pJNIEffect = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleEffect>(
                env, jEffect);
        if (pJNIEffect == NULL || pJNIEffect->getEffect() == NULL) {
            return EM_EFFECT_START;
        }

        return pJNIEffect->getEffect()->getType();
    }

    jobject CJNIModuleEffect::jni_getPicture(JNIEnv *env, jobject jEffect) {
        CJNIModuleEffect *pJNIEffect = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleEffect>(
                env, jEffect);
        if (pJNIEffect == NULL || pJNIEffect->getEffect() == NULL) {
            return NULL;
        }
        return NULL;

//        CPicture *p = NULL;
//        EMVFilter t = EM_V_FILTER_START;
//        pJNIEffect->getEffect()->getFilter(p, t);
//
//        env->PushLocalFrame(10);
//        do {
//            jclass jclass_String = env->FindClass("android/graphics/Bitmap");
//            jmethodID jcreate_method = env->GetStaticMethodID(jclass_String, "createBitmap",
//                                                      "([BLjava/lang/String;)V");
//
//
//            AndroidBitmapInfo bitmapInfo = {0};
//            void *pixelscolor = NULL;
//            int error;
//            if ((error = AndroidBitmap_getInfo(env, jbitmap, &bitmapInfo)) < 0) {
//                LOGE("AndroidBitmap_getInfo() failed ! error=%d", error);
//                break;
//            }
//            LOGI("original image :: width is %d; height is %d; stride is %d; format is %d;flags is   %d,stride is %u",
//                 bitmapInfo.width, bitmapInfo.height, bitmapInfo.stride, bitmapInfo.format,
//                 bitmapInfo.flags, bitmapInfo.stride);
//            if ((error = AndroidBitmap_lockPixels(env, jbitmap, &pixelscolor)) < 0) {
//                LOGE("AndroidBitmap_lockPixels() failed ! error=%d", error);
//                break;
//            }
//            EMPixelFormat pixelFormat = EM_PIXEL_FORMAT_START;
//            switch (bitmapInfo.format) {
//                case ANDROID_BITMAP_FORMAT_RGBA_8888: {
//                    pixelFormat = EM_PIXEL_FORMAT_RGBA_8888;
//                }
//                    break;
//                case ANDROID_BITMAP_FORMAT_RGB_565: {
//                    pixelFormat = EM_PIXEL_FORMAT_RGBA_8888;
//                }
//                    break;
//                case ANDROID_BITMAP_FORMAT_RGBA_4444: {
//                    pixelFormat = EM_PIXEL_FORMAT_RGBA_8888;
//                }
//                    break;
//                case ANDROID_BITMAP_FORMAT_A_8:
//                default: {
//                    LOGE("Bitmap format is invalid !");
//                }
//                    break;
//            }
//            if (pixelFormat == EM_PIXEL_FORMAT_START) {
//                AndroidBitmap_unlockPixels(env, jbitmap);
//                break;
//            } else {
//                pJNIEffect->getEffect()->update((EMEffect) jtype, (u8 *) pixelscolor, pixelFormat,
//                                                bitmapInfo.width, bitmapInfo.height);
//                AndroidBitmap_unlockPixels(env, jbitmap);
//            }
//        } while (0);
//        env->PopLocalFrame(NULL);
    }

    void
    CJNIModuleEffect::jni_update(JNIEnv *env, jobject jEffect, jint jtype, jbyteArray jbitmap) {
        CJNIModuleEffect *pJNIEffect = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleEffect>(
                env, jEffect);
        if (pJNIEffect == NULL ||
            pJNIEffect->getEffect() == NULL ||
            jtype <= EM_EFFECT_START ||
            jtype >= EM_EFFECT_END) {
            LOGE("effect type is invalid!");
            return;
        }
        if (jbitmap == NULL) {
            pJNIEffect->getEffect()->update((EMEffect) jtype, NULL, EM_PIXEL_FORMAT_START, 0, 0);
            return;
        }

        env->PushLocalFrame(10);
        do {
            AndroidBitmapInfo bitmapInfo = {0};
            void *pixelscolor = NULL;
            int error;
            if ((error = AndroidBitmap_getInfo(env, jbitmap, &bitmapInfo)) < 0) {
                LOGE("AndroidBitmap_getInfo() failed ! error=%d", error);
                break;
            }
            LOGI("original image :: width is %d; height is %d; stride is %d; format is %d;flags is   %d,stride is %u",
                 bitmapInfo.width, bitmapInfo.height, bitmapInfo.stride, bitmapInfo.format,
                 bitmapInfo.flags, bitmapInfo.stride);
            if ((error = AndroidBitmap_lockPixels(env, jbitmap, &pixelscolor)) < 0) {
                LOGE("AndroidBitmap_lockPixels() failed ! error=%d", error);
                break;
            }
            EMPixelFormat pixelFormat = EM_PIXEL_FORMAT_START;
            switch (bitmapInfo.format) {
                case ANDROID_BITMAP_FORMAT_RGBA_8888: {
                    pixelFormat = EM_PIXEL_FORMAT_RGBA_8888;
                }
                    break;
                case ANDROID_BITMAP_FORMAT_RGB_565: {
                    pixelFormat = EM_PIXEL_FORMAT_RGBA_8888;
                }
                    break;
                case ANDROID_BITMAP_FORMAT_RGBA_4444: {
                    pixelFormat = EM_PIXEL_FORMAT_RGBA_8888;
                }
                    break;
                case ANDROID_BITMAP_FORMAT_A_8:
                default: {
                    LOGE("Bitmap format is invalid !");
                }
                    break;
            }
            if (pixelFormat == EM_PIXEL_FORMAT_START) {
                AndroidBitmap_unlockPixels(env, jbitmap);
                break;
            } else {
                pJNIEffect->getEffect()->update((EMEffect) jtype, (u8 *) pixelscolor, pixelFormat,
                                                bitmapInfo.width, bitmapInfo.height);
                AndroidBitmap_unlockPixels(env, jbitmap);
            }
        } while (0);
        env->PopLocalFrame(NULL);
    }

    jlong CJNIModuleEffect::jni_getStart(JNIEnv *env, jobject jEffect) {
        CJNIModuleEffect *pJNIEffect = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleEffect>(
                env, jEffect);
        if (pJNIEffect == NULL || pJNIEffect->getEffect() == NULL) {
            return -1;
        }

        return pJNIEffect->getEffect()->getStart();
    }

    void CJNIModuleEffect::jni_setStart(JNIEnv *env, jobject jEffect, jlong jstart) {
        CJNIModuleEffect *pJNIEffect = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleEffect>(
                env, jEffect);
        if (pJNIEffect == NULL || pJNIEffect->getEffect() == NULL) {
            return;
        }
        pJNIEffect->getEffect()->setStart(jstart);
    }

    jlong CJNIModuleEffect::jni_getDuration(JNIEnv *env, jobject jEffect) {
        CJNIModuleEffect *pJNIEffect = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleEffect>(
                env, jEffect);
        if (pJNIEffect == NULL || pJNIEffect->getEffect() == NULL) {
            return -1;
        }
        return pJNIEffect->getEffect()->getDuration();
    }

    void CJNIModuleEffect::jni_setDuration(JNIEnv *env, jobject jEffect, jlong jduration) {
        CJNIModuleEffect *pJNIEffect = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleEffect>(
                env, jEffect);
        if (pJNIEffect == NULL || pJNIEffect->getEffect() == NULL) {
            return;
        }
        pJNIEffect->getEffect()->setDuration(jduration);
    }

}