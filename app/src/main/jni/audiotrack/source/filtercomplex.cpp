//
// Created by ASUS on 2018/4/16.
//
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#ifdef __cplusplus
}
#endif

#include <volume.h>
#include "filtercomplex.h"

namespace paomiantv {

    CAFilterComplex *CAFilterComplex::Create(EMSampleRate eDstSampleRate, AVSampleFormat eDstFormat,
                                             u64 ullDstChannelLayout) {
        CAFilterComplex *p = new CAFilterComplex(eDstSampleRate, eDstFormat, ullDstChannelLayout);
        if (p->init()) {
            return p;
        } else {
            delete p;
            return NULL;
        }
    }

    void CAFilterComplex::destroy() {
        delete this;
    }


    CAFilterComplex::CAFilterComplex(EMSampleRate eDstSampleRate, AVSampleFormat eDstFormat,
                                     u64 ullDstChannelLayout) :
            m_pGraph(NULL),
            m_pMix(NULL),
            m_pSink(NULL),
            m_pHeadMixed(NULL),
            m_pFrame(NULL),
            m_eDstSampleRate(eDstSampleRate),
            m_eDstFormat(eDstFormat),
            m_ullDstChannelLayout(ullDstChannelLayout),
            m_pbyBufferOut(NULL) {
        USE_LOG;
        m_vHeadsPremix.clear();
        m_mapPTSInput.clear();
        m_pLock = new CLock;
    }

    CAFilterComplex::~CAFilterComplex() {
        USE_LOG;
        uninit();
        if (m_pLock != NULL) {
            delete m_pLock;
            m_pLock = NULL;
        }
    }

    BOOL32 CAFilterComplex::init() {
        USE_LOG;
        CAutoLock autoLock(m_pLock);
        m_pbyBufferOut = (u8 *) av_malloc(MAX_OUTPUT_BUFFER_SIZE);
        if (m_pbyBufferOut == NULL) {
            LOGE("Error allocating the output buffer\n");
            return FALSE;
        }
        m_pFrame = av_frame_alloc();
        if (!m_pFrame) {
            LOGE("Error allocating the frame\n");
            return FALSE;
        }
        m_vHeadsPremix.clear();
        m_pHeadMixed = NULL;
        m_pGraph = CGraph::Create();
        if (m_pGraph == NULL) {
            return FALSE;
        }
        m_pSink = CASink::Create(m_pGraph);
        if (m_pSink == NULL ||
            m_pSink->setParams(m_eDstSampleRate, m_eDstFormat, m_ullDstChannelLayout) != 0) {
            return FALSE;
        }
        return TRUE;
    }

    void CAFilterComplex::uninit() {
        CAutoLock autoLock(m_pLock);
        for (int i = 0; i < m_vHeadsPremix.size(); i++) {
            TAFilterNode *head = m_vHeadsPremix[i];
            while (head != NULL) {
                if (head->m_pcFilter != NULL) {
                    head->m_pcFilter->destroy();
                }
                head = head->m_ptNext;
            }
        }
        m_vHeadsPremix.clear();

        while (m_pHeadMixed != NULL) {
            if (m_pHeadMixed->m_pcFilter != NULL) {
                m_pHeadMixed->m_pcFilter->destroy();
            }
            m_pHeadMixed = m_pHeadMixed->m_ptNext;
        }

        if (m_pMix != NULL) {
            m_pMix->destroy();
            m_pMix = NULL;
        }
        if (m_pSink != NULL) {
            m_pSink->destroy();
            m_pSink = NULL;
        }
        if (m_pGraph != NULL) {
            m_pGraph->destroy();
            m_pGraph = NULL;
        }
        if (&m_pFrame) {
            av_frame_free(&m_pFrame);
            m_pFrame = NULL;
        }
        if (m_pbyBufferOut != NULL) {
            av_free(m_pbyBufferOut);
            m_pbyBufferOut = NULL;
        }

        m_mapPTSInput.clear();
    }

