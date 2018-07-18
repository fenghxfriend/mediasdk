//
// Created by ASUS on 2018/3/28.
//

#include <webp/decode.h>
#include <autolog.h>
#include <unistd.h>
#include "wpdec.h"
#include "../../../webp/imageio/imageio_util.h"


namespace paomiantv {


    // Returns true if the frame covers full canvas.
    static bool isFullFrame(const WebPIterator &frame, int canvasWidth, int canvasHeight) {
        return (frame.width == canvasWidth && frame.height == canvasHeight);
    }

    static bool willBeCleared(const WebPIterator &iter) {
        return iter.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND;
    }

    // Clear all pixels in a line to transparent.
    static void clearLine(u32 *dst, int width) {
        memset(dst, 0, width * sizeof(*dst));  // Note: Assumes TRANSPARENT == 0x0.
    }


    // Copy all pixels from 'src' to 'dst'.
    static void copyFrame(const u32 *src, int srcStride, u32 *dst, int dstStride,
                          int width, int height) {
        for (int y = 0; y < height; y++) {
            memcpy(dst, src, width * sizeof(*dst));
            src += srcStride;
            dst += dstStride;
        }
    }

    // return true if area of 'target' completely covers area of 'covered'
    static bool checkIfCover(const WebPIterator &target, const WebPIterator &covered) {
        const int covered_x_max = covered.x_offset + covered.width;
        const int target_x_max = target.x_offset + target.width;
        const int covered_y_max = covered.y_offset + covered.height;
        const int target_y_max = target.y_offset + target.height;
        return target.x_offset <= covered.x_offset
               && covered_x_max <= target_x_max
               && target.y_offset <= covered.y_offset
               && covered_y_max <= target_y_max;
    }

    // Returns true if the rectangle defined by 'frame' contains pixel (x, y).
    static bool FrameContainsPixel(const WebPIterator &frame, int x, int y) {
        const int left = frame.x_offset;
        const int right = left + frame.width;
        const int top = frame.y_offset;
        const int bottom = top + frame.height;
        return x >= left && x < right && y >= top && y < bottom;
    }

    CVWebPDec::CVWebPDec(ITrack *const &pTrack) : CDec(pTrack),
                                                  m_bIsStopped(FALSE),
                                                  m_bIsStarted(FALSE),
                                                  m_bIsPaused(FALSE),
                                                  m_ptDmux(NULL),
                                                  m_bIsKeyFrames(NULL),
                                                  m_uWidth(0),
                                                  m_uHeight(0),
                                                  m_uFrameCount(0),
                                                  m_uBGColor(0),
                                                  m_nNextFrameToDecode(-1),
                                                  m_pLastLayer(NULL),
                                                  m_pCacheLayer(NULL),
                                                  m_uBufferSize(0),
                                                  m_sllCurrPlayUS(0),
                                                  m_sllStartPlayUS(0),
                                                  m_sllEndCutUS(0),
                                                  m_nCurrFrameIndex(0), m_sllCurrPTSUS(0),
                                                  m_sllLastTimestampUS(0) {
        m_pQueue = new CSafeQueue<CVLayerParam>();
        m_pLock = new CLock;
        m_pThread = new CThread(ThreadWrapper, this);
        memset(&m_tDecoderconfig, 0, sizeof(WebPDecoderConfig));
        memset(&m_tData, 0, sizeof(WebPData));
    }

