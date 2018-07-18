//
// Created by ASUS on 2018/4/20.
//

#include <bufferwriter.h>
#include <unistd.h>
#include <cstdio>

namespace paomiantv {

    CBufferWriter *
    CBufferWriter::Create(EMSampleRate eSampleRate, u16 wEncoding, u32 uChannel,
                          u32 uChannelMask) {
        CBufferWriter *p = new CBufferWriter(eSampleRate, wEncoding, uChannel, uChannelMask);
        if (p->prepare()) {
            return p;
        } else {
            delete p;
            return NULL;
        }
    }

    CBufferWriter::CBufferWriter(EMSampleRate eSampleRate, u16 wEncoding, u32 uChannel,
                                 u32 uChannelMask) :
            m_pbyRemainderBuffer(NULL),
            m_pbyFrameBuffer(NULL),
            m_uRemainderBufferSize(0),
            m_ullPlayTimeStampUS(0),
            m_bIsStopped(FALSE),
            m_ullOneFrameDurationUS(0),
            m_eSampleRate(eSampleRate),
            m_uChannel(uChannel),
            m_wEncoding(wEncoding),
            m_uChannelMask(uChannelMask),
            m_cbOnWrite(NULL),
            m_cbOnMessage(NULL),
            m_cbDelegate(NULL) {
        USE_LOG;
        m_pLock = new CLock;
    }

    CBufferWriter::~CBufferWriter() {
        USE_LOG;
        release();
        if (m_pLock != NULL) {
            delete m_pLock;
            m_pLock = NULL;
        }
    }

    void CBufferWriter::destroy() {
        delete this;
    }

    void CBufferWriter::release() {
        // destroy buffer queue audio player object, and invalidate all associated interfaces
        CAutoLock autoLock(m_pLock);
        m_bIsStopped = TRUE;
        if (m_pbyRemainderBuffer != NULL) {
            free(m_pbyRemainderBuffer);
            m_pbyRemainderBuffer = NULL;
        }
        m_uRemainderBufferSize = 0;
        m_ullOneFrameDurationUS = 0;
        m_ullPlayTimeStampUS = 0;
    }

    BOOL32
    CBufferWriter::prepare() {
        CAutoLock autoLock(m_pLock);
        m_uOneFrameCapacity = AUDIO_SAMPLE_COUNT_PER_FRAME * m_wEncoding * m_uChannel;
        m_pbyRemainderBuffer = (u8 *) malloc(m_uOneFrameCapacity);
        if (m_pbyRemainderBuffer == NULL) {
            return FALSE;
        }
        memset(m_pbyRemainderBuffer, 0, m_uOneFrameCapacity);
        m_pbyFrameBuffer = (u8 *) malloc(m_uOneFrameCapacity);
        if (m_pbyFrameBuffer == NULL) {
            return FALSE;
        }
        memset(m_pbyFrameBuffer, 0, m_uOneFrameCapacity);
        m_ullOneFrameDurationUS = AUDIO_SAMPLE_COUNT_PER_FRAME * 1000 * 1000 / m_eSampleRate;
        return TRUE;
    }

    BOOL32 CBufferWriter::writeBuffer(u8 *in, int size) {
        CAutoLock autoLock(m_pLock);
        // handle the processed data
        if (m_bIsStopped) {
            return FALSE;
        }
        if (in == NULL || size == 0) {
            if (m_uRemainderBufferSize > 0) {
                memcpy(m_pbyFrameBuffer, m_pbyRemainderBuffer, m_uRemainderBufferSize);
            }
            onWriter(m_ullPlayTimeStampUS, m_uRemainderBufferSize, m_pbyFrameBuffer, TRUE);
            onSendEOSMessage();
            m_uRemainderBufferSize = 0;
            m_ullPlayTimeStampUS = 0;
            return TRUE;
        }
        u32 sum = size + m_uRemainderBufferSize;
        u32 count = sum / m_uOneFrameCapacity;
        u32 rmd = sum % m_uOneFrameCapacity;

        u32 outOffset = 0;
        for (u32 i = 0; i < count; i++) {
            memset(m_pbyFrameBuffer, 0, m_uOneFrameCapacity);
            if (m_uRemainderBufferSize > 0) {
                memcpy(m_pbyFrameBuffer, m_pbyRemainderBuffer,
                       m_uRemainderBufferSize);
                memcpy(m_pbyFrameBuffer + m_uRemainderBufferSize, in,
                       m_uOneFrameCapacity - m_uRemainderBufferSize);
                outOffset += (m_uOneFrameCapacity - m_uRemainderBufferSize);
                m_uRemainderBufferSize = 0;
            } else {
                memcpy(m_pbyFrameBuffer, in + outOffset, m_uOneFrameCapacity);
                outOffset += m_uOneFrameCapacity;
            }
            onWriter(m_ullPlayTimeStampUS, m_uOneFrameCapacity, m_pbyFrameBuffer, FALSE);
            onSavedMessage(m_ullPlayTimeStampUS);
            m_ullPlayTimeStampUS += m_ullOneFrameDurationUS;
        }
#ifdef LOG_ENABLE
        if (rmd != size - outOffset + m_uRemainderBufferSize) {
            LOGE("this situation will never happend!");
        }
#endif
        memcpy(m_pbyRemainderBuffer + m_uRemainderBufferSize, in + outOffset,
               size - outOffset);
        m_uRemainderBufferSize += (size - outOffset);
        return TRUE;
    }

    void CBufferWriter::onSendEOSMessage() {
        CAutoLock autoLock(m_pLock);
        if (m_cbOnMessage != NULL && m_cbDelegate != NULL) {
            char message[128];
            sprintf(message, "{isEOS:%d}", 1);
            m_cbOnMessage(m_cbDelegate, EM_MESSAGE_ID_A_PLAY_COMPLETE, message);
        }
    }

    void CBufferWriter::onSavedMessage(u64 ullTimeStamp) {
        CAutoLock autoLock(m_pLock);
        if (m_cbOnMessage != NULL && m_cbDelegate != NULL) {
            char message[128];
            sprintf(message, "{timestamp:%llu}", ullTimeStamp / 1000 + 1);
            m_cbOnMessage(m_cbDelegate, EM_MESSAGE_ID_A_SAVE_PLAY_PROGRESS, message);
        }
    }

    void CBufferWriter::onWriter(u64 ullTimeStamp, s32 nSize, u8 *pBuffer, BOOL32 isEOS) {
        CAutoLock autoLock(m_pLock);
        if (m_cbOnWrite != NULL && m_cbDelegate != NULL) {
            m_cbOnWrite(m_cbDelegate, ullTimeStamp, nSize, pBuffer, isEOS);
        }
    }

    void
    CBufferWriter::setOnWriteCB(OnMessageCB cbOnMessage, OnWritePCMCB cbOnWrite, void *cbDelegate) {
        CAutoLock autoLock(m_pLock);
        m_cbOnMessage = cbOnMessage;
        m_cbOnWrite = cbOnWrite;
        m_cbDelegate = cbDelegate;
    }
}