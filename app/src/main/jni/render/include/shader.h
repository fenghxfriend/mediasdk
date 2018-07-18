/*******************************************************************************
 *        Module: render
 *          File: shader.h
 * Functionality: tools for opengl shader programs.
 *       Related: GLES2
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2014-01-19  v1.0        huangxuefeng  created
 ******************************************************************************/
#ifndef _PAOMIANTV_SHADER_H_
#define _PAOMIANTV_SHADER_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <string>


#define DEFAULT_VERTEXT_SHADER \
                "uniform vec3 u_posTranslate;                                           \n"\
                "uniform vec3 u_posScale;                                               \n"\
                "uniform vec3 u_posRotate;                                              \n"\
                "uniform vec3 u_uvTranslate;                                            \n"\
                "uniform vec3 u_uvScale;                                                \n"\
                "uniform vec3 u_uvRotate;                                               \n"\
                "attribute vec4 a_position;                                             \n"\
                "attribute vec4 a_texCoord;                                             \n"\
                "varying highp vec4 v_texCoord;                                         \n"\
                "void main(){                                                           \n"\
                "    vec3 v3_radian = vec3(radians(u_posRotate.x),                      \n"\
                "                          radians(u_posRotate.y),                      \n"\
                "                          radians(u_posRotate.z));                     \n"\
                "    vec2 v2_xProperty = vec2(sin(v3_radian.x),cos(v3_radian.x));       \n"\
                "    vec2 v2_yProperty = vec2(sin(v3_radian.y),cos(v3_radian.y));       \n"\
                "    vec2 v2_zProperty = vec2(sin(v3_radian.z),cos(v3_radian.z));       \n"\
                "    mat4 m4_scale = mat4(u_posScale.x, 0.0, 0.0, 0.0,                  \n"\
                "                         0.0, u_posScale.y, 0.0, 0.0,                  \n"\
                "                         0.0, 0.0, u_posScale.z, 0.0,                  \n"\
                "                         0.0, 0.0, 0.0, 1.0);                          \n"\
                "    mat4 m4_translate = mat4(1.0, 0.0, 0.0, 0.0,                       \n"\
                "                             0.0, 1.0, 0.0, 0.0,                       \n"\
                "                             0.0, 0.0, 1.0, 0.0,                       \n"\
                "                             u_posTranslate.x, u_posTranslate.y, u_posTranslate.z, 1.0);\n"\
                "    mat4 m4_xRotation = mat4(1.0, 0.0, 0.0, 0.0,                       \n"\
                "                             0.0, v2_xProperty.y, -v2_xProperty.x, 0.0,  \n"\
                "                             0.0, v2_xProperty.x, v2_xProperty.y, 0.0,   \n"\
                "                             0.0, 0.0, 0.0, 1.0);                        \n"\
                "    mat4 m4_yRotation = mat4(v2_yProperty.y, 0.0, v2_yProperty.x, 0.0,   \n"\
                "                             0.0, 1.0, 0.0, 0.0,                         \n"\
                "                             -v2_yProperty.x, 0.0, v2_yProperty.y, 0.0,  \n"\
                "                             0.0, 0.0, 0.0, 1.0);                        \n"\
                "    mat4 m4_zRotation = mat4(v2_zProperty.y, -v2_zProperty.x, 0.0, 0.0,  \n"\
                "                             v2_zProperty.x, v2_zProperty.y, 0.0, 0.0,   \n"\
                "                             0.0, 0.0, 1.0, 0.0,                         \n"\
                "                             0.0, 0.0, 0.0, 1.0);                        \n"\
                "    gl_Position = m4_translate * m4_zRotation * m4_yRotation * m4_xRotation * m4_scale * a_position; \n"\
                "    v3_radian = vec3(radians(u_uvRotate.x),                                \n"\
                "                     radians(u_uvRotate.y),                                \n"\
                "                     radians(u_uvRotate.z));                               \n"\
                "    v2_xProperty = vec2(sin(v3_radian.x),cos(v3_radian.x));                \n"\
                "    v2_yProperty = vec2(sin(v3_radian.y),cos(v3_radian.y));                \n"\
                "    v2_zProperty = vec2(sin(v3_radian.z),cos(v3_radian.z));                \n"\
                "    m4_scale = mat4(u_uvScale.x, 0.0, 0.0, 0.0,                            \n"\
                "                    0.0, u_uvScale.y, 0.0, 0.0,                            \n"\
                "                    0.0, 0.0, u_uvScale.z, 0.0,                            \n"\
                "                    0.0, 0.0, 0.0, 1.0);                                   \n"\
                "    m4_translate = mat4(1.0, 0.0, 0.0, 0.0,                                \n"\
                "                        0.0, 1.0, 0.0, 0.0,                                \n"\
                "                        0.0, 0.0, 1.0, 0.0,                                \n"\
                "                        u_uvTranslate.x, u_uvTranslate.y, u_uvTranslate.z, 1.0);\n"\
                "    m4_xRotation = mat4(1.0, 0.0, 0.0, 0.0,                                \n"\
                "                        0.0, v2_xProperty.y, -v2_xProperty.x, 0.0,         \n"\
                "                        0.0, v2_xProperty.x, v2_xProperty.y, 0.0,          \n"\
                "                        0.0, 0.0, 0.0, 1.0);                               \n"\
                "    m4_yRotation = mat4(v2_yProperty.y, 0.0, v2_yProperty.x, 0.0,          \n"\
                "                        0.0, 1.0, 0.0, 0.0,                                \n"\
                "                        -v2_yProperty.x, 0.0, v2_yProperty.y, 0.0,         \n"\
                "                        0.0, 0.0, 0.0, 1.0);                               \n"\
                "    m4_zRotation = mat4(v2_zProperty.y, -v2_zProperty.x, 0.0, 0.0,         \n"\
                "                        v2_zProperty.x, v2_zProperty.y, 0.0, 0.0,          \n"\
                "                        0.0, 0.0, 1.0, 0.0,                                \n"\
                "                        0.0, 0.0, 0.0, 1.0);                               \n"\
                "    v_texCoord = m4_translate * m4_zRotation * m4_yRotation * m4_xRotation * m4_scale * a_texCoord; \n"\
                "}                                                                          \n"\

