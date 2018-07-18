//
// Created by ASUS on 2018/4/17.
//

#ifndef MEDIAENGINE_AFILTER_H
#define MEDIAENGINE_AFILTER_H

#include <typedef.h>
#include <autolock.h>
#include <constant.h>
#include <enum.h>
#include "graph.h"

#ifdef __cplusplus
extern "C"
{
#include <libavutil/channel_layout.h>
};
#endif

namespace paomiantv {

    typedef struct tagAuidoParams {
        EMSampleRate m_eSampleRate;
        u64 m_ullChannelLayout;
        AVSampleFormat m_eFormat;
        s16 m_wWeigt;
        BOOL32 m_bIsSilence;

        tagAuidoParams() {
            m_eFormat = AV_SAMPLE_FMT_S16;
            m_ullChannelLayout = AV_CH_LAYOUT_STEREO;
            m_eSampleRate = EM_SAMPLE_RATE_44_1;
            m_wWeigt = 1;
            m_bIsSilence = FALSE;
        }
    } TAudioParams;

    class IAFilter {

    protected:
        IAFilter();

        virtual ~IAFilter();

        virtual s32 init(const CGraph *pGraph) =0;

    public:
        virtual void destroy();

        inline AVFilterContext *getContext();

        inline u32 getId();

        inline EMAFilter getType();

        inline s8 *getName();

    private:
        void uninit();

    protected:

        static u32 m_sCount;

        // filter id
        const u32 m_uId;
        ILock *m_pLock;

        EMAFilter m_eType;

        s8 m_achName[MAX_LEN_FILTER_NAME];
        AVFilterContext *m_ptContext;
        AVFilter *m_ptFilter;
    };

    inline AVFilterContext *IAFilter::getContext() {
        return m_ptContext;
    }

    inline u32 IAFilter::getId() {
        return m_uId;
    }

    inline EMAFilter IAFilter::getType() {
        return m_eType;
    }

    inline s8 *IAFilter::getName() {
        return m_achName;
    }
}

#endif //MEDIAENGINE_AFILTER_H
