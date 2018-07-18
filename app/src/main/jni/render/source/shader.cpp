/*******************************************************************************
 *        Module: render
 *          File: shader.cpp
 * Functionality: tools for opengl shader programs.
 *       Related: GLES2
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-08-01  v1.0        huangxuefeng  created
 ******************************************************************************/

#include <stdlib.h>
#include "../../common/autolog.h"
#include "shader.h"

namespace paomiantv {

    CShader::CShader(GLenum uType) : m_uType(uType) {
//        USE_LOG;
        m_uId = 0;
    }

    CShader::~CShader() {
//        USE_LOG;
        if (m_uId != 0) {
            glDeleteShader(m_uId);
            m_uId = 0;
            m_uType = 0;
        }
    }

    BOOL32 CShader::loadShader(const char *pchShader) {
        m_uId = glCreateShader(m_uType);
        if (m_uId == 0) {
            checkGlError("glCreateShader");
            return FALSE;
        }
        GLint compiled;
        glShaderSource(m_uId, 1, &pchShader, NULL);
        glCompileShader(m_uId);
        glGetShaderiv(m_uId, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(m_uId, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 1) {
                char *infoLog = new char[infoLen];
                glGetShaderInfoLog(m_uId, infoLen, NULL, infoLog);
                LOGE("Could not compile %s shader:\n%s\n",
                     m_uType == GL_VERTEX_SHADER ? "vertex" : "fragment",
                     infoLog);
                delete[] infoLog;
            }

            glDeleteShader(m_uId);
            m_uId = 0;
            return FALSE;
        }
        return TRUE;
    }

    bool CShader::checkGlError(const char *funcName) {
        GLint err = glGetError();
        if (err != GL_NO_ERROR) {
            LOGE("GL error after %s(): 0x%08x\n", funcName, err);
            return true;
        }
        return false;
    }

}
