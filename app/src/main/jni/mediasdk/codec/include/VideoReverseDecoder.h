//
// Created by ASUS on 2018/6/1.
//

#ifndef MEDIAENGINE_VIDEOREVERSEDECODER_H
#define MEDIAENGINE_VIDEOREVERSEDECODER_H

#include <unistd.h>
#include <image.h>
#include <safequeue.h>
#include <safelist.h>
#include "ReverseDecoder.h"

#ifdef __cplusplus
extern "C"
{
#include <libavformat/avformat.h>
#ifndef SwsContext
typedef struct SwsContext SwsContext;
#endif
}
#endif

namespace paomiantv {
    class CVChunk {
    public:
        s8 *m_pchPath;
        FILE *m_pFile;
    protected:
        CVChunk() : m_pchPath(NULL), m_pFile(NULL) {
        }

        virtual ~CVChunk() {
            reset();
        }

    public:

        void reset() {
            if (m_pFile != NULL) {
                fclose(m_pFile);
                m_pFile = NULL;
            }
            if (m_pchPath != NULL) {
                if (access(m_pchPath, F_OK) != -1) {
                    remove(m_pchPath);
                }
                free(m_pchPath);
                m_pchPath = NULL;
            }
        }

        static CVChunk *create() {
            CVChunk *re = NULL;
            m_sLock.lock();
            if (!m_svPool.empty()) {
                re = m_svPool.back();
                m_svPool.pop_back();
            } else {
                re = new CVChunk();
            }
            m_sLock.unlock();
            return re;
        }

        static void release(CVChunk *pChunk) {
            m_sLock.lock();
            if (pChunk != NULL) {
                pChunk->reset();
                std::vector<CVChunk *>::iterator it;
                it = std::find(m_svPool.begin(), m_svPool.end(), pChunk);
                if (it == m_svPool.end()) {
                    //vec中不存在value值
                    m_svPool.push_back(pChunk);
                }
            }
            m_sLock.unlock();
        }

        static void clear() {
            m_sLock.lock();
            while (!m_svPool.empty()) {
                CVChunk *chunk = m_svPool.back();
                m_svPool.pop_back();
                if (chunk != NULL) {
                    delete chunk;
                }
            }
            m_svPool.clear();
            m_sLock.unlock();
        }

    private:
        static std::vector<CVChunk *> m_svPool;
        static CLock m_sLock;
    };


    class VideoReverseDecoder : public ReverseDecoder {
    public:
        VideoReverseDecoder(CDemuxer *demuxer);

        virtual ~VideoReverseDecoder();

        int prepare(s8 *tempPath);

        void stop();

        void start();

        void getLayer(CVLayerParam *&pLayer);

        void release();

        inline u32 getHeight();

        inline u32 getWidth();

    protected:
        virtual long run();

        int decode();

        int decodeFrame();

        int decode_frame(AVFrame *&frame, AVCodecContext *input_codec_context, int *data_present,
                         int *input_finish, int *finished);

        CSafeQueue<CVChunk> *m_pChunkQueue;


        AVFormatContext *m_ptAVFormatContext;
        AVCodecContext *m_ptAVCodecContext;
        AVFrame *m_ptAVFrame;
        AVPixelFormat m_eDecoderOutputFmt;

        u32 m_uWidth;
        u32 m_uHeight;
        EMPixelFormat m_ePixFmt;
//        u32 m_uBufferSize;
        SwsContext *m_ptImgConvertContext;

        CVLayerParam *m_pLayerParams;
        u8 *m_pbyBuffer;


        // for decoder and write to temp file
        u32 m_uCurrChunkId;
        u32 m_uCurrChunkStartSampleId;
        u32 m_uCurrChunkSampleId;
        u32 m_uCurrChunkSampleEndId;

        CVChunk *m_pCacheChunk;

        s8 *m_pchTempPath;

        BOOL32 m_bIsInputFinished;


        u32 m_uLayerBufferSize;

        // for renderer queue
        u32 m_uRenderCurrSampleId;
        u32 m_uRenderCurrChunk;
        BOOL32 m_bIsRenderReadFinished;

        CVChunk *m_pCurrChunk;

        s64 m_sllFirstCTSUS;

    };

    u32 VideoReverseDecoder::getHeight() {
        return m_uHeight;
    }

    u32 VideoReverseDecoder::getWidth() {
        return m_uWidth;
    }
}

#endif //MEDIAENGINE_VIDEOREVERSEDECODER_H
