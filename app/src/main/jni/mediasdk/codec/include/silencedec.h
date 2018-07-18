//
// Created by ASUS on 2018/5/16.
//

#ifndef MEDIAENGINE_SILENCEDEC_H
#define MEDIAENGINE_SILENCEDEC_H


#include "dec.h"

namespace paomiantv {

    class CASilenceDec : public CDec {
    public:

        CASilenceDec(ITrack *const &pTrack);

        virtual ~CASilenceDec();

        virtual int prepare();

        virtual void release();

        virtual void start();

        virtual void stop();

        virtual void pause();

        virtual void resume();

        virtual BOOL32 getLayer(CVLayerParam *&pLayer);

        BOOL32 getAudio(CAudio *&pAudio);

        BOOL32 getRemainderBuffer(u8 *&out, u32 &size);

    protected:

        s64 m_sllCurrPlayUS;

        s64 m_sllStartPlayUS;
        s64 m_sllEndPlayUS;

        s64 m_sllLastPTSUS;

    private:
        BOOL32 m_bIsStarted;
        BOOL32 m_bIsStopped;
        BOOL32 m_bIsPaused;

        s64 m_sllEndCutUS;
        u32 m_uPerFrameSize;
    };
}

#endif //MEDIAENGINE_SILENCEDEC_H
