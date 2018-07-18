//
// Created by ASUS on 2018/3/28.
//

#include <webp/decode.h>
#include <autolog.h>
#include "webpdec.h"
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

    CWebpDec::CWebpDec() :
            m_bFinish(FALSE),
            m_bSuccess(FALSE),
            m_ptDmux(NULL),
            m_ptFrames(NULL),
            m_bIsKeyFrames(NULL),
            m_nDurations(NULL),
            m_nCanvasWidth(0),
            m_nCanvasHeight(0),
            m_nFrameCount(0),
            m_nLoopCount(0),
            m_uBGColor(0) {
        m_pLock = new CLock;
        m_pThread = new CThread(ThreadWrapper, this);
        memset(&m_tDecoderconfig, 0, sizeof(WebPDecoderConfig));
        memset(&m_tData, 0, sizeof(WebPData));
        memset(m_achFilePath, 0, sizeof(m_achFilePath));
    }

    CWebpDec::~CWebpDec() {
        m_pLock->lock();
        m_bFinish = TRUE;
        m_pLock->unlock();
        m_pThread->join();
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
        if (m_ptFrames != NULL) {
            delete[] m_ptFrames;
            m_ptFrames = NULL;
        }
        if (m_nDurations != NULL) {
            delete[] m_nDurations;
            m_nDurations = NULL;
        }

        if (m_pPreservedBuffer != NULL) {
            delete[] m_pPreservedBuffer;
            m_pPreservedBuffer = NULL;
        }

        if (m_pLock != NULL) {
            delete m_pLock;
            m_pLock = NULL;
        }
        if (m_pThread != NULL) {
            delete m_pThread;
            m_pThread = NULL;
        }
    }

    int CWebpDec::setDataSource(char *achPath) {
        CAutoLock autoLock(m_pLock);
        if (achPath == NULL || strlen(achPath) == 0) {
            LOGE("input file is invalid!\n");
            return -1;
        }
        strncpy(m_achFilePath, achPath, MAX_LEN_FILE_PATH);
        if (!initDemux(m_achFilePath)) {
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
        drawFrame(0, &m_ptFrames[0], m_nCanvasWidth, -1);
        m_pThread->start();
        return 0;
    }

    //init webp mux
    bool CWebpDec::initDemux(const char *path) {

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

    bool CWebpDec::initDecoder() {
        if (!WebPInitDecoderConfig(&m_tDecoderconfig)) {
            LOGE("Library version mismatch!\n");
            return false;
        }
        m_tDecoderconfig.output.is_external_memory = 1;
        m_tDecoderconfig.output.colorspace = MODE_rgbA;  // Pre-multiplied alpha mode.
        m_pPreservedBuffer = new u32[m_nCanvasWidth * m_nCanvasHeight];
        return true;
    }

    //static
    void *CWebpDec::ThreadWrapper(void *pThis) {
        CWebpDec *p = (CWebpDec *) pThis;
        int nErr = p->ThreadEntry();
        return (void *) nErr;
    }

    int CWebpDec::ThreadEntry() {
        m_pThread->setName("CWebpDec");
        int nNextFrameToDecode = 0;
        while (nNextFrameToDecode != m_nFrameCount && !m_bFinish) {
            if (drawFrame(nNextFrameToDecode,
                          &m_ptFrames[nNextFrameToDecode * m_nCanvasWidth * m_nCanvasHeight],
                          m_nCanvasWidth,
                          nNextFrameToDecode - 2) < 0) {
                LOGE("draw the %dth frame failed!", nNextFrameToDecode + 1);
                break;
            }
            nNextFrameToDecode++;
            if (nNextFrameToDecode == m_nFrameCount) {
                m_bSuccess = TRUE;
            }
        }
        m_bFinish = TRUE;
        return 0;
    }

    int CWebpDec::drawFrame(int frameNr,
                            u32 *outputPtr, int outputPixelStride, int previousFrameNr) {
        // Find the first frame to be decoded.
        int start = MAX(previousFrameNr + 1, 0);
        int earliestRequired = frameNr;
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

        // Use preserve buffer only if needed.
        u32 *prevBuffer = (frameNr == 0) ? outputPtr : m_pPreservedBuffer;
        int prevStride = (frameNr == 0) ? outputPixelStride : m_nCanvasWidth;
        u32 *currBuffer = outputPtr;
        int currStride = outputPixelStride;

        for (int i = start; i <= frameNr; i++) {
            prevIter = currIter;
            // Get ith frame.
            if (!WebPDemuxGetFrame(m_ptDmux, i + 1, &currIter)) {
                LOGE("Could not retrieve frame# %d", i);
                return -1;
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

            if (i == frameNr || !willBeCleared(currIter)) {
                if (!decodeFrame(currIter, currBuffer, currStride, prevIter, prevBuffer,
                                 prevStride)) {
                    LOGE("Error decoding frame# %d", i);
                    return -1;
                }
            }
        }

        if (outputPtr != currBuffer) {
            copyFrame(currBuffer, currStride, outputPtr, outputPixelStride, m_nCanvasWidth,
                      m_nCanvasHeight);
        }

        // Return last frame's delay.
        const int lastFrame = (frameNr + m_nFrameCount - 1) % m_nFrameCount;
        if (!WebPDemuxGetFrame(m_ptDmux, lastFrame + 1, &currIter)) {
            LOGE("Could not retrieve frame# %d", lastFrame);
        }

        const int lastFrameDelay = currIter.duration;

        WebPDemuxReleaseIterator(&currIter);
        WebPDemuxReleaseIterator(&prevIter);

        return lastFrameDelay;
    }


    void CWebpDec::initializeFrame(const WebPIterator &currIter, u32 *currBuffer,
                                   int currStride, const WebPIterator &prevIter,
                                   const u32 *prevBuffer, int prevStride) {
        if (m_bIsKeyFrames[currIter.frame_num - 1]) {  // Clear canvas.
            for (int y = 0; y < m_nCanvasHeight; y++) {
                u32 *dst = currBuffer + y * currStride;
                clearLine(dst, m_nCanvasWidth);
            }
        } else {
            // Preserve previous frame as starting state of current frame.
            copyFrame(prevBuffer, prevStride, currBuffer, currStride, m_nCanvasWidth,
                      m_nCanvasHeight);

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

    bool CWebpDec::decodeFrame(const WebPIterator &currIter, u32 *currBuffer,
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

    void CWebpDec::getFrame(int index, void **buffer, int *size) {
        CAutoLock autoLock(m_pLock);
        if (m_bSuccess && index >= 1 && index <= m_nFrameCount) {
            *buffer = &m_ptFrames[(index - 1) * m_nCanvasWidth * m_nCanvasHeight];
            *size = m_nCanvasWidth * m_nCanvasHeight * sizeof(u32);
        }
    }

    int CWebpDec::getFrameDuration(int index) {
        CAutoLock autoLock(m_pLock);
        if (m_bSuccess && index >= 1 && index <= m_nFrameCount) {
            return m_nDurations[index - 1];
        } else {
            return -1;
        }
    }

    bool CWebpDec::constructDependencyChain() {

        m_nCanvasWidth = WebPDemuxGetI(m_ptDmux, WEBP_FF_CANVAS_WIDTH);
        m_nCanvasHeight = WebPDemuxGetI(m_ptDmux, WEBP_FF_CANVAS_HEIGHT);
        m_nFrameCount = WebPDemuxGetI(m_ptDmux, WEBP_FF_FRAME_COUNT);

        m_bIsKeyFrames = new bool[m_nFrameCount];
        m_ptFrames = new u32[m_nFrameCount * m_nCanvasWidth * m_nCanvasHeight];
        m_nDurations = new int[m_nFrameCount];


        WebPIterator prev;
        WebPIterator curr;

        // Note: WebPDemuxGetFrame() uses base-1 counting.
        if (!WebPDemuxGetFrame(m_ptDmux, 1, &curr)) {
            LOGE("Could not retrieve frame# 0");
            return false;
        }
        m_bIsKeyFrames[0] = true;  // 0th frame is always a key frame.
        m_nDurations[0] = curr.duration;
        for (size_t i = 1; i < m_nFrameCount; i++) {
            prev = curr;
            if (!WebPDemuxGetFrame(m_ptDmux, i + 1, &curr)) {// Get ith frame.
                LOGE("Could not retrieve frame# %d", i);
                return false;
            }

            if ((!curr.has_alpha || curr.blend_method == WEBP_MUX_NO_BLEND) &&
                isFullFrame(curr, m_nCanvasWidth, m_nCanvasHeight)) {
                m_bIsKeyFrames[i] = true;
            } else {
                m_bIsKeyFrames[i] = (prev.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND) &&
                                    (isFullFrame(prev, m_nCanvasWidth, m_nCanvasHeight) ||
                                     m_bIsKeyFrames[i - 1]);
            }
            m_nDurations[i] = curr.duration;
        }

        WebPDemuxReleaseIterator(&prev);
        WebPDemuxReleaseIterator(&curr);
        return true;
    }

}




