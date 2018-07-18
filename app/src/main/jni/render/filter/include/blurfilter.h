//
// Created by ASUS on 2018/1/18.
//

#ifndef MEDIAENGINE_CBLURFILTER_H
#define MEDIAENGINE_CBLURFILTER_H

#include <vfilter.h>

#define MAX_ITERATE_TIME 10
#define DEFAULT_ITERATE_TIME 2
#define DEFAULT_BLUR_OFFSET 8.0f

#define GAUSSIAN_BLUR_VERTEX_SHADER \
                "uniform vec3 u_posTranslate;                                       \n"\
                "uniform vec3 u_posScale;                                           \n"\
                "uniform vec3 u_posRotate;                                          \n"\
                "uniform vec3 u_uvTranslate;                                        \n"\
                "uniform vec3 u_uvScale;                                            \n"\
                "uniform vec3 u_uvRotate;                                           \n"\
                "attribute vec4 a_position;                                         \n"\
                "attribute vec4 a_texCoord;                                         \n"\
                "varying highp vec4 v_texCoord;                                     \n"\
                "uniform vec2 u_texelOffset;                                        \n"\
                "varying mediump vec2 blur[8];                                      \n"\
                "void main(){                                                       \n"\
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
                "    blur[0] = v_texCoord.xy - u_texelOffset * 4.0;                     \n"\
                "    blur[1] = v_texCoord.xy - u_texelOffset * 3.0;                     \n"\
                "    blur[2] = v_texCoord.xy - u_texelOffset * 2.0;                     \n"\
                "    blur[3] = v_texCoord.xy - u_texelOffset;                           \n"\
                "    blur[4] = v_texCoord.xy + u_texelOffset;                           \n"\
                "    blur[5] = v_texCoord.xy + u_texelOffset * 2.0;                     \n"\
                "    blur[6] = v_texCoord.xy + u_texelOffset * 3.0;                     \n"\
                "    blur[7] = v_texCoord.xy + u_texelOffset * 4.0;                     \n"\
                "}                                                                      \n"\


#define GAUSSIAN_BLUR_RGBA_FRAGMENT_SHADER \
                "precision highp float;                                         \n"\
                "uniform float u_alpha      ;                                   \n"\
                "uniform sampler2D u_texture;                                   \n"\
                "varying highp vec4 v_texCoord;                                 \n"\
                "varying highp vec2 blur[8];                                    \n"\
                "vec4 blurColor(){                                              \n"\
                "    highp vec4 avg = vec4(0.0, 0.0, 0.0, 0.0);                 \n"\
                "    avg += texture2D(u_texture, blur[0]) * 0.05;               \n"\
                "    avg += texture2D(u_texture, blur[1]) * 0.09;               \n"\
                "    avg += texture2D(u_texture, blur[2]) * 0.12;               \n"\
                "    avg += texture2D(u_texture, blur[3]) * 0.15;               \n"\
                "    avg += texture2D(u_texture, v_texCoord.st) * 0.18;         \n"\
                "    avg += texture2D(u_texture, blur[4]) * 0.15;               \n"\
                "    avg += texture2D(u_texture, blur[5]) * 0.12;               \n"\
                "    avg += texture2D(u_texture, blur[6]) * 0.09;               \n"\
                "    avg += texture2D(u_texture, blur[7]) * 0.05;               \n"\
                "    return vec4(clamp(avg.r, 0.0, 1.0),                        \n"\
                "                clamp(avg.g, 0.0, 1.0),                        \n"\
                "                clamp(avg.b, 0.0, 1.0),                        \n"\
                "                clamp(avg.a * u_alpha, 0.0, 1.0));             \n"\
                "}                                                              \n"\
                "void main(){                                                   \n"\
                "    gl_FragColor = blurColor();                                \n"\
                "}                                                              \n"\

