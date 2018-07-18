//
// Created by ASUS on 2018/4/20.
//

#ifndef MEDIAENGINE_AMULTIDEC_H
#define MEDIAENGINE_AMULTIDEC_H


#include "adec.h"
#include "../../module/include/track.h"

namespace paomiantv {
    class CAMultiDec : public CADec {
    public:
        CAMultiDec(ITrack *const &pTrack);

        virtual ~CAMultiDec();

        virtual BOOL32 getAudio(CAudio *&pAudio);

        virtual int prepare();

        virtual void release();

        virtual void start();

        virtual void stop();

        virtual void pause();

        virtual void resume();

    private:
        int m_nIndex;
        std::vector<CDec *> m_vADecs;
    };

}


#endif //MEDIAENGINE_AMULTIDEC_H
