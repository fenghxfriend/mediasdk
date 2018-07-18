/*******************************************************************************
 *        Module: mediasdk
 *          File: lutfilter.cpp
 * Functionality: LUT filter entity.
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2018-01-18  v1.0        huangxuefeng  created
 ******************************************************************************/

#ifndef MEDIAENGINE_CLUTFILTER_H
#define MEDIAENGINE_CLUTFILTER_H

#include <typedef.h>
#include <vfilter.h>

#define LUT_RGBA_FRAGMENT_SHADER \
                "precision highp float;                                         \n"\
                "uniform float u_alpha      ;                                   \n"\
                "uniform sampler2D u_texture;                                   \n"\
                "uniform sampler2D u_toneMap;                                   \n"\
                "varying highp vec4 v_texCoord;                                 \n"\
                "vec4 mapColor(in vec4 pixelColor){                                \n"\
                "    vec4 reColor = clamp(pixelColor, 0.0, 1.0);                \n"\
                "    highp float blueColor = reColor.b * 63.0;               \n"\
                "    highp vec2 coord1;                                         \n"\
                "    coord1.y = floor(floor(blueColor) / 8.0);                  \n"\
                "    coord1.x = floor(blueColor) - (coord1.y * 8.0);            \n"\
                "    highp vec2 coord2;                                         \n"\
                "    coord2.y = floor(ceil(blueColor) / 8.0);                   \n"\
                "    coord2.x = ceil(blueColor) - (coord2.y * 8.0);             \n"\
                "    highp float tileSize = 0.125 - 2.0/512.0;                  \n"\
                "    highp vec2 texPos1;                                        \n"\
                "    texPos1.x = clamp((coord1.x * 0.125) + 0.5/512.0 + (tileSize * reColor.r), 0.0, 1.0);\n"\
                "    texPos1.y = clamp((coord1.y * 0.125) + 0.5/512.0 + (tileSize * reColor.g), 0.0, 1.0);\n"\
                "    highp vec2 texPos2;                                        \n"\
                "    texPos2.x = clamp((coord2.x * 0.125) + 0.5/512.0 + (tileSize * reColor.r), 0.0, 1.0);\n"\
                "    texPos2.y = clamp((coord2.y * 0.125) + 0.5/512.0 + (tileSize * reColor.g), 0.0, 1.0);\n"\
                "    highp vec4 newColor1 = texture2D(u_toneMap, texPos1);      \n"\
                "    highp vec4 newColor2 = texture2D(u_toneMap, texPos2);      \n"\
                "    reColor = mix(newColor1, newColor2, fract(blueColor));     \n"\
                "    return vec4(reColor.rgb, reColor.a * u_alpha);             \n"\
                "}                                                              \n"\
                "void main(){                                                   \n"\
                "    gl_FragColor = mapColor(texture2D(u_texture, v_texCoord.st)); \n"\
                "}                                                              \n"\

#define LUT_YUV_FRAGMENT_SHADER \
                "precision highp float;                                         \n"\
                "uniform float u_alpha;                                         \n"\
                "uniform sampler2D u_yTexture;                                  \n"\
                "uniform sampler2D u_uTexture;                                  \n"\
                "uniform sampler2D u_vTexture;                                  \n"\
                "uniform sampler2D u_toneMap;                                   \n"\
                "varying highp vec4 v_texCoord;                                 \n"\
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
                "vec4 mapColor(in vec4 pixelColor){                                \n"\
                "    vec4 reColor = clamp(pixelColor, 0.0, 1.0);                \n"\
                "    highp float blueColor = reColor.b * 63.0;               \n"\
                "    highp vec2 coord1;                                         \n"\
                "    coord1.y = floor(floor(blueColor) / 8.0);                  \n"\
                "    coord1.x = floor(blueColor) - (coord1.y * 8.0);            \n"\
                "    highp vec2 coord2;                                         \n"\
                "    coord2.y = floor(ceil(blueColor) / 8.0);                   \n"\
                "    coord2.x = ceil(blueColor) - (coord2.y * 8.0);             \n"\
                "    highp float tileSize = 0.125 - 2.0/512.0;                  \n"\
                "    highp vec2 texPos1;                                        \n"\
                "    texPos1.x = clamp((coord1.x * 0.125) + 0.5/512.0 + (tileSize * reColor.r), 0.0, 1.0);\n"\
                "    texPos1.y = clamp((coord1.y * 0.125) + 0.5/512.0 + (tileSize * reColor.g), 0.0, 1.0);\n"\
                "    highp vec2 texPos2;                                        \n"\
                "    texPos2.x = clamp((coord2.x * 0.125) + 0.5/512.0 + (tileSize * reColor.r), 0.0, 1.0);\n"\
                "    texPos2.y = clamp((coord2.y * 0.125) + 0.5/512.0 + (tileSize * reColor.g), 0.0, 1.0);\n"\
                "    highp vec4 newColor1 = texture2D(u_toneMap, texPos1);      \n"\
                "    highp vec4 newColor2 = texture2D(u_toneMap, texPos2);      \n"\
                "    reColor = mix(newColor1, newColor2, fract(blueColor));     \n"\
                "    return vec4(reColor.rgb, reColor.a * u_alpha);             \n"\
                "}                                                              \n"\
                "void main(){                                                   \n"\
                "    gl_FragColor = mapColor(textColor(v_texCoord.st));         \n"\
                "}                                                              \n"\


namespace paomiantv {
    class CLUTFilter : public CVFilter {
    public:
        CLUTFilter();

        virtual ~CLUTFilter();

        virtual BOOL32 init(EMPixelFormat eSourceFMT);

        virtual void changeOutputSize(u32 uWidth, u32 uHeight);

        virtual void draw(CTexture *pTexture, const float *pPositionBuffer, s32 uPPointSize,
                          const float *pUVBuffer, s32 uUVPointSize);

        BOOL32 fillMapTexture(u8 *pbyMap, u32 uWidth, u32 uHeight,
                              EMPixelFormat format = EM_PIXEL_FORMAT_RGBA_8888);

    private:

        // 像素映射表滤镜纹理，像素格式必须为(RGB, RGBA, BGR以及BGRA)的其中一种
        // 若为nullptr则不应用滤镜
        CTexture* m_pMapTexture;
    };
}


#endif //MEDIAENGINE_CLUTFILTER_H
