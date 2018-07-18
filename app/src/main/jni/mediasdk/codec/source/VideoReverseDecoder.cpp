//
// Created by ASUS on 2018/6/1.
//


#include <sound.h>
#include <unistd.h>
#include "VideoReverseDecoder.h"
#include "../../io/include/demuxer.h"

#ifdef __cplusplus
extern "C"
{
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#endif
#define SUFFIX ".pmtv"

namespace paomiantv {

    std::vector<CVChunk *> CVChunk::m_svPool;
    CLock CVChunk::m_sLock;

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
            switch (st->codecpar->codec_id) {
#if HW_MEDIACODEC_ENABLE
                case AV_CODEC_ID_H264: {
                    dec = avcodec_find_decoder_by_name("h264_mediacodec");
                    break;
                }
                case AV_CODEC_ID_MPEG4: {
                    dec = avcodec_find_decoder_by_name("mpeg4_mediacodec");
                    break;
                }
#endif
                default:
                    dec = avcodec_find_decoder(st->codecpar->codec_id);
                    break;
            }
            if (!dec) {
                LOGE("find hardware codec failed, find a software codec!");
                dec = avcodec_find_decoder(st->codecpar->codec_id);
            }

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

    VideoReverseDecoder::VideoReverseDecoder(CDemuxer *demuxer) :
            ReverseDecoder(demuxer),
            m_ptAVFormatContext(NULL),
            m_ptAVCodecContext(NULL),
            m_ptAVFrame(NULL),
            m_ptImgConvertContext(NULL),
            m_uWidth(0),
            m_uHeight(0),
            m_ePixFmt(EM_PIXEL_FORMAT_START),
            m_eDecoderOutputFmt(AV_PIX_FMT_NONE),
            m_bIsInputFinished(FALSE),
            m_pchTempPath(NULL),
            m_uCurrChunkId(0),
            m_uCurrChunkStartSampleId(0),
            m_uCurrChunkSampleId(0),
            m_uCurrChunkSampleEndId(0),
            m_pLayerParams(NULL),
            m_pbyBuffer(NULL),
            m_uRenderCurrSampleId(0),
            m_bIsRenderReadFinished(FALSE),
            m_uRenderCurrChunk(0),
            m_uLayerBufferSize(0),
            m_pCacheChunk(NULL),
            m_pCurrChunk(NULL) {
        m_pChunkQueue = new CSafeQueue<CVChunk>();
    }

    VideoReverseDecoder::~VideoReverseDecoder() {
        stop();
        release();
        if (m_pChunkQueue != NULL) {
            delete m_pChunkQueue;
            m_pChunkQueue = NULL;
        }
        CVChunk::clear();
    }

    int VideoReverseDecoder::prepare(s8 *tempPath) {
        CAutoLock autoLock(m_pLock);
        m_pChunkQueue->clear();
        m_pChunkQueue->enable();
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
                                        AVMEDIA_TYPE_VIDEO,
                                        m_pDemuxer->getSrc())) < 0) {
            LOGE("Could not open find codec (error '%d')\n", error);
            avformat_close_input(&m_ptAVFormatContext);
            return error;

        }

        m_ptImgConvertContext = sws_getContext(m_ptAVCodecContext->width,
                                               m_ptAVCodecContext->height,
                                               m_ptAVCodecContext->pix_fmt,
                                               m_ptAVCodecContext->width,
                                               m_ptAVCodecContext->height,
                                               AV_PIX_FMT_YUV420P,
                                               0,
                                               NULL, NULL, NULL);
        if (!m_ptImgConvertContext) {
            LOGE("Impossible to create scale context for the conversion "
                         "fmt:%s -> fmt:%s\n",
                 av_get_pix_fmt_name(m_ptAVCodecContext->pix_fmt),
                 av_get_pix_fmt_name(AV_PIX_FMT_RGBA));
            avcodec_free_context(&m_ptAVCodecContext);
            avformat_close_input(&m_ptAVFormatContext);
            return AVERROR(EINVAL);
        }
        m_ePixFmt = EM_PIXEL_FORMAT_I420;

        /* allocate image where the decoded image will be put */
        m_uWidth = m_ptAVCodecContext->width;
        m_uHeight = m_ptAVCodecContext->height;
        m_eDecoderOutputFmt = m_ptAVCodecContext->pix_fmt;


        // init the frame
        m_ptAVFrame = av_frame_alloc();
        if (!m_ptAVFrame) {
            LOGE("Could not allocate frame\n");
            error = AVERROR(ENOMEM);
            avcodec_free_context(&m_ptAVCodecContext);
            avformat_close_input(&m_ptAVFormatContext);
            return error;
        }

        if (!m_pDemuxer->parse()) {
            LOGE("parse mp4 file failed\n");
            error = AVERROR_EXTERNAL;
            avcodec_free_context(&m_ptAVCodecContext);
            avformat_close_input(&m_ptAVFormatContext);
            return error;
        } else {
            u8 *data = NULL;
            u32 size = 0;
            u64 renderoffset;
            u64 starttime;
            u64 duration;
            BOOL32 flags;
            if (!m_pDemuxer->getVidoeSampleById(1, data,
                                                size,
                                                starttime,
                                                duration,
                                                renderoffset,
                                                flags)) {
                error = AVERROR_EXTERNAL;
                avcodec_free_context(&m_ptAVCodecContext);
                avformat_close_input(&m_ptAVFormatContext);
                return error;
            } else {
                m_sllFirstCTSUS = starttime + renderoffset;
            }
        }

        u32 tempSize = strlen(tempPath) + 1;
        m_pchTempPath = (s8 *) malloc(tempSize);
        strncpy(m_pchTempPath, tempPath, tempSize);


        CVChunk *ptChunk = CVChunk::create();
        s8 name[MAX_LEN_FILE_PATH] = {0};
        snprintf(name, MAX_LEN_FILE_PATH, "%s/%u%s", m_pchTempPath, m_uCurrChunkId, SUFFIX);
        u32 pathSize = strlen(name) + 1;
        ptChunk->m_pchPath = (s8 *) malloc(pathSize);
        strncpy(ptChunk->m_pchPath, name, pathSize);
        if ((ptChunk->m_pFile = fopen(ptChunk->m_pchPath, "wb")) == NULL) {
            LOGE("open %s failed", ptChunk->m_pchPath);
            avcodec_free_context(&m_ptAVCodecContext);
            avformat_close_input(&m_ptAVFormatContext);
            CVChunk::release(ptChunk);
            return AVERROR_EXTERNAL;
        }
        m_pCacheChunk = ptChunk;
        if (m_pDemuxer->getVSyncNum() - m_uCurrChunkId - 1 >= 0) {
            u32 *ids = m_pDemuxer->getVSyncIds();
            m_uCurrChunkSampleId = ids[m_pDemuxer->getVSyncNum() - m_uCurrChunkId - 1];
            m_uCurrChunkStartSampleId = m_uCurrChunkSampleId;
            m_uCurrChunkSampleEndId = m_pDemuxer->getVSampleNum();
        }

        return 0;
    }

    void VideoReverseDecoder::stop() {
        m_pChunkQueue->disable();
        m_pChunkQueue->clear();
        ReverseDecoder::stop();
        m_pChunkQueue->enable();
    }

    void VideoReverseDecoder::start() {
        m_pChunkQueue->enable();
        ReverseDecoder::start();
    }

    void VideoReverseDecoder::getLayer(CVLayerParam *&pLayer) {
        NextFile:
        pLayer = NULL;
        if (m_uRenderCurrSampleId <= m_pDemuxer->getVSampleNum() - 1) {
            if (m_pCurrChunk == NULL) {
                m_pChunkQueue->pop(m_pCurrChunk);
                if (m_pCurrChunk == NULL) {
                    LOGE("get video chunk file failed");
                    return;
                }

                if ((m_pCurrChunk->m_pFile = fopen(m_pCurrChunk->m_pchPath, "rb")) == NULL) {
                    LOGE("open %s failed", m_pCurrChunk->m_pchPath);
                    return;
                }
                if (fseek(m_pCurrChunk->m_pFile, 0, SEEK_END) != 0) {
                    LOGE("seek to video chunk file end failed");
                    return;
                }
            }

            if (m_pLayerParams != NULL) {
                CVLayerParam *layerParam = CVLayerParam::create();
                layerParam->resize(m_pLayerParams->m_uSize);
                layerParam->m_uHeight = m_pLayerParams->m_uHeight;
                layerParam->m_uWidth = m_pLayerParams->m_uWidth;
                layerParam->m_afUVCropScale[EM_DIRECT_X] = m_pLayerParams->m_afUVCropScale[EM_DIRECT_X];
                layerParam->m_afUVCropScale[EM_DIRECT_Y] = m_pLayerParams->m_afUVCropScale[EM_DIRECT_Y];
                layerParam->m_eFormat = m_pLayerParams->m_eFormat;
                layerParam->m_uSize = m_pLayerParams->m_uSize;
                if (fseek(m_pCurrChunk->m_pFile, -m_uLayerBufferSize, SEEK_CUR) != 0) {
                    CVLayerParam::release(layerParam);
                    CVChunk::release(m_pCurrChunk);
                    m_pCurrChunk = NULL;
                    goto NextFile;
                }
                fread(&(layerParam->m_sllTimeStampUS), sizeof(layerParam->m_sllTimeStampUS), 1,
                      m_pCurrChunk->m_pFile);
                fread(&(layerParam->m_sllDurationUS), sizeof(layerParam->m_sllDurationUS), 1,
                      m_pCurrChunk->m_pFile);
                fread(layerParam->m_pbyPicture, sizeof(u8), layerParam->m_uSize,
                      m_pCurrChunk->m_pFile);
                fseek(m_pCurrChunk->m_pFile, -m_uLayerBufferSize, SEEK_CUR);
                pLayer = layerParam;
                m_uRenderCurrSampleId++;
            }
        } else {
            if (m_pCurrChunk != NULL) {
                CVChunk::release(m_pCurrChunk);
                m_pCurrChunk = NULL;
            }
        }
    }

    long VideoReverseDecoder::run() {
        m_pThread->setName("VideoReverseDecoder");
        LOGI("video reverse decoder is started");
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
        LOGI("video reverse decoder is stopped");
        m_pLock->unlock();
        return 0;
    }

