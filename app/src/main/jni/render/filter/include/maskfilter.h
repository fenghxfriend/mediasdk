//
// Created by ASUS on 2018/6/21.
//

#ifndef MEDIAENGINE_MASKFILTER_H
#define MEDIAENGINE_MASKFILTER_H


#include <vfilter.h>

// u_maskFlipAnchor: x,y is the center of the flipping

#define MASK_RGBA_FRAGMENT_SHADER \
                "precision highp float;                                             \n"\
                "uniform vec2 u_maskFlipAnchor;                                     \n"\
                "uniform vec2 u_maskFlip;                                           \n"\
                "uniform float u_alpha;                                             \n"\
                "uniform sampler2D u_texture;                                       \n"\
                "uniform sampler2D u_maskMap;                                       \n"\
                "varying highp vec4 v_texCoord;                                     \n"\
                "void main(){                                                       \n"\
                "    highp vec4 sourceColor = texture2D(u_texture, v_texCoord.st);  \n"\
                "    highp vec2 texCoord = (mat2(-u_maskFlip.x,0.0,0.0,-u_maskFlip.y)*v_texCoord.st)+(2.0*u_maskFlipAnchor);\n"\
                "    highp vec4 maskColor = texture2D(u_maskMap, texCoord);         \n"\
                "    gl_FragColor = vec4(sourceColor.rgb, maskColor.a * u_alpha);   \n"\
                "}                                                                  \n"\

#define MASK_YUV_FRAGMENT_SHADER \
                "precision highp float;                                             \n"\
                "uniform vec2 u_maskFlipAnchor;                                     \n"\
                "uniform vec2 u_maskFlip;                                           \n"\
                "uniform float u_alpha;                                             \n"\
                "uniform sampler2D u_yTexture;                                      \n"\
                "uniform sampler2D u_uTexture;                                      \n"\
                "uniform sampler2D u_vTexture;                                      \n"\
                "uniform sampler2D u_maskMap;                                       \n"\
                "varying highp vec4 v_texCoord;                                     \n"\
                "vec4 textColor(in vec2 texCoord){                                  \n"\
                "    highp vec4 yuv = vec4(texture2D(u_yTexture, texCoord).r - 0.0625,\n"\
                "                          texture2D(u_uTexture, texCoord).r - 0.5000,\n"\
                "                          texture2D(u_vTexture, texCoord).r - 0.5000,\n"\
                "                          1.0);                                      \n"\
                "    return mat4(1.164,    1.164,    1.164,    0.0,                 \n"\
                "                0.0,     -0.392,    2.018,    0.0,                 \n"\
                "                1.59,    -0.813,    0.0,      0.0,                 \n"\
                "                0.0,      0.0,      0.0,      1.0) * yuv;          \n"\
                "}                                                                  \n"\
                "void main(){                                                       \n"\
                "    highp vec4 sourceColor = textColor(v_texCoord.st);             \n"\
                "    highp vec2 texCoord = (mat2(-u_maskFlip.x,0.0,0.0,-u_maskFlip.y)*v_texCoord.st)+(2.0*u_maskFlipAnchor);\n"\
                "    highp vec4 maskColor = texture2D(u_maskMap, texCoord);         \n"\
                "    gl_FragColor = vec4(sourceColor.rgb, maskColor.a * u_alpha);   \n"\
                "}                                                                  \n"\


namespace paomiantv {
    class CMaskFilter : public CVFilter {
    public:
        CMaskFilter();

        virtual ~CMaskFilter();

        virtual BOOL32 init(EMPixelFormat eSourceFMT);

        virtual void changeOutputSize(u32 uWidth, u32 uHeight);

        virtual void draw(CTexture *pTexture, const float *pPositionBuffer, s32 uPPointSize,
                          const float *pUVBuffer, s32 uUVPointSize);

        BOOL32 fillMaskTexture(u8 *pbyMask, u32 uWidth, u32 uHeight,
                               EMPixelFormat format = EM_PIXEL_FORMAT_RGBA_8888);

        void setFlipAnchor(const float fAnchorX, const float fAnchorY);

    private:

        // 像素映射表蒙版纹理，像素格式必须为(RGB, RGBA, BGR以及BGRA)的其中一种
        // 若为nullptr则不应用滤镜
        CTexture *m_pMaskTexture;
        GLfloat m_afAnchorVec[2] = {.0f, .0f};
        GLfloat m_afFilpVec[2] = {-1.0f, -1.0f};
    };
}

#endif //MEDIAENGINE_MASKFILTER_H
