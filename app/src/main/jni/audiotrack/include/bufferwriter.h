//
// Created by ASUS on 2018/4/20.
//

#ifndef MEDIAENGINE_BUFFERWRITER_H
#define MEDIAENGINE_BUFFERWRITER_H

#include <SLES/OpenSLES_Android.h>
#include <safequeue.h>
#include <thread.h>
#include <enum.h>
#include "aengine.h"

namespace paomiantv {

    class CBufferWriter {
    public:
        static CBufferWriter *
        Create(EMSampleRate eSampleRate, u16 wEncoding, u32 uChannel, u32 uChannelMask);

        void destroy();

        BOOL32 writeBuffer(u8 *in, int size);

        void setOnWriteCB(OnMessageCB cbOnMessage, OnWritePCMCB cbOnWrite, void *cbDelegate);

    protected:
        void onWriter(u64 sllTimeStamp, s32 nSize, u8 *pBuffer, BOOL32 isEOS);

    private:
        CBufferWriter(EMSampleRate eSampleRate, u16 wEncoding, u32 uChannel,
                      u32 uChannelMask);

        virtual ~CBufferWriter();

        void release();

        BOOL32 prepare();


    protected:
        const EMSampleRate m_eSampleRate;
        const u32 m_uChannel;
        const u16 m_wEncoding;
        const u32 m_uChannelMask;

        ILock *m_pLock;
        BOOL32 m_bIsStopped;

        u8 *m_pbyFrameBuffer;

        u32 m_uOneFrameCapacity;// this can not modify,just init in configure

        u8 *m_pbyRemainderBuffer;
        u32 m_uRemainderBufferSize;

        u64 m_ullOneFrameDurationUS;
        u64 m_ullPlayTimeStampUS;

        OnMessageCB m_cbOnMessage;
        OnWritePCMCB m_cbOnWrite;
        void *m_cbDelegate;

        void onSendEOSMessage();

        void onSavedMessage(u64 ullTimeStamp);
    };
}


#endif //MEDIAENGINE_BUFFERWRITER_H
