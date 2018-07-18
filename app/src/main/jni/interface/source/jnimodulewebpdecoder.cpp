//
// Created by ASUS on 2018/1/4.
//

#include <android/bitmap.h>
#include <jnicommon.h>
#include <jnimodulewebpdecoder.h>
#include <jnimodulemanager.h>

namespace paomiantv {

    TJavaClazzParam *CJNIModuleWebpDecoder::GetJavaClazzParam() {
        TJavaClazzParam *ptJavaClazzParam = new TJavaClazzParam;
        JNINativeMethod arrMethods[] =
                {
                        {"_create",           "()Z",                           (void *) jni_create},
                        {"_destroy",          "()Z",                           (void *) jni_destroy},
                        {"_setDataSource",    "(Ljava/lang/String;)Z",         (void *) jni_setDataSource},
                        {"_getSrc",           "()Ljava/lang/String;",          (void *) jni_getSrc},
                        {"_getFrameCount",    "()I",                           (void *) jni_getFrameCount},
                        {"_getCanvasWidth",   "()I",                           (void *) jni_getCanvasWidth},
                        {"_getCanvasHeight",  "()I",                           (void *) jni_getCanvasHeight},
                        {"_getRGBAFrame",     "(I)Ljava/nio/ByteBuffer;",      (void *) jni_getRGBAFrame},
                        {"_getFrame",         "(ILandroid/graphics/Bitmap;)Z", (void *) jni_getFrame},
                        {"_getFrameDuration", "(I)J",                          (void *) jni_getFrameDuration},
                };
        ptJavaClazzParam->m_nMtdCount = NELEM(arrMethods);
        sprintf(ptJavaClazzParam->m_pchClazzName, "cn/paomiantv/mediasdk/PMWebpDecoder%s", "");
        ptJavaClazzParam->m_ptMethods = (JNINativeMethod *) malloc(sizeof(arrMethods));
        memcpy(ptJavaClazzParam->m_ptMethods, arrMethods, sizeof(arrMethods));
        return ptJavaClazzParam;
    }

    CJNIModuleWebpDecoder::CJNIModuleWebpDecoder(JNIEnv *env, jobject jdecoder) {
        USE_LOG;

        if (env == NULL || jdecoder == NULL) {
            LOGE("invalid parameters");
            return;
        }

        m_jObject = env->NewGlobalRef(jdecoder);
        if (m_jObject == NULL) {
            LOGE("new global reference failed ,jvm stack table is full or unknown reason");
        }
        m_pDecoder = new CWebpDec;
        if (m_pDecoder != NULL) {
            LOGD("Muxer instance allocated: %u", sizeof(CWebpDec));
            // only register valid ones
            CJNIModuleManager::getInstance()->add(this);
        } else {
            LOGE("new muxer failed ,memory is not enough!");
        }
    }

