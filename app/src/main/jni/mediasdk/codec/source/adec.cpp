/*******************************************************************************
 *        Module: mediasdk
 *          File:
 * Functionality: aac decoder
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-07-24  v1.0        huangxuefeng  created
 ******************************************************************************/

#include <adec.h>
#include <track.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}
#endif

namespace paomiantv {

    CADec::CADec(ITrack *const &pTrack) : CDec(pTrack),
                                          m_pAudioPreHandler(NULL),
                                          m_uSourceId(0),
                                          m_bIsStopped(FALSE),
                                          m_bIsStarted(FALSE),
                                          m_bIsPaused(FALSE),
                                          m_ptAVFormatContext(NULL),
                                          m_ptAVCodecContext(NULL),
                                          m_ptAVFrame(NULL),
                                          m_ptRemainderFrame(NULL),
                                          m_uRemainderSampleSize(0),
                                          m_nStreamIndex(-1),
                                          m_pbyRemainderBuffer(NULL),
                                          m_uOneFrameSize(0),
                                          m_uRemainderBufferCapacity(0),
                                          m_uRemainderBufferSize(0),
                                          m_sllCurrPlayUS(0),
                                          m_sllStartPlayUS(0),
                                          m_sllEndCutUS(0),
                                          m_fVolume(1.0) {
        USE_LOG;
        m_pQueue = new CSafeQueue<CAudio>();
        m_pLock = new CLock;
        m_pThread = new CThread(ThreadWrapper, this);
    }

    CADec::~CADec() {
        USE_LOG;
        stop();
        release();
        if (m_pThread != NULL) {
            delete m_pThread;
            m_pThread = NULL;
        }
        if (m_pQueue != NULL) {
            delete m_pQueue;
            m_pQueue = NULL;
        }
        if (m_pAudioPreHandler != NULL) {
            m_pAudioPreHandler->destroy();
            m_pAudioPreHandler = NULL;
        }
        if (m_pbyRemainderBuffer != NULL) {
            free(m_pbyRemainderBuffer);
            m_pbyRemainderBuffer = NULL;
        }
        m_uRemainderBufferSize = 0;
        if (m_pLock != NULL) {
            delete m_pLock;
            m_pLock = NULL;
        }

    }


