/*******************************************************************************
 *        Module: common
 *          File: 
 * Functionality: auto file
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
#ifndef _PAOMIANTV_AUTOFILE_H_
#define _PAOMIANTV_AUTOFILE_H_

#include "typedef.h"
#include <stdio.h>

namespace paomiantv {

class CAutoFile
{
public:
    CAutoFile(const char * szFilename, const char * szMode = "rb")
    {
        m_hFile = fopen(szFilename, szMode);
    }

    ~CAutoFile()
    {
        if (m_hFile)
        {
            fclose(m_hFile);
        }
    }

    u32 write(const void *ptr, u32 bytes)
    {
        if (!ptr || !m_hFile)
        {
            return 0;
        }
        return fwrite(ptr, 1, bytes, m_hFile);
    }

    u32 read(void *ptr, u32 max_bytes)
    {
        if (!ptr || !m_hFile)
        {
            return 0;
        }
        return fread(ptr, 1, max_bytes, m_hFile);
    }

    BOOL32 isValid()
    {
        return m_hFile != NULL;
    }

    void flush()
    {
        if (!m_hFile)
        {
            return;
        }
        fflush(m_hFile);
    }

    operator FILE*()
    {
        return m_hFile;
    }

private:
    FILE *m_hFile;
};


} // namepsace paomiantv

#endif // _PAOMIANTV_AUTOFILE_H_