    CJNIModuleWebpDecoder::~CJNIModuleWebpDecoder() {
        USE_LOG;

        if (m_pDecoder != NULL) {
            delete m_pDecoder;
            m_pDecoder = NULL;
            LOGD("Muxer instance freed: %u", sizeof(CWebpDec));
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

    jboolean CJNIModuleWebpDecoder::jni_create(JNIEnv *env, jobject jdecoder) {
        CJNIModuleWebpDecoder *pJNIDecoder = CJNIModuleManager::getInstance()->createJniObject<CJNIModuleWebpDecoder>(env,
                                                                                               jdecoder);
        if (pJNIDecoder == NULL || pJNIDecoder->getDecoder() == NULL) {
            return FALSE;
        }
        return TRUE;
    }

    jboolean CJNIModuleWebpDecoder::jni_destroy(JNIEnv *env, jobject jdecoder) {
        CJNIModuleWebpDecoder *pJNIDecoder = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleWebpDecoder>(env,
                                                                                            jdecoder);
        if (pJNIDecoder == NULL) {
            return FALSE;
        } else {
            CJNIModuleManager::getInstance()->destroyJniObject(pJNIDecoder);
        }
        return TRUE;
    }

    jboolean
    CJNIModuleWebpDecoder::jni_setDataSource(JNIEnv *env, jobject jdecoder, jstring jdst) {
        CJNIModuleWebpDecoder *pJNIDecoder = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleWebpDecoder>(env,
                                                                                            jdecoder);
        if (pJNIDecoder == NULL || pJNIDecoder->getDecoder() == NULL) {
            return FALSE;
        }
        s8 achDstPath[MAX_LEN_FILE_PATH] = {0};
        if ((jdst == NULL) ||
            (getStringBytes(env, jdst, achDstPath, MAX_LEN_FILE_PATH) <= 0)) {
            LOGW("file path is null or char array is not enough!");
        }
        return pJNIDecoder->getDecoder()->setDataSource(achDstPath) == 0 ? TRUE : FALSE;
    }


    jstring CJNIModuleWebpDecoder::jni_getSrc(JNIEnv *env, jobject jdecoder) {
        CJNIModuleWebpDecoder *pJNIDecoder = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleWebpDecoder>(env,
                                                                                            jdecoder);
        if (pJNIDecoder == NULL || pJNIDecoder->getDecoder() == NULL) {
            return NULL;
        }
        return stringToJString(env, reinterpret_cast<s8 *>(pJNIDecoder->getDecoder()->getSrc()));
    }


    jint
    CJNIModuleWebpDecoder::jni_getFrameCount(JNIEnv *env, jobject jdecoder) {
        CJNIModuleWebpDecoder *pJNIDecoder = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleWebpDecoder>(env,
                                                                                            jdecoder);
        if (pJNIDecoder == NULL || pJNIDecoder->getDecoder() == NULL) {
            return 0;
        }

        return pJNIDecoder->getDecoder()->getFrameCount();
    }

    jobject
    CJNIModuleWebpDecoder::jni_getRGBAFrame(JNIEnv *env, jobject jdecoder, jint jindex) {
        CJNIModuleWebpDecoder *pJNIDecoder = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleWebpDecoder>(env,
                                                                                            jdecoder);
        if (pJNIDecoder == NULL || pJNIDecoder->getDecoder() == NULL) {
            return NULL;
        }
        void *buffer = NULL;
        int size = 0;
        pJNIDecoder->getDecoder()->getFrame(jindex, &buffer, &size);
        return (buffer != NULL && size > 0) ? env->NewDirectByteBuffer(buffer, size) : NULL;
    }

    typedef struct {
        uint8_t alpha;
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    } argb;

    jboolean CJNIModuleWebpDecoder::jni_getFrame(JNIEnv *env, jobject jdecoder, jint jindex,
                                                 jobject jbitmap) {
        CJNIModuleWebpDecoder *pJNIDecoder = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleWebpDecoder>(env,
                                                                                            jdecoder);
        if (pJNIDecoder == NULL || pJNIDecoder->getDecoder() == NULL) {
            return JNI_FALSE;
        }
        void *buffer = NULL;
        int size = 0;
        pJNIDecoder->getDecoder()->getFrame(jindex, &buffer, &size);
        if (buffer == NULL || size <= 0)
            return JNI_FALSE;

        AndroidBitmapInfo infocolor;
        void *pixelscolor;
        int ret;
        if ((ret = AndroidBitmap_getInfo(env, jbitmap, &infocolor)) < 0) {
            LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
            return JNI_FALSE;
        }

        LOGI("color image :: width is %d; height is %d; stride is %d; format is %d;flags is %d",
             infocolor.width, infocolor.height, infocolor.stride, infocolor.format,
             infocolor.flags);
        if (infocolor.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
            LOGE("Bitmap format is not RGBA_8888 !");
            return JNI_FALSE;
        }

        if ((ret = AndroidBitmap_lockPixels(env, jbitmap, &pixelscolor)) < 0) {
            LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        }
        // modify pixels with image processing algorithm
//        int y;
//        int x;
//        for (y = 0; y < infocolor.height; y++) {
//            argb *line = (argb *) pixelscolor;
//            argb *lineO = (argb *) buffer;
//            for (x = 0; x < infocolor.width; x++) {
//                line[x].alpha = lineO[x].blue;
//                line[x].red = lineO[x].alpha;
//                line[x].green = lineO[x].red;
//                line[x].blue = lineO[x].green;
//            }
//            pixelscolor = (char *) pixelscolor + infocolor.stride;
//            buffer = (char *) buffer + infocolor.stride;
//        }
        memcpy(pixelscolor, buffer, size);

        LOGI("unlocking pixels");
        AndroidBitmap_unlockPixels(env, jbitmap);
        return JNI_TRUE;
    }

    jint CJNIModuleWebpDecoder::jni_getCanvasWidth(JNIEnv *env, jobject jdecoder) {
        CJNIModuleWebpDecoder *pJNIDecoder = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleWebpDecoder>(env,
                                                                                            jdecoder);
        if (pJNIDecoder == NULL || pJNIDecoder->getDecoder() == NULL) {
            return -1;
        }

        return pJNIDecoder->getDecoder()->getCanvasWidth();
    }

    jint CJNIModuleWebpDecoder::jni_getCanvasHeight(JNIEnv *env, jobject jdecoder) {
        CJNIModuleWebpDecoder *pJNIDecoder = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleWebpDecoder>(env,
                                                                                            jdecoder);
        if (pJNIDecoder == NULL || pJNIDecoder->getDecoder() == NULL) {
            return -1;
        }
        return pJNIDecoder->getDecoder()->getCanvasHeight();
    }

    jlong CJNIModuleWebpDecoder::jni_getFrameDuration(JNIEnv *env, jobject jdecoder, jint jindex) {
        CJNIModuleWebpDecoder *pJNIDecoder = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleWebpDecoder>(env,
                                                                                            jdecoder);
        if (pJNIDecoder == NULL || pJNIDecoder->getDecoder() == NULL) {
            return -1;
        }
        return pJNIDecoder->getDecoder()->getFrameDuration(jindex);
    }
}