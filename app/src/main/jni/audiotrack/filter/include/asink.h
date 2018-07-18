//
// Created by ASUS on 2018/4/17.
//

#ifndef MEDIAENGINE_ASINK_H
#define MEDIAENGINE_ASINK_H

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
    class CASink : public IAFilter {
    protected:
        CASink();

        virtual s32 init(const CGraph *pGraph);

    public:
        virtual ~CASink();

        static CASink *Create(const CGraph *pGraph);

        virtual void destroy();

        s32 setParams(EMSampleRate eDstSampleRate = EM_SAMPLE_RATE_44_1,
                      AVSampleFormat eDstFormat = AV_SAMPLE_FMT_S16,
                      u64 ullDstChannelLayout = AV_CH_LAYOUT_STEREO);
    };
}


#endif //MEDIAENGINE_ASINK_H
