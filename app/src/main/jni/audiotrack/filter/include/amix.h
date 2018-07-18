//
// Created by ASUS on 2018/4/16.
//

#ifndef MEDIAENGINE_AMIX_H
#define MEDIAENGINE_AMIX_H

#include <afilter.h>

#define MIX_SOURCE_NUM 2
namespace paomiantv {
    class CAMix : public IAFilter {
    protected:
        CAMix();

        virtual s32 init(const CGraph *pGraph);

    public:
        virtual ~CAMix();

        static CAMix *Create(const CGraph *pGraph);

        virtual void destroy();

        s32 setParams(const u32 uInputCount, const s16 *wWeight);

        inline u32 getInputCount();

    private:
        u32 m_uInputCount;
    };

    inline u32 CAMix::getInputCount() {
        return m_uInputCount;
    }
}


#endif //MEDIAENGINE_AMIX_H
