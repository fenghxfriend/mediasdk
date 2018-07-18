//
// Created by ASUS on 2018/4/16.
//

#include <autolog.h>
#include "ast.h"

namespace paomiantv {
    CASoundTouch::CASoundTouch() {
        USE_LOG;
        m_pSt = new SoundTouch();
    }

    CASoundTouch::~CASoundTouch() {
        USE_LOG;
        if (m_pSt != NULL) {
            delete m_pSt;
        }
    }

    void CASoundTouch::setTempo(float tempo) {
        if (m_pSt) {
            m_pSt->setTempo(tempo);
        }
    }

    void CASoundTouch::setPitchSemiTones(float pitch) {
        if (m_pSt) {
            m_pSt->setPitchSemiTones(pitch);
        }
    }

    void CASoundTouch::setSpeed(float speed) {
        if (m_pSt) {
            m_pSt->setRate(speed);
        }
    }
}
