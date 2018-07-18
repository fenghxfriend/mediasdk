//
// Created by ASUS on 2018/4/17.
//

#ifndef MEDIAENGINE_ABUFFER_H
#define MEDIAENGINE_ABUFFER_H

#include <afilter.h>
#include <enum.h>

namespace paomiantv {
    class CABuffer : public IAFilter {
    protected:
        CABuffer();

        virtual s32 init(const CGraph *pGraph);

    public:
        virtual ~CABuffer();

        static CABuffer *Create(const CGraph *pGraph);

        virtual void destroy();

        s32 setParams(EMSampleRate uSrcSampleRate, AVSampleFormat eSrcFormat,
                      u64 ullSrcChannelLayout, s16 wWeight);

        inline EMSampleRate getSrcSampleRate();

        inline AVSampleFormat getSrcSampleFormat();

        inline u64 getSrcChannelLayout();

        inline s16 getWeight();

    private:
        EMSampleRate m_eSrcSampleRate;
        AVSampleFormat m_eSrcFormat;
        u64 m_ullSrcChannelLayout;
        s16 m_wWeight;
    };

    inline EMSampleRate CABuffer::getSrcSampleRate() {
        return m_eSrcSampleRate;
    }

    inline AVSampleFormat CABuffer::getSrcSampleFormat() {
        return m_eSrcFormat;
    }

    inline u64 CABuffer::getSrcChannelLayout() {
        return m_ullSrcChannelLayout;
    }

    inline s16 CABuffer::getWeight() {
        return m_wWeight;
    }

}


#endif //MEDIAENGINE_ABUFFER_H
