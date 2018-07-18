//
// Created by ASUS on 2018/4/16.
//

#ifndef MEDIAENGINE_AFILTERCOMPLEX_H
#define MEDIAENGINE_AFILTERCOMPLEX_H

#include <autolock.h>
#include <afilter.h>
#include <asink.h>
#include <abuffer.h>
#include <vector>
#include <amix.h>
#include <enum.h>
#include <unordered_map>
#include <graph.h>
#include <volume.h>

#define MAX_OUTPUT_BUFFER_SIZE (128*AUDIO_SAMPLE_COUNT_PER_FRAME)
namespace paomiantv {

    typedef struct tagAFilterNode {
        IAFilter *m_pcFilter;
        tagAFilterNode *m_ptNext;
    public:
        tagAFilterNode() { memset(this, 0, sizeof(struct tagAFilterNode)); }
    } TAFilterNode, *TPAFilterNode;

    class CAFilterComplex {
    public:
        static CAFilterComplex *Create(EMSampleRate eDstSampleRate = EM_SAMPLE_RATE_44_1,
                                       AVSampleFormat eDstFormat = AV_SAMPLE_FMT_S16,
                                       u64 ullDstChannelLayout = AV_CH_LAYOUT_STEREO);

        virtual void destroy();

    protected:
        CAFilterComplex(EMSampleRate eDstSampleRate, AVSampleFormat eDstFormat,
                        u64 ullDstChannelLayout);

        virtual ~CAFilterComplex();

        virtual BOOL32 init();

        virtual void uninit();

        virtual const TAFilterNode *findHeadBySourceId(const u32 id);

        virtual int process_input(u8 *buffer, int nframeSize, CABuffer *inFilter);

        virtual int process_input(AVFrame *frame, CABuffer *inFilter);

        virtual int process_output(s32 &size);

    public:


        BOOL32 addInput(EMSampleRate uSrcSampleRate, AVSampleFormat eSrcFormat, u64 ullSrcChannelLayout, s16 wWeight, u32 &outSourceId);

        CABuffer* getInput(u32 sourceId);

        BOOL32 addVolumeInSource(const u32 id, float fVolume);
        CVolume *getInputVolume(u32 sourceId);


        BOOL32 addFormatInSource(const u32 id, EMSampleRate eDstSampleRate, AVSampleFormat eDstFormat, u64 ullDstChannelLayout);
        CAFormat *getInputFormat(u32 sourceId);

        BOOL32 configure();

        BOOL32 addVolume(float folume);

        BOOL32 addFormat(EMSampleRate eDstSampleRate, AVSampleFormat eDstFormat, u64 ullDstChannelLayout);


        s32 process(int inputNum, u32 *ids, u8 **in_buffer, int *in_frame_size, u8 *&out_buffer, int &out_size);

        s32 process(int inputNum, u32 *ids, AVFrame **inout, u8 *&out_buffer, int &out_size);

        inline EMSampleRate getDstSampleRate();

        inline AVSampleFormat getDstSampleFormat();

        inline u64 getDstChannelLayout();

        int getInputCount();

        inline CGraph *getGraph();

    protected:
        ILock *m_pLock;
        CGraph *m_pGraph;
        CASink *m_pSink;
        CAMix *m_pMix;

        // the heads of the source filter list with the pre mixed data
        std::vector<TAFilterNode *> m_vHeadsPremix;

        AVFrame *m_pFrame;

        // the heads of filter list with the mixed data
        TAFilterNode *m_pHeadMixed;

        const EMSampleRate m_eDstSampleRate;
        const AVSampleFormat m_eDstFormat;
        const u64 m_ullDstChannelLayout;

        std::unordered_map<u32, s64> m_mapPTSInput;
        u8 *m_pbyBufferOut;
    };

    inline EMSampleRate CAFilterComplex::getDstSampleRate() {
        return m_eDstSampleRate;
    }

    inline AVSampleFormat CAFilterComplex::getDstSampleFormat() {
        return m_eDstFormat;
    }

    inline u64 CAFilterComplex::getDstChannelLayout() {
        return m_ullDstChannelLayout;
    }

    inline CGraph *CAFilterComplex::getGraph() {
        return m_pGraph;
    }
}


#endif //MEDIAENGINE_AFILTERCOMPLEX_H
