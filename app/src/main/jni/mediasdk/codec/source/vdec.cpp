/*******************************************************************************
 *        Module: mediasdk
 *          File:
 * Functionality: video decoder
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


#include <vdec.h>
#include <image.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}
#endif

namespace paomiantv {

    CVDec::CVDec(ITrack *const &pTrack) : CDec(pTrack),
                                          m_bIsStopped(FALSE),
                                          m_bIsStarted(FALSE),
                                          m_bIsPaused(FALSE),
                                          m_ptAVFormatContext(NULL),
                                          m_ptAVCodecContext(NULL),
                                          m_ptAVFrame(NULL),
                                          m_ptImgConvertContext(NULL),
                                          m_pLastLayer(NULL),
                                          m_pCacheLayer(NULL),
                                          m_uWidth(0),
                                          m_uHeight(0),
                                          m_ePixFmt(EM_PIXEL_FORMAT_START),
                                          m_eDecoderOutputFmt(
                                                  AV_PIX_FMT_NONE),
//                                          m_uBufferSize(0),
                                          m_sllCurrPlayUS(0),
                                          m_sllStartPlayUS(0),
                                          m_sllEndCutUS(0),
                                          m_sllLastPTSUS(0) {
        USE_LOG;

        m_pQueue = new CSafeQueue<CVLayerParam>();
        m_pLock = new CLock;
        m_pThread = new CThread(ThreadWrapper, this);
    }


    CVDec::~CVDec() {
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

        if (m_pLock != NULL) {
            delete m_pLock;
            m_pLock = NULL;
        }


    }

    void CVDec::start() {
        LOGI("CVDec::startThread");
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

    void CVDec::stop() {
        LOGI("CVDec::stopThread");
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


//static
    void *CVDec::ThreadWrapper(void *pThis) {
        CVDec *p = (CVDec *) pThis;
        int nErr = p->ThreadEntry();
        return (void *) nErr;
    }

    int CVDec::ThreadEntry() {
        char name[64];
        memset(name, 0, sizeof(name));
        sprintf(name, "CVDecTrack%u", m_pTrack->getId());
        m_pThread->setName(name);
        int nErr = 0;
        m_pLock->lock();
        LOGI("track%u video thread is started", m_pTrack->getId());
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
        LOGI("track%u video thread is stopped", m_pTrack->getId());
        m_pLock->unlock();

        return 0;
    }

    static FILE *fp = NULL;
    static bool saved = false;

    static void saveYUV(u8 *in, u32 size) {
        if (saved) {
            return;
        }
        if (fp == NULL) {
            if ((fp = fopen("/storage/emulated/0/raw.yuv", "wb")) == NULL) {
                LOGE("open raw.pcm failed");
            }
        }
        if (fp != NULL) {
            fwrite(in, 1, size, fp);
        }
        saved = true;

    }

    int CVDec::decodeFrame() {
        int ret = 0;
        int data_present = 0;
        int finished = 0;
        if ((ret = decode_frame(m_ptAVFrame, m_ptAVFormatContext, m_ptAVCodecContext,
                                &data_present, &m_bIsInputFinished,
                                &finished, m_nStreamIndex))) {
            LOGE("Video track%d decode frame failed! \n", m_pTrack->getId());
            return ret;
        }
        do {
            /**
         * If we are at the end of the file and there are no more samples
         * in the decoder which are delayed, we are actually finished.
         * This must not be treated as an error.
         */
            s64 pts = 0;
            if (data_present) {
                pts = av_rescale_q(m_ptAVFrame->pts,
                                   m_ptAVFormatContext->streams[m_nStreamIndex]->time_base,
                                   AV_TIME_BASE_Q);
            }
            if (finished && !data_present || pts > m_sllEndCutUS) {
                if (m_pCacheLayer != NULL) {
                    m_pCacheLayer->m_sllDurationUS = m_sllEndCutUS - m_sllLastPTSUS;
                    LOGI("Push last layer to queue in TRACK%d, layer: timestamp = %lld, duration = %lld \n",
                         m_pTrack->getId(), m_pCacheLayer->m_sllTimeStampUS,
                         m_pCacheLayer->m_sllDurationUS);
                    if (!m_pQueue->push(m_pCacheLayer)) {
                        CVLayerParam::release(m_pCacheLayer);
                    }
                    m_pCacheLayer = NULL;
                }
                m_bIsFinished = TRUE;
                LOGE("Video track%d decode finished. :) \n", m_pTrack->getId());
            } else if (data_present && pts >= (((CTrack *) m_pTrack)->getCutStart() * 1000) &&
                       ((m_pCacheLayer == NULL) ? true : ((pts - m_sllLastPTSUS) >=
                                                          VIDEO_FRAME_MIN_DURATION))) {
                if (m_ptAVFrame->width != m_uWidth || m_ptAVFrame->height != m_uHeight ||
                    m_ptAVFrame->format != m_eDecoderOutputFmt) {
                    /* To handle this change, one could call av_image_alloc again and
                     * decode the following frames into another rawvideo file. */
                    LOGE("Video track%d Error: Width, height and pixel format have to be "
                                 "constant in a rawvideo file, but the width, height or "
                                 "pixel format of the input video changed:\n"
                                 "old: width = %d, height = %d, format = %s\n"
                                 "new: width = %d, height = %d, format = %s\n", m_pTrack->getId(),
                         m_uWidth, m_uHeight, av_get_pix_fmt_name(m_eDecoderOutputFmt),
                         m_ptAVFrame->width, m_ptAVFrame->height,
                         av_get_pix_fmt_name(static_cast<AVPixelFormat>(m_ptAVFrame->format)));
                    break;
                }

//            LOGI("Video track%d Frame: width = %d, height = %d, format = %s, pts = %lld, isKey = %d\n",
//                 m_pTrack->getId(),
//                 m_ptAVFrame->width,
//                 m_ptAVFrame->height,
//                 av_get_pix_fmt_name((AVPixelFormat) m_ptAVFrame->format),
//                 pts,
//                 m_ptAVFrame->key_frame);
                int stride = m_ptAVFrame->linesize[0];
                int adjustHeight = ALIGN(m_ptAVFrame->height, DEFAULT_ALIGN_DATA);
                int bufferSize = 0;
                if (m_ePixFmt == EM_PIXEL_FORMAT_I420) {
                    bufferSize = stride * adjustHeight * 3 / 2;
                } else {
                    bufferSize = stride * adjustHeight * 4;
                }

                if (m_pCacheLayer == NULL) {
                    m_pCacheLayer = CVLayerParam::create();
                    m_pCacheLayer->resize(bufferSize);
                    memset(m_pCacheLayer->m_pbyPicture, 0, bufferSize);
                    m_pCacheLayer->m_sllTimeStampUS = m_sllStartPlayUS;
                } else {
                    m_pCacheLayer->m_sllDurationUS = pts - m_sllLastPTSUS;
                    s64 nextPTS = m_pCacheLayer->m_sllTimeStampUS + m_pCacheLayer->m_sllDurationUS;
//                    LOGI("Push a layer to queue in TRACK%d, layer: timestamp = %lld, duration = %lld \n",
//                         m_pTrack->getId(), m_pCacheLayer->m_sllTimeStampUS,
//                         m_pCacheLayer->m_sllDurationUS);
                    if (!m_pQueue->push(m_pCacheLayer)) {
                        CVLayerParam::release(m_pCacheLayer);
                    }

                    m_pCacheLayer = CVLayerParam::create();
                    m_pCacheLayer->resize(bufferSize);
                    memset(m_pCacheLayer->m_pbyPicture, 0, bufferSize);
                    m_pCacheLayer->m_sllTimeStampUS = nextPTS;

                }
                m_pCacheLayer->m_uSize = bufferSize;
                m_pCacheLayer->m_uTrackId = m_pTrack->getId();
                m_pCacheLayer->m_eFormat = m_ePixFmt;
                m_pCacheLayer->m_uHeight = adjustHeight;
                m_pCacheLayer->m_uWidth = stride;
                if (adjustHeight != m_uHeight) {
                    u8 *dst_data[4];
                    int dst_linesize[4];

                    if (av_image_alloc(dst_data, dst_linesize, stride,
                                       adjustHeight, AV_PIX_FMT_YUV420P, 1) < 0) {
                        printf("alloc rgba buffer failed\n");
                        ret = -1;
                        break;
                    }
                    sws_scale(m_ptImgConvertContext,
                              (const uint8_t *const *) m_ptAVFrame->data,
                              m_ptAVFrame->linesize,
                              0,
                              m_ptAVFrame->height, dst_data, dst_linesize);
                    av_image_copy_to_buffer(m_pCacheLayer->m_pbyPicture, m_pCacheLayer->m_uSize,
                                            (const uint8_t *const *) dst_data,
                                            dst_linesize,
                                            AV_PIX_FMT_YUV420P,
                                            stride,
                                            adjustHeight,
                                            1);
                    av_freep(&dst_data[0]);
                } else {
                    av_image_copy_to_buffer(m_pCacheLayer->m_pbyPicture, m_pCacheLayer->m_uSize,
                                            (const uint8_t *const *) m_ptAVFrame->data,
                                            m_ptAVFrame->linesize,
                                            static_cast<AVPixelFormat>(m_ptAVFrame->format),
                                            stride,
                                            adjustHeight,
                                            1);
                }

//                saveYUV(m_pCacheLayer->m_pbyPicture, m_pCacheLayer->m_uSize);
                float cropScaleX = m_uWidth == m_pCacheLayer->m_uWidth ? 1.0f : (
                        m_uWidth * 1.0f / m_pCacheLayer->m_uWidth - 0.01f);
                float cropScaleY = m_uHeight == m_pCacheLayer->m_uHeight ? 1.0f : (
                        m_uHeight * 1.0f / m_pCacheLayer->m_uHeight - 0.01f);
                s32 asize = ((CTrack *) m_pTrack)->getAnimationCount();
                for (int i = 0; i < asize; i++) {
                    CAnimation *animation = ((CTrack *) m_pTrack)->getAnimation(i);
                    s64 offset = 0;
                    if (animation != NULL &&
                        (offset = (pts -
                                   ((((CTrack *) m_pTrack)->getCutStart() + animation->m_sllStart) *
                                    1000))) >=
                        0 && ((animation->m_sllDuration < 0) ? TRUE : (pts <=
                                                                       ((((CTrack *) m_pTrack)->getCutStart() +
                                                                         animation->m_sllStart +
                                                                         animation->m_sllDuration) *
                                                                        1000)))) {
                        offset /= 1000;
                        // transform params
                        m_pCacheLayer->m_afTranslate[EM_DIRECT_X] =
                                animation->getVecX() * offset + animation->m_fStartTransX;
                        m_pCacheLayer->m_afTranslate[EM_DIRECT_Y] =
                                animation->getVecY() * offset + animation->m_fStartTransY;
                        m_pCacheLayer->m_afTranslate[EM_DIRECT_Z] =
                                animation->getVecZ() * offset + animation->m_fStartTransZ;

                        m_pCacheLayer->m_afRotate[EM_DIRECT_X] =
                                animation->getVecDegreeX() * offset + animation->m_fStartDegreeX;
                        m_pCacheLayer->m_afRotate[EM_DIRECT_Y] =
                                animation->getVecDegreeY() * offset + animation->m_fStartDegreeY;
                        m_pCacheLayer->m_afRotate[EM_DIRECT_Z] =
                                animation->getVecDegreeZ() * offset + animation->m_fStartDegreeZ;

                        m_pCacheLayer->m_afScale[EM_DIRECT_X] =
                                animation->getVecScaleX() * offset + animation->m_fStartScaleX;
                        m_pCacheLayer->m_afScale[EM_DIRECT_Y] =
                                animation->getVecScaleY() * offset + animation->m_fStartScaleY;
                        m_pCacheLayer->m_afScale[EM_DIRECT_Z] =
                                animation->getVecScaleZ() * offset + animation->m_fStartScaleZ;

                        // crop params
                        m_pCacheLayer->m_afUVCropTranslate[EM_DIRECT_X] =
                                animation->getCropVecX() * offset + animation->m_fCropStartTransX;
                        m_pCacheLayer->m_afUVCropTranslate[EM_DIRECT_Y] =
                                animation->getCropVecY() * offset + animation->m_fCropStartTransY;
                        m_pCacheLayer->m_afUVCropTranslate[EM_DIRECT_Z] =
                                animation->getCropVecZ() * offset + animation->m_fCropStartTransZ;

                        m_pCacheLayer->m_afUVCropRotate[EM_DIRECT_X] =
                                animation->getCropVecDegreeX() * offset +
                                animation->m_fCropStartDegreeX;
                        m_pCacheLayer->m_afUVCropRotate[EM_DIRECT_Y] =
                                animation->getCropVecDegreeY() * offset +
                                animation->m_fCropStartDegreeY;
                        m_pCacheLayer->m_afUVCropRotate[EM_DIRECT_Z] =
                                animation->getCropVecDegreeZ() * offset +
                                animation->m_fCropStartDegreeZ;

                        m_pCacheLayer->m_afUVCropScale[EM_DIRECT_X] =
                                animation->getCropVecScaleX() * offset +
                                animation->m_fCropStartScaleX;
                        m_pCacheLayer->m_afUVCropScale[EM_DIRECT_Y] =
                                animation->getCropVecScaleY() * offset +
                                animation->m_fCropStartScaleY;
                        m_pCacheLayer->m_afUVCropScale[EM_DIRECT_Z] =
                                animation->getCropVecScaleZ() * offset +
                                animation->m_fCropStartScaleZ;

                        // color alpha
                        m_pCacheLayer->m_fAlpha =
                                animation->getVecAlpha() * offset + animation->m_fStartAlpha;

                    }
                }
                m_pCacheLayer->m_afUVCropScale[EM_DIRECT_X] *= cropScaleX;
                m_pCacheLayer->m_afUVCropScale[EM_DIRECT_Y] *= cropScaleY;

                s32 esize = ((CTrack *) m_pTrack)->getEffectCount();
                for (s32 i = 0; i < esize; i++) {
                    CEffect *effect = ((CTrack *) m_pTrack)->getEffect(i);
                    if (effect != NULL &&
                        (pts >=
                         ((((CTrack *) m_pTrack)->getCutStart() + effect->getStart()) * 1000)) &&
                        ((effect->getDuration() < 0) ? TRUE : (pts <=
                                                               ((((CTrack *) m_pTrack)->getCutStart() +
                                                                 effect->getStart() +
                                                                 effect->getDuration()) *
                                                                1000)))) {
                        CVFilterParam *filterParam = CVFilterParam::create();
                        filterParam->reset();
                        effect->getFilter(filterParam->m_pFilterSource, filterParam->m_eType);
                        if (filterParam->m_pFilterSource != NULL) {
                            m_pCacheLayer->m_vFilterParams.push_back(filterParam);
                        } else {
                            CVFilterParam::release(filterParam);
                        }
                    }
                }
                m_sllLastPTSUS = pts;
            }

            av_frame_unref(m_ptAVFrame);
        } while (0);
        if (ret < 0) {
            CVLayerParam::release(m_pCacheLayer);
            m_pCacheLayer = NULL;
        }
        return ret;
    }

    void CVDec::pause() {
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted && !m_bIsStopped && !m_bIsPaused) {
            m_bIsPaused = TRUE;
        }
    }

    void CVDec::resume() {
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted && !m_bIsStopped && m_bIsPaused) {
            m_bIsPaused = FALSE;
            m_pLock->acttive();
        }
    }

    int CVDec::prepare() {
        m_pQueue->clear();
        m_pQueue->enable();
        CAutoLock autoLock(m_pLock);
        int error;
        uint8_t *data = nullptr;
        uint32_t size = 0;
        if (m_pTrack != NULL &&
            (m_pTrack->getSourceData(data, size), (data != NULL && size != 0)) &&
            m_pTrack->getSourceType() == EM_SOURCE_FILE) {

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
                                            AVMEDIA_TYPE_VIDEO,
                                            (s8 *) data)) < 0) {
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

//            if (m_ePixFmt == EM_PIXEL_FORMAT_RGBA_8888) {
//                error = av_image_get_buffer_size(AV_PIX_FMT_RGBA,
//                                                 ALIGN(m_uWidth, DEFAULT_ALIGN_DATA),
//                                                 ALIGN(m_uHeight, DEFAULT_ALIGN_DATA),
//                                                 DEFAULT_ALIGN_DATA);
//            } else {
//                error = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
//                                                 ALIGN(m_uWidth, DEFAULT_ALIGN_DATA),
//                                                 ALIGN(m_uHeight, DEFAULT_ALIGN_DATA),
//                                                 DEFAULT_ALIGN_DATA);
//            }
//
//            if (error < 0) {
//                LOGE("Get raw video buffer size failed\n");
//                avcodec_free_context(&m_ptAVCodecContext);
//                avformat_close_input(&m_ptAVFormatContext);
//                return error;
//            } else {
//                m_uBufferSize = error;
//            }

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
            if (!m_ptAVFrame) {
                LOGE("Could not allocate frame\n");
                error = AVERROR(ENOMEM);
                avcodec_free_context(&m_ptAVCodecContext);
                avformat_close_input(&m_ptAVFormatContext);
                return error;
            }
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

    void CVDec::release() {
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

    }

    void CVDec::findLayer(const s64 pts, CVLayerParam *&layer) {
        layer = NULL;
        if (m_pQueue->empty() && m_bIsFinished) {
            return;
        }
        m_pQueue->pop(layer);
        if (layer == NULL) {
            return;
        }
        if (layer->m_sllTimeStampUS <= pts &&
            pts < (layer->m_sllTimeStampUS + layer->m_sllDurationUS)) {
            return;
        } else {
            CVLayerParam::release(layer);
            findLayer(pts, layer);
        }
    }

    BOOL32 CVDec::getLayer(CVLayerParam *&pLayer) {
        pLayer = NULL;
        if (m_bIsFinished) {
            if (!m_pQueue->empty()) {
                if (m_pLastLayer == NULL) {
                    m_pQueue->pop(m_pLastLayer);
                    if (m_sllCurrPlayUS < m_pLastLayer->m_sllTimeStampUS) {
                        if (m_pTrack->isShowFirstFrame()) {
                            pLayer = m_pLastLayer->clone();
                            LOGD("track%u: show fist frame when time is not reached!!!!!!!!!\n",
                                 m_pTrack->getId());
                        } else {
                            LOGD("track%u: show NULL when time is not reached!!!!!!!!!\n",
                                 m_pTrack->getId());
                        }
                    } else if (m_pLastLayer->m_sllTimeStampUS <= m_sllCurrPlayUS &&
                               m_sllCurrPlayUS <
                               (m_pLastLayer->m_sllTimeStampUS +
                                m_pLastLayer->m_sllDurationUS)) {
                        pLayer = m_pLastLayer->clone();
                    } else {
                        CVLayerParam *tmp = NULL;
                        findLayer(m_sllCurrPlayUS, tmp);
                        if (tmp == NULL) {
                            if (m_bIsFinished) {
                                if (m_pTrack->isShowLastFrame()) {
                                    pLayer = m_pLastLayer->clone();
                                    LOGD("track%u: show last frame when reach end!!!!!!!!!\n",
                                         m_pTrack->getId());
                                } else {
                                    LOGD("track%u: show NULL when reach end!!!!!!!!!\n",
                                         m_pTrack->getId());
                                }
                            } else {
                                LOGE("track%u: this situation will never happened\n",
                                     m_pTrack->getId());
                            }
                        } else {
                            CVLayerParam::release(m_pLastLayer);
                            m_pLastLayer = tmp;
                            pLayer = m_pLastLayer->clone();
                        }
                    }
                } else {
                    if (m_sllCurrPlayUS < m_pLastLayer->m_sllTimeStampUS) {
                        if (m_pTrack->isShowFirstFrame()) {
                            pLayer = m_pLastLayer->clone();
                            LOGD("track%u: show fist frame when time is not reached!!!!!!!!!\n",
                                 m_pTrack->getId());
                        } else {
                            LOGD("track%u: show NULL when time is not reached!!!!!!!!!\n",
                                 m_pTrack->getId());
                        }
                    } else if (m_pLastLayer->m_sllTimeStampUS <= m_sllCurrPlayUS &&
                               m_sllCurrPlayUS <
                               (m_pLastLayer->m_sllTimeStampUS +
                                m_pLastLayer->m_sllDurationUS)) {
                        pLayer = m_pLastLayer->clone();
                    } else {
                        CVLayerParam *tmp = NULL;
                        findLayer(m_sllCurrPlayUS, tmp);
                        if (tmp == NULL) {
                            if (m_bIsFinished) {
                                if (m_pTrack->isShowLastFrame()) {
                                    pLayer = m_pLastLayer->clone();
                                    LOGD("track%u: show last frame when reach end!!!!!!!!!\n",
                                         m_pTrack->getId());
                                } else {
//                                    LOGD("track%u: show NULL when reach end!!!!!!!!!\n",
//                                         m_pTrack->getId());
                                }
                            } else {
                                LOGE("track%u: this situation will never happened\n",
                                     m_pTrack->getId());
                            }
                        } else {
                            CVLayerParam::release(m_pLastLayer);
                            m_pLastLayer = tmp;
                            pLayer = m_pLastLayer->clone();
                        }
                    }
                }
            } else {
                if (m_pLastLayer != NULL) {
                    if (m_pLastLayer->m_sllTimeStampUS <= m_sllCurrPlayUS &&
                        m_sllCurrPlayUS <
                        (m_pLastLayer->m_sllTimeStampUS +
                         m_pLastLayer->m_sllDurationUS)) {
                        pLayer = m_pLastLayer->clone();
                    } else if (m_sllCurrPlayUS >=
                               (m_pLastLayer->m_sllTimeStampUS +
                                m_pLastLayer->m_sllDurationUS)) {
                        if (m_pTrack->isShowLastFrame()) {
                            pLayer = m_pLastLayer->clone();
                            LOGD("track%u: show last frame when reach end!!!!!!!!!\n",
                                 m_pTrack->getId());
                        } else {
//                            LOGD("track%u: show NULL when reach end!!!!!!!!!\n", m_pTrack->getId());
                        }
                    }
                }
            }
        } else {
            if (!m_pQueue->empty()) {
                if (m_pLastLayer == NULL) {
                    m_pQueue->pop(m_pLastLayer);
                    if (m_sllCurrPlayUS < m_pLastLayer->m_sllTimeStampUS) {
                        if (m_pTrack->isShowFirstFrame()) {
                            pLayer = m_pLastLayer->clone();
                            LOGD("track%u: show fist frame when time is not reached!!!!!!!!!\n",
                                 m_pTrack->getId());
                        } else {
                            LOGD("track%u: show NULL when time is not reached!!!!!!!!!\n",
                                 m_pTrack->getId());
                        }
                    } else if (m_pLastLayer->m_sllTimeStampUS <= m_sllCurrPlayUS &&
                               m_sllCurrPlayUS <
                               (m_pLastLayer->m_sllTimeStampUS +
                                m_pLastLayer->m_sllDurationUS)) {
                        pLayer = m_pLastLayer->clone();
                    } else {
                        CVLayerParam *tmp = NULL;
                        findLayer(m_sllCurrPlayUS, tmp);
                        if (tmp == NULL) {
                            if (m_bIsFinished) {
                                if (m_pTrack->isShowLastFrame()) {
                                    pLayer = m_pLastLayer->clone();
                                    LOGD("track%u: show last frame when reach end!!!!!!!!!\n",
                                         m_pTrack->getId());
                                } else {
                                    LOGD("track%u: show NULL when reach end!!!!!!!!!\n",
                                         m_pTrack->getId());
                                }
                            } else {
                                LOGE("track%u: this situation will never happened\n",
                                     m_pTrack->getId());
                            }
                        } else {
                            CVLayerParam::release(m_pLastLayer);
                            m_pLastLayer = tmp;
                            pLayer = m_pLastLayer->clone();
                        }
                    }
                } else {
                    if (m_sllCurrPlayUS < m_pLastLayer->m_sllTimeStampUS) {
                        if (m_pTrack->isShowFirstFrame()) {
                            pLayer = m_pLastLayer->clone();
                            LOGD("track%u: show fist frame when time is not reached!!!!!!!!!\n",
                                 m_pTrack->getId());
                        } else {
                            LOGD("track%u: show NULL when time is not reached!!!!!!!!!\n",
                                 m_pTrack->getId());
                        }
                    } else if (m_pLastLayer->m_sllTimeStampUS <= m_sllCurrPlayUS &&
                               m_sllCurrPlayUS <
                               (m_pLastLayer->m_sllTimeStampUS +
                                m_pLastLayer->m_sllDurationUS)) {
                        pLayer = m_pLastLayer->clone();
                    } else {
                        CVLayerParam *tmp = NULL;
                        findLayer(m_sllCurrPlayUS, tmp);
                        if (tmp == NULL) {
                            if (m_bIsFinished) {
                                if (m_pTrack->isShowLastFrame()) {
                                    pLayer = m_pLastLayer->clone();
                                    LOGD("track%u: show last frame when reach end!!!!!!!!!\n",
                                         m_pTrack->getId());
                                } else {
                                    LOGD("track%u: show NULL when reach end!!!!!!!!!\n",
                                         m_pTrack->getId());
                                }
                            } else {
                                LOGE("track%u: this situation will never happened\n",
                                     m_pTrack->getId());
                            }
                        } else {
                            CVLayerParam::release(m_pLastLayer);
                            m_pLastLayer = tmp;
                            pLayer = m_pLastLayer->clone();
                        }
                    }
                }
            } else {
                if (m_pLastLayer == NULL) {
                    findLayer(m_sllCurrPlayUS, m_pLastLayer);
                    if (m_pLastLayer == NULL) {
                        LOGE("track%u: this will never happened\n", m_pTrack->getId());
                    }
                    if (m_sllCurrPlayUS < m_pLastLayer->m_sllTimeStampUS) {
                        if (m_pTrack->isShowFirstFrame()) {
                            pLayer = m_pLastLayer->clone();
                            LOGD("track%u: show fist frame when time is not reached!!!!!!!!!\n",
                                 m_pTrack->getId());
                        } else {
                            LOGD("track%u: show NULL when time is not reached!!!!!!!!!\n",
                                 m_pTrack->getId());
                        }
                    } else if (m_pLastLayer->m_sllTimeStampUS <= m_sllCurrPlayUS &&
                               m_sllCurrPlayUS <
                               (m_pLastLayer->m_sllTimeStampUS +
                                m_pLastLayer->m_sllDurationUS)) {
                        pLayer = m_pLastLayer->clone();
                    } else {
                        CVLayerParam *tmp = NULL;
                        findLayer(m_sllCurrPlayUS, tmp);
                        if (tmp == NULL) {
                            if (m_bIsFinished) {
                                if (m_pTrack->isShowLastFrame()) {
                                    pLayer = m_pLastLayer->clone();
                                    LOGD("track%u: show last frame when reach end!!!!!!!!!\n",
                                         m_pTrack->getId());
                                } else {
                                    LOGD("track%u: show NULL when reach end!!!!!!!!!\n",
                                         m_pTrack->getId());
                                }
                            } else {
                                LOGE("track%u: this situation will never happened\n",
                                     m_pTrack->getId());
                            }
                        } else {
                            CVLayerParam::release(m_pLastLayer);
                            m_pLastLayer = tmp;
                            pLayer = m_pLastLayer->clone();
                        }
                    }
                } else {
                    if (m_sllCurrPlayUS < m_pLastLayer->m_sllTimeStampUS) {
                        if (m_pTrack->isShowFirstFrame()) {
                            pLayer = m_pLastLayer->clone();
                            LOGD("track%u: show fist frame when time is not reached!!!!!!!!!\n",
                                 m_pTrack->getId());
                        } else {
                            LOGD("track%u: show NULL when time is not reached!!!!!!!!!\n",
                                 m_pTrack->getId());
                        }
                    } else if (m_pLastLayer->m_sllTimeStampUS <= m_sllCurrPlayUS &&
                               m_sllCurrPlayUS <
                               (m_pLastLayer->m_sllTimeStampUS +
                                m_pLastLayer->m_sllDurationUS)) {
                        pLayer = m_pLastLayer->clone();
                    } else {
                        CVLayerParam *tmp = NULL;
                        findLayer(m_sllCurrPlayUS, tmp);
                        if (tmp == NULL) {
                            if (m_bIsFinished) {
                                if (m_pTrack->isShowLastFrame()) {
                                    pLayer = m_pLastLayer->clone();
                                    LOGD("track%u: show last frame when reach end!!!!!!!!!\n",
                                         m_pTrack->getId());
                                } else {
                                    LOGD("track%u: show NULL when reach end!!!!!!!!!\n",
                                         m_pTrack->getId());
                                }
                            } else {
                                LOGE("track%u: this situation will never happened\n",
                                     m_pTrack->getId());
                            }
                        } else {
                            CVLayerParam::release(m_pLastLayer);
                            m_pLastLayer = tmp;
                            pLayer = m_pLastLayer->clone();
                        }
                    }
                }
            }
        }

//
//        if ((!m_pQueue->empty() || !m_bIsFinished) &&
//            m_sllCurrPlayUS >= m_sllStartPlayUS &&
//            (m_pTrack->isIndependent() ? TRUE : (m_sllEndPlayUS < 0 ? TRUE : m_sllCurrPlayUS <=
//                                                                             m_sllEndPlayUS))) {
//            while (!m_bIsFinished && m_pLastLayer == NULL) {
//                m_pQueue->try_pop(m_pLastLayer);
//                usleep(2000);
//            }
//            if (m_pLastLayer->m_sllTimeStampUS <= m_sllCurrPlayUS && m_sllCurrPlayUS <
//                                                                     (m_pLastLayer->m_sllTimeStampUS +
//                                                                      m_pLastLayer->m_sllDurationUS)) {
//                pLayer = m_pLastLayer->clone();
//            } else {
//                CVLayerParam *tmp = NULL;
//                findLayer(m_sllCurrPlayUS, tmp);
//                if (tmp != NULL) {
//                    CVLayerParam::release(m_pLastLayer);
//                    m_pLastLayer = tmp;
//                }
//                pLayer = m_pLastLayer->clone();
//            }
//        }
//        if (pLayer == NULL) {
//            LOGE("Get NULL layer in TRACK%d\n ", m_pTrack->getId());
//        } else {
//            LOGI("Get a layer in TRACK%d, layer: timestamp = %lld, duration = %lld \n",
//                 m_pTrack->getId(), pLayer->m_sllTimeStampUS, pLayer->m_sllDurationUS);
//        }
        m_sllCurrPlayUS += VIDEO_FRAME_MIN_DURATION;
        return TRUE;
    }

    int CVDec::decode() {
        if (!m_bIsFinished) {
            decodeFrame();
        } else {
            usleep(5000);
        }
        return 0;
    }

    int CVDec::adjustSeekTimestamp(s64 time, s64 &out) {
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

    BOOL32 CVDec::getAudio(CAudio *&pAudio) {
        pAudio = NULL;
        return FALSE;
    }

    BOOL32 CVDec::getRemainderBuffer(u8 *&out, u32 &size) {
        out = NULL;
        size = 0;
        return FALSE;
    }

}

