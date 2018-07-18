//
// Created by John.Huang on 2017/9/26.
//

#ifndef MEDIAENGINE_PCMPROCESSOR_H
#define MEDIAENGINE_PCMPROCESSOR_H

#define FRAC_BITS 16

#define FRAC (1 << FRAC_BITS)

#define av_free(p) {if(p) free(p);}

#define av_malloc(size) malloc(size)

#define UINT unsigned int

namespace paomiantv {
    typedef struct {
        /* fractional resampling */
        UINT incr;
        /* fractional increment */
        UINT frac;
        int last_sample;
        /* integer down sample */
        int iratio;
        /* integer divison ratio */
        int icount, isum;
        int inv;
    } ReSampleChannelContext;

    typedef struct ReSampleContext {
        ReSampleChannelContext channel_ctx[2];
        float ratio;
        /* channel convert */
        int input_channels, output_channels, filter_channels;
    } ReSampleContext;

    class CPCMProcessor {
        static int init_PCM_resample(int output_channels, int input_channels, int output_rate, int input_rate);

        static int start_PCM_resample(short *output, short *input, int in_len);

        static int uninit_PCM_resample();

        static int bit_wide_transform(int flag, int in_len, unsigned char *in_buf, unsigned char *out_buf);

        static int volume_control(short *out_buf, short *in_buf, int in_len, float in_vol);

    private:
        static ReSampleContext *m_context;
        static void init_mono_resample(ReSampleChannelContext *s, float ratio);

        static int
        fractional_resample(ReSampleChannelContext *s, short *output, short *input, int nb_samples);

        static int
        integer_downsample(ReSampleChannelContext *s, short *output, short *input, int nb_samples);

        static void stereo_to_mono(short *output, short *input, int n1);

        static void mono_to_stereo(short *output, short *input, int n1);

        static void stereo_split(short *output1, short *output2, short *input, int n);

        static void stereo_mux(short *output, short *input1, short *input2, int n);

        static void ac3_5p1_mux(short *output, short *input1, short *input2, int n);

        static int
        mono_resample(ReSampleChannelContext *s, short *output, short *input, int nb_samples);

        static int mono_16bit_to_8bit(unsigned char *lp8bits, short *lp16bits, int len);

        static int mono_8bit_to_16bit(short *lp16bits, unsigned char *lp8bits, int len);

    };
}
#endif //MEDIAENGINE_PCMPROCESSOR_H