    int CADec::prepare() {
        m_pQueue->clear();
        m_pQueue->enable();
        int error;
        if (m_pTrack != NULL && m_pTrack->isSourceValid() &&
            m_pTrack->getSourceType() == EM_SOURCE_FILE) {

            uint8_t *data = nullptr;
            uint32_t size = 0;
            m_pTrack->getSourceData(data,size);
            /** Open the input file to read from it. */
            if ((error = avformat_open_input(&m_ptAVFormatContext,
                                             (s8 *) data,
                                             NULL,
                                             NULL)) < 0) {
                LOGE("Could not open input file '%s' (error '%d')\n",
                     (s8 *) data, error);
                m_ptAVFormatContext = NULL;
                return error;
            }

            /** Get information on the input file (number of streams etc.). */
            if ((error = avformat_find_stream_info(m_ptAVFormatContext, NULL)) < 0) {
                LOGE("Could not open find stream info (error '%d')\n", error);
                avformat_close_input(&m_ptAVFormatContext);
                return error;
            }
#ifdef LOG_ENABLE
            /* dump input information to stderr */
//            av_dump_format(m_ptAVFormatContext, 0, (s8 *) m_pTrack->getSource()->m_pbyData, 0);
#endif

            // find the audio stream index

            if ((error = open_codec_context(&m_nStreamIndex, &m_ptAVCodecContext,
                                            m_ptAVFormatContext,
                                            AVMEDIA_TYPE_AUDIO,
                                            (s8 *) data)) < 0) {
                LOGE("Could not open find codec (error '%d')\n", error);
                avformat_close_input(&m_ptAVFormatContext);
                return error;

            }

            // seek to the start frame
            s64 startTm = av_rescale_q(((CTrack *) m_pTrack)->getCutStart() * 1000,
                                       AV_TIME_BASE_Q,
                                       m_ptAVFormatContext->streams[m_nStreamIndex]->time_base);
            if (startTm < 0 && startTm >= m_ptAVFormatContext->streams[m_nStreamIndex]->duration) {
                LOGE("track params is invalid\n");
                error = AVERROR_INVALIDDATA;
                avcodec_free_context(&m_ptAVCodecContext);
                avformat_close_input(&m_ptAVFormatContext);
                return error;
            }
            s64 re = -1;
            error = adjustSeekTimestamp(startTm, re);
            if (error >= 0 && re >= 0) {
                error = av_seek_frame(m_ptAVFormatContext, m_nStreamIndex, re,
                                      AVSEEK_FLAG_BACKWARD);
            }
            if (error < 0 || re < 0) {
                LOGE("Seek to start pts failed (error '%d')\n", error);
                avcodec_free_context(&m_ptAVCodecContext);
                avformat_close_input(&m_ptAVFormatContext);
                return error;
            }

            s64 endTm = m_ptAVFormatContext->duration;
            if (((CTrack *) m_pTrack)->getCutDuration() < 0) {
                m_sllEndCutUS = endTm;
            } else {
                m_sllEndCutUS =
                        (((CTrack *) m_pTrack)->getCutStart() +
                         ((CTrack *) m_pTrack)->getCutDuration()) *
                        1000;
                m_sllEndCutUS = MIN (m_sllEndCutUS, endTm);
            }

            m_sllStartPlayUS = m_pTrack->getPlayStart() * 1000;
            m_sllEndPlayUS = m_pTrack->getPlayDuration() < 0 ? -1 :
                             (m_sllStartPlayUS + m_pTrack->getPlayDuration()) * 1000;


            // init the frame
            m_ptAVFrame = av_frame_alloc();
            m_ptRemainderFrame = av_frame_alloc();
            if (!m_ptAVFrame || !m_ptRemainderFrame) {
                LOGE("Could not allocate frame\n");
                error = AVERROR(ENOMEM);
                avcodec_free_context(&m_ptAVCodecContext);
                avformat_close_input(&m_ptAVFormatContext);
                if (m_ptAVFrame) {
                    av_frame_free(&m_ptAVFrame);
                }
                if (m_ptRemainderFrame) {
                    av_frame_free(&m_ptRemainderFrame);
                }
                return error;
            }
            m_ptRemainderFrame->sample_rate = m_ptAVCodecContext->sample_rate;
            m_ptRemainderFrame->format = m_ptAVCodecContext->sample_fmt;
            m_ptRemainderFrame->channel_layout = m_ptAVCodecContext->channel_layout;
            m_ptRemainderFrame->channels = av_get_channel_layout_nb_channels(
                    m_ptAVCodecContext->channel_layout);
            m_ptRemainderFrame->nb_samples = AUDIO_SAMPLE_COUNT_PER_FRAME;
            if (av_frame_get_buffer(m_ptRemainderFrame, 0) < 0) {
                error = AVERROR(ENOMEM);
                avcodec_free_context(&m_ptAVCodecContext);
                avformat_close_input(&m_ptAVFormatContext);
                av_frame_free(&m_ptAVFrame);
                av_frame_free(&m_ptRemainderFrame);
                return error;
            }


            // prepare the filters for resample output data
            m_pAudioPreHandler = CAFilterComplex::Create(m_eSampleRate, m_eFormat,
                                                         m_ullChannelLayout);
            if (m_pAudioPreHandler != NULL) {
                m_fVolume = ((CTrack *) m_pTrack)->getVolume();
                BOOL32 ret = FALSE;
                do {
                    if (!m_pAudioPreHandler->addInput(
                            (EMSampleRate) m_ptAVCodecContext->sample_rate,
                            m_ptAVCodecContext->sample_fmt,
                            m_ptAVCodecContext->channel_layout,
                            m_pTrack->getWeight(),
                            m_uSourceId)) {
                        break;
                    }
                    if (!m_pAudioPreHandler->addVolumeInSource(m_uSourceId, m_fVolume)) {
                        break;
                    }
                    if (!m_pAudioPreHandler->addFormatInSource(m_uSourceId, EM_SAMPLE_RATE_44_1,
                                                               AV_SAMPLE_FMT_S16,
                                                               AV_CH_LAYOUT_STEREO)) {
                        break;
                    }
                    if (!m_pAudioPreHandler->configure()) {
                        break;
                    }
                    ret = TRUE;
                } while (0);

                if (!ret) {
                    m_pAudioPreHandler->destroy();
                    m_pAudioPreHandler = NULL;
                }
            }
            if (m_pAudioPreHandler == NULL) {
                error = AVERROR(ENOMEM);
                avcodec_free_context(&m_ptAVCodecContext);
                avformat_close_input(&m_ptAVFormatContext);
                return error;
            }


            m_uOneFrameSize = (u32) (AUDIO_SAMPLE_COUNT_PER_FRAME *
                                     av_get_channel_layout_nb_channels(
                                             m_ullChannelLayout) *
                                     (0x0001 << m_eFormat));
            m_uRemainderBufferCapacity = m_uOneFrameSize * BLOCK_QUEUE_SIZE;
            m_pbyRemainderBuffer = (u8 *) malloc(m_uRemainderBufferCapacity);

            if (!m_pTrack->isIndependent()) {
                m_sllCurrPlayUS = m_sllStartPlayUS;
                if (m_sllEndPlayUS < 0) {
                    m_sllEndPlayUS = m_sllStartPlayUS + ((ITrack *) m_pTrack)->getDataDuration();
                }
            }
        } else {
            return -1;
        }

        return 0;
    }


