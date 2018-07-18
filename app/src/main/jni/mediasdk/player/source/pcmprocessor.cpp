//
// Created by John.Huang on 2017/9/26.
//

#include "pcmprocessor.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "../../../common/autolog.h"
namespace paomiantv {
    ReSampleContext* CPCMProcessor::m_context = NULL;
    void CPCMProcessor::init_mono_resample(ReSampleChannelContext *s, float ratio) {
        ratio = (float) (1.0 / ratio);
        s->iratio = (int) floorf(ratio);
        if (s->iratio == 0){
            s->iratio = 1;
        }
        s->incr = (int) ((ratio / s->iratio) * FRAC);
        s->frac = FRAC;
        s->last_sample = 0;
        s->icount = s->iratio;
        s->isum = 0;
        s->inv = (FRAC / s->iratio);
    }

    /* fractional audio resampling */
    int CPCMProcessor::fractional_resample(ReSampleChannelContext *s, short *output, short *input,
                                           int nb_samples) {
        unsigned int frac, incr;
        int l0, l1;
        short *q, *p, *pend;
        l0 = s->last_sample;
        incr = s->incr;
        frac = s->frac;
        p = input;
        pend = input + nb_samples;
        q = output;
        l1 = *p++;
        for (;;) {
            /* interpolate */
            *q++ = (l0 * (FRAC - frac) + l1 * frac) >> FRAC_BITS;
            frac = frac + s->incr;
            while (frac >= FRAC) {
                frac -= FRAC;
                if (p >= pend)
                    goto the_end;
                l0 = l1;
                l1 = *p++;
            }
        }
        the_end:
        s->last_sample = l1;
        s->frac = frac;
        return q - output;
    }

    int CPCMProcessor::integer_downsample(ReSampleChannelContext *s, short *output, short *input,
                                          int nb_samples) {
        short *q, *p, *pend;
        int c, sum;
        p = input;
        pend = input + nb_samples;
        q = output;
        c = s->icount;
        sum = s->isum;
        for (;;) {
            sum += *p++;
            if (--c == 0) {
                *q++ = (sum * s->inv) >> FRAC_BITS;
                c = s->iratio;
                sum = 0;
            }
            if (p >= pend)
                break;
        }
        s->isum = sum;
        s->icount = c;
        return q - output;
    }

    /* n1: number of samples */
    void CPCMProcessor::stereo_to_mono(short *output, short *input, int n1) {
        short *p, *q;
        int n = n1;
        p = input;
        q = output;
        while (n >= 4) {
            q[0] = (p[0] + p[1]) >> 1;
            q[1] = (p[2] + p[3]) >> 1;
            q[2] = (p[4] + p[5]) >> 1;
            q[3] = (p[6] + p[7]) >> 1;
            q += 4;
            p += 8;
            n -= 4;
        }
        while (n > 0) {
            q[0] = (p[0] + p[1]) >> 1;
            q++;
            p += 2;
            n--;
        }
    }

    /* n1: number of samples */
    void CPCMProcessor::mono_to_stereo(short *output, short *input, int n1) {
        short *p, *q;
        int n = n1;
        int v;
        p = input;
        q = output;
        while (n >= 4) {
            v = p[0];
            q[0] = v;
            q[1] = v;
            v = p[1];
            q[2] = v;
            q[3] = v;
            v = p[2];
            q[4] = v;
            q[5] = v;
            v = p[3];
            q[6] = v;
            q[7] = v;
            q += 8;
            p += 4;
            n -= 4;
        }
        while (n > 0) {
            v = p[0];
            q[0] = v;
            q[1] = v;
            q += 2;
            p += 1;
            n--;
        }
    }

    /* XXX: should use more abstract 'N' channels system */
    void CPCMProcessor::stereo_split(short *output1, short *output2, short *input, int n) {
        int i;
        for (i = 0; i < n; i++) {
            *output1++ = *input++;
            *output2++ = *input++;
        }
    }

    void CPCMProcessor::stereo_mux(short *output, short *input1, short *input2, int n) {
        int i;
        for (i = 0; i < n; i++) {
            *output++ = *input1++;
            *output++ = *input2++;
        }
    }

    void CPCMProcessor::ac3_5p1_mux(short *output, short *input1, short *input2, int n) {
        int i;
        short l, r;
        for (i = 0; i < n; i++) {
            l = *input1++;
            r = *input2++;
            *output++ = l;           /* left */
            *output++ = (l / 2) + (r / 2); /* center */
            *output++ = r;           /* right */
            *output++ = 0;           /* left surround */
            *output++ = 0;           /* right surroud */
            *output++ = 0;           /* low freq */
        }
    }

