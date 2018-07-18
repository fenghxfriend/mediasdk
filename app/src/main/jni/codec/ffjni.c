/*
 * JNI utility functions
 *
 * Copyright (c) 2015-2016 Matthieu Bouron <matthieu.bouron stupeflix.com>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <jni.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <autolog.h>
#include "jnivm.h"
#include "ffjni.h"

static JavaVM *java_vm;
static pthread_key_t current_env;
static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void jni_detach_env(void *data) {
    if (java_vm) {
        (*java_vm)->DetachCurrentThread(java_vm);
    }
}

static void jni_create_pthread_key(void) {
    pthread_key_create(&current_env, jni_detach_env);
}

JNIEnv *ff_jni_get_env() {
    int ret = 0;
    JNIEnv *env = NULL;

    pthread_mutex_lock(&lock);
    if (java_vm == NULL) {
        java_vm = av_jni_get_java_vm();
    }

    if (!java_vm) {
        LOGE("No Java virtual machine has been registered\n");
        goto done;
    }

    pthread_once(&once, jni_create_pthread_key);

    if ((env = pthread_getspecific(current_env)) != NULL) {
        goto done;
    }

    ret = (*java_vm)->GetEnv(java_vm, (void **) &env, JNI_VERSION_1_6);
    switch (ret) {
        case JNI_EDETACHED:
            if ((*java_vm)->AttachCurrentThread(java_vm, &env, NULL) != 0) {
                LOGE("Failed to attach the JNI environment to the current thread\n");
                env = NULL;
            } else {
                pthread_setspecific(current_env, env);
            }
            break;
        case JNI_OK:
            break;
        case JNI_EVERSION:
            LOGE("The specified JNI version is not supported\n");
            break;
        default:
            LOGE("Failed to get the JNI environment attached to this thread\n");
            break;
    }

    done:
    pthread_mutex_unlock(&lock);
    return env;
}

char *ff_jni_jstring_to_utf_chars(JNIEnv *env, jstring string) {
    char *ret = NULL;
    const char *utf_chars = NULL;

    jboolean copy = 0;

    if (!string) {
        return NULL;
    }

    utf_chars = (*env)->GetStringUTFChars(env, string, &copy);
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
        LOGE("String.getStringUTFChars() threw an exception\n");
        return NULL;
    }

    if (utf_chars) {
        size_t len = strlen(utf_chars) + 1;
        if (len > 0) {
            ret = realloc(NULL, len);
            if (ret){
                memcpy(ret, utf_chars, len);
                ret[len] = '\0';
            }
        }
    }

    (*env)->ReleaseStringUTFChars(env, string, utf_chars);
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
        LOGE("String.releaseStringUTFChars() threw an exception\n");
        return NULL;
    }

    return ret;
}

jstring ff_jni_utf_chars_to_jstring(JNIEnv *env, const char *utf_chars) {
    jstring ret;

    ret = (*env)->NewStringUTF(env, utf_chars);
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
        LOGE("NewStringUTF() threw an exception\n");
        return NULL;
    }

    return ret;
}

int ff_jni_exception_get_summary(JNIEnv *env, jthrowable exception, char **error) {
    int ret = 0;

    char *name = NULL;
    char *message = NULL;

    jclass class_class = NULL;
    jmethodID get_name_id = NULL;

    jclass exception_class = NULL;
    jmethodID get_message_id = NULL;

    jstring string = NULL;

    exception_class = (*env)->GetObjectClass(env, exception);
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
        LOGE("Could not find Throwable class\n");
        ret = AV_FLAG_EXTERNAL;
        goto done;
    }

    class_class = (*env)->GetObjectClass(env, exception_class);
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
        LOGE("Could not find Throwable class's class\n");
        ret = AV_FLAG_EXTERNAL;
        goto done;
    }

    get_name_id = (*env)->GetMethodID(env, class_class, "getName", "()Ljava/lang/String;");
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
        LOGE("Could not find method Class.getName()\n");
        ret = AV_FLAG_EXTERNAL;
        goto done;
    }

    string = (*env)->CallObjectMethod(env, exception_class, get_name_id);
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
        LOGE("Class.getName() threw an exception\n");
        ret = AV_FLAG_EXTERNAL;
        goto done;
    }

    if (string) {
        name = ff_jni_jstring_to_utf_chars(env, string);
        (*env)->DeleteLocalRef(env, string);
        string = NULL;
    }

    get_message_id = (*env)->GetMethodID(env, exception_class, "getMessage",
                                         "()Ljava/lang/String;");
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
        LOGE("Could not find method java/lang/Throwable.getMessage()\n");
        ret = AV_FLAG_EXTERNAL;
        goto done;
    }

    string = (*env)->CallObjectMethod(env, exception, get_message_id);
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
        LOGE("Throwable.getMessage() threw an exception\n");
        ret = AV_FLAG_EXTERNAL;
        goto done;
    }

    if (string) {
        message = ff_jni_jstring_to_utf_chars(env, string);
        (*env)->DeleteLocalRef(env, string);
        string = NULL;
    }
    char *buffer = NULL;
    size_t len = (name ? strlen(name) : 0) + (message ? strlen(message) : 0) + 32;
    buffer = realloc(NULL, len);
    if (!buffer) {
        ret = AV_FLAG_NOMEM;
        goto done;
    }
    if (name && message) {
        snprintf(buffer, len, "%s: %s", name, message);
    } else if (name && !message) {
        snprintf(buffer, "%s occurred", name);
    } else if (!name && message) {
        snprintf(buffer, len, "Exception: %s", message);
    } else {
        LOGW("Could not retrieve exception name and message\n");
        snprintf(buffer, len, "Exception occurred");
    }

    if (error) {
        *error = buffer;
    }

    done:
    if (name) {
        free(name);
    }
    if (message) {
        free(message);
    }

    if (class_class) {
        (*env)->DeleteLocalRef(env, class_class);
    }

    if (exception_class) {
        (*env)->DeleteLocalRef(env, exception_class);
    }

    if (string) {
        (*env)->DeleteLocalRef(env, string);
    }

    return ret;
}

int ff_jni_exception_check(JNIEnv *env, int log) {
    int ret;

    jthrowable exception;

    char *message = NULL;

    if (!(*(env))->ExceptionCheck((env))) {
        return 0;
    }

    if (!log) {
        (*(env))->ExceptionClear((env));
        return -1;
    }

    exception = (*env)->ExceptionOccurred(env);
    (*(env))->ExceptionClear((env));

    if ((ret = ff_jni_exception_get_summary(env, exception, &message)) < 0) {
        (*env)->DeleteLocalRef(env, exception);
        return ret;
    }

    (*env)->DeleteLocalRef(env, exception);

    LOGE("%s\n", message);
    free(message);

    return -1;
}

int ff_jni_init_jfields(JNIEnv *env, void *jfields, const struct FFJniField *jfields_mapping,
                        int global) {
    int i, ret = 0;
    jclass last_clazz = NULL;

    for (i = 0; jfields_mapping[i].name; i++) {
        int mandatory = jfields_mapping[i].mandatory;
        enum FFJniFieldType type = jfields_mapping[i].type;

        if (type == FF_JNI_CLASS) {
            jclass clazz;

            last_clazz = NULL;

            clazz = (*env)->FindClass(env, jfields_mapping[i].name);
            if ((ret = ff_jni_exception_check(env, mandatory)) < 0 && mandatory) {
                goto done;
            }

            last_clazz = *(jclass *) ((uint8_t *) jfields + jfields_mapping[i].offset) =
                    global ? (*env)->NewGlobalRef(env, clazz) : clazz;

            if (global) {
                (*env)->DeleteLocalRef(env, clazz);
            }

        } else {

            if (!last_clazz) {
                ret = AV_FLAG_EXTERNAL;
                break;
            }

            switch (type) {
                case FF_JNI_FIELD: {
                    jfieldID field_id = (*env)->GetFieldID(env, last_clazz,
                                                           jfields_mapping[i].method,
                                                           jfields_mapping[i].signature);
                    if ((ret = ff_jni_exception_check(env, mandatory)) < 0 && mandatory) {
                        goto done;
                    }

                    *(jfieldID *) ((uint8_t *) jfields + jfields_mapping[i].offset) = field_id;
                    break;
                }
                case FF_JNI_STATIC_FIELD: {
                    jfieldID field_id = (*env)->GetStaticFieldID(env, last_clazz,
                                                                 jfields_mapping[i].method,
                                                                 jfields_mapping[i].signature);
                    if ((ret = ff_jni_exception_check(env, mandatory)) < 0 && mandatory) {
                        goto done;
                    }

                    *(jfieldID *) ((uint8_t *) jfields + jfields_mapping[i].offset) = field_id;
                    break;
                }
                case FF_JNI_METHOD: {
                    jmethodID method_id = (*env)->GetMethodID(env, last_clazz,
                                                              jfields_mapping[i].method,
                                                              jfields_mapping[i].signature);
                    if ((ret = ff_jni_exception_check(env, mandatory)) < 0 && mandatory) {
                        goto done;
                    }

                    *(jmethodID *) ((uint8_t *) jfields + jfields_mapping[i].offset) = method_id;
                    break;
                }
                case FF_JNI_STATIC_METHOD: {
                    jmethodID method_id = (*env)->GetStaticMethodID(env, last_clazz,
                                                                    jfields_mapping[i].method,
                                                                    jfields_mapping[i].signature);
                    if ((ret = ff_jni_exception_check(env, mandatory)) < 0 && mandatory) {
                        goto done;
                    }

                    *(jmethodID *) ((uint8_t *) jfields + jfields_mapping[i].offset) = method_id;
                    break;
                }
                default:
                    LOGE("Unknown JNI field type\n");
                    ret = AV_FLAG_INVAL;
                    goto done;
            }

            ret = 0;
        }
    }

    done:
    if (ret < 0) {
        /* reset jfields in case of failure so it does not leak references */
        ff_jni_reset_jfields(env, jfields, jfields_mapping, global);
    }

    return ret;
}