    CVWebPDec::~CVWebPDec() {
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

    int CVWebPDec::prepare() {
        m_pQueue->clear();
        m_pQueue->enable();
        CAutoLock autoLock(m_pLock);
        uint8_t *data = nullptr;
        uint32_t size = 0;
        if (m_pTrack != NULL &&
            (m_pTrack->getSourceData(data, size), (data != NULL && size != 0)) &&
            m_pTrack->getSourceType() == EM_SOURCE_WEBP) {
            if (!initDemux((const char *) data)) {
                LOGE("Init demux failed!\n");
                return -1;
            }

            if (!constructDependencyChain()) {
                LOGE("Init data, Create frame array data failed!\n");
                return -1;

            }

            if (!initDecoder()) {
                LOGE("Init decoder failed!\n");
                return -1;
            }
            if (!getStartFrame()) {
                LOGE("get the start frame wrong!\n");
                return -1;
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


    void CVWebPDec::pause() {
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted && !m_bIsStopped && !m_bIsPaused) {
            m_bIsPaused = TRUE;
        }
    }

    void CVWebPDec::resume() {
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted && !m_bIsStopped && m_bIsPaused) {
            m_bIsPaused = FALSE;
            m_pLock->acttive();
        }
    }


    void CVWebPDec::release() {
        if (m_ptDmux != NULL) {
            WebPDemuxDelete(m_ptDmux);
            m_ptDmux = NULL;
        }
        if (m_tData.bytes != NULL) {
            delete[] m_tData.bytes;
        }
        if (m_bIsKeyFrames != NULL) {
            delete[] m_bIsKeyFrames;
            m_bIsKeyFrames = NULL;
        }

        if (m_pPreservedBuffer != NULL) {
            delete[] m_pPreservedBuffer;
            m_pPreservedBuffer = NULL;
        }
    }

    void CVWebPDec::start() {
        LOGI("CVWebPDec::startThread");
        m_pQueue->enable();
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted) {
            return;
        }
        if (!m_pThread->start()) {
            LOGE("start webp decoder thread failed!");
            return;
        }
    }

    void CVWebPDec::stop() {
        LOGI("CVWebPDec::stopThread");
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

//    static void findLayer(CSafeQueue<CVLayerParam> *queue, const s64 pts, CVLayerParam *&layer) {
//        queue->pop(layer);
//        if (layer == NULL) {
//            return;
//        }
//        if (layer->m_sllTimeStampUS <= pts &&
//            (layer->m_sllTimeStampUS + layer->m_sllDurationUS) > pts) {
//            return;
//        } else {
//            CVLayerParam::release(layer);
//            findLayer(queue, pts, layer);
//        }
//    }


    void CVWebPDec::findLayer(const s64 pts, CVLayerParam *&layer) {
        layer = NULL;
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

    BOOL32 CVWebPDec::getLayer(CVLayerParam *&pLayer) {
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
        m_sllCurrPlayUS += VIDEO_FRAME_MIN_DURATION;
        return TRUE;
    }

    //init webp mux
    bool CVWebPDec::initDemux(const char *path) {

        if (!ImgIoUtilReadFile(path,
                               &m_tData.bytes, &m_tData.size)) {
            LOGE("Get webp data from source file failed.\n");
            return false;
        }

        if (!WebPGetInfo(m_tData.bytes, m_tData.size, NULL, NULL)) {
            LOGE("Input file doesn't appear to be WebP format.\n");
            return false;
        }

        m_ptDmux = WebPDemux(&m_tData);
        if (m_ptDmux == NULL) {
            LOGE("Could not create demuxing object!\n");
            return false;
        }
        return true;
    }

    bool CVWebPDec::initDecoder() {
        if (!WebPInitDecoderConfig(&m_tDecoderconfig)) {
            LOGE("Library version mismatch!\n");
            return false;
        }
        m_tDecoderconfig.output.is_external_memory = 1;
        m_tDecoderconfig.output.colorspace = MODE_rgbA;  // Pre-multiplied alpha mode.
        m_pPreservedBuffer = new u32[m_uWidth * m_uHeight];
        m_uBufferSize = m_uWidth * m_uHeight * 4;
        return true;
    }

    //static
    void *CVWebPDec::ThreadWrapper(void *pThis) {
        prctl(PR_SET_NAME, (unsigned long) "CVWebPDec", 0, 0, 0);
        CVWebPDec *p = (CVWebPDec *) pThis;
        int nErr = p->ThreadEntry();
        return (void *) nErr;
    }

    int CVWebPDec::ThreadEntry() {
        int nErr = 0;
        char name[64];
        memset(name, 0, sizeof(name));
        sprintf(name, "CVWebPDec%u", m_pTrack->getId());
        m_pThread->setName(name);
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

        return nErr;
    }

    int CVWebPDec::decode() {
        if (!m_bIsFinished) {

            if (drawFrame() < 0) {
                LOGE("draw the %dth frame failed!", m_nCurrFrameIndex + 1);
            }

        } else {
            usleep(5000);
        }
        return 0;
    }

    int CVWebPDec::drawFrame() {
        if (m_nCurrFrameIndex == m_uFrameCount || m_sllCurrPTSUS > m_sllEndCutUS) {
            if (m_pCacheLayer != NULL) {
                m_pCacheLayer->m_sllDurationUS = m_sllEndCutUS - m_sllLastPTSUS;
                if (!m_pQueue->push(m_pCacheLayer)) {
                    CVLayerParam::release(m_pCacheLayer);
                } else {
                    LOGI("Push last layer to queue in TRACK%d, layer: timestamp = %lld, duration = %lld \n",
                         m_pCacheLayer->m_uTrackId, m_pCacheLayer->m_sllTimeStampUS,
                         m_pCacheLayer->m_sllDurationUS);
                }
                m_pCacheLayer = NULL;
            }
            m_bIsFinished = TRUE;
            LOGE("Video track%d Input finished. Write NULL frame \n", m_pTrack->getId());
            return -1;
        }
        if (m_sllCurrPTSUS >= (((CTrack *) m_pTrack)->getCutStart() * 1000) &&
            (m_sllCurrPTSUS - m_sllLastPTSUS) >= VIDEO_FRAME_MIN_DURATION) {
            if (m_pCacheLayer != NULL) {
                m_pCacheLayer->m_sllDurationUS = m_sllCurrPTSUS - m_sllLastPTSUS;
                if (!m_pQueue->push(m_pCacheLayer)) {
                    CVLayerParam::release(m_pCacheLayer);
                } else {
//                    LOGI("Push a layer to queue in TRACK%d, layer: timestamp = %lld, duration = %lld \n",
//                         m_pCacheLayer->m_uTrackId, m_pCacheLayer->m_sllTimeStampUS,
//                         m_pCacheLayer->m_sllDurationUS);
                }
                m_pCacheLayer = NULL;
            }
        }

        int outputPixelStride = m_uWidth;
        // Find the first frame to be decoded.
        int start = MAX(m_nCurrFrameIndex - 1, 0);
        int earliestRequired = m_nCurrFrameIndex;
        while (earliestRequired > start) {
            if (m_bIsKeyFrames[earliestRequired]) {
                start = earliestRequired;
                break;
            }
            earliestRequired--;
        }

        WebPIterator currIter;
        WebPIterator prevIter;
        // Get frame number 'start - 1'.
        if (!WebPDemuxGetFrame(m_ptDmux, start, &currIter)) {
            LOGE("Could not retrieve frame# %d", start - 1);
            return -1;
        }

        CVLayerParam *layer = CVLayerParam::create();
        layer->resize(m_uBufferSize);
        memset(layer->m_pbyPicture, 0, m_uBufferSize);
        int error = 0;
        do {
            // Use preserve buffer only if needed.
            u32 *prevBuffer = (m_nCurrFrameIndex == 0) ? (u32 *) layer->m_pbyPicture
                                                       : m_pPreservedBuffer;
            int prevStride = (m_nCurrFrameIndex == 0) ? outputPixelStride : m_uWidth;
            u32 *currBuffer = (u32 *) layer->m_pbyPicture;
            int currStride = outputPixelStride;

            for (int i = start; i <= m_nCurrFrameIndex; i++) {
                prevIter = currIter;
                // Get ith frame.
                if (!WebPDemuxGetFrame(m_ptDmux, i + 1, &currIter)) {
                    LOGE("Could not retrieve frame# %d", i);
                    error = -1;
                    break;
                }

                // We swap the prev/curr buffers as we go.
                u32 *tmpBuffer = prevBuffer;
                prevBuffer = currBuffer;
                currBuffer = tmpBuffer;

                int tmpStride = prevStride;
                prevStride = currStride;
                currStride = tmpStride;


                // Process this frame.
                initializeFrame(currIter, currBuffer, currStride, prevIter, prevBuffer, prevStride);

                if (i == m_nCurrFrameIndex || !willBeCleared(currIter)) {
                    if (!decodeFrame(currIter, currBuffer, currStride, prevIter, prevBuffer,
                                     prevStride)) {
                        LOGE("Error decoding frame# %d", i);
                        error = -1;
                        break;
                    }
                }
            }

            if ((u32 *) layer->m_pbyPicture != currBuffer) {
                copyFrame(currBuffer, currStride, (u32 *) layer->m_pbyPicture,
                          outputPixelStride, m_uWidth,
                          m_uHeight);
            }
        } while (0);

        if (error < 0) {
            CVLayerParam::release(layer);
            return error;
        }
        s64 duration = currIter.duration * 1000;
        if (m_pCacheLayer != NULL) {
            if (m_sllCurrPTSUS + duration - m_sllLastPTSUS >=
                VIDEO_FRAME_MIN_DURATION) {
                m_pCacheLayer->m_sllDurationUS = m_sllCurrPTSUS + duration - m_sllLastPTSUS;

                m_sllLastTimestampUS = m_pCacheLayer->m_sllTimeStampUS;
                if (!m_pQueue->push(m_pCacheLayer)) {
                    CVLayerParam::release(m_pCacheLayer);
                } else {
//                    LOGI("Push a layer to queue in TRACK%d, layer: timestamp = %lld, duration = %lld \n",
//                         m_pCacheLayer->m_uTrackId, m_pCacheLayer->m_sllTimeStampUS,
//                         m_pCacheLayer->m_sllDurationUS);
                }
                m_pCacheLayer = NULL;
            }
            CVLayerParam::release(layer);
            m_sllCurrPTSUS += duration;
        } else {
            layer->m_sllTimeStampUS = m_sllLastTimestampUS + m_sllCurrPTSUS - m_sllLastPTSUS;
            layer->m_uSize = m_uBufferSize;
            layer->m_uTrackId = m_pTrack->getId();
            layer->m_eFormat = EM_PIXEL_FORMAT_RGBA_8888;

            layer->m_uHeight = m_uHeight;
            layer->m_uWidth = m_uWidth;
            setLayerParams(layer, m_sllCurrPTSUS);

            if (duration >= VIDEO_FRAME_MIN_DURATION) {

                layer->m_sllDurationUS = duration;

                m_sllLastTimestampUS = layer->m_sllTimeStampUS;
                m_sllLastPTSUS = m_sllCurrPTSUS;
                m_sllCurrPTSUS += duration;
                if (!m_pQueue->push(layer)) {
                    CVLayerParam::release(layer);
                } else {
//                    LOGI("Push a layer to queue in TRACK%d, layer: timestamp = %lld, duration = %lld \n",
//                         layer->m_uTrackId, layer->m_sllTimeStampUS,
//                         layer->m_sllDurationUS);
                }
            } else {
                m_pCacheLayer = layer;
                m_sllCurrPTSUS += duration;
            }
        }

        WebPDemuxReleaseIterator(&currIter);
        WebPDemuxReleaseIterator(&prevIter);
        m_nCurrFrameIndex++;
        return 0;
    }


    void CVWebPDec::initializeFrame(const WebPIterator &currIter, u32 *currBuffer,
                                    int currStride, const WebPIterator &prevIter,
                                    const u32 *prevBuffer, int prevStride) {
        if (m_bIsKeyFrames[currIter.frame_num - 1]) {  // Clear canvas.
            for (int y = 0; y < m_uHeight; y++) {
                u32 *dst = currBuffer + y * currStride;
                clearLine(dst, m_uWidth);
            }
        } else {
            // Preserve previous frame as starting state of current frame.
            copyFrame(prevBuffer, prevStride, currBuffer, currStride, m_uWidth,
                      m_uHeight);

            // Dispose previous frame rectangle to Background if needed.
            bool prevFrameCompletelyCovered =
                    (!currIter.has_alpha || currIter.blend_method == WEBP_MUX_NO_BLEND) &&
                    checkIfCover(currIter, prevIter);
            if ((prevIter.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND) &&
                !prevFrameCompletelyCovered) {
                u32 *dst = currBuffer + prevIter.x_offset + prevIter.y_offset * currStride;
                for (int j = 0; j < prevIter.height; j++) {
                    clearLine(dst, prevIter.width);
                    dst += currStride;
                }
            }
        }
    }

    bool CVWebPDec::decodeFrame(const WebPIterator &currIter, u32 *currBuffer,
                                int currStride, const WebPIterator &prevIter, const u32 *prevBuffer,
                                int prevStride) {
        u32 *dst = currBuffer + currIter.x_offset + currIter.y_offset * currStride;
        m_tDecoderconfig.output.u.RGBA.rgba = (uint8_t *) dst;
        m_tDecoderconfig.output.u.RGBA.stride = currStride * 4;
        m_tDecoderconfig.output.u.RGBA.size =
                m_tDecoderconfig.output.u.RGBA.stride * currIter.height;

        const WebPData &currFrame = currIter.fragment;
        if (WebPDecode(currFrame.bytes, currFrame.size, &m_tDecoderconfig) != VP8_STATUS_OK) {
            return false;
        }
        // During the decoding of current frame, we may have set some pixels to be transparent
        // (i.e. alpha < 255). However, the value of each of these pixels should have been determined
        // by blending it against the value of that pixel in the previous frame if WEBP_MUX_BLEND was
        // specified. So, we correct these pixels based on disposal method of the previous frame and
        // the previous frame buffer.
        if (currIter.blend_method == WEBP_MUX_BLEND && !m_bIsKeyFrames[currIter.frame_num - 1]) {
            if (prevIter.dispose_method == WEBP_MUX_DISPOSE_NONE) {
                for (int y = 0; y < currIter.height; y++) {
                    const int canvasY = currIter.y_offset + y;
                    for (int x = 0; x < currIter.width; x++) {
                        const int canvasX = currIter.x_offset + x;
                        u32 &currPixel = currBuffer[canvasY * currStride + canvasX];
                        // FIXME: Use alpha-blending when alpha is between 0 and 255.
                        if (!(currPixel & COLOR_8888_ALPHA_MASK)) {
                            const u32 prevPixel = prevBuffer[canvasY * prevStride + canvasX];
                            currPixel = prevPixel;
                        }
                    }
                }
            } else {  // prevIter.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND
                // Need to restore transparent pixels to as they were just after frame initialization.
                // That is:
                //   * Transparent if it belongs to previous frame rectangle <-- This is a no-op.
                //   * Pixel in the previous canvas otherwise <-- Need to restore.
                for (int y = 0; y < currIter.height; y++) {
                    const int canvasY = currIter.y_offset + y;
                    for (int x = 0; x < currIter.width; x++) {
                        const int canvasX = currIter.x_offset + x;
                        u32 &currPixel = currBuffer[canvasY * currStride + canvasX];
                        // FIXME: Use alpha-blending when alpha is between 0 and 255.
                        if (!(currPixel & COLOR_8888_ALPHA_MASK)
                            && !FrameContainsPixel(prevIter, canvasX, canvasY)) {
                            const u32 prevPixel = prevBuffer[canvasY * prevStride + canvasX];
                            currPixel = prevPixel;
                        }
                    }
                }
            }
        }
        return true;
    }

    bool CVWebPDec::constructDependencyChain() {

        m_uWidth = WebPDemuxGetI(m_ptDmux, WEBP_FF_CANVAS_WIDTH);
        m_uHeight = WebPDemuxGetI(m_ptDmux, WEBP_FF_CANVAS_HEIGHT);
        m_uFrameCount = WebPDemuxGetI(m_ptDmux, WEBP_FF_FRAME_COUNT);

        m_bIsKeyFrames = new bool[m_uFrameCount];

        bool findfirstframe = false;
        int ptsMS = 0;
        int lastKeyFrame = 0;
        int lastKeyFramePtsMS = 0;

        if (ptsMS >= ((CTrack *) m_pTrack)->getCutStart()) {
            m_nStartKeyFrameIndex = 0;
            m_nStartKeyFramePtsMS = 0;
            m_nStartFrameIndex = 0;
            findfirstframe = true;
        }

        WebPIterator prev;
        WebPIterator curr;

        // Note: WebPDemuxGetFrame() uses base-1 counting.
        if (!WebPDemuxGetFrame(m_ptDmux, 1, &curr)) {
            LOGE("Could not retrieve frame# 0");
            return false;
        }

        m_bIsKeyFrames[0] = true;  // 0th frame is always a key frame.

        for (size_t i = 1; i < m_uFrameCount; i++) {
            prev = curr;
            if (!WebPDemuxGetFrame(m_ptDmux, i + 1, &curr)) {// Get ith frame.
                LOGE("Could not retrieve frame# %d", i);
                return false;
            }

            if ((!curr.has_alpha || curr.blend_method == WEBP_MUX_NO_BLEND) &&
                isFullFrame(curr, m_uWidth, m_uHeight)) {
                m_bIsKeyFrames[i] = true;
            } else {
                m_bIsKeyFrames[i] = (prev.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND) &&
                                    (isFullFrame(prev, m_uWidth, m_uHeight) ||
                                     m_bIsKeyFrames[i - 1]);
            }
            ptsMS += prev.duration;
            if (m_bIsKeyFrames[i] && !findfirstframe) {
                lastKeyFrame = i;
                lastKeyFramePtsMS = ptsMS;
            }
            if (ptsMS >= ((CTrack *) m_pTrack)->getCutStart() && !findfirstframe) {
                m_nStartKeyFrameIndex = lastKeyFrame;
                m_nStartKeyFramePtsMS = lastKeyFramePtsMS;
                m_nStartFrameIndex = i;
                findfirstframe = true;
            }
        }
        ptsMS += curr.duration;
        WebPDemuxReleaseIterator(&prev);
        WebPDemuxReleaseIterator(&curr);

        if (((CTrack *) m_pTrack)->getCutStart() < 0 &&
            ((CTrack *) m_pTrack)->getCutStart() >= ptsMS) {
            LOGE("track params is invalid\n");
            return false;
        }

        if (((CTrack *) m_pTrack)->getCutDuration() < 0) {
            m_sllEndCutUS = ptsMS * 1000;
        } else {
            m_sllEndCutUS =
                    (((CTrack *) m_pTrack)->getCutStart() +
                     ((CTrack *) m_pTrack)->getCutDuration()) *
                    1000;
            m_sllEndCutUS = MIN (m_sllEndCutUS, ptsMS * 1000);
        }

        m_sllStartPlayUS = m_pTrack->getPlayStart() * 1000;
        m_sllEndPlayUS = m_pTrack->getPlayDuration() < 0 ? -1 :
                         (m_sllStartPlayUS + m_pTrack->getPlayDuration()) * 1000;
        return true;
    }

    BOOL32 CVWebPDec::getAudio(CAudio *&pAudio) {
        pAudio = NULL;
        return FALSE;
    }

    bool CVWebPDec::getStartFrame() {
        // Find the first frame to be decoded.

        WebPIterator currIter;
        WebPIterator prevIter;
        // Get frame number 'start - 1'.
        if (!WebPDemuxGetFrame(m_ptDmux, m_nStartKeyFrameIndex, &currIter)) {
            LOGE("Could not retrieve frame# %d", m_nStartKeyFrameIndex - 1);
            return false;
        }
        s64 ptsUS = m_nStartKeyFramePtsMS * 1000;

        m_pCacheLayer = CVLayerParam::create();
        m_pCacheLayer->resize(m_uBufferSize);
        memset(m_pCacheLayer->m_pbyPicture, 0, m_uBufferSize);
        m_pCacheLayer->m_sllTimeStampUS = m_sllStartPlayUS;
        m_pCacheLayer->m_uSize = m_uBufferSize;
        m_pCacheLayer->m_uTrackId = m_pTrack->getId();
        m_pCacheLayer->m_eFormat = EM_PIXEL_FORMAT_RGBA_8888;

        // Use preserve buffer only if needed.
        u32 *prevBuffer = (m_nStartFrameIndex == 0) ? (u32 *) m_pCacheLayer->m_pbyPicture
                                                    : m_pPreservedBuffer;
        int prevStride = m_uWidth;
        u32 *currBuffer = (u32 *) m_pCacheLayer->m_pbyPicture;
        int currStride = m_uWidth;

        ptsUS -= (currIter.duration * 1000);
        for (int i = m_nStartKeyFrameIndex; i <= m_nStartFrameIndex; i++) {
            prevIter = currIter;
            ptsUS += (prevIter.duration * 1000);
            // Get ith frame.
            if (!WebPDemuxGetFrame(m_ptDmux, i + 1, &currIter)) {
                LOGE("Could not retrieve frame# %d", i);
                return false;
            }

            // We swap the prev/curr buffers as we go.
            u32 *tmpBuffer = prevBuffer;
            prevBuffer = currBuffer;
            currBuffer = tmpBuffer;

            int tmpStride = prevStride;
            prevStride = currStride;
            currStride = tmpStride;


            // Process this frame.
            initializeFrame(currIter, currBuffer, currStride, prevIter, prevBuffer, prevStride);

            if (i == m_nStartFrameIndex || !willBeCleared(currIter)) {
                if (!decodeFrame(currIter, currBuffer, currStride, prevIter, prevBuffer,
                                 prevStride)) {
                    LOGE("Error decoding frame# %d", i);
                    return false;
                }
            }
        }

        if ((u32 *) m_pCacheLayer->m_pbyPicture != currBuffer) {
            copyFrame(currBuffer, currStride, (u32 *) m_pCacheLayer->m_pbyPicture, m_uWidth,
                      m_uWidth,
                      m_uHeight);
        }

        m_pCacheLayer->m_uHeight = m_uHeight;
        m_pCacheLayer->m_uWidth = m_uWidth;

        setLayerParams(m_pCacheLayer, ptsUS);

        m_sllLastTimestampUS = m_pCacheLayer->m_sllTimeStampUS;
        m_sllLastPTSUS = ptsUS;
        m_sllCurrPTSUS = ptsUS + m_pCacheLayer->m_sllDurationUS;
//        LOGI("Push a m_pCacheLayer to queue in TRACK%d, m_pCacheLayer: timestamp = %lld, duration = %lld \n",
//             m_pTrack->getId(), m_pCacheLayer->m_sllTimeStampUS,
//             m_pCacheLayer->m_sllDurationUS);

        WebPDemuxReleaseIterator(&currIter);
        WebPDemuxReleaseIterator(&prevIter);

        m_nCurrFrameIndex = m_nStartFrameIndex + 1;

        return true;
    }

    void CVWebPDec::setLayerParams(CVLayerParam *layer, s64 pts) {
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
                layer->m_afTranslate[EM_DIRECT_X] =
                        animation->getVecX() * offset + animation->m_fStartTransX;
                layer->m_afTranslate[EM_DIRECT_Y] =
                        animation->getVecY() * offset + animation->m_fStartTransY;
                layer->m_afTranslate[EM_DIRECT_Z] =
                        animation->getVecZ() * offset + animation->m_fStartTransZ;

                layer->m_afRotate[EM_DIRECT_X] =
                        animation->getVecDegreeX() * offset + animation->m_fStartDegreeX;
                layer->m_afRotate[EM_DIRECT_Y] =
                        animation->getVecDegreeY() * offset + animation->m_fStartDegreeY;
                layer->m_afRotate[EM_DIRECT_Z] =
                        animation->getVecDegreeZ() * offset + animation->m_fStartDegreeZ;

                layer->m_afScale[EM_DIRECT_X] =
                        animation->getVecScaleX() * offset + animation->m_fStartScaleX;
                layer->m_afScale[EM_DIRECT_Y] =
                        animation->getVecScaleY() * offset + animation->m_fStartScaleY;
                layer->m_afScale[EM_DIRECT_Z] =
                        animation->getVecScaleZ() * offset + animation->m_fStartScaleZ;

                // crop params
                layer->m_afUVCropTranslate[EM_DIRECT_X] =
                        animation->getCropVecX() * offset + animation->m_fCropStartTransX;
                layer->m_afUVCropTranslate[EM_DIRECT_Y] =
                        animation->getCropVecY() * offset + animation->m_fCropStartTransY;
                layer->m_afUVCropTranslate[EM_DIRECT_Z] =
                        animation->getCropVecZ() * offset + animation->m_fCropStartTransZ;

                layer->m_afUVCropRotate[EM_DIRECT_X] =
                        animation->getCropVecDegreeX() * offset +
                        animation->m_fCropStartDegreeX;
                layer->m_afUVCropRotate[EM_DIRECT_Y] =
                        animation->getCropVecDegreeY() * offset +
                        animation->m_fCropStartDegreeY;
                layer->m_afUVCropRotate[EM_DIRECT_Z] =
                        animation->getCropVecDegreeZ() * offset +
                        animation->m_fCropStartDegreeZ;

                layer->m_afUVCropScale[EM_DIRECT_X] =
                        animation->getCropVecScaleX() * offset +
                        animation->m_fCropStartScaleX;
                layer->m_afUVCropScale[EM_DIRECT_Y] =
                        animation->getCropVecScaleY() * offset +
                        animation->m_fCropStartScaleY;
                layer->m_afUVCropScale[EM_DIRECT_Z] =
                        animation->getCropVecScaleZ() * offset +
                        animation->m_fCropStartScaleZ;

                // color alpha
                layer->m_fAlpha =
                        animation->getVecAlpha() * offset + animation->m_fStartAlpha;

            }
        }

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
                    layer->m_vFilterParams.push_back(filterParam);
                } else {
                    CVFilterParam::release(filterParam);
                }
            }
        }
    }

    BOOL32 CVWebPDec::getRemainderBuffer(u8 *&out, u32 &size) {
        out = NULL;
        size = 0;
        return FALSE;
    }
}




