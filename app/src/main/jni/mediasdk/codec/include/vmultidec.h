//
// Created by ASUS on 2018/4/20.
//

#ifndef MEDIAENGINE_VMULTIDEC_H
#define MEDIAENGINE_VMULTIDEC_H


#include "vdec.h"
#include "../../module/include/track.h"

namespace paomiantv {
    class CVMultiDec : public CVDec {
    public:
        CVMultiDec(ITrack *const &pTrack);

        virtual ~CVMultiDec();

        virtual BOOL32 getLayer(CVLayerParam *&pLayer);

        virtual int prepare();

        virtual void release();

        virtual void start();

        virtual void stop();

        virtual void pause();

        virtual void resume();

    private:
        int m_nIndex;
        std::vector<CDec *> m_vVDecs;
    };

}


#endif //MEDIAENGINE_VMULTIDEC_H