int ff_jni_reset_jfields(JNIEnv *env, void *jfields, const struct FFJniField *jfields_mapping,
                         int global) {
    int i;

    for (i = 0; jfields_mapping[i].name; i++) {
        enum FFJniFieldType type = jfields_mapping[i].type;

        switch (type) {
            case FF_JNI_CLASS: {
                jclass clazz = *(jclass *) ((uint8_t *) jfields + jfields_mapping[i].offset);
                if (!clazz)
                    continue;

                if (global) {
                    (*env)->DeleteGlobalRef(env, clazz);
                } else {
                    (*env)->DeleteLocalRef(env, clazz);
                }

                *(jclass *) ((uint8_t *) jfields + jfields_mapping[i].offset) = NULL;
                break;
            }
            case FF_JNI_FIELD: {
                *(jfieldID *) ((uint8_t *) jfields + jfields_mapping[i].offset) = NULL;
                break;
            }
            case FF_JNI_STATIC_FIELD: {
                *(jfieldID *) ((uint8_t *) jfields + jfields_mapping[i].offset) = NULL;
                break;
            }
            case FF_JNI_METHOD: {
                *(jmethodID *) ((uint8_t *) jfields + jfields_mapping[i].offset) = NULL;
                break;
            }
            case FF_JNI_STATIC_METHOD: {
                *(jmethodID *) ((uint8_t *) jfields + jfields_mapping[i].offset) = NULL;
                break;
            }
            default:
                LOGE("Unknown JNI field type\n");
        }
    }

    return 0;
}