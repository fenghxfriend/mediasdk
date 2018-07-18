/*******************************************************************************
 *        Module: interface
 *          File: jnicommon.h
 * Functionality: common jni.
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

#ifndef _PAOMIANTV_JNICOMMON_H_
#define _PAOMIANTV_JNICOMMON_H_

#include <jni.h>
#include "../../common/typedef.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

namespace paomiantv {

    BOOL32 setIntField(JNIEnv *env, jobject obj, const s8 *fieldName, s32 value);

    BOOL32 getIntField(JNIEnv *env, jobject obj, const s8 *fieldName, jint *value);

    BOOL32 setLongField(JNIEnv *env, jobject obj, const s8 *fieldName, s64 value);

    BOOL32 getLongField(JNIEnv *env, jobject obj, const s8 *fieldName, jlong *value);

    BOOL32 getObjectField(JNIEnv *env, jobject obj, const s8 *fieldName,const s8* fieldSigniture, jobject *pValue);

    s32 setByteArrayField(JNIEnv *env, jobject obj, const s8 *fieldName, const s8 *value, s32 size);

    BOOL32
    setIntArrayField(JNIEnv *env, jobject obj, const s8 *fieldName, const s32 *value, s32 size);

    BOOL32 set2DInt2ArrayField(JNIEnv *env, jobject obj, const s8 *fieldName, const s32 (*value)[2],
                               s32 size);

    s32
    getStringBytesFromObjFld(JNIEnv *env, jobject obj, const s8 *fieldName, u8 *buffer, s32 size);

    s32 getStringBytes(JNIEnv *env, jstring obj, s8 *buffer, s32 size);

    jstring stringToJString(JNIEnv *env, const s8 *pchPat);

    jbyteArray StringToJByteArr(JNIEnv *env, unsigned char *pchPat, int nPatLen);

    jclass findClass(JNIEnv *env, jobject classLoader,jmethodID findClassMethod,const s8* name);



} // namespace paomiantv

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // _PAOMIANTV_JNICOMMON_H_