    void CADec::release() {
        if (m_ptAVCodecContext != NULL) {
            avcodec_free_context(&m_ptAVCodecContext);
            m_ptAVCodecContext = NULL;
        }
        if (m_ptAVFormatContext != NULL) {
            avformat_close_input(&m_ptAVFormatContext);
            m_ptAVFormatContext = NULL;
        }
        if (m_ptAVFrame != NULL) {
            av_frame_free(&m_ptAVFrame);
            m_ptAVFrame = NULL;
        }
        if (m_ptRemainderFrame != NULL) {
            av_frame_free(&m_ptRemainderFrame);
            m_ptRemainderFrame = NULL;
        }
        m_uRemainderSampleSize = 0;
        if (m_pbyRemainderBuffer != NULL) {
            free(m_pbyRemainderBuffer);
            m_pbyRemainderBuffer = NULL;
        }
        m_uRemainderBufferCapacity = 0;
        m_uRemainderBufferSize = 0;
        m_uOneFrameSize = 0;
    }

    void CADec::start() {
        m_pQueue->enable();
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted) {
            return;
        }
        if (!m_pThread->start()) {
            LOGE("start audio decoder thread failed!");
            return;
        }
    }

    void CADec::pause() {
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted && !m_bIsStopped && !m_bIsPaused) {
            m_bIsPaused = TRUE;
        }
    }

    void CADec::resume() {
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted && !m_bIsStopped && m_bIsPaused) {
            m_bIsPaused = FALSE;
            m_pLock->acttive();
        }
    }

    void CADec::stop() {
        LOGI("CADec::stopThread");
        m_pQueue->disable();
        m_pLock->lock();
        if (m_bIsStarted && !m_bIsStopped) {
            m_bIsStopped = TRUE;
            m_pLock->acttive();
        }
        m_pLock->unlock();
        m_pThread->join();
        m_pQueue->clear();
    }

    BOOL32 CADec::getAudio(CAudio *&pAudio) {
        pAudio = NULL;

        if (m_bIsFinished) {
            if (!m_pQueue->empty()) {
                if (m_sllCurrPlayUS < m_sllStartPlayUS) {
                    LOGD("track%u: does not reach play start time!!!!!!!!!\n", m_pTrack->getId());
                } else if (m_sllEndPlayUS < 0 ? TRUE : m_sllCurrPlayUS <= m_sllEndPlayUS) {
                    m_pQueue->pop(pAudio);
                } else {
                    CAudio *tmp = NULL;
                    m_pQueue->pop(tmp);
                    CAudio::release(tmp);
                    LOGE("track%u: reach play end time!!!!!!!!!\n", m_pTrack->getId());
                }
            } else {
                if (m_sllEndPlayUS < 0 ? TRUE : m_sllCurrPlayUS <= m_sllEndPlayUS) {
                    pAudio = CAudio::create();
                    pAudio->resize(m_uOneFrameSize);
                    pAudio->m_uSize = m_uOneFrameSize;
                    pAudio->m_uTrackId = m_pTrack->getId();
                    pAudio->m_tParams.m_eFormat = m_eFormat;
                    pAudio->m_tParams.m_ullChannelLayout = m_ullChannelLayout;
                    pAudio->m_tParams.m_eSampleRate = m_eSampleRate;
                }
//                LOGD("track%u: play NULL when reach end!!!!!!!!!\n", m_pTrack->getId());
            }
        } else {
            if (!m_pQueue->empty()) {
                if (m_sllCurrPlayUS < m_sllStartPlayUS) {
                    LOGD("track%u: does not reach play start time!!!!!!!!!\n", m_pTrack->getId());
                } else if (m_sllEndPlayUS < 0 ? TRUE : m_sllCurrPlayUS <= m_sllEndPlayUS) {
                    m_pQueue->pop(pAudio);
                } else {
                    CAudio *tmp = NULL;
                    m_pQueue->pop(tmp);
                    CAudio::release(tmp);
                    LOGE("track%u: reach play start time!!!!!!!!!\n", m_pTrack->getId());
                }
            } else {
                m_pQueue->pop(pAudio);
            }
        }

        m_sllCurrPlayUS += (AUDIO_SAMPLE_COUNT_PER_FRAME * 1000 * 1000 / m_eSampleRate);
        return TRUE;
    }