    // the input buffer have to exist 1024 sample
    s32 CAFilterComplex::process(int inputNum, u32 *ids, u8 **in_buffer, int *in_frame_size,
                                 u8 *&out_buffer,
                                 int &out_size) {
        CAutoLock autoLock(m_pLock);
        out_buffer = m_pbyBufferOut;
        out_size = 0;
        int err;
        for (int i = 0; i < m_vHeadsPremix.size(); i++) {
            u8 *in = NULL;
            int size = AUDIO_SAMPLE_COUNT_PER_FRAME;
            const TAFilterNode *node = m_vHeadsPremix[i];
            for (int j = 0; j < inputNum; j++) {
                if (ids[j] == node->m_pcFilter->getId() &&
                    in_buffer[j] != NULL &&
                    in_frame_size[j] == AUDIO_SAMPLE_COUNT_PER_FRAME) {
                    in = in_buffer[j];
                    break;
                }
            }
            err = process_input(in, size, dynamic_cast<CABuffer *>(node->m_pcFilter));
            if (err < 0) {
                LOGE("Error generating input frame:");
                return err;
            }
        }

        while ((err = av_buffersink_get_frame(m_pSink->getContext(), m_pFrame)) >= 0) {
            /* now do something with our filtered frame */
            err = process_output(out_size);
            if (err < 0) {
                LOGE("Error processing the filtered frame:");
                return err;
            }
            av_frame_unref(m_pFrame);
        }

        if (err == AVERROR(EAGAIN)) {
            /* Need to feed more frames in. */
//            LOGW("Need to feed more frames in.");
        } else if (err == AVERROR_EOF) {
            /* Nothing more to do, finish. */
//            LOGW("Nothing more to do, finish.");
        } else if (err < 0) {
            /* An error occurred. */
            LOGE("Error filtering the task:");
        }
        return err;
    }

    // the input buffer have to exist 1024 sample
    s32 CAFilterComplex::process(int inputNum, u32 *ids, AVFrame **frame,
                                 u8 *&out_buffer,
                                 int &out_size) {
        CAutoLock autoLock(m_pLock);
        int err;
        out_buffer = m_pbyBufferOut;
        for (int i = 0; i < m_vHeadsPremix.size(); i++) {
            AVFrame *in = NULL;
            const TAFilterNode *node = m_vHeadsPremix[i];
            for (int j = 0; j < inputNum; j++) {
                if (ids[j] == node->m_pcFilter->getId() &&
                    frame[j] != NULL &&
                    frame[j]->nb_samples == AUDIO_SAMPLE_COUNT_PER_FRAME) {
                    in = frame[j];
                    break;
                }
            }
            err = process_input(in, dynamic_cast<CABuffer *>(node->m_pcFilter));
            if (err < 0) {
                LOGE("Error generating input frame:");
                return err;
            }
        }

        while ((err = av_buffersink_get_frame(m_pSink->getContext(), m_pFrame)) >= 0) {
            /* now do something with our filtered frame */
            err = process_output(out_size);
            if (err < 0) {
                LOGE("Error processing the filtered frame:");
                return err;
            }
            av_frame_unref(m_pFrame);
        }

        if (err == AVERROR(EAGAIN)) {
            /* Need to feed more frames in. */
//            LOGW("Need to feed more frames in.");
        } else if (err == AVERROR_EOF) {
            /* Nothing more to do, finish. */
//            LOGW("Nothing more to do, finish.");
        } else if (err < 0) {
            /* An error occurred. */
            LOGE("Error filtering the task:");
        }
        return err;
    }

