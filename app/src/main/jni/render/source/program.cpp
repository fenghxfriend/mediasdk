//
// Created by ASUS on 2018/1/19.
//

#include "../../common/autolog.h"
#include "program.h"

namespace paomiantv {

    CProgram::CProgram() :
            m_uId(0) {
        m_mapAttr.clear();
        m_mapUniform.clear();
    }

    CProgram::~CProgram() {
        uninit();
    }


    void CProgram::uninit() {
        if (m_uId != 0) {
            glDeleteProgram(m_uId);
            m_uId = 0;
        }
        m_mapUniform.clear();
        m_mapAttr.clear();
    }

    BOOL32 CProgram::compileProgram(CShader *pVertexShader, CShader *pFragmentShader) {
        if (pVertexShader->getId() == 0 || pVertexShader->getId() == 0) {
            return FALSE;
        }

        m_uId = glCreateProgram();

        if (m_uId == 0) {
            CShader::checkGlError("glCreateProgram");
            return FALSE;
        }
        glAttachShader(m_uId, pVertexShader->getId());
        glAttachShader(m_uId, pFragmentShader->getId());
        glLinkProgram(m_uId);

        GLint status = 0;
        glGetProgramiv(m_uId, GL_LINK_STATUS, &status);
        if (GL_FALSE == status) {
            GLint infoLen = 0;
            glGetProgramiv(m_uId, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 1) {
                char *infoLog = new char[infoLen];
                glGetProgramInfoLog(m_uId, infoLen, NULL, infoLog);
                LOGE("glProgram error : %s", infoLog);
                delete[] infoLog;
            }
            //CShader::checkGlError("glLinkProgram");
            glDeleteProgram(m_uId);
            m_uId = 0;
            return FALSE;
        }

        parseLocations();
        parseUniforms();
        return TRUE;
    }

    void CProgram::parseLocations() {
        GLint attrCount = 0;
        glGetProgramiv(m_uId, GL_ACTIVE_ATTRIBUTES, &attrCount);
        if (0 < attrCount) {
            GLint len = 0;
            glGetProgramiv(m_uId, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &len);
            if (0 < len) {
                GLchar attrName[64] = {0};
                GLint attrSize = 0;
                GLenum attrType = 0;

                for (GLuint i = 0; i < attrCount; ++i) {
                    glGetActiveAttrib(m_uId, i, len, NULL, &attrSize, &attrType, attrName);
                    attrName[len] = '\0';
                    m_mapAttr[attrName] = glGetAttribLocation(m_uId, attrName);
                }
            }
        }
    }

    void CProgram::parseUniforms() {
        GLint uniformCount = 0;
        glGetProgramiv(m_uId, GL_ACTIVE_UNIFORMS, &uniformCount);
        if (0 < uniformCount) {
            GLint len = 0;
            glGetProgramiv(m_uId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &len);
            if (0 < len) {
                GLchar uniformName[64] = {0};
                GLint uniformSize;
                GLenum uniformType;

                for (GLuint i = 0; i < uniformCount; ++i) {
                    glGetActiveUniform(m_uId, i, len, NULL, &uniformSize, &uniformType,
                                       uniformName);
                    uniformName[len] = {0};
                    m_mapUniform[uniformName] = glGetUniformLocation(m_uId, uniformName);
                }
            }
        }
    }

    GLuint CProgram::getProgramId() const {
        return m_uId;
    }

    GLuint CProgram::getUniform(const char *uniformName) {
        GLuint uniform = 0;
        if (m_mapUniform.end() != m_mapUniform.find(uniformName)) {
            uniform = m_mapUniform[uniformName];
        }
        return uniform;
    }

    GLuint CProgram::getAttribute(const char *attributeName) {
        GLuint attribute = 0;
        if (m_mapAttr.end() != m_mapAttr.find(attributeName)) {
            attribute = m_mapAttr[attributeName];
        }
        return attribute;
    }

    void CProgram::enable() {
        glValidateProgram(m_uId);
        GLint status;
        glGetProgramiv(m_uId, GL_LINK_STATUS, &status);
        if (GL_FALSE == status) {
            glDeleteProgram(m_uId);
            m_uId = 0;
            LOGE("use program failed!");
        } else {
            glUseProgram(m_uId);
        }
    }

    void CProgram::disable() {
        glUseProgram(0);
    }
}