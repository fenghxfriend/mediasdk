/*******************************************************************************
 *        Module: paomiantv
 *          File: jnimoduletransition.cpp
 * Functionality: transition jni.
 *       Related: mediasdk
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-31  v1.0        huangxuefeng  created
 ******************************************************************************/
#include "jnicommon.h"
#include "jnimodulemanager.h"
#include "jnimoduleanimation.h"

namespace paomiantv {

    TJavaClazzParam *CJNIModuleAnimation::GetJavaClazzParam() {
        TJavaClazzParam *ptJavaClazzParam = new TJavaClazzParam;
        JNINativeMethod arrMethods[] =
                {
                        {"_create",                  "()Z",  (void *) jni_init},
                        {"_destroy",                 "()Z",  (void *) jni_uninit},

                        {"_get_start_X",             "()F",  (void *) jni_get_start_X},
                        {"_set_start_X",             "(F)V", (void *) jni_set_start_X},

                        {"_get_start_Y",             "()F",  (void *) jni_get_start_Y},
                        {"_set_start_Y",             "(F)V", (void *) jni_set_start_Y},

                        {"_get_start_degree_Z",      "()F",  (void *) jni_get_start_degree_Z},
                        {"_set_start_degree_Z",      "(F)V", (void *) jni_set_start_degree_Z},

                        {"_get_start_scale_X",       "()F",  (void *) jni_get_start_scale_X},
                        {"_set_start_scale_X",       "(F)V", (void *) jni_set_start_scale_X},

                        {"_get_start_scale_Y",       "()F",  (void *) jni_get_start_scale_Y},
                        {"_set_start_scale_Y",       "(F)V", (void *) jni_set_start_scale_Y},

                        {"_get_end_X",               "()F",  (void *) jni_get_end_X},
                        {"_set_end_X",               "(F)V", (void *) jni_set_end_X},

                        {"_get_end_Y",               "()F",  (void *) jni_get_end_Y},
                        {"_set_end_Y",               "(F)V", (void *) jni_set_end_Y},

                        {"_get_end_degree_Z",        "()F",  (void *) jni_get_end_degree_Z},
                        {"_set_end_degree_Z",        "(F)V", (void *) jni_set_end_degree_Z},

                        {"_get_end_scale_X",         "()F",  (void *) jni_get_end_scale_X},
                        {"_set_end_scale_X",         "(F)V", (void *) jni_set_end_scale_X},

                        {"_get_end_scale_Y",         "()F",  (void *) jni_get_end_scale_Y},
                        {"_set_end_scale_Y",         "(F)V", (void *) jni_set_end_scale_Y},


                        {"_get_crop_start_X",        "()F",  (void *) jni_get_crop_start_X},
                        {"_set_crop_start_X",        "(F)V", (void *) jni_set_crop_start_X},

                        {"_get_crop_start_Y",        "()F",  (void *) jni_get_crop_start_Y},
                        {"_set_crop_start_Y",        "(F)V", (void *) jni_set_crop_start_Y},

                        {"_get_crop_start_degree_Z", "()F",  (void *) jni_get_crop_start_degree_Z},
                        {"_set_crop_start_degree_Z", "(F)V", (void *) jni_set_crop_start_degree_Z},

                        {"_get_crop_start_scale_X",  "()F",  (void *) jni_get_crop_start_scale_X},
                        {"_set_crop_start_scale_X",  "(F)V", (void *) jni_set_crop_start_scale_X},

                        {"_get_crop_start_scale_Y",  "()F",  (void *) jni_get_crop_start_scale_Y},
                        {"_set_crop_start_scale_Y",  "(F)V", (void *) jni_set_crop_start_scale_Y},


                        {"_get_crop_end_X",          "()F",  (void *) jni_get_crop_end_X},
                        {"_set_crop_end_X",          "(F)V", (void *) jni_set_crop_end_X},

                        {"_get_crop_end_Y",          "()F",  (void *) jni_get_crop_end_Y},
                        {"_set_crop_end_Y",          "(F)V", (void *) jni_set_crop_end_Y},

                        {"_get_crop_end_degree_Z",   "()F",  (void *) jni_get_crop_end_degree_Z},
                        {"_set_crop_end_degree_Z",   "(F)V", (void *) jni_set_crop_end_degree_Z},

                        {"_get_crop_end_scale_X",    "()F",  (void *) jni_get_crop_end_scale_X},
                        {"_set_crop_end_scale_X",    "(F)V", (void *) jni_set_crop_end_scale_X},

                        {"_get_crop_end_scale_Y",    "()F",  (void *) jni_get_crop_end_scale_Y},
                        {"_set_crop_end_scale_Y",    "(F)V", (void *) jni_set_crop_end_scale_Y},


                        {"_get_start",               "()J",  (void *) jni_get_start},
                        {"_set_start",               "(J)V", (void *) jni_set_start},

                        {"_get_duration",            "()J",  (void *) jni_get_duration},
                        {"_set_duration",            "(J)V", (void *) jni_set_duration},

                        {"_get_start_alpha",         "()F",  (void *) jni_get_start_alpha},
                        {"_set_start_alpha",         "(F)V", (void *) jni_set_start_alpha},

                        {"_get_end_alpha",           "()F",  (void *) jni_get_end_alpha},
                        {"_set_end_alpha",           "(F)V", (void *) jni_set_end_alpha}
                };
        ptJavaClazzParam->m_nMtdCount = NELEM(arrMethods);
        sprintf(ptJavaClazzParam->m_pchClazzName, "cn/paomiantv/mediasdk/module/PMAnimation%s", "");
        ptJavaClazzParam->m_ptMethods = (JNINativeMethod *) malloc(sizeof(arrMethods));
        memcpy(ptJavaClazzParam->m_ptMethods, arrMethods, sizeof(arrMethods));
        return ptJavaClazzParam;
    }