#define GAUSSIAN_BLUR_YUV_FRAGMENT_SHADER \
                "precision highp float;                                         \n"\
                "uniform float u_alpha      ;                                   \n"\
                "uniform sampler2D u_yTexture;                                  \n"\
                "uniform sampler2D u_uTexture;                                  \n"\
                "uniform sampler2D u_vTexture;                                  \n"\
                "varying highp vec4 v_texCoord;                                 \n"\
                "varying highp vec2 blur[8];                                    \n"\
                "vec4 textColor(in vec2 texCoord){                              \n"\
                "    highp vec4 yuv = vec4(texture2D(u_yTexture, texCoord).r - 0.0625,\n"\
                "                          texture2D(u_uTexture, texCoord).r - 0.5000,\n"\
                "                          texture2D(u_vTexture, texCoord).r - 0.5000,\n"\
                "                          1.0);                                      \n"\
                "    return mat4(1.164,    1.164,    1.164,    0.0,                 \n"\
                "                0.0,     -0.392,    2.018,    0.0,                 \n"\
                "                1.59,    -0.813,    0.0,      0.0,                 \n"\
                "                0.0,      0.0,      0.0,      1.0) * yuv;          \n"\
                "}                                                              \n"\
                "vec4 blurColor(){                                              \n"\
                "    highp vec4 avg = vec4(0.0, 0.0, 0.0, 0.0);                 \n"\
                "    avg += textColor(blur[0]) * 0.05;                          \n"\
                "    avg += textColor(blur[1]) * 0.09;                          \n"\
                "    avg += textColor(blur[2]) * 0.12;                          \n"\
                "    avg += textColor(blur[3]) * 0.15;                          \n"\
                "    avg += textColor(v_texCoord.st) * 0.18;                    \n"\
                "    avg += textColor(blur[4]) * 0.15;                          \n"\
                "    avg += textColor(blur[5]) * 0.12;                          \n"\
                "    avg += textColor(blur[6]) * 0.09;                          \n"\
                "    avg += textColor(blur[7]) * 0.05;                          \n"\
                "    return vec4(clamp(avg.r, 0.0, 1.0),                        \n"\
                "                clamp(avg.g, 0.0, 1.0),                        \n"\
                "                clamp(avg.b, 0.0, 1.0),                        \n"\
                "                clamp(avg.a * u_alpha, 0.0, 1.0));             \n"\
                "}                                                              \n"\
                "void main(){                                                   \n"\
                "    gl_FragColor = blurColor();                                \n"\
                "}                                                              \n"\

namespace paomiantv {
    class CBlurFilter : public CVFilter {
    public:
        CBlurFilter();

        virtual ~CBlurFilter();

        virtual BOOL32 init(EMPixelFormat eSourceFMT);

        virtual void changeOutputSize(u32 nWidth, u32 nHeight);


        virtual void draw(CTexture *pTexture, const float *pPositionBuffer, s32 uPPointSize,
                          const float *pUVBuffer, s32 uUVPointSize);

        void setBlurOffset(const float fBlurOffset);

        BOOL32 setItNumber(u32 m_uItNumber);

        inline const u32 getItNumber() const;

        inline const float getBlurOffset() const;

    private:

        void destroyFrameBuffers();

        void addFrameBuffers(GLuint &uFrameBuffer, CTexture *&pcFrameBufferTexture);

        void deleteFrameBuffers(GLuint &uFrameBuffer, CTexture *&pcFrameBufferTexture);

        void drawBlur(CTexture *pTexture, const float *pPositionBuffer, s32 uPPointSize,
                      const float *pUVBuffer,
                      s32 uUVPointSize, EMDirection direction);

    private:
        /*blur*/
        //! the layer blur step in pixel on horizontal/vertical/depth direction.
        GLfloat m_fBlurOffset;
        //! the layer blur number of Iterations.
        u32 m_uItNumber;

        GLuint m_auFrameBuffers[2 * MAX_ITERATE_TIME - 1];
        CTexture *m_pcFrameBufferTextures[2 * MAX_ITERATE_TIME - 1];
    };

    inline const u32 CBlurFilter::getItNumber() const {
        return m_uItNumber;
    }

    inline const float CBlurFilter::getBlurOffset() const {
        return m_fBlurOffset;
    }
}


#endif //MEDIAENGINE_CBLURFILTER_H