//    static void savefirsth264(u8 *buffer, u32 size) {
//        FILE *fp = fopen("/storage/emulated/0/raw_r.h264", "wb");
//        if (fp == NULL) {
//            LOGE("open raw.pcm failed");
//            return;
//        }
//        fwrite(buffer, 1, size, fp);
//        fflush(fp);
//        fclose(fp);
//    }

    int VideoReverseDecoder::decode_frame(AVFrame *&frame,
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
            u32 size = 0;
            u64 starttime;
            u64 duration;

            /* Read one frame from the input file into a temporary packet. */
            if (m_uCurrChunkSampleId <= m_uCurrChunkSampleEndId) {
                if (!m_pDemuxer->getVidoeSampleById(m_uCurrChunkSampleId, input_packet.data,
                                                    size,
                                                    starttime,
                                                    duration,
                                                    renderoffset,
                                                    input_packet.flags)) {
                    error = AVERROR_EXTERNAL;
                } else {
                    input_packet.size = size;
                    input_packet.pts = starttime + renderoffset;
                    input_packet.duration = duration;
                    input_packet.dts = starttime;
                    m_uCurrChunkSampleId++;
                    error = 0;

//                    // to do
//                    static bool save = false;
//                    if (!save) {
//                        savefirsth264(input_packet.data, input_packet.size);
//                        save = true;
//                    }
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
            fclose(m_pCacheChunk->m_pFile);
            m_pCacheChunk->m_pFile = NULL;
            if (!m_pChunkQueue->push(m_pCacheChunk)) {
                CVChunk::release(m_pCacheChunk);
                m_pCacheChunk = NULL;
                error = AVERROR_EXTERNAL;
                goto cleanup;
            }

            m_uCurrChunkId++;
            if (m_pDemuxer->getVSyncNum() - m_uCurrChunkId >= 1) {
                u32 *ids = m_pDemuxer->getVSyncIds();
                m_uCurrChunkSampleId = ids[m_pDemuxer->getVSyncNum() - m_uCurrChunkId - 1];
                m_uCurrChunkStartSampleId = m_uCurrChunkSampleId;
                m_uCurrChunkSampleEndId =
                        m_pDemuxer->getVSyncIds()[m_pDemuxer->getVSyncNum() - m_uCurrChunkId] - 1;

                // create a chunk
                CVChunk *ptChunk = CVChunk::create();
                s8 name[MAX_LEN_FILE_PATH] = {0};
                snprintf(name, MAX_LEN_FILE_PATH, "%s/%u%s", m_pchTempPath, m_uCurrChunkId,
                         SUFFIX);
                u32 pathSize = strlen(name) + 1;
                ptChunk->m_pchPath = (s8 *) malloc(pathSize);
                strncpy(ptChunk->m_pchPath, name, pathSize);
                if ((ptChunk->m_pFile = fopen(ptChunk->m_pchPath, "wb")) == NULL) {
                    LOGE("open %s failed", ptChunk->m_pchPath);
                    CVChunk::release(ptChunk);
                    error = AVERROR_EXTERNAL;
                    goto cleanup;
                }
                m_pCacheChunk = ptChunk;
                *input_finish = 0;
            }
            error = 0;
            LOGE("decode EOS of Chunk!")
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

    int VideoReverseDecoder::decode() {
        if (!m_bIsFinished) {
            decodeFrame();
        } else {
            usleep(5000);
        }
        return 0;
    }

    int VideoReverseDecoder::decodeFrame() {
        int ret = 0;
        int data_present = 0;
        int finished = 0;
        if ((ret = decode_frame(m_ptAVFrame, m_ptAVCodecContext, &data_present, &m_bIsInputFinished,
                                &finished))) {
            LOGE("Video  decode frame failed! \n");
            return ret;
        }
        do {
            /**
         * If we are at the end of the file and there are no more samples
         * in the decoder which are delayed, we are actually finished.
         * This must not be treated as an error.
         */
            if (finished && !data_present) {
                if (m_uCurrChunkId == m_pDemuxer->getVSyncNum()) {
                    m_bIsFinished = TRUE;
                    LOGE("Video decode finished.) \n");
                }
                avcodec_flush_buffers(m_ptAVCodecContext);
                LOGE("Video chunk%d  decode finished.) \n", m_uCurrChunkId - 1);
            } else if (data_present) {
                if (m_ptAVFrame->width != m_uWidth || m_ptAVFrame->height != m_uHeight ||
                    m_ptAVFrame->format != m_eDecoderOutputFmt) {
                    /* To handle this change, one could call av_image_alloc again and
                     * decode the following frames into another rawvideo file. */
                    LOGE("Video Error: Width, height and pixel format have to be "
                                 "constant in a rawvideo file, but the width, height or "
                                 "pixel format of the input video changed:\n"
                                 "old: width = %d, height = %d, format = %s\n"
                                 "new: width = %d, height = %d, format = %s\n",
                         m_uWidth, m_uHeight, av_get_pix_fmt_name(m_eDecoderOutputFmt),
                         m_ptAVFrame->width, m_ptAVFrame->height,
                         av_get_pix_fmt_name(static_cast<AVPixelFormat>(m_ptAVFrame->format)));
                    break;
                }

                int stride = m_ptAVFrame->linesize[0];
                int adjustHeight = ALIGN(m_ptAVFrame->height, DEFAULT_ALIGN_DATA);
                if (m_pLayerParams == NULL) {
                    u32 bufferSize = 0;
                    if (m_ePixFmt == EM_PIXEL_FORMAT_I420) {
                        bufferSize = stride * adjustHeight * 3 / 2;
                    } else {
                        bufferSize = stride * adjustHeight * 4;
                    }

                    m_pLayerParams = CVLayerParam::create();
                    m_pLayerParams->resize(0);
                    m_pLayerParams->m_uHeight = adjustHeight;
                    m_pLayerParams->m_uWidth = stride;
                    m_pLayerParams->m_afUVCropScale[EM_DIRECT_X] =
                            m_uWidth == m_pLayerParams->m_uWidth ? 1.0f : (
                                    m_uWidth * 1.0f / m_pLayerParams->m_uWidth - 0.01f);
                    m_pLayerParams->m_afUVCropScale[EM_DIRECT_Y] =
                            m_uHeight == m_pLayerParams->m_uHeight ? 1.0f : (
                                    m_uHeight * 1.0f / m_pLayerParams->m_uHeight - 0.01f);
                    m_pLayerParams->m_eFormat = m_ePixFmt;
                    m_pLayerParams->m_uSize = bufferSize;
                    m_uLayerBufferSize =
                            sizeof(m_ptAVFrame->pts) + sizeof(m_ptAVFrame->pkt_duration) +
                            bufferSize;
                }

                if (m_pbyBuffer == NULL && m_pLayerParams != NULL) {
                    m_pbyBuffer = (u8 *) malloc(m_pLayerParams->m_uSize);
                }

                if (adjustHeight != m_uHeight) {
                    u8 *dst_data[4];
                    int dst_linesize[4];

                    if (av_image_alloc(dst_data, dst_linesize, stride,
                                       adjustHeight, AV_PIX_FMT_YUV420P, 1) < 0) {
                        printf("alloc rgba buffer failed\n");
                        ret = -1;
                        break;
                    }
                    sws_scale(m_ptImgConvertContext, (const uint8_t *const *) m_ptAVFrame->data,
                              m_ptAVFrame->linesize,
                              0,
                              m_ptAVFrame->height, dst_data, dst_linesize);
                    av_image_copy_to_buffer(m_pbyBuffer, m_pLayerParams->m_uSize,
                                            (const uint8_t *const *) (dst_data),
                                            dst_linesize,
                                            AV_PIX_FMT_YUV420P,
                                            stride,
                                            adjustHeight,
                                            1);
                    av_freep(&dst_data[0]);
                } else {
                    av_image_copy_to_buffer(m_pbyBuffer, m_pLayerParams->m_uSize,
                                            (const uint8_t *const *) m_ptAVFrame->data,
                                            m_ptAVFrame->linesize,
                                            static_cast<AVPixelFormat>(m_ptAVFrame->format),
                                            m_ptAVFrame->linesize[0],
                                            m_ptAVFrame->height,
                                            1);
                }

                s64 pts = m_pDemuxer->getVRealDuration() -
                          (m_ptAVFrame->pts + m_ptAVFrame->pkt_duration - m_sllFirstCTSUS);
                s64 duration = m_ptAVFrame->pkt_duration;
                fwrite(&pts, sizeof(pts), 1, m_pCacheChunk->m_pFile);
                fwrite(&duration, sizeof(duration), 1, m_pCacheChunk->m_pFile);
                fwrite(m_pbyBuffer, sizeof(u8), m_pLayerParams->m_uSize, m_pCacheChunk->m_pFile);
            }

            av_frame_unref(m_ptAVFrame);
        } while (0);
        return ret;
    }

    void VideoReverseDecoder::release() {
        CAutoLock autoLock(m_pLock);

        if (m_pchTempPath != NULL) {
            free(m_pchTempPath);
            m_pchTempPath = NULL;
        }

        if (m_pbyBuffer != NULL) {
            free(m_pbyBuffer);
            m_pbyBuffer = NULL;
        }

        if (m_pLayerParams != NULL) {
            CVLayerParam::release(m_pLayerParams);
            m_pLayerParams = NULL;
        }

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
        if (m_ptImgConvertContext != NULL) {
            sws_freeContext(m_ptImgConvertContext);
            m_ptImgConvertContext = NULL;
        }

        if (m_pCacheChunk != NULL) {
            CVChunk::release(m_pCacheChunk);
            m_pCacheChunk = NULL;
        }

        if (m_pCurrChunk != NULL) {
            CVChunk::release(m_pCurrChunk);
            m_pCurrChunk = NULL;
        }
    }
}