    /* Construct a frame of audio task to be filtered;
     * this simple example just synthesizes a sine wave.
     */
    int CAFilterComplex::process_input(u8 *buffer, int nframeSize, CABuffer *inFilter) {
        int err;
        av_frame_unref(m_pFrame);
        m_mapPTSInput[inFilter->getId()] += nframeSize;
        /* Set up the frame properties and allocate the buffer for the task. */
        m_pFrame->sample_rate = inFilter->getSrcSampleRate();
        m_pFrame->format = inFilter->getSrcSampleFormat();
        m_pFrame->channel_layout = inFilter->getSrcChannelLayout();
        m_pFrame->nb_samples = nframeSize;
        m_pFrame->pts = m_mapPTSInput[inFilter->getId()];

        err = av_frame_get_buffer(m_pFrame, 0);
        if (err < 0) {
            m_mapPTSInput[inFilter->getId()] -= nframeSize;
            LOGE("Allocate new buffer(s) for audio failed.")
            return err;
        }
        int src_nb_channels = av_get_channel_layout_nb_channels(inFilter->getSrcChannelLayout());
        int src_is_planar = av_sample_fmt_is_planar(inFilter->getSrcSampleFormat());
        int nb_planes = src_is_planar ? src_nb_channels : 1;
        int src_bytes_per_sample = av_get_bytes_per_sample(inFilter->getSrcSampleFormat());
        int src_bytes_per_planar =
                m_pFrame->nb_samples * src_bytes_per_sample * (src_is_planar ? 1 : src_nb_channels);

        /* Fill the task for each channel. */
        for (int i = 0; i < nb_planes; i++) {
            if (buffer != NULL) {
                memcpy(m_pFrame->extended_data[i], buffer + (i * src_bytes_per_planar),
                       src_bytes_per_planar);
            } else {
                memset(m_pFrame->extended_data[i], 0, src_bytes_per_planar);
            }

        }

        /* Send the frame to the input of the filtergraph. */
        err = av_buffersrc_add_frame(inFilter->getContext(), m_pFrame);
        if (err < 0) {
            av_frame_unref(m_pFrame);
            LOGE("Error submitting the frame to the filtergraph:");
            return err;
        }
    }

    /* Construct a frame of audio task to be filtered;
     * this simple example just synthesizes a sine wave.
     */
    int CAFilterComplex::process_input(AVFrame *frame, CABuffer *inFilter) {
        int err;
        if (frame == NULL) {
            av_frame_unref(m_pFrame);
            m_mapPTSInput[inFilter->getId()] += AUDIO_SAMPLE_COUNT_PER_FRAME;
            /* Set up the frame properties and allocate the buffer for the task. */
            m_pFrame->sample_rate = inFilter->getSrcSampleRate();
            m_pFrame->format = inFilter->getSrcSampleFormat();
            m_pFrame->channel_layout = inFilter->getSrcChannelLayout();
            m_pFrame->nb_samples = AUDIO_SAMPLE_COUNT_PER_FRAME;
            m_pFrame->pts = m_mapPTSInput[inFilter->getId()];

            err = av_frame_get_buffer(m_pFrame, 0);
            if (err < 0) {
                m_mapPTSInput[inFilter->getId()] -= AUDIO_SAMPLE_COUNT_PER_FRAME;
                LOGE("Allocate new buffer(s) for audio failed.")
                return err;
            }
            int src_nb_channels = av_get_channel_layout_nb_channels(
                    inFilter->getSrcChannelLayout());
            int src_is_planar = av_sample_fmt_is_planar(inFilter->getSrcSampleFormat());
            int nb_planes = src_is_planar ? src_nb_channels : 1;
            int src_bytes_per_sample = av_get_bytes_per_sample(inFilter->getSrcSampleFormat());
            int src_bytes_per_planar =
                    m_pFrame->nb_samples * src_bytes_per_sample *
                    (src_is_planar ? 1 : src_nb_channels);

            /* Fill the task for each channel. */
            for (int i = 0; i < nb_planes; i++) {
                memset(m_pFrame->extended_data[i], 0, src_bytes_per_planar);
            }
            err = av_buffersrc_add_frame(inFilter->getContext(), m_pFrame);
            av_frame_unref(m_pFrame);
            if (err < 0) {
                LOGE("Error submitting the frame to the filtergraph:");
                return err;
            }
        } else {
            m_mapPTSInput[inFilter->getId()] += frame->nb_samples;
            /* Send the frame to the input of the filtergraph. */
            err = av_buffersrc_add_frame(inFilter->getContext(), frame);
            if (err < 0) {
//                av_frame_unref(frame);
                LOGE("Error submitting the frame to the filtergraph:");
                return err;
            }
        }

    }


