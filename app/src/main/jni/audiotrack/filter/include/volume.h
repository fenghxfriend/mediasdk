//
// Created by ASUS on 2018/4/16.
//

#ifndef MEDIAENGINE_AVOLUME_H
#define MEDIAENGINE_AVOLUME_H

#include <afilter.h>
#include "aformat.h"

namespace paomiantv {
    class CVolume : public IAFilter {
    protected:
        CVolume();

        virtual ~CVolume();

        virtual s32 init(const CGraph *pGraph);

    public:

        static CVolume *Create(const CGraph *pGraph);

        virtual void destroy();

        s32 setParams(float fVolume);
    };
}


#endif //MEDIAENGINE_AVOLUME_H
