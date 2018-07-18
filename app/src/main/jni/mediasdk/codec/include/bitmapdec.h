//
// Created by ASUS on 2018/5/4.
//

#ifndef MEDIAENGINE_BITMAPDEC_H
#define MEDIAENGINE_BITMAPDEC_H

#include "dec.h"

namespace paomiantv {

    class CVBitmapDec : public CDec {
    public:

        CVBitmapDec(ITrack *const &pTrack);

        virtual ~CVBitmapDec();

        virtual int prepare();

        virtual void release();

        virtual void start();

        virtual void stop();

        virtual void pause();

        virtual void resume();

        virtual BOOL32 getLayer(CVLayerParam *&pLayer);

        BOOL32 getAudio(CAudio *&pAudio);

        virtual BOOL32 getRemainderBuffer(u8 *&out, u32 &size);

    protected:

        s64 m_sllCurrPlayUS;

        s64 m_sllStartPlayUS;
        s64 m_sllEndPlayUS;
    protected:
        void setParams(CVLayerParam *layerparam);

    private:

        s64 m_sllEndCutUS;

        AVPixelFormat m_eDecoderOutputFmt;

        u32 m_uWidth;
        u32 m_uHeight;
        EMPixelFormat m_ePixFmt;
    };
}


#endif //MEDIAENGINE_BITMAPDEC_H
