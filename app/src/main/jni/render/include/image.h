//
// Created by ASUS on 2018/1/30.
//

#ifndef MEDIAENGINE_RENDERPARAMS_H
#define MEDIAENGINE_RENDERPARAMS_H

#include <algorithm>
#include "texture.h"
#include "typedef.h"
#include "autolock.h"
#include "enum.h"
#include "vlayer.h"

namespace paomiantv {

    class CPicture {
    public:
        u8 *m_pbyPicture;
        //buffer capacity
        u32 m_uCapacity;
        u32 m_uOffset;
        u32 m_uSize;
        u32 m_uWidth;
        u32 m_uHeight;
        EMPixelFormat m_eFormat;
    protected:
        ILock *m_pLock;
    protected:
        CPicture() : m_pbyPicture(NULL),
                     m_uCapacity(0),
                     m_uOffset(0),
                     m_uSize(0),
                     m_uWidth(0),
                     m_uHeight(0),
                     m_eFormat(EM_PIXEL_FORMAT_RGBA_8888) {
            m_pLock = new CLock;
        }


        virtual ~CPicture() {
            m_pLock->lock();
            if (m_pbyPicture != NULL) {
                free(m_pbyPicture);
                m_pbyPicture = NULL;
            }
			m_uCapacity = 0;
			m_uOffset = 0;
            m_uSize = 0;
            m_uWidth = 0;
            m_uHeight = 0;
            m_eFormat = EM_PIXEL_FORMAT_RGBA_8888;
            m_pLock->unlock();
            delete m_pLock;
            m_pLock = NULL;
        }

    public:
        static CPicture *create() {
            CPicture *re = NULL;
            m_sLock.lock();

            if (!m_svPool.empty()) {
                re = m_svPool.back();
                m_svPool.pop_back();
            } else {
                re = new CPicture();
            }
            m_sLock.unlock();
            return re;
        }

        static void release(CPicture *pPicture) {
            if (pPicture != NULL) {
                pPicture->resize(pPicture->m_uCapacity);
                m_sLock.lock();
                std::vector<CPicture *>::iterator it;
                it = std::find(m_svPool.begin(), m_svPool.end(), pPicture);
                if (it == m_svPool.end()) {
                    //vec中不存在value值
                    m_svPool.push_back(pPicture);
                }
                m_sLock.unlock();
            }
        }

        static void clear() {
            m_sLock.lock();
            while (!m_svPool.empty()) {
                CPicture *picture = m_svPool.back();
                m_svPool.pop_back();
                if (picture != NULL) {
                    m_sLock.unlock();
                    delete picture;
                    m_sLock.lock();
                }
            }
            m_svPool.clear();
            m_sLock.unlock();
        }

    public:
        virtual void resize(const u32 size) {
            CAutoLock autoLock(m_pLock);
            if (size > 0) {
                if (m_pbyPicture == NULL) {
                    m_pbyPicture = (u8 *) malloc(size);
                } else if (m_uCapacity < size) {
                    free(m_pbyPicture);
                    m_pbyPicture = (u8 *) malloc(size);
                }
                m_uCapacity = MAX(size, m_uCapacity);
                memset(m_pbyPicture, 0, m_uCapacity);
            } else {
                if (m_pbyPicture != NULL) {
                    free(m_pbyPicture);
                    m_pbyPicture = NULL;
                }
                m_uCapacity = 0;
            }
            m_uOffset = 0;
            m_uSize = 0;
            m_uWidth = 0;
            m_uHeight = 0;
            m_eFormat = EM_PIXEL_FORMAT_RGBA_8888;
        }

        virtual CPicture *clone() {
            CPicture *p = create();
            CAutoLock autoLock(m_pLock);
            p->resize(m_uCapacity);
            if (m_pbyPicture != NULL) {
                memcpy(p->m_pbyPicture, m_pbyPicture, m_uCapacity);
            }
            p->m_uSize = m_uSize;
            p->m_uWidth = m_uWidth;
            p->m_uHeight = m_uHeight;
            p->m_uOffset = m_uOffset;
            p->m_eFormat = m_eFormat;
            return p;
        }

    private:
        static std::vector<CPicture *> m_svPool;
        static CLock m_sLock;
    };