    CJNIModuleAnimation::CJNIModuleAnimation(JNIEnv *env, jobject jTransition) {
        USE_LOG;

        if (env == NULL || jTransition == NULL) {
            LOGE("invalid parameters");
            return;
        }
        m_jObject = env->NewGlobalRef(jTransition);
        if (m_jObject == NULL) {
            LOGE("new global reference failed ,jvm stack table is full or unknown reason");
        }
        m_pAnimation = new CAnimation;
        if (m_pAnimation != NULL) {
            LOGD("Transition instance allocated: %u", sizeof(CAnimation));
            // only register valid ones
            CJNIModuleManager::getInstance()->add(this);
        } else {
            LOGE("new Transition failed ,memory is not enough!");
        }

    }

    CJNIModuleAnimation::~CJNIModuleAnimation() {
        USE_LOG;

        if (m_pAnimation != NULL) {
            delete m_pAnimation;
            m_pAnimation = NULL;
            LOGD("Transition instance freed: %u", sizeof(CAnimation));
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
    CJNIModuleAnimation::jni_init(JNIEnv *env, jobject jAnimation) {
        USE_LOG;
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->createJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL) {
            return FALSE;
        }
        return TRUE;
    }

    jboolean CJNIModuleAnimation::jni_uninit(JNIEnv *env, jobject jAnimation) {
        USE_LOG;
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);

        if (pJNIAnimation == NULL) {
            return FALSE;
        } else {
            CJNIModuleManager::getInstance()->destroyJniObject(pJNIAnimation);
        }
        return TRUE;
    }

