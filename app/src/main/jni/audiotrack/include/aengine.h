//
// Created by ASUS on 2018/4/16.
//

#ifndef MEDIAENGINE_OPENSLENGINE_H
#define MEDIAENGINE_OPENSLENGINE_H


#include <SLES/OpenSLES.h>
#include <typedef.h>

namespace paomiantv {

    class CAEngine {
    public:
        CAEngine();

        virtual ~CAEngine();

    private:
        // engine interfaces
        SLObjectItf engineObject = NULL;
        SLEngineItf engineEngine;

// output mix interfaces
        SLObjectItf outputMixObject = NULL;
        SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

// aux effect on the output mix, used by the buffer queue player
        const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;

        void createEngine();

        void releaseEngine();

    public:
        inline const SLEngineItf_ *const *getEngineObject() const;

        inline const SLObjectItf_ *const *getMixObject() const;
    };

    inline const SLEngineItf_ *const *CAEngine::getEngineObject() const {
        return engineEngine;
    }

    inline const SLObjectItf_ *const *CAEngine::getMixObject() const {
        return outputMixObject;
    }
}


#endif //MEDIAENGINE_OPENSLENGINE_H
