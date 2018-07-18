//
// Created by ASUS on 2018/6/12.
//

#ifndef MEDIAENGINE_CMESSAGE_H
#define MEDIAENGINE_CMESSAGE_H

#include <typedef.h>
#include <algorithm>
#include "event.h"

#define MAX_MESSAGE_CONTENT 8192
namespace paomiantv {
    class CMessage {
    public:
        static CMessage *create();

        static void release(CMessage *message);

        static void clear();

    protected:
        CMessage();

        virtual ~CMessage();

        bool validate(u32 id, u32 tarId, s32 nId, u32 size);

    public:
        virtual bool send(u32 uSrcId, u32 uTarId, s32 nId, u32 uSize, void *pData);

        void reset();

        u32 m_uSrcId;//the source module id
        u32 m_uTarId;//the target module id
        s32 m_nId; // the message (event) id
        u32 m_uSize; // the data size (for validate the data)
        void *m_pData; // message data content (this memory have to be assigned by malloc)

    private:
        static std::vector<CMessage *> m_svPool;
        static CLock m_sLock;
    };
}

#endif //MEDIAENGINE_CMESSAGE_H