    /* Do something useful with the filtered task: this simple
     * example just prints the MD5 checksum of each plane to stdout.
     */
    int CAFilterComplex::process_output(s32 &size) {
        int planar = av_sample_fmt_is_planar((AVSampleFormat) m_pFrame->format);
        int channels = av_get_channel_layout_nb_channels(m_pFrame->channel_layout);
        int planes = planar ? channels : 1;
        int bps = av_get_bytes_per_sample((AVSampleFormat) m_pFrame->format);
        int plane_size = bps * m_pFrame->nb_samples * (planar ? 1 : channels);
        for (int i = 0; i < planes; i++) {
            if (size + plane_size > MAX_OUTPUT_BUFFER_SIZE) {
                LOGE("Error the output buffer is too large.There is not enough memory");
                return -10000;
            }
            memcpy(m_pbyBufferOut + (i * plane_size), m_pFrame->extended_data[i], plane_size);
            size += plane_size;
        }
        return 0;
    }

    BOOL32 CAFilterComplex::configure() {
        USE_LOG;
        CAutoLock autoLock(m_pLock);
        s32 err = 0;
        const u32 inputs = m_vHeadsPremix.size();

        /* Connect the filters;
         * in this simple case the filters just form a linear chain. */
        std::vector<IAFilter *> sources;
        // connect the sources filters
        s16 *weights = new s16[inputs];
        for (int i = 0; i < inputs; i++) {
            TAFilterNode *tmp = m_vHeadsPremix[i];
            if (tmp == NULL || tmp->m_pcFilter == NULL) {
                LOGE("source%d input header filter incorrect", i);
                return FALSE;
            }
            weights[i] = ((CABuffer *) tmp->m_pcFilter)->getWeight();
            TAFilterNode *pre = NULL;
            while (tmp->m_ptNext != NULL) {
                if (tmp->m_pcFilter == NULL) {
                    LOGE("source%d input filter incorrect", i);
                    return FALSE;
                }
                if (pre != NULL) {
                    err = avfilter_link(pre->m_pcFilter->getContext(), 0,
                                        tmp->m_pcFilter->getContext(), 0);
                    if (err < 0) {
                        LOGE("source%d connect filters failed\n", i);
                        return FALSE;
                    }
                }
                pre = tmp;
                tmp = tmp->m_ptNext;
            }
            if (tmp->m_pcFilter == NULL) {
                LOGE("source%d input filter incorrect", i);
                return FALSE;
            }

            if (pre != NULL) {
                err = avfilter_link(pre->m_pcFilter->getContext(), 0, tmp->m_pcFilter->getContext(),
                                    0);
                if (err < 0) {
                    LOGE("source%d connect filters failed\n", i);
                    return FALSE;
                }
            }
            sources.push_back(tmp->m_pcFilter);
        }

        if ((inputs > 1 && m_pMix->setParams(inputs, weights) != 0) || inputs <= 0) {
            LOGE("configure mixer failed!");
            delete[] weights;
            return FALSE;
        }
        delete[] weights;

        IAFilter *last = NULL;
        if (inputs > 1) {
            // connect the sources to mix
            for (unsigned i = 0; i < inputs; i++) {
                err = avfilter_link(sources[i]->getContext(), 0,
                                    m_pMix->getContext(), i);
                if (err < 0) {
                    LOGE("source%d connect to mix failed\n", i);
                    return FALSE;
                }
            }
            last = m_pMix;
        } else {
            last = sources[0];
        }

        // connect the mixed data filters;
        if (m_pHeadMixed != NULL) {
            TAFilterNode *tmp = m_pHeadMixed;
            TAFilterNode *pre = NULL;
            while (tmp->m_ptNext != NULL) {
                if (tmp->m_pcFilter == NULL) {
                    LOGE("the after mix filter incorrect");
                    return FALSE;
                }
                if (pre != NULL) {
                    err = avfilter_link(pre->m_pcFilter->getContext(), 0,
                                        tmp->m_pcFilter->getContext(), 0);
                } else {
                    err = avfilter_link(last->getContext(), 0,
                                        tmp->m_pcFilter->getContext(), 0);

                }
                if (err < 0) {
                    LOGE("the after mix filter connect filters failed\n");
                    return FALSE;
                }
                pre = tmp;
                tmp = tmp->m_ptNext;
            }
            if (tmp->m_pcFilter == NULL) {
                LOGE("the after mix filter incorrect");
                return FALSE;
            }
            if (pre != NULL) {
                err = avfilter_link(pre->m_pcFilter->getContext(), 0,
                                    tmp->m_pcFilter->getContext(), 0);
            } else {
                err = avfilter_link(last->getContext(), 0,
                                    tmp->m_pcFilter->getContext(), 0);
            }

            if (err < 0) {
                LOGE("the after mix filter connect filters failed\n");
                return FALSE;
            }

            last = tmp->m_pcFilter;
        }

        // connect to sink;
        err = avfilter_link(last->getContext(), 0, m_pSink->getContext(), 0);
        if (err < 0) {
            LOGE("connect to sink filter failed\n");
            return FALSE;
        }

        /* Configure the graph. */
        err = avfilter_graph_config(m_pGraph->getGraph(), NULL);
        if (err < 0) {
            LOGE("Error configuring the filter graph\n");
            return FALSE;
        }
#if LOG_ENABLE
        char *str = avfilter_graph_dump(m_pGraph->getGraph(), NULL);
        LOGE("Graph :\n%s\n", str);
        av_free(str);
#endif
        return TRUE;
    }

