//
// Created by ASUS on 2018/4/23.
//

#ifndef MEDIAENGINE_DEC_H
#define MEDIAENGINE_DEC_H

#include <typedef.h>
#include <track.h>
#include <sound.h>

#ifdef __cplusplus
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
}
#endif
namespace paomiantv {
    class CDec {
    public:

        static int decode_frame(AVFrame *&frame,
                                AVFormatContext *input_format_context,
                                AVCodecContext *input_codec_context,
                                int *data_present, int *input_finish, int *finished,
                                const int stream_index);

        static int open_codec_context(int *stream_idx,
                                      AVCodecContext **dec_ctx,
                                      AVFormatContext *fmt_ctx,
                                      enum AVMediaType type,
                                      char *file_path);

        CDec(ITrack *const &pTrack);

        virtual ~CDec();

        virtual int prepare() = 0;

        virtual void release() = 0;

        virtual void start() = 0;

        virtual void stop() =0;

        virtual void pause()=0;

        virtual void resume()=0;

        virtual BOOL32 getLayer(CVLayerParam *&pLayer) = 0;

        virtual BOOL32 getAudio(CAudio *&pAudio) = 0;

        virtual BOOL32 getRemainderBuffer(u8 *&out, u32 &size) =0;

        inline u32 getTrackId() const;

        inline EMSource getSourceType() const;

        inline BOOL32 isFinish();

    protected:
        ITrack *m_pTrack;
        BOOL32 m_bIsInputFinished;

        BOOL32 m_bIsFinished;
    };

    inline u32 CDec::getTrackId() const {
        return m_pTrack->getId();
    }

    inline EMSource CDec::getSourceType() const {
        return m_pTrack->getSourceType();
    }

    inline BOOL32 CDec::isFinish() {
        return m_bIsFinished;
    }
}
#endif //MEDIAENGINE_DEC_H
