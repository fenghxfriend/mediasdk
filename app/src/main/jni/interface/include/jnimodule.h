/*******************************************************************************
 *        Module: interface
 *          File: jnimodule.h
 * Functionality: define the super jnimodule class
 *       Related: 
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-20  v1.0        huangxuefeng  created
 ******************************************************************************/
#ifndef _PAOMIANTV_JNIMODULE_H_
#define _PAOMIANTV_JNIMODULE_H_

#include <cstdio>
#include <jni.h>
#include <memory.h>
#include <string.h>
#include <constant.h>
#include <typedef.h>
#include <autolog.h>

#ifndef MODULE_FIELD_NATIVE_ADDRESS_NAME
#define MODULE_FIELD_NATIVE_ADDRESS_NAME "mNativeAddress"
#endif

namespace paomiantv {

/*!
 * \brief    used in interface.
 * \author  huangxuefeng
 * \date    2017-07-21
 */
    typedef struct tagJavaClazzParam {
        //! count of java native method
        s32 m_nMtdCount;
        //! java native methods
        JNINativeMethod *m_ptMethods;
        //! class name
        s8 m_pchClazzName[CLASS_NAME_MAX];

        //! constructure
        tagJavaClazzParam() {
            memset(this, 0, sizeof(tagJavaClazzParam));
        }

    } TJavaClazzParam;

    class CJNIModule {
    protected:
        volatile jobject m_jObject;
    public:
        static JavaVM *m_spJVM;
    public:
        CJNIModule() : m_jObject(NULL) {
        }

        virtual ~CJNIModule() {

            if (m_jObject != NULL) {
                JNIEnv *env = NULL;
                if (m_spJVM->GetEnv((void **) &env, JNI_VERSION_1_6) == JNI_OK) {
                    env->DeleteGlobalRef(m_jObject);
                    m_jObject = NULL;
                } else {
                    LOGE("get JNIEnv failed");
                }
            }
        }

        inline jobject getObject();

    };

    inline jobject CJNIModule::getObject() {
        return m_jObject;
    }
}

#endif /* _PAOMIANTV_JNIMODULE_H_ */