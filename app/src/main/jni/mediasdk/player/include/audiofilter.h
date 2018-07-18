/*******************************************************************************
 *        Module: paomiantv
 *          File:
 * Functionality: audio filter (for resample)
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-08-03  v1.0        huangxuefeng  created
 ******************************************************************************/
#ifndef MEDIAENGINE_CAUDIOFILTER_H
#define MEDIAENGINE_CAUDIOFILTER_H

#include "typedef.h"
#include "constant.h"
#ifdef __cplusplus
extern "C"
{
#endif
#include "libavutil/channel_layout.h"
#include "libavutil/md5.h"
#include "libavutil/mem.h"
#include "libavutil/opt.h"
#include "libavutil/samplefmt.h"

#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#ifdef __cplusplus
}
#endif
namespace paomiantv {

    class CAudioFilter {
    public:
        CAudioFilter();

        virtual  ~CAudioFilter();

        s32
        init(s32 nInputSampleRate, s32 nInputFormat, u64 ullInputChannelLayout, float nOutVolume);

        void uninit();

        int process(u8 *in_buffer, int in_frame_size,u8* &out_buffer,int &out_size);

    private:
        AVFilterGraph *m_pGraph;
        AVFilterContext *m_pSrc;
        AVFilterContext *m_pSink;
        AVFrame *m_pFrame;
        s32 m_nInputSampleRate;
        AVSampleFormat m_eInputFormat;
        u64 m_ullInputChannelLayout;
        float m_fOutVolume;

        s64 m_ullFramePTS;
        u8 *m_pOutBuffer;
    private:
        s32 init_filter_graph(AVFilterGraph **graph, AVFilterContext **src, AVFilterContext **sink);

        AVSampleFormat shiftfmt(s32 format);

        u64 shiftchnlyt(u64 nInputChannelLayout);

        s32 process_output(s32 &size, AVFrame *frame);

        s32 process_input(u8 *buffer, s32 frameSize);

    };
};
#endif //MEDIAENGINE_CAUDIOFILTER_H
