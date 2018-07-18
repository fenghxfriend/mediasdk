//
// Created by ASUS on 2018/4/13.
//

#ifndef MEDIAENGINE_MULTITRACK_H
#define MEDIAENGINE_MULTITRACK_H

#include "track.h"
#include "transition.h"

namespace paomiantv {
    typedef struct tagTrackNode {
        ITrack *m_pcTrack;
        CTransition *m_pTransition;
        tagTrackNode *m_ptPre;
        tagTrackNode *m_ptNext;
    public:
        tagTrackNode() { memset(this, 0, sizeof(struct tagTrackNode)); }
    } TTrackNode;

    class CMultiTrack : public ITrack {
    public:
        CMultiTrack();

        virtual ~CMultiTrack();

        virtual BOOL32 pushTrack(ITrack *pcTrack);

        virtual BOOL32 insertTrack(ITrack *pcTrack, u32 index);

        virtual BOOL32 popTrack(ITrack *&pcTrack, CTransition *&pcTransition);

        virtual BOOL32 removeTrackByIndex(ITrack *&pcTrack, CTransition *&pcTransition, u32 index);

        virtual BOOL32 removeTrackById(u32 id, ITrack *&pcTrack, CTransition *&pcTransition);

        virtual BOOL32 replaceTrack(ITrack *&pcTrack, u32 index);

        virtual BOOL32 getTrack(ITrack *&pcTrack, u32 index);

        virtual u32 getTrackCount();

        void clearTracks();

        virtual BOOL32 replaceTransitionByIndex(CTransition *&pcTransition, u32 index);

        virtual BOOL32 replaceTransitionById(u32 id, CTransition *&pTransition);

        virtual BOOL32 getTransition(CTransition *&pcTransition, u32 index);

        virtual s64 getDataDuration();

    private:
        TTrackNode *m_ptHead;
        TTrackNode *m_ptTail;
        u32 m_uTrackSize;
    };
}

#endif //MEDIAENGINE_MULTITRACK_H