#define DEFAULT_RGBA_FRAGMENT_SHADER \
                "precision highp float;                                         \n"\
                "uniform float u_alpha;                                         \n"\
                "uniform sampler2D u_texture;                                   \n"\
                "varying highp vec4 v_texCoord;                                 \n"\
                "void main(){                                                   \n"\
                "    highp vec4 rgbaColor = texture2D(u_texture, v_texCoord.st);\n"\
                "    gl_FragColor = vec4(rgbaColor.rgb, rgbaColor.a * u_alpha); \n"\
                "}                                                              \n"\

#define DEFAULT_YUV_FRAGMENT_SHADER \
                "precision highp float;                                         \n"\
                "uniform float u_alpha;                                         \n"\
                "uniform sampler2D u_yTexture;                                  \n"\
                "uniform sampler2D u_uTexture;                                  \n"\
                "uniform sampler2D u_vTexture;                                  \n"\
                "varying highp vec4 v_texCoord;                                 \n"\
                "void main(){                                                   \n"\
                "    highp vec4 yuv = vec4(texture2D(u_yTexture, v_texCoord.st).r - 0.0625,   \n"\
                "                          texture2D(u_uTexture, v_texCoord.st).r - 0.5000,   \n"\
                "                          texture2D(u_vTexture, v_texCoord.st).r - 0.5000,   \n"\
                "                          1.0);                                           \n"\
                "    highp vec4 rgbaColor = mat4(1.164,    1.164,    1.164,    0.0,        \n"\
                "                                0.0,     -0.392,    2.018,    0.0,        \n"\
                "                                1.59,    -0.813,    0.0,      0.0,        \n"\
                "                                0.0,      0.0,      0.0,      1.0) * yuv; \n"\
                "    gl_FragColor = vec4(rgbaColor.rgb, rgbaColor.a * u_alpha); \n"\
                "}                                                              \n"\

namespace paomiantv {

    class CShader {
    public:
        CShader(GLenum uType);

        virtual ~CShader();

        BOOL32 loadShader(const char *pchShader);

        inline GLenum getType() const;

        inline GLuint getId() const;

        static bool checkGlError(const char *funcName);

    private:
        GLenum m_uType;
        GLuint m_uId;
    };

    inline GLenum CShader::getType() const {
        return m_uType;
    }

    inline GLuint CShader::getId() const {
        return m_uId;
    }
} // namespace paomiantv

#endif // _PAOMIANTV_SHADER_H_