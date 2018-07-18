//
// Created by ASUS on 2018/4/17.
//

#include <autolog.h>
#include "graph.h"

namespace paomiantv {
    CGraph *CGraph::Create() {
        CGraph *p = new CGraph;
        if (p->init()) {
            return p;
        } else {
            delete p;
            return NULL;
        }
    }

    void CGraph::destroy() {
        delete this;
    }

    CGraph::CGraph() : m_pGraph(NULL) {
        USE_LOG;
    }

    CGraph::~CGraph() {
        USE_LOG;
        uninit();
    }

    BOOL32 CGraph::init() {
        /* Create a new filtergraph, which will contain all the filters. */
        m_pGraph = avfilter_graph_alloc();
        if (!m_pGraph) {
            LOGE("Unable to create filter graph.No memory\n");
            return FALSE;
        }
        return TRUE;
    }

    void CGraph::uninit() {
        if (&m_pGraph != NULL) {
            avfilter_graph_free(&m_pGraph);
        }
    }
}

