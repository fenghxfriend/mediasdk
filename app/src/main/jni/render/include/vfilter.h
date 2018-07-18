//
// Created by ASUS on 2018/1/18.
//

#ifndef MEDIAENGINE_CFILTER_H
#define MEDIAENGINE_CFILTER_H

#include <string>
#include <GLES2/gl2.h>
#include <unordered_map>
#include "texture.h"
#include "program.h"
// every point has {x,y,z}, three element
#define DEFAULT_GRAPH_POINT_SIZE 4
#define DEFAULT_SIZE_PER_POINT 3
#define DEFAULT_GRAPH_SIZE (DEFAULT_GRAPH_POINT_SIZE * DEFAULT_SIZE_PER_POINT)
namespace paomiantv {

    class CVFilter {

    public:
        CVFilter();

        virtual ~CVFilter();

        virtual BOOL32 init(EMPixelFormat eSourceFMT);

        virtual void changeOutputSize(u32 uWidth, u32 uHeight);

        virtual void draw(CTexture *pTexture, const float *pPositionBuffer, s32 uPPointSize,
                          const float *pUVBuffer, s32 uUVPointSize);

        void setTranslate(GLfloat fTranslateX, GLfloat fTranslateY, GLfloat fTranslateZ);

        void setScale(GLfloat fScaleX, GLfloat fScaleY, GLfloat fScaleZ);

        // parameter is degree
        void setRotation(GLfloat fRotationX, GLfloat fRotationY, GLfloat fRotationZ);

        void setAlpha(GLfloat fAlpha);

        void setTranslate(GLfloat fTranslate, EMDirection direction);

        void setScale(GLfloat fScale, EMDirection direction);

        // parameter is degree
        void setRotation(GLfloat fRotation, EMDirection direction);

        //crop the texture area
        void setUVCropTranslate(GLfloat fTranslateX, GLfloat fTranslateY, GLfloat fTranslateZ);

        void setUVCropScale(GLfloat fScaleX, GLfloat fScaleY, GLfloat fScaleZ);

        void setUVCropRotation(GLfloat fRotationX, GLfloat fRotationY, GLfloat fRotationZ);

        void setUVCropTranslate(GLfloat fTranslate, EMDirection direction);

        void setUVCropScale(GLfloat fScale, EMDirection direction);

        void setUVCropRotation(GLfloat fRotation, EMDirection direction);

        // flip texture uv
        static void flipUV(float *pUVData, u32 uUVSize, EMDirection dir);

        inline u32 getOutputWidth() const;

        inline u32 getOutputHeight() const;

        inline EMVFilter getFilterType() const;

        static const float CUBE[DEFAULT_GRAPH_SIZE];
        static const float NORMALUV[DEFAULT_GRAPH_SIZE];
        static const float VFLIPUV[DEFAULT_GRAPH_SIZE];
        static const float HFLIPUV[DEFAULT_GRAPH_SIZE];

    protected:

        u32 m_uOutputWidth;
        //the area height for our drawing on screen width pixel
        u32 m_uOutputHeight;
        GLfloat m_afRotationVec[DEFAULT_SIZE_PER_POINT] = {.0f, .0f, .0f};
        GLfloat m_afScaleVec[DEFAULT_SIZE_PER_POINT] = {1.0f, 1.0f, 1.0f};
        GLfloat m_afTranslateVec[DEFAULT_SIZE_PER_POINT] = {.0f, .0f, .0f};
        GLfloat m_afUVCropRotationVec[DEFAULT_SIZE_PER_POINT] = {.0f, .0f, .0f};
        GLfloat m_afUVCropScaleVec[DEFAULT_SIZE_PER_POINT] = {1.0f, 1.0f, 1.0f};
        GLfloat m_afUVCropTranslateVec[DEFAULT_SIZE_PER_POINT] = {.0f, .0f, .0f};
        GLfloat m_fAlpha;
        CProgram *m_pProgram;
        GLuint m_uPositionVBO;
        GLuint m_uCoordinateVBO;
        ILock *m_pLock;
        EMVFilter m_eFilterType;
    protected:
        void sendUniform1f(const char *uniformName, const float value);

        void sendUniformMatrix4fv(const char *uniformName, const float *value);

        BOOL32 configure(const char *pchFragmentShader, const char *pchVertexShader);

        float getUVCropTranslate(EMDirection direction);

        float getUVCropScale(EMDirection direction);

        float getUVCropRotation(EMDirection direction);
    };

    inline u32 CVFilter::getOutputWidth() const {
        return m_uOutputWidth;
    }

    inline u32 CVFilter::getOutputHeight() const {
        return m_uOutputHeight;
    }

    inline EMVFilter CVFilter::getFilterType() const {
        return m_eFilterType;
    }
}


#endif //MEDIAENGINE_CFILTER_H
