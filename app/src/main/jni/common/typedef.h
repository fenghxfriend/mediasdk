/*******************************************************************************
 *        Module: common
 *          File: 
 * Functionality: Type definition (the types we used to want).
 *       Related: 
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 PAOMIANTV, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-20  v1.0        huangxuefeng  created
 ******************************************************************************/
#ifndef _PAOMIANTV_TYPEDEF_H_
#define _PAOMIANTV_TYPEDEF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>
typedef char s8;
/* see comments above */
//typedef int8_t s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;

typedef s32 BOOL32;

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef NULL
#define NULL  0
#endif

#ifndef MAX
#define MAX(a, b) ((a)>(b)?(a):(b))
#endif

#ifndef MIN
#define MIN(a, b) ((a)>(b)?(b):(a))
#endif

#define DEFAULT_ALIGN_DATA 16

#ifndef ALIGN
#define ALIGN(x, a) (((x)+(a)-1)&~((a)-1))
#endif

#ifndef NELEM
#define NELEM(x) ((int)(sizeof(x) / sizeof((x)[0])))
#endif

#ifndef OnMessageCB
typedef void (*OnMessageCB)(void *, s32, s8 *);
#endif

#ifndef OnWriteCB
typedef void (*OnWritePCMCB)(void *, u64, s32, u8 *, BOOL32);
#endif


#ifndef OnStartVEncodeCB
typedef BOOL32 (*OnStartVEncodeCB)(void *, const s8 *, int, int, int, int, int, int, int);
#endif

#ifndef OnStartAEncodeCB
typedef BOOL32 (*OnStartAEncodeCB)(void *, const s8 *, int, int, int, int);
#endif

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _PAOMIANTV_TYPEDEF_H_ */