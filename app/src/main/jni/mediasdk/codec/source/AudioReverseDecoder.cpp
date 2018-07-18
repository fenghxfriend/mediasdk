//
// Created by ASUS on 2018/6/1.
//

#include <filtercomplex.h>
#include <unistd.h>
#include "AudioReverseDecoder.h"

namespace paomiantv {

    static int
    open_codec_context(AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx,
                       enum AVMediaType type, char *file_path) {
        int ret, stream_index;
        AVStream *st;
        AVCodec *dec = NULL;

        ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
        if (ret < 0) {
            LOGE("Could not find %s stream in input file '%s'\n",
                 av_get_media_type_string(type), file_path);
            return ret;
        } else {
            stream_index = ret;
            st = fmt_ctx->streams[stream_index];

            /* find decoder for the stream */
            dec = avcodec_find_decoder(st->codecpar->codec_id);
            if (!dec) {
                LOGE("Failed to find %s codec\n",
                     av_get_media_type_string(type));
                return AVERROR(EINVAL);
            }

            /* Allocate a codec context for the decoder */
            *dec_ctx = avcodec_alloc_context3(dec);
            if (!*dec_ctx) {
                LOGE("Failed to allocate the %s codec context\n",
                     av_get_media_type_string(type));
                return AVERROR(ENOMEM);
            }

            /* Copy codec parameters from input stream to output codec context */
            if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
                LOGE("Failed to copy %s codec parameters to decoder context\n",
                     av_get_media_type_string(type));
                return ret;
            }