    int CPCMProcessor::mono_resample(ReSampleChannelContext *s, short *output, short *input,
                                     int nb_samples) {
        short *buf1;
        short *buftmp;
        buf1 = (short *) av_malloc(nb_samples * sizeof(short));
        /* first downsample by an integer factor with averaging filter */
        if (s->iratio > 1) {
            buftmp = buf1;
            nb_samples = integer_downsample(s, buftmp, input, nb_samples);
        } else {
            buftmp = input;
        }
        /* then do a fractional resampling with linear interpolation */
        if (s->incr != FRAC) {
            nb_samples = fractional_resample(s, output, buftmp, nb_samples);
        } else {
            memcpy(output, buftmp, nb_samples * sizeof(short));
        }
        av_free(buf1);
        return nb_samples;
    }

//==========================================================================================================

    int CPCMProcessor::init_PCM_resample(int output_channels, int input_channels, int output_rate,
                                         int input_rate) {
        int i;
        if (input_channels > 2) {
            LOGE("Resampling with input channels greater than 2 unsupported.");
            return -1;
        }
        m_context = (ReSampleContext *) malloc(sizeof(ReSampleContext));
        memset(m_context, 0, sizeof(ReSampleContext));
        m_context->ratio = (float) output_rate / (float) input_rate;
        m_context->input_channels = input_channels;
        m_context->output_channels = output_channels;
        m_context->filter_channels = m_context->input_channels;
        if (m_context->output_channels < m_context->filter_channels)
            m_context->filter_channels = m_context->output_channels;
        /*
         * ac3 output is the only case where filter_channels could be greater than 2.
         * input channels can't be greater than 2, so resample the 2 channels and then
         * expand to 6 channels after the resampling.
         */
        if (m_context->filter_channels > 2)
            m_context->filter_channels = 2;
        for (i = 0; i < m_context->filter_channels; i++) {
            init_mono_resample(&m_context->channel_ctx[i], m_context->ratio);
        }
        return 0;
    }

    /* resample audio. 'nb_samples' is the number of input samples */
    /* XXX: optimize it ! */
    /* XXX: do it with polyphase filters, since the quality here is
     * HORRIBLE. Return the number of samples available in output */

    // 重采样
    // output para1:重采样后输出的数据
    // input  para2:输入数据
    //        para3:此帧音频的采样点数
    // 注:44100的采样点数固定为1024，其他采样率不固定，需要通道此帧长度，通道数，位宽，计算采样点数

    int CPCMProcessor::start_PCM_resample(short *output, short *input, int in_len) {
        int i, nb_samples1;
        short *bufin[2];
        short *bufout[2];
        short *buftmp2[2], *buftmp3[2];
        int lenout;
        int nb_samples = in_len / (m_context->input_channels * sizeof(short));
        if (m_context->input_channels == m_context->output_channels && m_context->ratio == 1.0) {
            /* nothing to do */
            memcpy(output, input, nb_samples * m_context->input_channels * sizeof(short));
            return nb_samples;
        }
        /* XXX: move those malloc to resample init code */
        bufin[0] = (short *) av_malloc(nb_samples * sizeof(short));
        bufin[1] = (short *) av_malloc(nb_samples * sizeof(short));
        /* make some zoom to avoid round pb */
        lenout = (int) (nb_samples * m_context->ratio) + 16;
        bufout[0] = (short *) av_malloc(lenout * sizeof(short));
        bufout[1] = (short *) av_malloc(lenout * sizeof(short));
        if (m_context->input_channels == 2 &&
            m_context->output_channels == 1) {
            buftmp2[0] = bufin[0];
            buftmp3[0] = output;
            stereo_to_mono(buftmp2[0], input, nb_samples);
        } else if (m_context->output_channels >= 2 && m_context->input_channels == 1) {
            buftmp2[0] = input;
            buftmp3[0] = bufout[0];
        } else if (m_context->output_channels >= 2) {
            buftmp2[0] = bufin[0];
            buftmp2[1] = bufin[1];
            buftmp3[0] = bufout[0];
            buftmp3[1] = bufout[1];
            stereo_split(buftmp2[0], buftmp2[1], input, nb_samples);
        } else {
            buftmp2[0] = input;
            buftmp3[0] = output;
        }
        /* resample each channel */
        nb_samples1 = 0; /* avoid warning */
        for (i = 0; i < m_context->filter_channels; i++) {
            nb_samples1 = mono_resample(&m_context->channel_ctx[i], buftmp3[i], buftmp2[i],
                                        nb_samples);
        }
        if (m_context->output_channels == 2 && m_context->input_channels == 1) {
            mono_to_stereo(output, buftmp3[0], nb_samples1);
        } else if (m_context->output_channels == 2) {
            stereo_mux(output, buftmp3[0], buftmp3[1], nb_samples1);
        } else if (m_context->output_channels == 6) {
            ac3_5p1_mux(output, buftmp3[0], buftmp3[1], nb_samples1);
        }
        av_free(bufin[0]);
        av_free(bufin[1]);
        av_free(bufout[0]);
        av_free(bufout[1]);
        return nb_samples1 * m_context->output_channels * sizeof(short);
    }