    class CVFilterParam {
    public:
        CPicture *m_pFilterSource;
        EMVFilter m_eType;
    protected:
        CVFilterParam() : m_pFilterSource(NULL),
                          m_eType(EM_V_FILTER_START) {
            m_pLock = new CLock;
        }

    public:
        virtual ~CVFilterParam() {
            m_pLock->lock();
            if (m_pFilterSource != NULL) {
                CPicture::release(m_pFilterSource);
                m_pFilterSource = NULL;
            }
            m_eType = EM_V_FILTER_START;
            m_pLock->unlock();
            delete m_pLock;
        }

    public:
        virtual void reset() {
            CAutoLock autoLock(m_pLock);
            if (m_pFilterSource != NULL) {
                CPicture::release(m_pFilterSource);
                m_pFilterSource = NULL;
            }
            m_eType = EM_V_FILTER_START;
        }

        virtual CVFilterParam *clone() {
            CVFilterParam *filterParam = CVFilterParam::create();
            CAutoLock autoLock(m_pLock);
            if (m_pFilterSource != NULL) {
                filterParam->m_pFilterSource = m_pFilterSource->clone();
            }
            filterParam->m_eType = m_eType;
            return filterParam;
        }

        static CVFilterParam *create() {
            CVFilterParam *re = NULL;
            m_sLock.lock();

            if (!m_svPool.empty()) {
                re = m_svPool.back();
                m_svPool.pop_back();
            } else {
                re = new CVFilterParam();
            }
            m_sLock.unlock();
            return re;
        }

        static void release(CVFilterParam *pFilter) {

            if (pFilter != NULL) {
                pFilter->reset();
                m_sLock.lock();
                std::vector<CVFilterParam *>::iterator it;
                it = std::find(m_svPool.begin(), m_svPool.end(), pFilter);
                if (it == m_svPool.end()) {
                    //vec中不存在value值
                    m_svPool.push_back(pFilter);
                }
                m_sLock.unlock();
            }

        }

        static void clear() {
            m_sLock.lock();
            while (!m_svPool.empty()) {
                CVFilterParam *filterParam = m_svPool.back();
                m_svPool.pop_back();
                if (filterParam != NULL) {
                    m_sLock.unlock();
                    delete filterParam;
                    m_sLock.lock();
                }
            }
            m_svPool.clear();
            m_sLock.unlock();
        }

    private:
        static std::vector<CVFilterParam *> m_svPool;
        static CLock m_sLock;
    protected:
        ILock *m_pLock;
    };

    class CVLayerParam : public CPicture {
    public:
        s64 m_sllTimeStampUS;
        s64 m_sllDurationUS;
        u32 m_uTrackId;
        std::vector<CVFilterParam *> m_vFilterParams;
        float m_afRotate[EM_DIRECT_END];
        float m_afScale[EM_DIRECT_END];
        float m_afTranslate[EM_DIRECT_END];
        float m_afUVCropRotate[EM_DIRECT_END];
        float m_afUVCropScale[EM_DIRECT_END];
        float m_afUVCropTranslate[EM_DIRECT_END];
        float m_fAlpha;

    protected:
        CVLayerParam() : m_fAlpha(1.0f),
                         m_sllTimeStampUS(0),
                         m_sllDurationUS(0),
                         m_afRotate{},
                         m_afTranslate{},
                         m_afUVCropRotate{},
                         m_afUVCropTranslate{} {
            m_afScale[EM_DIRECT_X] = 1.0f;
            m_afScale[EM_DIRECT_Y] = 1.0f;
            m_afScale[EM_DIRECT_Z] = 1.0f;
            m_afUVCropScale[EM_DIRECT_X] = 1.0f;
            m_afUVCropScale[EM_DIRECT_Y] = 1.0f;
            m_afUVCropScale[EM_DIRECT_Z] = 1.0f;
            m_uTrackId = 0;
            m_vFilterParams.clear();
        }

    public:
        virtual ~CVLayerParam() {
            m_pLock->lock();
            for (auto it = m_vFilterParams.begin(); it != m_vFilterParams.end(); it++) {
                CVFilterParam::release(*it);
            }
            m_vFilterParams.clear();
            m_pLock->unlock();
        }

