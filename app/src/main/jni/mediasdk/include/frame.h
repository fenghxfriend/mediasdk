/*******************************************************************************
 *        Module: common
 *          File:
 * Functionality: frame defination
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 PAOMIANTV, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-08-09  v1.0        huangxuefeng  created
 ******************************************************************************/

#ifndef _PAOMIANTV_FRAME_H_
#define _PAOMIANTV_FRAME_H_

#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <typedef.h>
#include <autolock.h>


namespace paomiantv {

    class CFrame {
    public:
        u8 *data;
        u32 size;
        u32 offset;
        u32 clipId;
        u32 sampleId;
    protected:
        u32 maxSize;
        ILock *lock;
    public:
        virtual void reset(const u32 max_size)=0;

        virtual u32 drift(const u32 distance) {
            CAutoLock autoLock(lock);
            if (data == NULL) {
                return 0;
            }
            u32 total = size + offset + distance;
            if (total <= maxSize) {
                memmove(data + distance, data, size + offset);
            } else {
                u8 *newData = (u8 *) realloc(data, total);
                if (newData != NULL) {
                    data = newData;
                    memmove(data + distance, data, size + offset);
                    maxSize = total;
                } else {
                    return 0;
                }
            }
            offset += distance;
            return offset;
        }

    protected:
        CFrame()
                : data(NULL),
                  maxSize(0),
                  size(0),
                  offset(0),
                  clipId(0),
                  sampleId(0) {
            lock = new CLock;
        }

        void init(const u32 max_size) {
            CAutoLock autoLock(lock);
            if (data == NULL) {
                data = (u8 *) malloc(max_size);
            } else if (maxSize < max_size) {
                free(data);
                data = (u8 *) malloc(max_size);
            }
            maxSize = maxSize < max_size ? max_size : maxSize;
            size = maxSize;
            offset = 0;
            clipId = 0;
            sampleId = 0;
        }

        virtual ~CFrame() {
            lock->lock();
            if (data != NULL) {
                free(data);
                data = NULL;
            }
            maxSize = 0;
            size = 0;
            clipId = 0;
            sampleId = 0;
            lock->unlock();
            delete lock;
        }

        void uninit() {
            CAutoLock autoLock(lock);
            if (data != NULL) {
                free(data);
                data = NULL;
            }
            maxSize = 0;
            size = 0;
            clipId = 0;
            sampleId = 0;
        }
    };


    class CAACFrame : public CFrame {
    public:
        u64 startTm;
        u64 duration;
        u64 renderOffset;
        BOOL32 isSync;
        BOOL32 isLast;
        BOOL32 isESDS;
        BOOL32 isEos;

        CAACFrame()
                : startTm(0),
                  duration(0),
                  renderOffset(0),
                  isSync(FALSE),
                  isLast(FALSE),
                  isESDS(FALSE),
                  isEos(FALSE) {

        }

        virtual ~CAACFrame() {

        }

    public:
        virtual void reset(const u32 max_size) {
            CAutoLock autoLock(lock);
            init(max_size);
            startTm = 0;
            duration = 0;
            renderOffset = 0;
            isSync = FALSE;
            isLast = FALSE;
            isESDS = FALSE;
            isEos = FALSE;
        }

        static CAACFrame *create() {
            CAACFrame *re = NULL;
            m_sLock.lock();

            if (!m_svPool.empty()) {
                re = m_svPool.back();
                m_svPool.pop_back();
            } else {
                re = new CAACFrame();
            }

            m_sLock.unlock();
            return re;
        }

        static void release(CAACFrame *pframe) {
            m_sLock.lock();
            if (pframe != NULL) {
                m_svPool.push_back(pframe);
            }
            m_sLock.unlock();
        }

        static void clear() {
            m_sLock.lock();
            int i = 0;
            while (i < m_svPool.size()) {
                if(m_svPool[i]!=NULL){
                    m_sLock.unlock();
                    delete m_svPool[i];
                    m_sLock.lock();
                }
                i++;
            }
            m_svPool.clear();
            m_sLock.unlock();
        }

