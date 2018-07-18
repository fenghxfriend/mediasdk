//
// Created by ASUS on 2018/6/7.
//

#include "cutter.h"

namespace paomiantv {
    CCutter::CCutter(MP4FileHandle pFile) : m_pDstFile(pFile), m_bIsStarted(FALSE),
                                            m_bIsStopped(FALSE),
                                            m_bIsPaused(FALSE), m_cbDelegate(NULL),
                                            m_cbOnMessage(NULL) {
        m_pLock = new CLock;
        m_pThread = new CThread(ThreadWrapper, this);
        m_vTracks.clear();
    }

    CCutter::~CCutter() {
        stop();
        release();
        if (m_pThread != NULL) {
            m_pThread->join();
            delete m_pThread;
            m_pThread = NULL;
        }
        m_vTracks.clear();
        if (m_pLock != NULL) {
            delete m_pLock;
            m_pLock = NULL;
        }
    }

    void *CCutter::ThreadWrapper(void *pData) {
        CCutter *p = (CCutter *) pData;
        return (void *) p->run();
    }

    long CCutter::run() {
        m_pThread->setName("MP4Cutter");
        LOGI("MP4Cutter is started");
        if (!MP4_IS_VALID_FILE_HANDLE(m_pSrcFile)) {
            LOGE("read source failed!");
            return -1;
        }
        m_pLock->lock();
        m_bIsStarted = TRUE;

        for (u32 i = 0; i < m_vTracks.size() && !m_bIsStopped; i++) {
            MP4Timestamp trackStartTime = MP4ConvertToTrackTimestamp(
                    m_pSrcFile,
                    m_vTracks[i],
                    m_sllStartTimeUS,
                    MP4_USECS_TIME_SCALE);

            MP4Duration trackDuration = MP4ConvertToTrackDuration(
                    m_pSrcFile,
                    m_vTracks[i],
                    m_sllDurationUS,
                    MP4_USECS_TIME_SCALE);

            MP4TrackId dstTrackId = MP4CloneTrack(m_pSrcFile, m_vTracks[i], m_pDstFile);

            if (!MP4_IS_VALID_TRACK_ID(dstTrackId)) {
                continue;
            }

            MP4Timestamp when = 0;
            const MP4SampleId numSamples = MP4GetTrackNumberOfSamples(m_pSrcFile, m_vTracks[i]);
            MP4SampleId sampleId = MP4GetSampleIdFromTime(m_pSrcFile, m_vTracks[i], trackStartTime,
                                                          true);

            while (!m_bIsStopped) {
                while (!m_bIsStopped && m_bIsPaused) {
                    m_pLock->wait(1000);
                }
                if (!m_bIsStopped) {
                    m_pLock->unlock();
                    if (when >= trackDuration) {
                        break;
                    }
                    if (sampleId > numSamples) {
                        break;
                    }
                    MP4Duration sampleDuration = MP4GetSampleDuration(m_pSrcFile, m_vTracks[i],
                                                                      sampleId);
                    bool rc = MP4CopySample(
                            m_pSrcFile,
                            m_vTracks[i],
                            sampleId,
                            m_pDstFile,
                            dstTrackId,
                            sampleDuration);
                    if (!rc) {
                        MP4DeleteTrack(m_pDstFile, dstTrackId);
                        break;
                    }
                    sampleId++;
                    when += sampleDuration;
                    m_pLock->lock();
                }

            }
        }
        if (!m_bIsStopped && m_cbOnMessage != NULL && m_cbDelegate != NULL) {
            switch (m_eType) {
                case EM_TRACK_VIDEO: {
                    m_cbOnMessage(m_cbDelegate, EM_MESSAGE_ID_V_CUT_RENDER_COMPLETE, NULL);
                }
                    break;
                case EM_TRACK_AUDIO: {
                    m_cbOnMessage(m_cbDelegate, EM_MESSAGE_ID_A_CUT_PLAY_COMPLETE, NULL);
                }
                    break;
                default:
                    break;
            }
        }
        m_bIsStarted = FALSE;
        m_bIsStopped = FALSE;
        LOGI("MP4Cutter is stopped");
        m_pLock->unlock();
        return 0;
    }

    void CCutter::start() {
        LOGI("MP4Cutter::startThread");
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted) {
            return;
        }
        if (!m_pThread->start()) {
            LOGE("start MP4Cutter thread failed!");
            return;
        }
    }

    void CCutter::stop() {
        char name[64];
        memset(name, 0, sizeof(name));
        m_pThread->getName(name);
        LOGI("%s::stopThread", name);
        m_pLock->lock();
        if (m_bIsStarted && !m_bIsStopped) {
            m_bIsStopped = TRUE;
            m_pLock->acttive();
        }
        m_pLock->unlock();
        m_pThread->join();
    }

    void CCutter::resume() {
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted && !m_bIsStopped && m_bIsPaused) {
            m_bIsPaused = FALSE;
            m_pLock->acttive();
        }
    }

    void CCutter::pause() {
        CAutoLock autoLock(m_pLock);
        if (m_bIsStarted && !m_bIsStopped && !m_bIsPaused) {
            m_bIsPaused = TRUE;
        }
    }

    BOOL32 CCutter::prepare(s8 *pSrcPath, EMTrack type, s64 startTimeUS, s64 durationUS,
                            OnMessageCB cbOnMessage, void *cbDelegate) {
        CAutoLock autoLock(m_pLock);
        if (pSrcPath == NULL || strlen(pSrcPath) <= 0 || durationUS < 0 ||
            type < EM_TRACK_VIDEO ||
            type > EM_TRACK_AUDIO) {
            LOGE("parameter is invalid!");
            return FALSE;
        }

        m_pSrcFile = MP4Read(pSrcPath);

        if (!MP4_IS_VALID_FILE_HANDLE(m_pSrcFile)) {
            LOGE("read source failed!");
            return FALSE;
        }
        m_eType = type;
        m_sllStartTimeUS = startTimeUS;
        m_sllDurationUS = durationUS;
        u32 uTrackNum = MP4GetNumberOfTracks(m_pSrcFile);
        MP4TrackId trackId = MP4_INVALID_TRACK_ID;
        for (u32 i = 0; i < uTrackNum && !m_bIsStopped; i++) {
            trackId = MP4FindTrackId(m_pSrcFile, i);
            const s8 *trackType = MP4GetTrackType(m_pSrcFile, trackId);
            if (m_eType == EM_TRACK_AUDIO && MP4_IS_AUDIO_TRACK_TYPE(trackType)) {
                m_vTracks.push_back(trackId);
            }
            if (m_eType == EM_TRACK_VIDEO && MP4_IS_VIDEO_TRACK_TYPE(trackType)) {
                m_vTracks.push_back(trackId);
            }
        }
        m_cbDelegate = cbDelegate;
        m_cbOnMessage = cbOnMessage;
        return TRUE;
    }

    void CCutter::release() {
        CAutoLock autoLock(m_pLock);
        if (MP4_IS_VALID_FILE_HANDLE(m_pSrcFile)) {
            MP4Close(m_pSrcFile);
        }
    }
}