    // 去初始化
    int CPCMProcessor::uninit_PCM_resample() {
        if (NULL == m_context) {
            free(m_context);
            m_context = NULL;
        }
        return 0;
    }

    // 16bit_to_8bit(注意输出数据是有符号的还是无符号的)
    // output:   para 1: 8bit的数据
    // input:    para 2: 16bit数据
    //           para 3: 要转换数据的次数，为原数据长度的一半，因为一次转换2个字节
    int CPCMProcessor::mono_16bit_to_8bit(unsigned char *lp8bits, short *lp16bits, int len) {
        int i = 0;
        for (i = 0; i < len; i++) {
            *lp8bits++ = ((*lp16bits++) >> 8) + 128;
        }
        return i >> 1;
    }

    // 8bit_to_16bit(注意:输出数据是有符号的还是无符号的)
    // output:   para 1: 16bit的数据
    // input:    para 2: 8bit数据
    //           para 3: 要转换数据的次数，为原数据长度，
    // 注:因为一次转换1个字节变两个字节，所以转换后数据的总长度为8位数据的两倍
    int CPCMProcessor::mono_8bit_to_16bit(short *lp16bits, unsigned char *lp8bits, int len) {
        int i = 0;
        for (i = 0; i < len; i++) {
            *lp16bits++ = ((*lp8bits++) - 128) << 8;
        }
        return i << 1;
    }

    // 位宽转换
    // input : para1 重采样对象
    //         para2 flag : [8]--16位转8位 ,[16]--8位转16位
    //         para3 输入数据长度
    //         para4 输入数据
    // output: para5 输出数据
    // return 输出数据长度
    int CPCMProcessor::bit_wide_transform(int flag, int in_len, unsigned char *in_buf,
                                          unsigned char *out_buf) {
        // 转换次数，转换单位是源数据一次采样，
        // 即8位转16位:源数据是一次转换一个字节
        // 即16位转8位:源数据是一次转换两个字节
        int ns_sample = 0;
        if (8 == flag) {
            ns_sample = in_len / 2;
            mono_16bit_to_8bit(out_buf, (short *) in_buf, ns_sample);
            return ns_sample * 1;
        } else if (16 == flag) {
            ns_sample = in_len;
            mono_8bit_to_16bit((short *) out_buf, in_buf, ns_sample);
            return ns_sample * 2;
        }else{
            return ns_sample;
        }
    }

    // 音量控制
    // output: para1 输出数据
    // input : para2 输入数据
    //         para3 输入长度
    //         para4 音量控制参数,有效控制范围[0,100]
    int CPCMProcessor::volume_control(short *out_buf, short *in_buf, int in_len, float in_vol) {
        int i, tmp;
        // in_vol[0,100]
        float vol = in_vol - 98;
        if (-98 < vol && vol < 0) {
            vol = 1 / (vol * (-1));
        } else if (0 <= vol && vol <= 1) {
            vol = 1;
        }
        /*else if(1 < vol && vol <= 2)
        {
            vol = vol;
        }*/
        else if (vol <= -98) {
            vol = 0;
        } else if (2 <= vol) {
            vol = 2;
        }

        for (i = 0; i < in_len / 2; i++) {
            tmp = in_buf[i] * vol;
            if (tmp > 32767) {
                tmp = 32767;
            } else if (tmp < -32768) {
                tmp = -32768;
            }
            out_buf[i] = tmp;
        }
        return 0;
    }
}