    public:
        virtual void resize(const u32 size) {
            CPicture::resize(size);
            CAutoLock autoLock(m_pLock);
            for (auto it = m_vFilterParams.begin(); it != m_vFilterParams.end(); it++) {
                CVFilterParam::release(*it);
            }
            m_vFilterParams.clear();
            m_sllTimeStampUS = 0;
            m_sllDurationUS = 0;
            m_afRotate[EM_DIRECT_X] = 0.0f;
            m_afRotate[EM_DIRECT_Y] = 0.0f;
            m_afRotate[EM_DIRECT_Z] = 0.0f;
            m_afScale[EM_DIRECT_X] = 1.0f;
            m_afScale[EM_DIRECT_Y] = 1.0f;
            m_afScale[EM_DIRECT_Z] = 1.0f;
            m_afTranslate[EM_DIRECT_X] = 0.0f;
            m_afTranslate[EM_DIRECT_Y] = 0.0f;
            m_afTranslate[EM_DIRECT_Z] = 0.0f;
            m_afUVCropRotate[EM_DIRECT_X] = 0.0f;
            m_afUVCropRotate[EM_DIRECT_Y] = 0.0f;
            m_afUVCropRotate[EM_DIRECT_Z] = 0.0f;
            m_afUVCropScale[EM_DIRECT_X] = 1.0f;
            m_afUVCropScale[EM_DIRECT_Y] = 1.0f;
            m_afUVCropScale[EM_DIRECT_Z] = 1.0f;
            m_afUVCropTranslate[EM_DIRECT_X] = 0.0f;
            m_afUVCropTranslate[EM_DIRECT_Y] = 0.0f;
            m_afUVCropTranslate[EM_DIRECT_Z] = 0.0f;
            m_uTrackId = 0;
            m_fAlpha = 1.0f;
        }

        virtual CVLayerParam *clone() {
            CVLayerParam *layer = CVLayerParam::create();
            CAutoLock autoLock(m_pLock);
            layer->resize(m_uCapacity);
            layer->m_uSize = m_uSize;
            if (m_pbyPicture != NULL) {
                memcpy(layer->m_pbyPicture, m_pbyPicture, m_uSize);
            }

            layer->m_uWidth = m_uWidth;
            layer->m_uHeight = m_uHeight;
            layer->m_uOffset = m_uOffset;
            layer->m_eFormat = m_eFormat;

            for (auto it = m_vFilterParams.begin(); it != m_vFilterParams.end(); it++) {
                CVFilterParam *filter = *it;
                if (filter != NULL) {
                    layer->m_vFilterParams.push_back(filter->clone());
                }
            }

            layer->m_sllTimeStampUS = m_sllTimeStampUS;
            layer->m_sllDurationUS = m_sllDurationUS;
            layer->m_afRotate[EM_DIRECT_X] = m_afRotate[EM_DIRECT_X];
            layer->m_afRotate[EM_DIRECT_Y] = m_afRotate[EM_DIRECT_Y];
            layer->m_afRotate[EM_DIRECT_Z] = m_afRotate[EM_DIRECT_Z];
            layer->m_afScale[EM_DIRECT_X] = m_afScale[EM_DIRECT_X];
            layer->m_afScale[EM_DIRECT_Y] = m_afScale[EM_DIRECT_Y];
            layer->m_afScale[EM_DIRECT_Z] = m_afScale[EM_DIRECT_Z];
            layer->m_afTranslate[EM_DIRECT_X] = m_afTranslate[EM_DIRECT_X];
            layer->m_afTranslate[EM_DIRECT_Y] = m_afTranslate[EM_DIRECT_Y];
            layer->m_afTranslate[EM_DIRECT_Z] = m_afTranslate[EM_DIRECT_Z];
            layer->m_afUVCropRotate[EM_DIRECT_X] = m_afUVCropRotate[EM_DIRECT_X];
            layer->m_afUVCropRotate[EM_DIRECT_Y] = m_afUVCropRotate[EM_DIRECT_Y];
            layer->m_afUVCropRotate[EM_DIRECT_Z] = m_afUVCropRotate[EM_DIRECT_Z];
            layer->m_afUVCropScale[EM_DIRECT_X] = m_afUVCropScale[EM_DIRECT_X];
            layer->m_afUVCropScale[EM_DIRECT_Y] = m_afUVCropScale[EM_DIRECT_Y];
            layer->m_afUVCropScale[EM_DIRECT_Z] = m_afUVCropScale[EM_DIRECT_Z];
            layer->m_afUVCropTranslate[EM_DIRECT_X] = m_afUVCropTranslate[EM_DIRECT_X];
            layer->m_afUVCropTranslate[EM_DIRECT_Y] = m_afUVCropTranslate[EM_DIRECT_Y];
            layer->m_afUVCropTranslate[EM_DIRECT_Z] = m_afUVCropTranslate[EM_DIRECT_Z];
            layer->m_uTrackId = m_uTrackId;
            layer->m_fAlpha = m_fAlpha;
            return layer;
        }

