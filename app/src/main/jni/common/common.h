/*******************************************************************************
 *        Module: common
 *          File:
 * Functionality:
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 PAOMIANTV, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-08-11  v1.0        huangxuefeng  created
 ******************************************************************************/
#ifndef _PAOMIANTV_COMMON_H
#define _PAOMIANTV_COMMON_H

#include "typedef.h"
#include <stdlib.h>
#include<time.h>
#include <string>
#include <SLES/OpenSLES.h>

#ifdef __cplusplus
extern "C"
{
#endif

u32 LToB(u32 InputNum) {
    u8 *p = (u8 *) &InputNum;
    return ((*p << 24) & 0xFF000000) + ((*(p + 1) << 16) & 0x00FF0000) +
           ((*(p + 2) << 8) & 0x0000FF00) + ((*(p + 3)) & 0x000000FF);
}

u32 BToL(u32 InputNum) {
    u8 *p = (u8 *) &InputNum;
    return (*p & 0x000000FF) + ((*(p + 1) << 8) & 0x0000FF00) +
           ((*(p + 2) << 16) & 0x00FF0000) + ((*(p + 3) << 24) & 0xFF000000);
}

#ifdef __cplusplus
};
#endif


#endif //_PAOMIANTV_COMMON_H