//static
    void *CADec::ThreadWrapper(void *pThis) {
        CADec *p = (CADec *) pThis;
        int nErr = p->ThreadEntry();
        return (void *) nErr;
    }

    int CADec::ThreadEntry() {
        USE_LOG;
        char name[64];
        memset(name, 0, sizeof(name));
        sprintf(name, "CADecTrack%u", m_pTrack->getId());
        m_pThread->setName(name);
        m_pLock->lock();
        LOGI("track%u audio thread is started", m_pTrack->getId());
        m_bIsStarted = TRUE;

        while (!m_bIsStopped) {
            while (!m_bIsStopped && m_bIsPaused) {
                m_pLock->wait(1000);
            }
            if (!m_bIsStopped) {
                m_pLock->unlock();
                decode();
                m_pLock->lock();
            }
        }

        m_bIsStarted = FALSE;
        m_bIsStopped = FALSE;
        LOGI("track%u audio thread is stopped", m_pTrack->getId());
        m_pLock->unlock();
        return 0;

    }

    int CADec::decodeFrame() {
        int ret = 0;
        int data_present = 0;
        int finished = 0;
        if ((ret = decode_frame(m_ptAVFrame, m_ptAVFormatContext, m_ptAVCodecContext,
                                &data_present, &m_bIsInputFinished,
                                &finished, m_nStreamIndex))) {
            LOGE("Audio track%d decode frame failed! \n", m_pTrack->getId());
            return ret;

        }
        /**
         * If we are at the end of the file and there are no more samples
         * in the decoder which are delayed, we are actually finished.
         * This must not be treated as an error.
         */
        s64 pts = 0;
        if (data_present) {
            pts = av_rescale_q(m_ptAVFrame->best_effort_timestamp,
                               m_ptAVFormatContext->streams[m_nStreamIndex]->time_base,
                               AV_TIME_BASE_Q);
        }

        if ((finished && !data_present) || pts > m_sllEndCutUS) {
            ret = 0;
            m_bIsFinished = TRUE;
            LOGE("Audio track%d Input finished. Write NULL frame \n", m_pTrack->getId());
        } else if (data_present && pts >= (((CTrack *) m_pTrack)->getCutStart() *
                                           1000)) { /** If there is decoded data, convert and store it */
            /* push the audio data from decoded frame into the filtergraph */
//            LOGD("add %d samples on input %d (%d Hz, time=%f)\n",
//                 m_ptAVFrame->nb_samples, 1, m_ptAVCodecContext->sample_rate,
//                 (double) m_ptAVFrame->nb_samples / m_ptAVCodecContext->sample_rate);
            processBuffer();
            writeBuffer();
        }

        av_frame_unref(m_ptAVFrame);

        return ret;
    }

    BOOL32 CADec::processBuffer() {
//        if (m_uRemainderSampleSize)
//            av_samples_copy(frame->extended_data, frame->extended_data, 0,
//                            avctx->internal->skip_samples,
//                            frame->nb_samples - avctx->internal->skip_samples, avctx->channels,
//                            frame->format);
        if (m_fVolume != ((CTrack *) m_pTrack)->getVolume()) {
            m_fVolume = ((CTrack *) m_pTrack)->getVolume();
            char tmp[64];
            sprintf(tmp, "%.4f", m_fVolume);
            avfilter_graph_send_command(m_pAudioPreHandler->getGraph()->getGraph(),
                                        m_pAudioPreHandler->getInputVolume(m_uSourceId)->getName(),
                                        "volume", tmp, NULL, 0, 0);
        }
        if (m_uRemainderSampleSize == 0 &&
            m_ptAVFrame->nb_samples == AUDIO_SAMPLE_COUNT_PER_FRAME) {
            u8 *tmp = NULL;
            int tmps = 0;
            m_pAudioPreHandler->process(1, &m_uSourceId, &m_ptAVFrame, tmp, tmps);
            if (tmp != NULL && tmps > 0) {
                if (m_uRemainderBufferSize + tmps > m_uRemainderBufferCapacity) {
                    u8 *p = (u8 *) realloc(m_pbyRemainderBuffer, m_uRemainderBufferSize + tmps);
                    if (p == NULL) {
                        LOGE("realloc mem failed!");
                        return FALSE;
                    } else {
                        m_pbyRemainderBuffer = p;
                        m_uRemainderBufferCapacity = m_uRemainderBufferSize + tmps;
                    }
                }
                memcpy(m_pbyRemainderBuffer + m_uRemainderBufferSize, tmp, (u32) tmps);
                m_uRemainderBufferSize += tmps;
            }

        } else {
            // handle the processed data
            u32 sum = m_ptAVFrame->nb_samples + m_uRemainderSampleSize;
            u32 count = sum / AUDIO_SAMPLE_COUNT_PER_FRAME;
            u32 rmd = sum % AUDIO_SAMPLE_COUNT_PER_FRAME;
            AVFrame *frame = av_frame_alloc();
            u32 outOffset = 0;
            for (u32 i = 0; i < count; i++) {

                frame->sample_rate = m_ptAVFrame->sample_rate;
                frame->format = m_ptAVFrame->format;
                frame->channel_layout = m_ptAVFrame->channel_layout;
                frame->nb_samples = AUDIO_SAMPLE_COUNT_PER_FRAME;
                if (av_frame_get_buffer(frame, 0) < 0) {
                    LOGE("Allocate new buffer(s) for audio failed.")
                    av_frame_free(&frame);
                    return FALSE;
                }
                if (m_uRemainderSampleSize > 0) {
                    av_samples_copy(frame->extended_data,
                                    m_ptRemainderFrame->extended_data,
                                    0,
                                    0,
                                    m_uRemainderSampleSize,
                                    m_ptRemainderFrame->channels,
                                    static_cast<AVSampleFormat>(m_ptRemainderFrame->format));
                    av_samples_copy(frame->extended_data,
                                    m_ptAVFrame->extended_data,
                                    m_uRemainderSampleSize,
                                    outOffset,
                                    AUDIO_SAMPLE_COUNT_PER_FRAME - m_uRemainderSampleSize,
                                    m_ptAVFrame->channels,
                                    static_cast<AVSampleFormat>(m_ptAVFrame->format));
                    outOffset += (AUDIO_SAMPLE_COUNT_PER_FRAME - m_uRemainderSampleSize);
                    m_uRemainderSampleSize = 0;
                } else {
                    av_samples_copy(frame->extended_data, m_ptAVFrame->extended_data,
                                    0,
                                    outOffset,
                                    AUDIO_SAMPLE_COUNT_PER_FRAME,
                                    m_ptAVFrame->channels,
                                    static_cast<AVSampleFormat>(m_ptAVFrame->format));
                    outOffset += AUDIO_SAMPLE_COUNT_PER_FRAME;
                }
                u8 *tmp = NULL;
                int tmps = 0;
                m_pAudioPreHandler->process(1, &m_uSourceId, &frame, tmp, tmps);
                if (tmp != NULL && tmps > 0) {
                    if (m_uRemainderBufferSize + tmps > m_uRemainderBufferCapacity) {
                        u8 *p = (u8 *) realloc(m_pbyRemainderBuffer, m_uRemainderBufferSize + tmps);
                        if (p == NULL) {
                            LOGE("realloc mem failed!")
                            av_frame_free(&frame);
                            return FALSE;
                        } else {
                            m_pbyRemainderBuffer = p;
                            m_uRemainderBufferCapacity = m_uRemainderBufferSize + tmps;
                        }
                    }
                    memcpy(m_pbyRemainderBuffer + m_uRemainderBufferSize, tmp, (u32) tmps);
                    m_uRemainderBufferSize += tmps;
                }
            }
#ifdef LOG_ENABLE
            if (rmd != m_ptAVFrame->nb_samples - outOffset + m_uRemainderSampleSize) {
                LOGE("this situation will never happend!");
            }
#endif
            av_samples_copy(m_ptRemainderFrame->extended_data, m_ptAVFrame->extended_data,
                            m_uRemainderSampleSize,
                            outOffset,
                            m_ptAVFrame->nb_samples - outOffset,
                            m_ptAVFrame->channels,
                            static_cast<AVSampleFormat>(m_ptAVFrame->format));
            m_uRemainderSampleSize += (m_ptAVFrame->nb_samples - outOffset);
            av_frame_free(&frame);
        }

        return TRUE;
    }

    BOOL32 CADec::writeBuffer() {
        if (m_uRemainderBufferSize > 0) {
            // handle the processed data
            u32 count = m_uRemainderBufferSize / m_uOneFrameSize;
            u32 rmd = m_uRemainderBufferSize % m_uOneFrameSize;

            u32 outOffset = 0;
            for (u32 i = 0; i < count; i++) {
                CAudio *audio = CAudio::create();
                audio->resize(m_uOneFrameSize);
                memcpy(audio->m_pbyVoice, m_pbyRemainderBuffer + outOffset, m_uOneFrameSize);
                outOffset += m_uOneFrameSize;
                audio->m_uSize = m_uOneFrameSize;
                audio->m_uTrackId = m_pTrack->getId();
                audio->m_tParams.m_eFormat = m_eFormat;
                audio->m_tParams.m_ullChannelLayout = m_ullChannelLayout;
                audio->m_tParams.m_eSampleRate = m_eSampleRate;
                if (!m_pQueue->push(audio)) {
                    CAudio::release(audio);
                    LOGE("push audio failed!");
                }
            }
#ifdef LOG_ENABLE
            if (rmd != m_uRemainderBufferSize - outOffset) {
                LOGE("this situation will never happend!");
            }
#endif
            if (count > 0) {
                memmove(m_pbyRemainderBuffer, m_pbyRemainderBuffer + outOffset, rmd);
            }
            m_uRemainderBufferSize = rmd;
            return TRUE;
        }
        return FALSE;
    }

    int CADec::decode() {
        if (!m_bIsFinished) {
            decodeFrame();
        } else {
            usleep(100000);
        }
        return 0;
    }

    BOOL32 CADec::getRemainderBuffer(u8 *&out, u32 &size) {
        if (m_bIsFinished) {
            out = m_pbyRemainderBuffer;
            size = m_uRemainderBufferSize;
        } else {
            out = NULL;
            size = 0;
        }
        return TRUE;
    }

    BOOL32 CADec::getLayer(CVLayerParam *&pLayer) {
        pLayer = NULL;
        return FALSE;
    }

    int CADec::adjustSeekTimestamp(s64 time, s64 &out) {
        int error = 0;

        if (time < 0 && time >= m_ptAVFormatContext->streams[m_nStreamIndex]->duration) {
            LOGE("track params is invalid\n");
            error = AVERROR_INVALIDDATA;
            return error;
        }

        error = av_seek_frame(m_ptAVFormatContext, m_nStreamIndex, time, AVSEEK_FLAG_BACKWARD);
        if (error < 0)
            error = av_seek_frame(m_ptAVFormatContext, m_nStreamIndex, time, AVSEEK_FLAG_ANY);
        if (error < 0) {
            LOGE("Seek to start pts failed (error '%d')\n", error);
            return error;
        }

        /* Packet used for temporary storage. */
        AVPacket pkt;
        av_init_packet(&pkt);
        /** Set the packet data and size so that it is recognized as being empty. */
        pkt.data = NULL;
        pkt.size = 0;

        error = av_read_frame(m_ptAVFormatContext, &pkt);
        if (error < 0) {
            LOGE("Read packet failed (error '%d')\n", error);
            return error;
        }

        int flag = pkt.flags;
        s64 pts = pkt.pts;
        s64 duration = pkt.duration;

        av_packet_unref(&pkt);
        if (!(flag & AV_PKT_FLAG_KEY)) {
            LOGE("Seek&Read packet is not a key frame \n");
            return AVERROR_INVALIDDATA;
        }
        if ((pts > time) && (time >= duration)) {
            adjustSeekTimestamp(time - duration, out);
        } else {
            out = time;
        }
        return 0;
    }

    s16 CADec::getWeight() {
        return m_pTrack->getWeight();
    }

}

