/*******************************************************************************
 *        Module: mediasdk
 *          File:
 * Functionality: define filter entity.
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-27  v1.0        huangxuefeng  created
 ******************************************************************************/

#ifndef _PAOMIANTV_FILTER_H_
#define _PAOMIANTV_FILTER_H_

#include <vector>
#include <image.h>

namespace paomiantv {

    class CEffect {
    public:
        CEffect();

        virtual ~CEffect();

    private:
        void parse();

    public:
        void
        init(EMEffect nType, u8 *pbyPicture, EMPixelFormat emPixelFormat, u32 uWidth, u32 uHeight,
             s64 sllStart,
             s64 sllDurantion);

        void uninit();

        void update(EMEffect nType, u8 *pbyPicture, EMPixelFormat emPixelFormat, u32 uWidth,
                    u32 uHeight);

        void setStart(s64 sllStart);

        void setDuration(s64 sllDuration);

        EMEffect getType();

        s64 getStart();

        s64 getDuration();

        void getFilter(CPicture *&p, EMVFilter &eType);

        inline const u32 getId() const;

    private:
        static u32 m_sCount;
        // effect id
        const u32 m_uId;
        ILock *m_pLock;
        EMEffect m_eType;
        CPicture *m_pPicture;
        // microsecond, the start time of filter relative to start time of clip
        s64 m_sllStart;
        // microsecond, the duration of filter
        s64 m_sllDuration;

    };

    inline const u32 CEffect::getId() const {
        return m_uId;
    }


} // namespace paomiantv

#endif // _PAOMIANTV_FILTER_H_