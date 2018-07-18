//
// Created by ASUS on 2018/4/12.
//

#ifndef _PAOMIANTV_TRACK_H_
#define _PAOMIANTV_TRACK_H_

#include <typedef.h>
#include <enum.h>
#include <autolock.h>
#include "effect.h"
#include "animation.h"
#include "transition.h"

namespace paomiantv {
    typedef struct tagTrackSource {
        // track source type
        EMSource m_eSourceType;
        BOOL32 m_bIsValid;
        // track source path/data/uri
        u8 *m_pbyData;
        u32 m_uSize;
        u32 m_uWidth;
        u32 m_uHeight;

        tagTrackSource() {
            m_bIsValid = FALSE;
            m_pbyData = NULL;
            m_eSourceType = EM_SOURCE_START;
            m_uSize = 0;
            m_uWidth = 0;
            m_uHeight = 0;
        }

        ~tagTrackSource() {
            if (m_pbyData != NULL) {
                free(m_pbyData);
            }
        }
    } TTrackSource;

    class ITrack {
    public:
        ITrack() :
                m_uId(m_sCount),
                m_eType(EM_TRACK_START),
                m_sllPlayStart(0),
                m_sllPlayDuration(-1),
                m_wWeight(1),
                m_wZIndex(1),
                m_bIsLoop(FALSE),
                m_bIsShowLastFrame(FALSE),
                m_bIsShowFirstFrame(FALSE),
                m_bIsIndependent(TRUE) {
            m_pLock = new CLock;
            m_ptTrackSource = new TTrackSource;
            if (m_sCount == UNSIGNED_INTEGER32_MAX_VALUE) {
                LOGE("the id is overflowed! please restart app!");
            } else {
                m_sCount++;
            }
        };

        virtual ~ITrack() {
            if (m_ptTrackSource != NULL) {
                delete m_ptTrackSource;
                m_ptTrackSource = NULL;
            }
            if (m_pLock != NULL) {
                delete m_pLock;
            }
        }

        bool operator()(const ITrack &x, const ITrack &y) const {
            return x.m_wZIndex < y.m_wZIndex;
        }

        inline EMTrack getType() const;

        inline const u32 getId() const;

        void setType(EMTrack type);

        void setPlayStart(s64 sllPlayStart);

        void setPlayDuration(s64 sllPlayDuration);

        void setZIndex(s16 wZIndex);

        void setWeight(s16 wWeight);

        void setShowFirstFrame(BOOL32 bShow);

        void setShowLastFrame(BOOL32 bShow);

        void setIndependent(BOOL32 bIndependent);

        void setLoop(BOOL32 bIndependent);

        BOOL32 isSourceValid() const;

        void getSourceData(u8 *&pbyData, u32 &uSize);

        EMSource getSourceType() const;

        u32 getWidth();

        u32 getHeight();

        inline s64 getPlayStart() const;

        inline s64 getPlayDuration() const;

        inline s16 getZIndex() const;

        inline s16 getWeight() const;

        inline BOOL32 isShowFirstFrame() const;

        inline BOOL32 isShowLastFrame() const;

        inline BOOL32 isIndependent() const;

        inline BOOL32 isLoop() const;

        virtual s64 getDataDuration()=0;

        static bool compareByZindexCB(ITrack *t1, ITrack *t2);

        static bool compareByIdCB(u64 id, ITrack *pTrack);

    protected:
        static u32 m_sCount;

        ILock *m_pLock;
        // track id
        const u32 m_uId;
        // type of track
        EMTrack m_eType;

        // the path of the source
        TTrackSource *m_ptTrackSource;

        // play from the timestamp of the storyboard
        s64 m_sllPlayStart;
        // play duration in the storyboard
        s64 m_sllPlayDuration;

        // the layer index of track
        s16 m_wZIndex;
        // the weight of track when the same ZIndex more than one track
        s16 m_wWeight;

        // when play start timestamp >0 show the first frame
        BOOL32 m_bIsShowFirstFrame;
        // when play start+duration timestamp < storyboard end show the last frame
        BOOL32 m_bIsShowLastFrame;

        // loop or not (true:loop play,false:play once however the play duration > cut duration)
        BOOL32 m_bIsLoop;

        // is independent track
        BOOL32 m_bIsIndependent;

    };

    inline const u32 ITrack::getId() const {
        return m_uId;
    }

    inline EMTrack ITrack::getType() const {
        return m_eType;
    }


    inline s64 ITrack::getPlayStart() const {
        return m_sllPlayStart;
    }

    inline s64 ITrack::getPlayDuration() const {
        return m_sllPlayDuration;
    }


    inline s16 ITrack::getZIndex() const {
        return m_wZIndex;
    }

    inline s16 ITrack::getWeight() const {
        return m_wWeight;
    }

    inline BOOL32 ITrack::isShowFirstFrame() const {
        return m_bIsShowFirstFrame;
    }

    inline BOOL32 ITrack::isShowLastFrame() const {
        return m_bIsShowLastFrame;
    }

    inline BOOL32 ITrack::isIndependent() const {
        return m_bIsIndependent;
    }

    inline BOOL32 ITrack::isLoop() const {
        return m_bIsLoop;
    }


    class CTrack : public ITrack {
    public:
        CTrack();

        virtual ~CTrack();

    public:

        virtual BOOL32
        setDataSource(const EMSource eType, const u8 *data = NULL, const u32 size = 0,
                      const u32 width = 0,
                      const u32 height = 0);

        void setCutStart(s64 sllStart);

        void setCutDuration(s64 sllDuration);

        void setPlayRate(float fRate);

        void setVolume(float fVolume);


        virtual s64 getCutStart() const;

        virtual s64 getCutDuration() const;

        virtual float getPlayRate() const;

        virtual float getVolume() const;


        CEffect *getEffect(s32 nIndex) const;

        BOOL32 addEffect(CEffect *pEffect);

        CEffect *removeEffectByIndex(s32 nIndex);

        CEffect *removeEffectById(s32 id);

        inline s32 getEffectCount() const;


        BOOL32 addAnimation(CAnimation *pAnimation);

        CAnimation *getAnimation(s32 nIndex) const;

        CAnimation *removeAnimationByIndex(s32 nIndex);

        CAnimation *removeAnimationById(s32 id);

        inline s32 getAnimationCount() const;

        virtual s64 getDataDuration();

    private:
        int parse();

        int updateDataDuration();

    private:
        // the path of the source
        // not useful for multi track!!!
        // cut from the timestamp of the source(must < the duration of the track)
        s64 m_sllCutStart;
        // not useful for multi track!!!
        // cut data of the duration from the source(-1: END-START)
        s64 m_sllCutDuration;

        // play rate
        float m_fPlayRate;

        // for audio
        float m_fVolume;

        ILock *m_pvEffectLock;
        ILock *m_pvAnimationLock;
        std::vector<CEffect *> m_vEffects;
        //sticker or linear transform
        std::vector<CAnimation *> m_vAnimations;

        s64 m_sllOriginDurationUS;
        s64 m_sllDataDurationUS;
    };

    inline s32 CTrack::getEffectCount() const {
        return m_vEffects.size();
    }

    inline s32 CTrack::getAnimationCount() const {
        return m_vAnimations.size();
    }


    inline s64 CTrack::getCutStart() const {
        return m_sllCutStart;
    }

    inline s64 CTrack::getCutDuration() const {
        return m_sllCutDuration;
    }

    inline float CTrack::getPlayRate() const {
        return m_fPlayRate;
    }

    inline float CTrack::getVolume() const {
        return m_fVolume;
    }
}

#endif //_PAOMIANTV_TRACK_H_
