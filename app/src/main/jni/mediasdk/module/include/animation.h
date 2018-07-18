/*******************************************************************************
 *        Module: mediasdk
 *          File:
 * Functionality: animation entity.
 *       Related:
 *        System: android
 *      Language: C++
 *        Author: huangxuefeng
 *       Version: V1.0 Copyright(C) 2017 paomiantv, All rights reserved.
 * -----------------------------------------------------------------------------
 * Revisions:
 * Date        Version     Reviser       Description
 * 2017-08-01  v1.0        huangxuefeng  created
 ******************************************************************************/

#ifndef _PAOMIANTV_ANIMATION_H_
#define _PAOMIANTV_ANIMATION_H_

#include "typedef.h"

namespace paomiantv {

    class CAnimation {
    public:
        CAnimation();

        virtual ~CAnimation();

    public:
        // start transform params
        //! the position X in base picture
        float m_fStartTransX;
        //! the position Y in base picture
        float m_fStartTransY;
        //! the position Z in base picture
        float m_fStartTransZ;
        //! the degree X for rotation(0.0-360.0-...)
        float m_fStartDegreeX;
        //! the degree Y for rotation(0.0-360.0-...)
        float m_fStartDegreeY;
        //! the degree Z for rotation(0.0-360.0-...)
        float m_fStartDegreeZ;
        //! the scale X
        float m_fStartScaleX;
        //! the scale Y
        float m_fStartScaleY;
        //! the scale Z
        float m_fStartScaleZ;

        // end transform params
        //! the position X in base picture
        float m_fEndTransX;
        //! the position Y in base picture
        float m_fEndTransY;
        //! the position Z in base picture
        float m_fEndTransZ;
        //! the degree X for rotation(0.0-360.0-...)
        float m_fEndDegreeX;
        //! the degree Y for rotation(0.0-360.0-...)
        float m_fEndDegreeY;
        //! the degree Z for rotation(0.0-360.0-...)
        float m_fEndDegreeZ;
        //! the scale X
        float m_fEndScaleX;
        //! the scale Y
        float m_fEndScaleY;
        //! the scale Z
        float m_fEndScaleZ;


        // start crop params
        //! the position X in base picture
        float m_fCropStartTransX;
        //! the position Y in base picture
        float m_fCropStartTransY;
        //! the position Z in base picture
        float m_fCropStartTransZ;
        //! the degree X for rotation(0.0-360.0-...)
        float m_fCropStartDegreeX;
        //! the degree Y for rotation(0.0-360.0-...)
        float m_fCropStartDegreeY;
        //! the degree Z for rotation(0.0-360.0-...)
        float m_fCropStartDegreeZ;
        //! the scale X
        float m_fCropStartScaleX;
        //! the scale Y
        float m_fCropStartScaleY;
        //! the scale Z
        float m_fCropStartScaleZ;

        // end crop params
        //! the position X in base picture
        float m_fCropEndTransX;
        //! the position Y in base picture
        float m_fCropEndTransY;
        //! the position Z in base picture
        float m_fCropEndTransZ;
        //! the degree X for rotation(0.0-360.0-...)
        float m_fCropEndDegreeX;
        //! the degree Y for rotation(0.0-360.0-...)
        float m_fCropEndDegreeY;
        //! the degree Z for rotation(0.0-360.0-...)
        float m_fCropEndDegreeZ;
        //! the scale X
        float m_fCropEndScaleX;
        //! the scale Y
        float m_fCropEndScaleY;
        //! the scale Z
        float m_fCropEndScaleZ;


        //! the alpha start
        float m_fStartAlpha;

        //! the alpha end
        float m_fEndAlpha;

        //! the start time of the animation(microsecond) related to track start time
        s64 m_sllStart;
        //! the duration of the animation(microsecond)
        s64 m_sllDuration;

        float getVecX();

        float getVecY();

        float getVecZ();

        float getVecScaleX();

        float getVecScaleY();

        float getVecScaleZ();

        float getVecDegreeX();

        float getVecDegreeY();

        float getVecDegreeZ();


        float getCropVecX();

        float getCropVecY();

        float getCropVecZ();

        float getCropVecScaleX();

        float getCropVecScaleY();

        float getCropVecScaleZ();

        float getCropVecDegreeX();

        float getCropVecDegreeY();

        float getCropVecDegreeZ();


        float getVecAlpha();

        inline const u32 getId() const;

    private:
        static u32 m_sCount;
        // effect id
        const u32 m_uId;
    };

    inline const u32 CAnimation::getId() const {
        return m_uId;
    }


} // namespace paomiantv

#endif // _PAOMIANTV_ANIMATION_H_