//
// Created by ASUS on 2018/1/19.
//

#ifndef MEDIAENGINE_CPROGRAM_H
#define MEDIAENGINE_CPROGRAM_H

#include <GLES2/gl2.h>
#include <string>
#include <unordered_map>
#include "../../common/typedef.h"
#include "shader.h"

namespace paomiantv {
    class CProgram {
    public:
        CProgram();

        virtual ~CProgram();

        void uninit();

        BOOL32 compileProgram(CShader *pVertexShader, CShader *pFragmentShader);

        GLuint getUniform(const char *uniformName);

        GLuint getAttribute(const char *uniformName);

        GLuint getProgramId() const;

        void enable();

        void disable();

    private:
        GLuint m_uId;
        std::unordered_map<std::string, GLuint> m_mapAttr;
        std::unordered_map<std::string, GLuint> m_mapUniform;
        void parseLocations();

        void parseUniforms();
    };
}
#endif //MEDIAENGINE_CPROGRAM_H