        static CVLayerParam *create() {
            CVLayerParam *re = NULL;
            m_sLock.lock();

            if (!m_svPool.empty()) {
                re = m_svPool.back();
                m_svPool.pop_back();
            } else {
                re = new CVLayerParam();
            }
            m_sLock.unlock();
            return re;
        }

        static void release(CVLayerParam *pLayer) {
            if (pLayer != NULL) {
                pLayer->resize(pLayer->m_uCapacity);
                m_sLock.lock();
                std::vector<CVLayerParam *>::iterator it;
                it = std::find(m_svPool.begin(), m_svPool.end(), pLayer);
                if (it == m_svPool.end()) {
                    //vec中不存在value值
                    m_svPool.push_back(pLayer);
                }
                m_sLock.unlock();
            }
        }

        static void clear() {
            m_sLock.lock();
            while (!m_svPool.empty()) {
                CVLayerParam *layerParam = m_svPool.back();
                m_svPool.pop_back();
                if (layerParam != NULL) {
                    m_sLock.unlock();
                    delete layerParam;
                    m_sLock.lock();
                }
            }
            m_svPool.clear();
            m_sLock.unlock();
        }

    private:
        static std::vector<CVLayerParam *> m_svPool;
        static CLock m_sLock;
    };

    class CImage {
    public:
        s64 m_sllTimeStampUS;
        BOOL32 isEOS;
        std::vector<CVLayerParam *> m_vVLayerParam;
    protected:
        CImage() : isEOS(FALSE),
                   m_sllTimeStampUS(0) {
            m_vVLayerParam.clear();
            m_pLock = new CLock;
        }

        virtual ~CImage() {
            m_pLock->lock();
            for (auto it = m_vVLayerParam.begin(); it != m_vVLayerParam.end(); it++) {
                CVLayerParam::release(*it);
            }
            m_vVLayerParam.clear();
            m_pLock->unlock();
            delete m_pLock;
        }

    public:
        virtual void reset() {
            CAutoLock autoLock(m_pLock);
            isEOS = FALSE;
            m_sllTimeStampUS = 0;
            for (auto it = m_vVLayerParam.begin(); it != m_vVLayerParam.end(); it++) {
                CVLayerParam::release(*it);
            }
            m_vVLayerParam.clear();
        }

        static CImage *create() {
            CImage *re = NULL;
            m_sLock.lock();

            if (!m_svPool.empty()) {
                re = m_svPool.back();
                m_svPool.pop_back();
            } else {
                re = new CImage();
            }
            m_sLock.unlock();
            return re;
        }

        static void release(CImage *pImage) {
            if (pImage != NULL) {
                pImage->reset();
                m_sLock.lock();
                std::vector<CImage *>::iterator it;
                it = std::find(m_svPool.begin(), m_svPool.end(), pImage);
                if (it == m_svPool.end()) {
                    //vec中不存在value值
                    m_svPool.push_back(pImage);
                }
                m_sLock.unlock();
            }
        }

        static void clear() {
            m_sLock.lock();
            while (!m_svPool.empty()) {
                CImage *image = m_svPool.back();
                m_svPool.pop_back();
                if (image != NULL) {
                    m_sLock.unlock();
                    delete image;
                    m_sLock.lock();
                }
            }
            m_svPool.clear();
            m_sLock.unlock();
        }

    private:
        static std::vector<CImage *> m_svPool;
        static CLock m_sLock;
        ILock *m_pLock;
    };

    class CImageManager {
    private:

        CImageManager() {
        }

        virtual ~CImageManager() {
        }

    public:
        static void clearImage() {
            CImage::clear();
            CVLayerParam::clear();
            CVFilterParam::clear();
            CPicture::clear();
        }

    };
}
#endif //MEDIAENGINE_RENDERPARAMS_H
