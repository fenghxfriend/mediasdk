//
// Created by ASUS on 2017/12/28.
//

#ifndef MEDIAENGINE_TEXTURE_H
#define MEDIAENGINE_TEXTURE_H


#include <vector>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <enum.h>
#include <typedef.h>
#include <autolock.h>

namespace paomiantv {

    class CTexture {

    public:
        CTexture(EMPixelFormat emPixelFormat = EM_PIXEL_FORMAT_RGBA_8888);

        virtual ~CTexture();

    public:

        BOOL32 fillTexture(u8 *pData, u32 uWidth, u32 uHeight);

        GLuint getTextureId(EMTexture emTexture = EM_TEXTURE_Y_OR_RGBA);

        inline EMPixelFormat getFormat() const;

    private:
        /// 纹理ID数组，针对不同像素格式有不同个数的纹理
        /// 像素格式            纹理个数
        /// RGB                 1
        /// RGBA                1
        /// YUV444_INTERLEAVED  1
        /// NV12                2
        /// NV21                2
        /// YV12                3
        GLuint *m_puIds;
        EMPixelFormat m_eFormat;
        ILock* m_pcLock;

    private:
        void deleteTexture(u32 &uId);

        void addTexture(u32 &uId);

        void initTexture();

        void clearTexture();
    };

    inline EMPixelFormat CTexture::getFormat() const {
        return m_eFormat;
    }
}

#endif //MEDIAENGINE_TEXTURE_H