    BOOL32
    CAFilterComplex::addInput(EMSampleRate uSrcSampleRate, AVSampleFormat eSrcFormat,
                              u64 ullSrcChannelLayout, s16 wWeight, u32 &outSourceId) {
        USE_LOG;
        CAutoLock autoLock(m_pLock);
        CABuffer *pb = CABuffer::Create(m_pGraph);
        if (pb == NULL ||
            pb->setParams(uSrcSampleRate, eSrcFormat, ullSrcChannelLayout, wWeight) != 0) {
            if (pb != NULL) {
                pb->destroy();
            }
            return FALSE;
        }
        TAFilterNode *node = new TAFilterNode;
        node->m_pcFilter = pb;
        node->m_ptNext = NULL;
        m_vHeadsPremix.push_back(node);
        outSourceId = pb->getId();
        m_mapPTSInput[outSourceId] = 0;
        if (m_vHeadsPremix.size() > 1) {
            if (m_pMix == NULL) {
                m_pMix = CAMix::Create(m_pGraph);
                if (m_pMix == NULL) {
                    return FALSE;
                }
            }
        }
        return TRUE;
    }

    CABuffer *CAFilterComplex::getInput(u32 sourceId) {
        CABuffer *ret = NULL;
        CAutoLock autoLock(m_pLock);
        const TAFilterNode *head = findHeadBySourceId(sourceId);
        if (head == NULL) {
            LOGE("can not find the source input!");
            return NULL;
        }
        while (head != NULL) {
            if (head->m_pcFilter != NULL && head->m_pcFilter->getType() == EM_A_FILTER_SOURCE) {
                ret = (CABuffer *) head->m_pcFilter;
                break;
            }
            head = head->m_ptNext;
        }
        return ret;
    }

    CVolume *CAFilterComplex::getInputVolume(u32 sourceId) {
        CVolume *ret = NULL;
        CAutoLock autoLock(m_pLock);
        const TAFilterNode *head = findHeadBySourceId(sourceId);
        if (head == NULL) {
            LOGE("can not find the source input!");
            return NULL;
        }
        while (head != NULL) {
            if (head->m_pcFilter != NULL && head->m_pcFilter->getType() == EM_A_FILTER_VOLUME) {
                ret = (CVolume *) head->m_pcFilter;
                break;
            }
            head = head->m_ptNext;
        }
        return ret;
    }

    CAFormat *CAFilterComplex::getInputFormat(u32 sourceId) {
        CAFormat *ret = NULL;
        CAutoLock autoLock(m_pLock);
        const TAFilterNode *head = findHeadBySourceId(sourceId);
        if (head == NULL) {
            LOGE("can not find the source input!");
            return NULL;
        }
        while (head != NULL) {
            if (head->m_pcFilter != NULL && head->m_pcFilter->getType() == EM_A_FILTER_FORMAT) {
                ret = (CAFormat *) head->m_pcFilter;
                break;
            }
            head = head->m_ptNext;
        }
        return ret;
    }