    private:
        static std::vector<CAACFrame *> m_svPool;
        static CLock m_sLock;
    };

    typedef enum {
        EM_TYPE_KNOWN,
        EM_TYPE_SPS,
        EM_TYPE_PPS,
        EM_TYPE_I,
        EM_TYPE_P,
        EM_TYPE_B,
        EM_TYPE_PB,//(p or b, not i)
        EM_TYPE_OTHER,//(sei,etc)
        EM_TYPE_END
    } EMH264FrameType;

    class CH264Frame : public CFrame {
    public:
        u64 startTm;
        u64 duration;
        u64 renderOffset;
        EMH264FrameType type;
//        BOOL32 isLast;
        BOOL32 isEos;

        CH264Frame()
                : startTm(0),
                  duration(0),
                  renderOffset(0),
                  type(EM_TYPE_KNOWN),
//                  isLast(FALSE),
                  isEos(FALSE) {

        }

        virtual ~CH264Frame() {

        }

    public:

        virtual void reset(const u32 max_size) {
            CAutoLock autoLock(lock);
            init(max_size);
            startTm = 0;
            duration = 0;
            renderOffset = 0;
            type = EM_TYPE_KNOWN;
//            isLast = FALSE;
            isEos = FALSE;
        }

        static CH264Frame *create() {
            CH264Frame *re = NULL;
            m_sLock.lock();

            if (!m_svPool.empty()) {
                re = m_svPool.back();
                m_svPool.pop_back();
            } else {
                re = new CH264Frame();
            }


            m_sLock.unlock();
            return re;
        }

        static void release(CH264Frame *pframe) {
            m_sLock.lock();
            if (pframe) {
                m_svPool.push_back(pframe);
            }
            m_sLock.unlock();
        }

        static void clear() {
            m_sLock.lock();
            int i = 0;
            while (i < m_svPool.size()) {
                if(m_svPool[i]!=NULL){
                    m_sLock.unlock();
                    delete m_svPool[i];
                    m_sLock.lock();
                }
                i++;
            }
            m_svPool.clear();
            m_sLock.unlock();
        }

    private:
        static std::vector<CH264Frame *> m_svPool;
        static CLock m_sLock;
    };


    class CI420Frame : public CFrame {
    public:
        u64 timeStamp;
        s32 width;
        s32 height;

        CI420Frame()
                : timeStamp(0),
                  width(0),
                  height(0) {

        }

        virtual ~CI420Frame() {
        }

    public:

        virtual void reset(const u32 max_size) {
            CAutoLock autoLock(lock);
            init(max_size);
            timeStamp = 0;
            width = 0;
            height = 0;
        }

        static CI420Frame *create() {
            CI420Frame *re = NULL;
            m_sLock.lock();

            if (!m_svPool.empty()) {
                re = m_svPool.back();
                m_svPool.pop_back();
            } else {
                re = new CI420Frame();
            }
            m_sLock.unlock();
            return re;
        }

        static void release(CI420Frame *pframe) {
            m_sLock.lock();
            if (pframe) {
                m_svPool.push_back(pframe);
            }
            m_sLock.unlock();
        }

        static void clear() {
            m_sLock.lock();
            int i = 0;
            while (i < m_svPool.size()) {
                if(m_svPool[i]!=NULL){
                    m_sLock.unlock();
                    delete m_svPool[i];
                    m_sLock.lock();
                }
                i++;
            }
            m_svPool.clear();
            m_sLock.unlock();
        }

    private:
        static std::vector<CI420Frame *> m_svPool;
        static CLock m_sLock;
    };


