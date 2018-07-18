/*******************************************************************************
 *        Module: common
 *          File:
 * Functionality: frame defination
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 PAOMIANTV, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-08-09  v1.0        huangxuefeng  created
 ******************************************************************************/


#include "frame.h"


namespace paomiantv {

    std::vector<CAACFrame *> CAACFrame::m_svPool;
    CLock CAACFrame::m_sLock;

    std::vector<CH264Frame *> CH264Frame::m_svPool;
    CLock CH264Frame::m_sLock;


    std::vector<CI420Frame *> CI420Frame::m_svPool;
    CLock CI420Frame::m_sLock;


    std::vector<CYUVFrame *> CYUVFrame::m_svPool;
    CLock CYUVFrame::m_sLock;


    std::vector<CPCMFrame *> CPCMFrame::m_svPool;
    CLock CPCMFrame::m_sLock;
}