    BOOL32 CAFilterComplex::addVolumeInSource(const u32 id, float fVolume) {
        USE_LOG;
        CAutoLock autoLock(m_pLock);
        const TAFilterNode *head = findHeadBySourceId(id);
        if (head == NULL) {
            LOGE("can not find the source input!");
            return FALSE;
        }
        CVolume *pv = CVolume::Create(m_pGraph);
        if (pv == NULL || pv->setParams(fVolume) != 0) {
            if (pv != NULL) {
                pv->destroy();
            }
            return FALSE;
        }

        TAFilterNode *node = new TAFilterNode;
        node->m_pcFilter = pv;
        node->m_ptNext = NULL;

        TAFilterNode *tail = const_cast<TAFilterNode *>(head);
        while (tail->m_ptNext != NULL) {
            tail = tail->m_ptNext;
        }
        tail->m_ptNext = node;
        return TRUE;
    }

    BOOL32
    CAFilterComplex::addFormatInSource(const u32 id, EMSampleRate eDstSampleRate,
                                       AVSampleFormat eDstFormat,
                                       u64 ullDstChannelLayout) {
        USE_LOG;
        CAutoLock autoLock(m_pLock);
        const TAFilterNode *head = findHeadBySourceId(id);
        if (head == NULL) {
            LOGE("can not find the source input!");
            return FALSE;
        }
        CAFormat *pf = CAFormat::Create(m_pGraph);
        if (pf == NULL || pf->setParams(eDstSampleRate, eDstFormat, ullDstChannelLayout) != 0) {
            if (pf != NULL) {
                pf->destroy();
            }
            return FALSE;
        }

        TAFilterNode *node = new TAFilterNode;
        node->m_pcFilter = pf;
        node->m_ptNext = NULL;

        TAFilterNode *tail = const_cast<TAFilterNode *>(head);
        while (tail->m_ptNext != NULL) {
            tail = tail->m_ptNext;
        }
        tail->m_ptNext = node;
        return TRUE;
    }

    BOOL32 CAFilterComplex::addVolume(float fVolume) {
        USE_LOG;
        CAutoLock autoLock(m_pLock);
        CVolume *pv = CVolume::Create(m_pGraph);
        if (pv == NULL || pv->setParams(fVolume) != 0) {
            if (pv != NULL) {
                pv->destroy();
            }
            return FALSE;
        }

        TAFilterNode *node = new TAFilterNode;
        node->m_pcFilter = pv;
        node->m_ptNext = NULL;

        if (m_pHeadMixed == NULL) {
            m_pHeadMixed = node;
        } else {
            TAFilterNode *tail = m_pHeadMixed;
            while (tail->m_ptNext != NULL) {
                tail = tail->m_ptNext;
            }
            tail->m_ptNext = node;
        }
        return TRUE;
    }

    BOOL32 CAFilterComplex::addFormat(EMSampleRate eDstSampleRate, AVSampleFormat eDstFormat,
                                      u64 ullDstChannelLayout) {
        USE_LOG;
        CAutoLock autoLock(m_pLock);
        CAFormat *pf = CAFormat::Create(m_pGraph);
        if (pf == NULL || pf->setParams(eDstSampleRate, eDstFormat, ullDstChannelLayout) != 0) {
            if (pf != NULL) {
                pf->destroy();
            }
            return FALSE;
        }

        TAFilterNode *node = new TAFilterNode;
        node->m_pcFilter = pf;
        node->m_ptNext = NULL;

        if (m_pHeadMixed == NULL) {
            m_pHeadMixed = node;
        } else {
            TAFilterNode *tail = m_pHeadMixed;
            while (tail->m_ptNext != NULL) {
                tail = tail->m_ptNext;
            }
            tail->m_ptNext = node;
        }
        return TRUE;
    }

    const TAFilterNode *CAFilterComplex::findHeadBySourceId(const u32 id) {
        TAFilterNode *re = NULL;
        CAutoLock autoLock(m_pLock);
        for (int i = 0; i < m_vHeadsPremix.size(); i++) {
            if (m_vHeadsPremix[i] != NULL && m_vHeadsPremix[i]->m_pcFilter->getId() == id) {
                re = m_vHeadsPremix[i];
                break;
            }
        }
        return re;
    }

    int CAFilterComplex::getInputCount() {
        int ret = 0;
        CAutoLock autoLock(m_pLock);
        ret = m_vHeadsPremix.size();
        return ret;
    }
}