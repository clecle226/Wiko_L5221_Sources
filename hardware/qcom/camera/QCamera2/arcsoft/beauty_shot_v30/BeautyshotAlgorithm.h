/*----------------------------------------------------------------------------------------------
 *
 * This file is ArcSoft's property. It contains ArcSoft's trade secret, proprietary and
 * confidential information.
 *
 * The information and code contained in this file is only for authorized ArcSoft employees
 * to design, create, modify, or review.
 *
 * DO NOT DISTRIBUTE, DO NOT DUPLICATE OR TRANSMIT IN ANY FORM WITHOUT PROPER AUTHORIZATION.
 *
 * If you are not an intended recipient of this file, you must not copy, distribute, modify,
 * or take any action in reliance on it.
 *
 * If you have received this file in error, please immediately notify ArcSoft and
 * permanently delete the original and any copy of any file and any printout thereof.
 *
 *-------------------------------------------------------------------------------------------------*/
/*
 * BeautyshotAlgorithm.h
 *
 * Reference:
 *
 * Description:
 *
 *
 */

#ifndef _BEAUTYSHOTALGORITHM_H_
#define _BEAUTYSHOTALGORITHM_H_

#include "amcomdef.h"
#include "merror.h"
#include "ammem.h"
#include "AISFReferenceInter.h"
#include "AISFCommonDef.h"
#include "asvloffscreen.h"
#include "abstypes.h"


/*
 * !!!Notes:
 * You can define TESTBED in Android.mk to enable dump feature, like:
 *      LOCAL_CFLAGS   += -DTESTBED
 *      LOCAL_CPPFLAGS += -DTESTBED
 *
 * If you would release SwipeSelfie_Algorithm to customer, Please MUST disable all code in TESTBED.
 * And please refine all comments on this .h file,
 * specially, MUST remove the code in TESTBED on this .h file.
 */

class BeautyShot_Algorithm: public AISF_NS::AISFReferenceInter
{

public:
	/*
	 *The unique ID of BeautyShot_Algorithm , use it in Create_BeautyShot_Algorithm().
	 */
	static const MInt32 CLASSID = 0;
	/*
	 * Initialize Algorithm SDK.
	 * You can redefine Init() base on Algorithm SDK
	 *
	 */

	virtual MRESULT Init(DataType eType)=0;

	/*
	 * Uninitialize Algorithm SDK. MUST correspond with Init().
	 */
	virtual MVoid Uninit()=0;

	virtual MVoid SetSkinSoftenLevel(MLong lLevel)=0;
	virtual MLong GetSkinSoftenLevel()=0;

	virtual MVoid SetShapeStrengthLevel(MLong lLevel)=0;
	virtual MLong GetShapeStrengthLevel()=0;

	virtual MVoid SetSkinBrightLevel(MLong lLevel)=0;
	virtual MLong GetSkinBrightLevel()=0;

	virtual MVoid SetFaceSlenderLevel(MLong lLevel)=0;
	virtual MLong GetFaceSlenderLevel()=0;

	virtual MVoid SetEyeEnlargementLevel(MLong lLevel)=0;
	virtual MLong GetEyeEnlargementLevel()=0;

	virtual MVoid SetDeblemishState(MBool bState)=0;
	virtual MBool GetDeblemishState()=0;

	virtual MVoid SetDepouchState(MBool bState)=0;
	virtual MBool GetDepouchState()=0;

	virtual MRESULT FaceDetect(LPASVLOFFSCREEN pImgSrc, TFaces *pInFaces, TFaces *pOutFaces)=0;
	virtual MRESULT Process(LPASVLOFFSCREEN pImgSrc, LPASVLOFFSCREEN pImgDst)=0;

	virtual MTChar* GetVersion()=0;

};

/*
 * The dynamic so export function. To create Algorithm object.
 * Parameters:
 *   [IN] ClassID : The unique class id, it will be defined on such as: BeautyShot_Algorithm::CLASSID.
 *   [OUT] ppalgorithm: The BeautyShot_Algorithm object will be created in the function with reference count as 1.
 *                and MUST release it by calling (*ppalgorithm)->Release() when not use it anymore.
 * Return:
 *   MOK is success, other is fail.
 */
#ifdef __cplusplus
extern "C" {
#endif

	MInt32 Create_BeautyShot_Algorithm(const MInt32 ClassID,
										BeautyShot_Algorithm** ppalgorithm);

#ifdef __cplusplus
}
#endif

#endif /* _BEAUTYSHOTALGORITHM_H_ */
