//
// Created by ASUS on 2018/4/17.
//

#ifndef MEDIAENGINE_GRAPH_H
#define MEDIAENGINE_GRAPH_H

#include <autolock.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavfilter/avfilter.h>
#ifdef __cplusplus
}
#endif

namespace paomiantv {
    class CGraph {
    public:
        static CGraph *Create();

        virtual void destroy();

    protected:
        CGraph();

        virtual ~CGraph();

    private:
        BOOL32 init();

        void uninit();

    private:
//        ILock m_pLock;
        AVFilterGraph *m_pGraph;
    public:

        inline AVFilterGraph *getGraph() const;
    };

    inline AVFilterGraph *CGraph::getGraph() const {
        return m_pGraph;
    }
}


#endif //MEDIAENGINE_GRAPH_H
