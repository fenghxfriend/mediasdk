//
// Created by ASUS on 2018/4/16.
//

#include "sound.h"

namespace paomiantv {
    std::vector<CAudio *> CAudio::m_svPool;
    CLock CAudio::m_sLock;

//    std::vector<CAFilterParam *> CAFilterParam::m_svPool;
//    CLock CAFilterParam::m_sLock;
//
//    std::vector<CALayerParam *> CALayerParam::m_svPool;
//    CLock CALayerParam::m_sLock;

    std::vector<CSound *> CSound::m_svPool;
    CLock CSound::m_sLock;
}
