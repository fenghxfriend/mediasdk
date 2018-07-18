//
// Created by ASUS on 2018/2/5.
//

#include "image.h"

namespace paomiantv {

    std::vector<CPicture *> CPicture::m_svPool;
    CLock CPicture::m_sLock;

    std::vector<CVFilterParam *> CVFilterParam::m_svPool;
    CLock CVFilterParam::m_sLock;

    std::vector<CVLayerParam *> CVLayerParam::m_svPool;
    CLock CVLayerParam::m_sLock;

    std::vector<CImage *> CImage::m_svPool;
    CLock CImage::m_sLock;
}