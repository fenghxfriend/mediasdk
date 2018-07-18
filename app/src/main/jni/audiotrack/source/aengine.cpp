//
// Created by ASUS on 2018/4/16.
//

#include <aengine.h>
#include <autolog.h>

namespace paomiantv {
    CAEngine::CAEngine() {
        USE_LOG;
        createEngine();
    }

    CAEngine::~CAEngine() {
        USE_LOG;
        releaseEngine();
    }

    // create the engine and output mix objects
    void CAEngine::createEngine() {
        SLresult result;
        do {
            // create engine
            result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
            if (SL_RESULT_SUCCESS != result)break;

            // realize the engine
            result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
            if (SL_RESULT_SUCCESS != result)break;

            // get the engine interface, which is needed in order to create other objects
            result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
            if (SL_RESULT_SUCCESS != result)break;

            // create output mix, with environmental reverb specified as a non-required interface
            const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
            const SLboolean req[1] = {SL_BOOLEAN_FALSE};
            result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
            if (SL_RESULT_SUCCESS != result)break;

            // realize the output mix
            result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
            if (SL_RESULT_SUCCESS != result)break;

            // get the environmental reverb interface
            // this could fail if the environmental reverb effect is not available,
            // either because the feature is not present, excessive CPU load, or
            // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
            result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                                      &outputMixEnvironmentalReverb);
            if (SL_RESULT_SUCCESS != result)break;


            result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                    outputMixEnvironmentalReverb, &reverbSettings);

            if (SL_RESULT_SUCCESS != result)break;
            // ignore unsuccessful result codes for environmental reverb, as it is optional for this example
        } while (0);
        if (SL_RESULT_SUCCESS != result) {
            LOGE("create openSL engine failed!")
            releaseEngine();
        }
    }

// shut down the native audio system
    void CAEngine::releaseEngine() {
        // destroy output mix object, and invalidate all associated interfaces
        if (outputMixObject != NULL) {
            (*outputMixObject)->Destroy(outputMixObject);
            outputMixObject = NULL;
            outputMixEnvironmentalReverb = NULL;
        }

        // destroy engine object, and invalidate all associated interfaces
        if (engineObject != NULL) {
            (*engineObject)->Destroy(engineObject);
            engineObject = NULL;
            engineEngine = NULL;
        }
    }

}