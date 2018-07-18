//
// Created by ASUS on 2018/4/16.
//

#ifndef MEDIAENGINE_AFORMAT_H
#define MEDIAENGINE_AFORMAT_H

#include <afilter.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavutil/channel_layout.h>
#ifdef __cplusplus
}
#endif
namespace paomiantv {
    class CAFormat : public IAFilter {
    protected:
        CAFormat();

        virtual s32 init(const CGraph *pGraph);

    public:
        virtual ~CAFormat();

        static CAFormat *Create(const CGraph *pGraph);

        virtual void destroy();

        s32 setParams(EMSampleRate eDstSampleRate = EM_SAMPLE_RATE_44_1,
                      AVSampleFormat eDstFormat = AV_SAMPLE_FMT_S16,
                      u64 ullDstChannelLayout = AV_CH_LAYOUT_STEREO);
    };
}

#endif //MEDIAENGINE_AFORMAT_H
