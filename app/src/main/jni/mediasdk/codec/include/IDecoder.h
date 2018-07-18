//
// Created by Shower Young on 2018/5/9.
//

#ifndef MEDIAENGINE_IDECODER_H
#define MEDIAENGINE_IDECODER_H

#include <stdint.h>
#include <track.h>
#include <atomic>

namespace paomiantv {

    class ITrack;

    class CVLayerParam;

    class CAudio;

    typedef enum _DecoderState {
        EDecoderInvalid = 0,
        EDecoderStopped,
        EDecoderPaused,
        EDecoderPlaying,
    } DecoderState;

    class IDecoder {

    public:
        IDecoder(ITrack *mediaTrack);

        virtual ~IDecoder();

    public:
        uint32_t getTrackID();

        int16_t getWeight();

        EMSource getSourceType();

        virtual void setLoop(bool isLoop) {
            _isLoop = isLoop;
        }

        virtual bool isLoop() {
            return _isLoop;
        }

        virtual void setShowFirstFrame(bool flag) {
            _showFirstFrame = flag;
        }

        virtual void setShowLastFrame(bool flag) {
            _showLastFrame = flag;
        }

        virtual bool isShowFirstFrame() {
            return _showFirstFrame;
        }

        virtual bool isShowLastFrame() {
            return _showLastFrame;
        }

        virtual void setSourceTime(int64_t startTime, int64_t durTime);

        virtual void setTargetTime(int64_t startTime, int64_t durTime);

        virtual int64_t getSourceStartTime() {
            return _sourceStartTime;
        }

        virtual int64_t getSourceEndTime() {
            return _sourceEndTime;
        }

        virtual int64_t getTargetStartTime() {
            return _targetStartTime;
        }

        virtual int64_t getTargetEndTime() {
            return _targetEndTime;
        }

        /**
         * @param cacheCount Frame cache count, default value is 5
         */
        virtual void setCacheCount(int cacheCount) {
            if (cacheCount > 0) {
                _cacheCount = cacheCount;
            }
        }

        virtual int getCacheCount() {
            return _cacheCount;
        }

        virtual void setMinInterval(int64_t usTime) {
            _minInterval = usTime;
        }

        virtual int64_t getMinInterval() {
            return _minInterval;
        }

        /**
         * @return Current time in microseconds.
         */
        virtual int64_t getCurrentTime() {
            return _currentTime;
        }

        virtual bool prepare() = 0;

        virtual void release() = 0;

        virtual void start() = 0;

        virtual void stop() = 0;

        virtual void pause() = 0;

        virtual void resume() = 0;

        virtual bool isStart() {
            return EDecoderPlaying == _decoderState || EDecoderPaused == _decoderState;
        }

        virtual bool isPaused() {
            return EDecoderPaused == _decoderState;
        }

        virtual void seekTo(int64_t usTime) = 0;

        virtual bool isSeeking() {
            return false;
        }

        /**
         * 预留方法，暂未实现
         */
        virtual void flush() = 0;

        virtual CVLayerParam *getImageLayer(int64_t usPTS) = 0;

        virtual CAudio *getAudio(int64_t usPTS) = 0;

        virtual uint8_t *getRemainderBuffer(uint32_t &size) {
            size = 0;
            return nullptr;
        }

    protected:
        virtual bool _validateTrack(EMTrack trackType, EMSource sourceType);

        virtual void _parseTime();

    protected:
        /**
         *
         */
        ITrack *_pTrack;
        /**
         *
         */
        int _cacheCount;

        /**
         * Minimum frame interval in microseconds, frame-droping threshold.
         */
        int64_t _minInterval;

        /**
         * Source Start time in microseconds.
         */
        int64_t _sourceStartTime;

        /**
         * Source End time in microseconds.
         */
        int64_t _sourceEndTime;

        /**
         * Target Start time in microseconds.
         */
        int64_t _targetStartTime;

        /**
         * Target End time in microseconds.
         */
        int64_t _targetEndTime;

        /*
         * Current time in microseconds
         */
        int64_t _currentTime;

        std::atomic<DecoderState> _decoderState;

        bool _showFirstFrame;

        bool _showLastFrame;

        bool _isLoop;
    };

}
#endif //MEDIAENGINE_IDECODER_H