    class CYUVFrame : public CFrame {
    public:
        u64 timeStamp;
        s32 picWidth;//图片中图像的宽
        s32 picHeight;//图片中图像高
        s32 width;//码流中spspps图像参数
        s32 height;//码流中spspps图像参数
        s32 alignWidth;//图像解码后实际宽度
        s32 alignHeight;//图像解码后实际高
        u32 colorFmt;//解码图像色域
        s32 crop_X;//解码后图像在图片中x方向偏移，即width图像相对图片偏移
        s32 crop_Y;//解码后图像在图片中y方向偏移，即height图像相对图片偏移

        CYUVFrame()
                : timeStamp(0),
                  picWidth(0),
                  picHeight(0),
                  width(0),
                  height(0),
                  alignWidth(0),
                  alignHeight(0),
                  colorFmt(0),
                  crop_X(0),
                  crop_Y(0) {

        }

        virtual ~CYUVFrame() {
        }

    public:
        virtual void reset(const u32 max_size) {
            CAutoLock autoLock(lock);
            init(max_size);
            timeStamp = 0;
            picWidth = 0;
            picHeight = 0;
            width = 0;
            height = 0;
            alignWidth = 0;
            alignHeight = 0;
            colorFmt = 0;
            crop_X = 0;
            crop_Y = 0;
        }

        static CYUVFrame *create() {
            CYUVFrame *re = NULL;
            m_sLock.lock();
            if (!m_svPool.empty()) {
                re = m_svPool.back();
                m_svPool.pop_back();
            } else {
                re = new CYUVFrame();
            }
            m_sLock.unlock();
            return re;
        }

        static void release(CYUVFrame *pframe) {
            m_sLock.lock();
            if (pframe) {
                m_svPool.push_back(pframe);
            }
            m_sLock.unlock();
        }

        static void clear() {
            m_sLock.lock();
            int i = 0;
            while (i < m_svPool.size()) {
                if(m_svPool[i]!=NULL){
                    m_sLock.unlock();
                    delete m_svPool[i];
                    m_sLock.lock();
                }
                i++;
            }
            m_svPool.clear();
            m_sLock.unlock();
        }

    private:
        static std::vector<CYUVFrame *> m_svPool;
        static CLock m_sLock;
    };

    class CPCMFrame : public CFrame {
    public:
        u64 timeStamp;
        s32 channelCount;
        s32 bitsPerSample;
        s32 sampleRate;

        CPCMFrame()
                : timeStamp(0),
                  channelCount(0),
                  bitsPerSample(0),
                  sampleRate(0) {

        }

        virtual ~CPCMFrame() {
        }


    public:
        virtual void reset(const u32 max_size) {
            CAutoLock autoLock(lock);
            init(max_size);
            timeStamp = 0;
            channelCount = 0;
            bitsPerSample = 0;
            sampleRate = 0;
        }

        static CPCMFrame *create() {
            CPCMFrame *re = NULL;
            m_sLock.lock();

            if (!m_svPool.empty()) {
                re = m_svPool.back();
                m_svPool.pop_back();
            } else {
                re = new CPCMFrame();
            }
            m_sLock.unlock();
            return re;
        }

        static void release(CPCMFrame *pframe) {
            m_sLock.lock();
            if (pframe) {
                m_svPool.push_back(pframe);
            }
            m_sLock.unlock();
        }

        static void clear() {
            m_sLock.lock();
            int i = 0;
            while (i < m_svPool.size()) {
                if(m_svPool[i]!=NULL){
                    m_sLock.unlock();
                    delete m_svPool[i];
                    m_sLock.lock();
                }
                i++;
            }
            m_svPool.clear();
            m_sLock.unlock();
        }

    private:
        static std::vector<CPCMFrame *> m_svPool;
        static CLock m_sLock;
    };

    class CFrameManager {
    private:

        CFrameManager() {
        }

        virtual ~CFrameManager() {
        }

    public:
        static void clearFrame() {
            CAACFrame::clear();
            CH264Frame::clear();
            CI420Frame::clear();
            CYUVFrame::clear();
            CPCMFrame::clear();
        }

    };

}

#endif //_PAOMIANTV_FRAME_H_
