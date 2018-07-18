/*******************************************************************************
 *        Module: mediasdk
 *          File:
 * Functionality: storyboard entity.
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-08-01  v1.0        huangxuefeng  created
 ******************************************************************************/
#ifndef _PAOMIANTV_STORYBOARD_H_
#define _PAOMIANTV_STORYBOARD_H_

#include <jni.h>
#include <set>
#include <safelist.h>
#include "track.h"

namespace paomiantv {

    class CStoryboard {
    public:
        CStoryboard();

        virtual ~CStoryboard();

        BOOL32 addTrack(ITrack *pTrack, s16 wZIndex = 1, BOOL32 bIsIndexEnable = FALSE);

        BOOL32 removeTrack(const s32 id, ITrack *&pTrack);

        BOOL32 getTrack(const s32 id, ITrack *&pTrack);

        CSafeList<ITrack> *getTrackByType(EMTrack type);

        s32 getTrackCount();

        inline const u32 getId() const;

        s64 getDuration();

    private:
        ILock *m_pLock;

        static u32 m_sCount;
        // track id
        const u32 m_uId;

        s16 m_wZIndex;

        CSafeList<ITrack> *m_pTrackLists[EM_TRACK_END];
    };

    inline const u32 CStoryboard::getId() const {
        return m_uId;
    }

} // namespace paomiantv

#endif // _PAOMIANTV_STORYBOARD_H_