//
// Created by ASUS on 2018/4/16.
//

#ifndef MEDIAENGINE_AST_H
#define MEDIAENGINE_AST_H

#include "SoundTouch.h"

using namespace soundtouch;

namespace paomiantv {
    class CASoundTouch {
    public:
        CASoundTouch();

        virtual ~CASoundTouch();

        void setTempo(float tempo);

        void setPitchSemiTones(float pitch);

        void setSpeed(float speed);

    private:
        SoundTouch *m_pSt;
    };
}


#endif //MEDIAENGINE_AST_H