    jfloat CJNIModuleAnimation::jni_get_start_X(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 1.0f;
        }
        return pJNIAnimation->getAnimation()->m_fStartTransX;
    }

    void CJNIModuleAnimation::jni_set_start_X(JNIEnv *env, jobject jAnimation, jfloat jstartX) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fStartTransX = jstartX;
    }

    jfloat CJNIModuleAnimation::jni_get_start_Y(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 1.0f;
        }
        return pJNIAnimation->getAnimation()->m_fStartTransY;
    }

    void CJNIModuleAnimation::jni_set_start_Y(JNIEnv *env, jobject jAnimation, jfloat jstartY) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fStartTransY = jstartY;
    }

    jfloat CJNIModuleAnimation::jni_get_start_degree_Z(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 0;
        }
        return pJNIAnimation->getAnimation()->m_fStartDegreeZ;
    }

    void CJNIModuleAnimation::jni_set_start_degree_Z(JNIEnv *env, jobject jAnimation,
                                                     jfloat jstartDegree) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fStartDegreeZ = jstartDegree;
    }

    jfloat CJNIModuleAnimation::jni_get_start_scale_X(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 1.0f;
        }
        return pJNIAnimation->getAnimation()->m_fStartScaleX;
    }

    void
    CJNIModuleAnimation::jni_set_start_scale_X(JNIEnv *env, jobject jAnimation,
                                               jfloat jstartScaleX) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fStartScaleX = jstartScaleX;
    }

    jfloat CJNIModuleAnimation::jni_get_start_scale_Y(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 1.0f;
        }
        return pJNIAnimation->getAnimation()->m_fStartScaleY;
    }

    void
    CJNIModuleAnimation::jni_set_start_scale_Y(JNIEnv *env, jobject jAnimation,
                                               jfloat jstartScaleY) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fStartScaleY = jstartScaleY;
    }

    jfloat CJNIModuleAnimation::jni_get_end_X(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 0;
        }
        return pJNIAnimation->getAnimation()->m_fEndTransX;
    }

    void CJNIModuleAnimation::jni_set_end_X(JNIEnv *env, jobject jAnimation, jfloat jendX) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fEndTransX = jendX;
    }

    jfloat CJNIModuleAnimation::jni_get_end_Y(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 0;
        }
        return pJNIAnimation->getAnimation()->m_fEndTransY;
    }

    void CJNIModuleAnimation::jni_set_end_Y(JNIEnv *env, jobject jAnimation, jfloat jendY) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fEndTransY = jendY;
    }

    jfloat CJNIModuleAnimation::jni_get_end_degree_Z(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 0;
        }
        return pJNIAnimation->getAnimation()->m_fEndDegreeZ;
    }

    void
    CJNIModuleAnimation::jni_set_end_degree_Z(JNIEnv *env, jobject jAnimation, jfloat jendDegree) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fEndDegreeZ = jendDegree;
    }

    jfloat CJNIModuleAnimation::jni_get_end_scale_X(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 1.0f;
        }
        return pJNIAnimation->getAnimation()->m_fEndScaleX;
    }

    void
    CJNIModuleAnimation::jni_set_end_scale_X(JNIEnv *env, jobject jAnimation, jfloat jendScaleX) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fEndScaleX = jendScaleX;
    }

    jfloat CJNIModuleAnimation::jni_get_end_scale_Y(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 1.0f;
        }
        return pJNIAnimation->getAnimation()->m_fEndScaleY;
    }

    void
    CJNIModuleAnimation::jni_set_end_scale_Y(JNIEnv *env, jobject jAnimation, jfloat jendScaleY) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fEndScaleY = jendScaleY;
    }

    jfloat CJNIModuleAnimation::jni_get_crop_start_X(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 1.0f;
        }
        return pJNIAnimation->getAnimation()->m_fCropStartTransX;
    }

    void
    CJNIModuleAnimation::jni_set_crop_start_X(JNIEnv *env, jobject jAnimation, jfloat jcropstartX) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fCropStartTransX = jcropstartX;
    }

    jfloat CJNIModuleAnimation::jni_get_crop_start_Y(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 1.0f;
        }
        return pJNIAnimation->getAnimation()->m_fCropStartTransY;
    }

    void
    CJNIModuleAnimation::jni_set_crop_start_Y(JNIEnv *env, jobject jAnimation, jfloat jcropstartY) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fCropStartTransY = jcropstartY;
    }

    jfloat CJNIModuleAnimation::jni_get_crop_start_degree_Z(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 0;
        }
        return pJNIAnimation->getAnimation()->m_fCropStartDegreeZ;
    }

    void CJNIModuleAnimation::jni_set_crop_start_degree_Z(JNIEnv *env, jobject jAnimation,
                                                          jfloat jcropstartDegree) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fCropStartDegreeZ = jcropstartDegree;
    }

    jfloat CJNIModuleAnimation::jni_get_crop_start_scale_X(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 1.0f;
        }
        return pJNIAnimation->getAnimation()->m_fCropStartScaleX;
    }

    void
    CJNIModuleAnimation::jni_set_crop_start_scale_X(JNIEnv *env, jobject jAnimation,
                                                    jfloat jcropstartScaleX) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fCropStartScaleX = jcropstartScaleX;
    }

    jfloat CJNIModuleAnimation::jni_get_crop_start_scale_Y(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 1.0f;
        }
        return pJNIAnimation->getAnimation()->m_fCropStartScaleY;
    }

    void
    CJNIModuleAnimation::jni_set_crop_start_scale_Y(JNIEnv *env, jobject jAnimation,
                                                    jfloat jcropstartScaleY) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fCropStartScaleY = jcropstartScaleY;
    }


    jfloat CJNIModuleAnimation::jni_get_crop_end_X(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 0;
        }
        return pJNIAnimation->getAnimation()->m_fCropEndTransX;
    }

    void
    CJNIModuleAnimation::jni_set_crop_end_X(JNIEnv *env, jobject jAnimation, jfloat jcropendX) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fCropEndTransX = jcropendX;
    }

    jfloat CJNIModuleAnimation::jni_get_crop_end_Y(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 0;
        }
        return pJNIAnimation->getAnimation()->m_fCropEndTransY;
    }

    void
    CJNIModuleAnimation::jni_set_crop_end_Y(JNIEnv *env, jobject jAnimation, jfloat jcropendY) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fCropEndTransY = jcropendY;
    }

    jfloat CJNIModuleAnimation::jni_get_crop_end_degree_Z(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 0;
        }
        return pJNIAnimation->getAnimation()->m_fCropEndDegreeZ;
    }

    void
    CJNIModuleAnimation::jni_set_crop_end_degree_Z(JNIEnv *env, jobject jAnimation,
                                                   jfloat jcropendDegree) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fCropEndDegreeZ = jcropendDegree;
    }

    jfloat CJNIModuleAnimation::jni_get_crop_end_scale_X(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 1.0f;
        }
        return pJNIAnimation->getAnimation()->m_fCropEndScaleX;
    }

    void
    CJNIModuleAnimation::jni_set_crop_end_scale_X(JNIEnv *env, jobject jAnimation,
                                                  jfloat jcropendScaleX) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fCropEndScaleX = jcropendScaleX;
    }

    jfloat CJNIModuleAnimation::jni_get_crop_end_scale_Y(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 1.0f;
        }
        return pJNIAnimation->getAnimation()->m_fCropEndScaleY;
    }

    void
    CJNIModuleAnimation::jni_set_crop_end_scale_Y(JNIEnv *env, jobject jAnimation,
                                                  jfloat jcropendScaleY) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fCropEndScaleY = jcropendScaleY;
    }

    jfloat CJNIModuleAnimation::jni_get_start_alpha(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 1.0f;
        }
        return pJNIAnimation->getAnimation()->m_fStartAlpha;
    }

    void
    CJNIModuleAnimation::jni_set_start_alpha(JNIEnv *env, jobject jAnimation, jfloat jstartAlpha) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fStartAlpha = jstartAlpha;
    }

    jfloat CJNIModuleAnimation::jni_get_end_alpha(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 1.0f;
        }
        return pJNIAnimation->getAnimation()->m_fEndAlpha;
    }

    void CJNIModuleAnimation::jni_set_end_alpha(JNIEnv *env, jobject jAnimation, jfloat jendAlpha) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_fEndAlpha = jendAlpha;
    }


    jlong CJNIModuleAnimation::jni_get_start(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 0;
        }
        return pJNIAnimation->getAnimation()->m_sllStart;
    }

    void CJNIModuleAnimation::jni_set_start(JNIEnv *env, jobject jAnimation, jlong jstart) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_sllStart = jstart;
    }

    jlong CJNIModuleAnimation::jni_get_duration(JNIEnv *env, jobject jAnimation) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return 0;
        }
        return pJNIAnimation->getAnimation()->m_sllDuration;
    }

    void CJNIModuleAnimation::jni_set_duration(JNIEnv *env, jobject jAnimation, jlong jdurantion) {
        CJNIModuleAnimation *pJNIAnimation = CJNIModuleManager::getInstance()->getJniObject<CJNIModuleAnimation>(
                env, jAnimation);
        if (pJNIAnimation == NULL || pJNIAnimation->getAnimation() == NULL) {
            return;
        }
        pJNIAnimation->getAnimation()->m_sllDuration = jdurantion;
    }
}