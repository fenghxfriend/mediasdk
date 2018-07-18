//
// Created by ASUS on 2018/4/23.
//

#include <autolog.h>
#include "dec.h"

namespace paomiantv {

    int
    CDec::open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx,
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
            *stream_idx = stream_index;
        }
        return 0;
    }

    /**
    * Decode one frame from the input file.
    * @param      frame                Audio frame to be decoded
    * @param      input_format_context Format context of the input file
    * @param      input_codec_context  Codec context of the input file
    * @param[out] data_present         Indicates whether data has been decoded
    * @param[out] finished             Indicates whether the end of file has
    *                                  been reached and all data has been
    *                                  decoded. If this flag is false, there
    *                                  is more data to be decoded, i.e., this
    *                                  function has to be called again.
    * @return Error code (0 if successful)
    */
    int CDec::decode_frame(AVFrame *&frame,
                           AVFormatContext *input_format_context,
                           AVCodecContext *input_codec_context,
                           int *data_present, int *input_finish, int *finished,
                           const int stream_index) {
        int error = 0;
        if (!(*input_finish)) {
            /* Packet used for temporary storage. */
            AVPacket input_packet;

            av_init_packet(&input_packet);
            /** Set the packet data and size so that it is recognized as being empty. */
            input_packet.data = NULL;
            input_packet.size = 0;

            /* Read one frame from the input file into a temporary packet. */
            while ((error = av_read_frame(input_format_context, &input_packet)) == 0) {
                if (input_packet.stream_index != stream_index) {
                    av_packet_unref(&input_packet);
                    av_init_packet(&input_packet);
                    /** Set the packet data and size so that it is recognized as being empty. */
                    input_packet.data = NULL;
                    input_packet.size = 0;
                } else {
                    break;
                }
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

    CDec::CDec(ITrack *const &pTrack) : m_pTrack(pTrack), m_bIsFinished(FALSE),
                                        m_bIsInputFinished(FALSE) {

    }

    CDec::~CDec() {

    }

}
