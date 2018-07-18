/*******************************************************************************
 *        Module: paomiantv
 *          File: 
 * Functionality: define jni objects manager
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
#ifndef _PAOMIANTV_JNIMODULEMANAGER_H_
#define _PAOMIANTV_JNIMODULEMANAGER_H_

#include <stdlib.h>
#include <typeinfo>
#include <residentreg.h>
#include "jnicommon.h"
#include "jnimodule.h"
#include <typeinfo>

namespace paomiantv {

    class CJNIModuleManager {
    private:
        CJNIModuleManager();

        CJNIModuleManager(const CJNIModuleManager &);

        CJNIModuleManager &operator=(const CJNIModuleManager &);

        virtual ~CJNIModuleManager();

        static CJNIModuleManager *m_pInstance;
        CResidentReg m_Registry;

        class Garbo {
        public:
            ~Garbo() {
                if (CJNIModuleManager::m_pInstance) {
                    delete CJNIModuleManager::m_pInstance;
                }
            }
        };

        static Garbo garbo;

    public:
        static CJNIModuleManager *getInstance();

        void add(CJNIModule *p);

        void remove(CJNIModule *p);

        bool contains(CJNIModule *p);

        void clear();

        template<typename T>
        T *createJniObject(JNIEnv *env, jobject jobj) {
//            USE_LOG;
            T *t = NULL;
            env->PushLocalFrame(10);
            const char *name = typeid(T).name();
            do {
                jlong value = 0;
                if (!getLongField(env, jobj, MODULE_FIELD_NATIVE_ADDRESS_NAME, &value)) {
                    LOGE("invalid parameters");
                    break;
                }
                if (value != 0 &&
                    CJNIModuleManager::getInstance()->contains((CJNIModule *) value)) {
                    LOGI("the %s is already created", name);
                    t = (T *) value;
                    break;
                }

                T *pNew = new T(env, jobj);
                if (!CJNIModuleManager::getInstance()->contains(pNew)) {
                    LOGE("create %s failed", name);
                    delete pNew;
                    t = NULL;
                    break;
                } else {
                    LOGD("%s instance allocated: %u\n", name, sizeof(T));
                }

                if (!setLongField(env, jobj, MODULE_FIELD_NATIVE_ADDRESS_NAME, (jlong) pNew)) {
                    LOGE("invalid parameters");
                    break;
                }
                t = pNew;

                LOGI("create %s ok\n", name);
            } while (0);
            env->PopLocalFrame(NULL);
            return t;
        }

        template<typename T>
        T *getJniObject(JNIEnv *env, jobject jobj) {
//            USE_LOG;
            T *t = NULL;
            env->PushLocalFrame(10);
            const char *name = typeid(T).name();
            do {
                jlong value = 0;
                if (!getLongField(env, jobj, MODULE_FIELD_NATIVE_ADDRESS_NAME, &value)) {
                    LOGE("invalid parameters");
                    break;
                }
                if (value == 0 ||
                    !CJNIModuleManager::getInstance()->contains((T *) value)) {
                    LOGE("get %s from java object failed", name);

                } else {
                    t = (T *) value;
                }

            } while (0);

            env->PopLocalFrame(NULL);
            return t;
        }

        void destroyJniObject(CJNIModule *t) {
//            USE_LOG;
            if (t == NULL || !CJNIModuleManager::getInstance()->contains(t)) {
                LOGE("invalid parameters");
                return;
            }

            JNIEnv *env = NULL;
            if (CJNIModule::m_spJVM->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
                LOGE("get JNIEnv failed");
                return;
            }
            env->PushLocalFrame(10);
            const char *name = typeid(*t).name();
            do {
                jlong value = 0;
                if (!getLongField(env, t->getObject(), MODULE_FIELD_NATIVE_ADDRESS_NAME, &value)) {
                    LOGE("invalid parameters");
                    break;
                }
                if (value != (jlong) t) {
                    LOGE("java field is changed improperly!!!!!!");
                }

                if (!setLongField(env, t->getObject(), MODULE_FIELD_NATIVE_ADDRESS_NAME, 0)) {
                    LOGE("invalid parameters");
                    break;
                }
                delete t;

                LOGI("destroy %s ok", name);
            } while (0);
            env->PopLocalFrame(NULL);
        }
    };
}

#endif /* _PAOMIANTV_JNIMODULEMANAGER_H_ */
