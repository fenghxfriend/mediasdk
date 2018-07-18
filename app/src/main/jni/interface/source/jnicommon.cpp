/*******************************************************************************
 *        Module: interface
 *          File: jnicommon.cpp
 * Functionality: jnicommon function.
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
#include "jnicommon.h"
#include "string.h"

namespace paomiantv {

    BOOL32 setIntField(JNIEnv *env, jobject obj, const s8 *fieldName, s32 value) {
        BOOL32 result = FALSE;

        do {
            if (!env || !obj || !fieldName)
                break;

            jclass clazz = (env)->GetObjectClass(obj);
            if (!clazz)
                break;

            jfieldID fieldID = (env)->GetFieldID(clazz, fieldName, "I");
            if (!fieldID)
                break;

            (env)->SetIntField(obj, fieldID, value);

            result = TRUE;

        } while (0);

        return result;
    }

    BOOL32 setLongField(JNIEnv *env, jobject obj, const s8 *fieldName, s64 value) {
        BOOL32 result = FALSE;

        do {
            if (!env || !obj || !fieldName)
                break;

            jclass clazz = (env)->GetObjectClass(obj);
            if (!clazz)
                break;

            jfieldID fieldID = (env)->GetFieldID(clazz, fieldName, "J");
            if (!fieldID)
                break;

            (env)->SetLongField(obj, fieldID, value);

            result = TRUE;

        } while (0);

        return result;
    }

    BOOL32 getIntField(JNIEnv *env, jobject obj, const s8 *fieldName, jint *pValue) {
        BOOL32 result = FALSE;

        do {
            if (!env || !obj || !fieldName || !pValue)
                break;

            jclass clazz = (env)->GetObjectClass(obj);
            if (!clazz)
                break;

            jfieldID fieldID = (env)->GetFieldID(clazz, fieldName, "I");
            if (!fieldID)
                break;
            *pValue = (env)->GetIntField(obj, fieldID);
            result = TRUE;

        } while (0);

        return result;
    }

    BOOL32 getLongField(JNIEnv *env, jobject obj, const s8 *fieldName, jlong *pValue) {
        BOOL32 result = FALSE;

        do {
            if (!env || !obj || !fieldName || !pValue)
                break;

            jclass clazz = (env)->GetObjectClass(obj);
            if (!clazz)
                break;

            jfieldID fieldID = (env)->GetFieldID(clazz, fieldName, "J");
            if (!fieldID)
                break;
            *pValue = (env)->GetLongField(obj, fieldID);
            result = TRUE;

        } while (0);

        return result;
    }

    BOOL32 getObjectField(JNIEnv *env, jobject obj, const s8 *fieldName,const s8* fieldSigniture, jobject *pValue) {
        BOOL32 result = FALSE;

        do {
            if (!env || !obj || !fieldName || !pValue)
                break;

            jclass clazz = (env)->GetObjectClass(obj);
            if (!clazz)
                break;

            jfieldID fieldID = (env)->GetFieldID(clazz, fieldName, fieldSigniture);
            if (!fieldID)
                break;
            *pValue = (env)->GetObjectField(obj, fieldID);
            result = TRUE;

        } while (0);

        return result;
    }

    s32
    setByteArrayField(JNIEnv *env, jobject obj, const s8 *fieldName, const s8 *value, s32 size) {
        s32 result = -1;

        do {
            if (!env || !obj || !fieldName || !value || size <= 0)
                break;
            result = strlen(value);
            if (result <= 0 || result > size) {
                break;
            }

            jclass clazz = (env)->GetObjectClass(obj);
            if (!clazz) {
                break;
            }

            jfieldID fieldID = (env)->GetFieldID(clazz, fieldName, "[B");
            if (!fieldID) {
                break;
            }

            jbyteArray byteArray = (jbyteArray) ((env)->GetObjectField(obj, fieldID));

            if (!byteArray) {
                break;
            }

            if (((env)->GetArrayLength(byteArray)) != size) {
                break;
            }

            (env)->SetByteArrayRegion(byteArray, 0, result, (jbyte *) value);

        } while (0);

        return result;
    }

    BOOL32
    setIntArrayField(JNIEnv *env, jobject obj, const s8 *fieldName, const s32 *value, s32 size) {
        BOOL32 result = FALSE;

        do {
            if (!env || !obj || !fieldName || !value || size <= 0)
                break;

            jclass clazz = (env)->GetObjectClass(obj);
            if (!clazz)
                break;

            jfieldID fieldID = (env)->GetFieldID(clazz, fieldName, "[I");
            if (!fieldID)
                break;

            jintArray intArray = (jintArray) ((env)->GetObjectField(obj, fieldID));

            if (!intArray)
                break;

            if (((env)->GetArrayLength(intArray)) != size)
                break;

            (env)->SetIntArrayRegion(intArray, 0, size, value);

            result = TRUE;

        } while (0);

        return result;
    }

    BOOL32 set2DInt2ArrayField(JNIEnv *env, jobject obj, const s8 *fieldName, const s32 (*value)[2],
                               s32 size) {
        BOOL32 result = FALSE;

        do {
            if (!env || !obj || !fieldName || !value || size <= 0)
                break;

            jclass clazz = (env)->GetObjectClass(obj);
            if (!clazz)
                break;

            jfieldID fieldID = (env)->GetFieldID(clazz, fieldName, "[[I");
            if (!fieldID)
                break;

            jobjectArray objectArray = (jobjectArray) ((env)->GetObjectField(obj, fieldID));
            if (!objectArray)
                break;

            if (((env)->GetArrayLength(objectArray)) != size)
                break;

            jintArray intArray;
            for (int i = 0; i < size; i++) {
                intArray = (jintArray) ((env)->GetObjectArrayElement(objectArray, i));
                (env)->SetIntArrayRegion(intArray, 0, 2, value[i]);
            }

            result = TRUE;

        } while (0);

        return result;
    }

    s32 getStringBytes(JNIEnv *env, jstring obj, s8 *buffer, s32 size) {
        s32 result = -1;

        do {
            if (!env || !obj || !buffer || size <= 0)
                break;

            const s8 *stringPtr = (env)->GetStringUTFChars(obj, JNI_FALSE);
            //printf("RTSP server addr is %s",stringPtr);
            if (!stringPtr)
                break;

            result = strlen(stringPtr);
            //if ( result <= size && result > 0 )
            if (result < size && result > 0) {
                strncpy(buffer, stringPtr, size);
            }
            (env)->ReleaseStringUTFChars(obj, stringPtr);

        } while (0);

        return result;
    }

    s32
    getStringBytesFromObjFld(JNIEnv *env, jobject obj, const s8 *fieldName, s8 *buffer, s32 size) {
        s32 result = -1;

        do {
            if (!env || !obj || !fieldName || !buffer || size <= 0)
                break;

            jclass clazz = (env)->GetObjectClass(obj);
            if (!clazz)
                break;

            jfieldID fieldID = (env)->GetFieldID(clazz, fieldName, "Ljava/lang/String;");
            if (!fieldID)
                break;

            jstring stringObject = (jstring) ((env)->GetObjectField(obj, fieldID));
            if (!stringObject)
                break;

            result = getStringBytes(env, stringObject, buffer, size);

        } while (0);

        return result;
    }

    jstring stringToJString(JNIEnv *env, const s8 *pchPat) {
        if (pchPat == NULL || strlen(pchPat) == 0)
            return NULL;
        env->PushLocalFrame(10);
        int nPatLen = strlen(pchPat);
        jclass jclass_String = env->FindClass("java/lang/String");
        jmethodID jinit_method = env->GetMethodID(jclass_String, "<init>",
                                                  "([BLjava/lang/String;)V");
        jbyteArray jbytes = env->NewByteArray(nPatLen);
        env->SetByteArrayRegion(jbytes, 0, nPatLen, (jbyte *) pchPat);
        jstring jencoding = env->NewStringUTF("UTF-8");
        jobject jobj = env->NewObject(jclass_String, jinit_method, jbytes, jencoding);
        jstring result = (jstring) env->PopLocalFrame(jobj);

        return result;
    }

    jbyteArray StringToJByteArr(JNIEnv *env, unsigned char *pchPat, int nPatLen) {
        if (pchPat == NULL || nPatLen == 0)
            return NULL;
        env->PushLocalFrame(10);
        jbyteArray jbytes = env->NewByteArray(nPatLen);
        env->SetByteArrayRegion(jbytes, 0, nPatLen, (jbyte *) pchPat);
        jbyteArray result = (jbyteArray) env->PopLocalFrame(jbytes);
        return result;
    }

    jclass findClass(JNIEnv *env, jobject classLoader, jmethodID findClassMethod, const s8 *name) {
        return static_cast<jclass>(env->CallObjectMethod(classLoader, findClassMethod,
                                                         env->NewStringUTF(name)));
    }
}
