//
// Created by ASUS on 2018/4/16.
//

#ifndef MEDIAENGINE_SOUND_H
#define MEDIAENGINE_SOUND_H

#include <typedef.h>
#include <autolock.h>
#include <vector>
#include <algorithm>
#include <enum.h>
#include <afilter.h>

#define AUDIO_TRACK_HWAAC_BUFFER_SIZE 5


namespace paomiantv {

    class CAudio {
    public:
        u8 *m_pbyVoice;
        //buffer capacity
        u32 m_uCapacity;
        u32 m_uOffset;
        u32 m_uSize;
        u32 m_uTrackId;
        TAudioParams m_tParams;
        BOOL32 isEOS;
    protected:
        CAudio() {
            isEOS = FALSE;
            m_uTrackId = 0;
            m_pbyVoice = NULL;
            m_uCapacity = 0;
            m_uOffset = 0;
            m_uSize = 0;
            m_tParams.m_eSampleRate = EM_SAMPLE_RATE_START;
            m_tParams.m_ullChannelLayout = 0;
            m_tParams.m_eFormat = AV_SAMPLE_FMT_NONE;
            m_pLock = new CLock;
        }

    public:
        virtual ~CAudio() {
            m_pLock->lock();
            if (m_pbyVoice != NULL) {
                free(m_pbyVoice);
            }
            m_pLock->unlock();
            delete m_pLock;
        }

        virtual void resize(const u32 size) {
            CAutoLock autoLock(m_pLock);
            if (size > 0) {
                if (m_pbyVoice == NULL) {
                    m_pbyVoice = (u8 *) malloc(size);
                } else if (m_uCapacity < size) {
                    free(m_pbyVoice);
                    m_pbyVoice = (u8 *) malloc(size);
                }
                m_uCapacity = MAX(size, m_uCapacity);
                memset(m_pbyVoice, 0, m_uCapacity);
            } else {
                if (m_pbyVoice != NULL) {
                    free(m_pbyVoice);
                    m_pbyVoice = NULL;
                }
                m_uCapacity = 0;
            }
            isEOS = FALSE;
            m_uTrackId = 0;
            m_uOffset = 0;
            m_uSize = 0;
            m_tParams.m_eSampleRate = EM_SAMPLE_RATE_START;
            m_tParams.m_ullChannelLayout = 0;
            m_tParams.m_eFormat = AV_SAMPLE_FMT_NONE;
        }

        static CAudio *create() {
            CAudio *re = NULL;
            m_sLock.lock();

            if (!m_svPool.empty()) {
                re = m_svPool.back();
                m_svPool.pop_back();
            } else {
                re = new CAudio();
            }
            m_sLock.unlock();
            return re;
        }

        static void release(CAudio *audio) {
            if (audio != NULL) {
                audio->resize(audio->m_uCapacity);
                m_sLock.lock();
                std::vector<CAudio *>::iterator it;
                it = std::find(m_svPool.begin(), m_svPool.end(), audio);
                if (it == m_svPool.end()) {
                    //vec中不存在value值
                    m_svPool.push_back(audio);
                }
                m_sLock.unlock();
            }
        }

        static void clear() {
            m_sLock.lock();
            while (!m_svPool.empty()) {
                CAudio *audio = m_svPool.back();
                m_svPool.pop_back();
                if (audio != NULL) {
                    m_sLock.unlock();
                    delete audio;
                    m_sLock.lock();
                }
            }
            m_svPool.clear();
            m_sLock.unlock();
        }

    private:
        static std::vector<CAudio *> m_svPool;
        static CLock m_sLock;
        ILock *m_pLock;
    };

    class CSound {
    public:
        EMSampleRate m_eSampleRate;
        u8 m_byChannel;
        u32 m_uFormat;
        //microsecond
        s64 m_sllTimeStampUS;
        BOOL32 isEOS;
        std::vector<CAudio *> m_vAudios;
    protected:
        CSound() : m_eSampleRate(EM_SAMPLE_RATE_START),
                   m_byChannel(0),
                   m_uFormat(0),
                   m_sllTimeStampUS(0),
                   isEOS(FALSE) {
            m_vAudios.clear();
            m_pLock = new CLock;
        }

        virtual ~CSound() {
            m_pLock->lock();
            for (auto it = m_vAudios.begin(); it != m_vAudios.end(); it++) {
                CAudio::release(*it);
            }
            m_vAudios.clear();
            m_pLock->unlock();
            delete m_pLock;
        }

    public:
        virtual void reset() {
            CAutoLock autoLock(m_pLock);
            for (auto it = m_vAudios.begin(); it != m_vAudios.end(); it++) {
                CAudio::release(*it);
            }
            m_vAudios.clear();
            isEOS = FALSE;
            m_eSampleRate = EM_SAMPLE_RATE_START;
            m_byChannel = 0;
            m_uFormat = 0;
            m_sllTimeStampUS = 0;
        }

        static CSound *create() {
            CSound *re = NULL;
            m_sLock.lock();

            if (!m_svPool.empty()) {
                re = m_svPool.back();
                m_svPool.pop_back();
            } else {
                re = new CSound();
            }
            m_sLock.unlock();
            return re;
        }

        static void release(CSound *pSound) {
            if (pSound != NULL) {
                pSound->reset();
                m_sLock.lock();
                std::vector<CSound *>::iterator it;
                it = std::find(m_svPool.begin(), m_svPool.end(), pSound);
                if (it == m_svPool.end()) {
                    //vec中不存在value值
                    m_svPool.push_back(pSound);
                }
                m_sLock.unlock();
            }
        }

        static void clear() {
            m_sLock.lock();
            while (!m_svPool.empty()) {
                CSound *sound = m_svPool.back();
                m_svPool.pop_back();
                if (sound != NULL) {
                    m_sLock.unlock();
                    delete sound;
                    m_sLock.lock();
                }
            }
            m_svPool.clear();
            m_sLock.unlock();
        }

    private:
        static std::vector<CSound *> m_svPool;
        static CLock m_sLock;
        ILock *m_pLock;
    };

    class CSoundManager {
    private:

        CSoundManager() {
        }

        virtual ~CSoundManager() {
        }

    public:
        static void clearSound() {
            CSound::clear();
            CAudio::clear();
        }
    };
}
#endif //MEDIAENGINE_SOUND_H