            if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0) {
                LOGE("Failed to open %s codec\n",
                     av_get_media_type_string(type));
                return ret;
            }
        }
        return 0;
    }

    AudioReverseDecoder::AudioReverseDecoder(CDemuxer *demuxer) :
            ReverseDecoder(demuxer),
            m_pAudioPreHandler(NULL),
            m_uSourceId(0),
            m_ptAVFormatContext(NULL),
            m_ptAVCodecContext(NULL),
            m_ptAVFrame(NULL),
            m_ptRemainderFrame(NULL),
            m_uRemainderSampleSize(0),
            m_pbyRemainderBuffer(NULL),
            m_uOneFrameSize(0),
            m_uRemainderBufferCapacity(0),
            m_uRemainderBufferSize(0),
            m_bIsInputFinished(FALSE),
            m_uCurrSampleId(0) {
        m_pQueue = new CSafeQueue<CAudio>();
    }

    AudioReverseDecoder::~AudioReverseDecoder() {
        stop();
        release();
        if (m_pQueue != NULL) {
            delete m_pQueue;
            m_pQueue = NULL;
        }
    }

    int AudioReverseDecoder::prepare(s8 *tempPath) {
        CAutoLock autoLock(m_pLock);
        int error;

        /** Open the input file to read from it. */
        if ((error = avformat_open_input(&m_ptAVFormatContext,
                                         m_pDemuxer->getSrc(),
                                         NULL,
                                         NULL)) < 0) {
            LOGE("Could not open input file '%s' (error '%d')\n", m_pDemuxer->getSrc(), error);
            m_ptAVFormatContext = NULL;
            return error;
        }

        /** Get information on the input file (number of streams etc.). */
        if ((error = avformat_find_stream_info(m_ptAVFormatContext, NULL)) < 0) {
            LOGE("Could not open find stream info (error '%d')\n", error);
            avformat_close_input(&m_ptAVFormatContext);
            return error;
        }

        // find the audio stream index

        if ((error = open_codec_context(&m_ptAVCodecContext,
                                        m_ptAVFormatContext,
                                        AVMEDIA_TYPE_AUDIO,
                                        m_pDemuxer->getSrc())) < 0) {
            LOGE("Could not open find codec (error '%d')\n", error);
            avformat_close_input(&m_ptAVFormatContext);
            return error;

        }

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

        if (!m_pDemuxer->parse()) {
            LOGE("parse mp4 file failed\n");
            error = AVERROR_EXTERNAL;
            avcodec_free_context(&m_ptAVCodecContext);
            avformat_close_input(&m_ptAVFormatContext);
            return error;
        }

        m_uCurrSampleId = m_pDemuxer->getASampleNum();

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
        m_pAudioPreHandler = CAFilterComplex::Create(EM_SAMPLE_RATE_44_1, AV_SAMPLE_FMT_S16,
                                                     AV_CH_LAYOUT_STEREO);
        if (m_pAudioPreHandler != NULL) {
            BOOL32 ret = FALSE;
            do {
                if (!m_pAudioPreHandler->addInput(
                        (EMSampleRate) m_ptAVCodecContext->sample_rate,
                        m_ptAVCodecContext->sample_fmt,
                        m_ptAVCodecContext->channel_layout,
                        1,
                        m_uSourceId)) {
                    break;
                }
                if (!m_pAudioPreHandler->addVolumeInSource(m_uSourceId, 1.0)) {
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
                                         AV_CH_LAYOUT_STEREO) *
                                 (0x0001 << AV_SAMPLE_FMT_S16));
        m_uRemainderBufferCapacity = m_uOneFrameSize * BLOCK_QUEUE_SIZE;
        m_pbyRemainderBuffer = (u8 *) malloc(m_uRemainderBufferCapacity);

        return 0;
    }

    void AudioReverseDecoder::release() {
        CAutoLock autoLock(m_pLock);
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

        if (m_pbyRemainderBuffer != NULL) {
            free(m_pbyRemainderBuffer);
            m_pbyRemainderBuffer = NULL;
        }

        if (m_pAudioPreHandler != NULL) {
            m_pAudioPreHandler->destroy();
            m_pAudioPreHandler = NULL;
        }

        m_uRemainderSampleSize = 0;
        m_uRemainderBufferCapacity = 0;
        m_uRemainderBufferSize = 0;
        m_uOneFrameSize = 0;
    }

    void AudioReverseDecoder::stop() {
        m_pQueue->disable();
        m_pQueue->clear();
        ReverseDecoder::stop();
        m_pQueue->enable();
    }

    void AudioReverseDecoder::start() {
        ReverseDecoder::start();
    }

    void AudioReverseDecoder::getAudio(CAudio *&pAudio) {
        pAudio = NULL;
        if (m_bIsFinished && m_pQueue->empty()) {
            return;
        }
        m_pQueue->pop(pAudio);
    }

    long AudioReverseDecoder::run() {
        m_pThread->setName("AudioReverseDecoder");
        LOGI("audio reverse decoder is started");
        m_pLock->lock();
        m_bIsStarted = TRUE;
        while (!m_bIsStopped) {
            if (!m_bIsStopped) {
                m_pLock->unlock();
                decode();
                m_pLock->lock();
            }
        }
        m_bIsStarted = FALSE;
        m_bIsStopped = FALSE;
        LOGI("audio reverse decoder is stopped");
        m_pLock->unlock();
        return 0;
    }

    int AudioReverseDecoder::decode_frame(AVFrame *&frame,
                                          AVCodecContext *input_codec_context,
                                          int *data_present, int *input_finish, int *finished) {
        int error = 0;
        if (!(*input_finish)) {
            /* Packet used for temporary storage. */
            AVPacket input_packet;

            av_init_packet(&input_packet);
            /** Set the packet data and size so that it is recognized as being empty. */
            input_packet.data = NULL;
            input_packet.size = 0;

            u64 renderoffset;
            u32 size;
            u64 starttime;
            u64 duration;

            /* Read one frame from the input file into a temporary packet. */
            if (m_uCurrSampleId > 0) {
                if (!m_pDemuxer->getAudioSampleById(m_uCurrSampleId, input_packet.data,
                                                   size,
                                                   starttime,
                                                   duration,
                                                   renderoffset,
                                                   input_packet.flags)) {
                    error = AVERROR_EXTERNAL;
                } else {
                    input_packet.size = size;
                    m_uCurrSampleId--;
                    error = 0;
                }

            } else {
                error = AVERROR_EOF;
            }
            /* If we are at the end of the file, flush the decoder below. */
            if (error == AVERROR_EOF) {
                *input_finish = 1;
            } else if (error != 0) {
                LOGE("Could not read frame (error '%d')\n", error);
                return error;
            }

            /* Send the audio frame stored in the temporary packet to the decoder.
             * The input audio stream decoder is used to do this. */
            if ((error = avcodec_send_packet(input_codec_context, &input_packet)) < 0) {
                LOGE("Could not send packet for decoding (error '%d')\n", error);
                av_packet_unref(&input_packet);
                return error;
            }
            av_packet_unref(&input_packet);
        }


        /* Receive one frame from the decoder. */
        error = avcodec_receive_frame(input_codec_context, frame);
        /* If the decoder asks for more data to be able to decode a frame,
         * return indicating that no data is present. */
        if (error == AVERROR(EAGAIN)) {
            error = 0;
            goto cleanup;
            /* If the end of the input file is reached, stop decoding. */
        } else if (error == AVERROR_EOF) {
            *finished = 1;
            error = 0;
            LOGE("decode EOS of stream!")
            goto cleanup;
        } else if (error < 0) {
            LOGE("Could not decode frame (error '%d')\n", error);
            goto cleanup;
            /* Default case: Return decoded data. */
        } else {
            *data_present = 1;
            goto cleanup;
        }

        cleanup:
        return error;
    }

    int AudioReverseDecoder::decode() {
        if (!m_bIsFinished) {
            decodeFrame();
        } else {
            usleep(5000);
        }
        return 0;
    }

    int AudioReverseDecoder::decodeFrame() {
        int ret = 0;
        int data_present = 0;
        int finished = 0;
        if ((ret = decode_frame(m_ptAVFrame, m_ptAVCodecContext, &data_present, &m_bIsInputFinished,
                                &finished)) != 0) {
            LOGE("Audio decode frame failed! \n");
            return ret;

        }
        /**
         * If we are at the end of the file and there are no more samples
         * in the decoder which are delayed, we are actually finished.
         * This must not be treated as an error.
         */
        if (finished && !data_present) {
            ret = 0;
            m_bIsFinished = TRUE;
            LOGE("Audio Input finished. Write NULL frame \n");
        } else if (data_present) {
            /** If there is decoded data, convert and store it */
            /* push the audio data from decoded frame into the filtergraph */
            reverseBuffer();
            processBuffer();
            writeBuffer();
        }

        av_frame_unref(m_ptAVFrame);

        return ret;
    }


    BOOL32 AudioReverseDecoder::processBuffer() {
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

    BOOL32 AudioReverseDecoder::writeBuffer() {
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
                audio->m_tParams.m_eFormat = AV_SAMPLE_FMT_S16;
                audio->m_tParams.m_ullChannelLayout = AV_CH_LAYOUT_STEREO;
                audio->m_tParams.m_eSampleRate = EM_SAMPLE_RATE_44_1;
                if (!m_pQueue->push(audio)) {
                    CAudio::release(audio);
                    LOGE("push audio failed!");
                }
            }

            if (count > 0) {
                memmove(m_pbyRemainderBuffer, m_pbyRemainderBuffer + outOffset, rmd);
            }
            m_uRemainderBufferSize = rmd;
            return TRUE;
        }
        return FALSE;
    }

    int AudioReverseDecoder::reverseBuffer() {
        int ret = 0, p, i, j;
        for (p = 0; p < m_ptAVFrame->channels; p++) {
            switch (m_ptAVFrame->format) {
                case AV_SAMPLE_FMT_U8P: {
                    uint8_t *dst = (uint8_t *) m_ptAVFrame->extended_data[p];
                    for (i = 0, j = m_ptAVFrame->nb_samples - 1; i < j; i++, j--)
                        FFSWAP(uint8_t, dst[i], dst[j]);
                }
                    break;
                case AV_SAMPLE_FMT_S16P: {
                    int16_t *dst = (int16_t *) m_ptAVFrame->extended_data[p];
                    for (i = 0, j = m_ptAVFrame->nb_samples - 1; i < j; i++, j--)
                        FFSWAP(int16_t, dst[i], dst[j]);
                }
                    break;
                case AV_SAMPLE_FMT_S32P: {
                    int32_t *dst = (int32_t *) m_ptAVFrame->extended_data[p];
                    for (i = 0, j = m_ptAVFrame->nb_samples - 1; i < j; i++, j--)
                        FFSWAP(int32_t, dst[i], dst[j]);
                }
                    break;
                case AV_SAMPLE_FMT_FLTP: {
                    float *dst = (float *) m_ptAVFrame->extended_data[p];
                    for (i = 0, j = m_ptAVFrame->nb_samples - 1; i < j; i++, j--)
                        FFSWAP(float, dst[i], dst[j]);
                }
                    break;
                case AV_SAMPLE_FMT_DBLP: {
                    double *dst = (double *) m_ptAVFrame->extended_data[p];
                    for (i = 0, j = m_ptAVFrame->nb_samples - 1; i < j; i++, j--)
                        FFSWAP(double, dst[i], dst[j]);
                }
                    break;
                default:
                    ret = AVERROR_EXTERNAL;
                    LOGE("do not support the pcm audio format!");
                    break;
            }
        }
        return ret;
    }
}
