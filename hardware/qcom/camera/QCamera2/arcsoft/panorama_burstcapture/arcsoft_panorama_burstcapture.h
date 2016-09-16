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
*-----------------------------------------------------------------------------------------------*/
/*
* 
*
* Description:  arcsoft_panorama_burstcapture.h
*
*
* History
*
*-----------------------------------------------------------------------------------------------*/

#ifndef ARCSOFT_PANORAMA_BURSTCAPTURE
#define ARCSOFT_PANORAMA_BURSTCAPTURE

#include "amcomdef.h"
#include "merror.h"
#include "asvloffscreen.h"


#ifdef __cplusplus
extern "C" {
#endif

// APBC: ArcSoft Panorama BurstCapture

#define IN
#define OUT
#define INOUT
#define APBC_API

// ******************** Constants ******************** //
// Error Code
#define MERR_BASE					0X7000
#define ERR_TRACE_DIRECTION			(MERR_BASE + 1)
#define ERR_TRACE_NO_FEATURE		(MERR_BASE + 2)
#define ERR_TRACE_LESS_FEATURE		(MERR_BASE + 3)
#define ERR_TRACE_TOO_QUICK			(MERR_BASE + 4)
#define	ERR_TRACE_TOO_SLOW			(MERR_BASE + 5)
#define ERR_STITCH_DIRECTION        (MERR_BASE + 6)
#define ERR_SHAKE_TOO_BIG	        (MERR_BASE + 7)
#define ERR_MAX_FRAME_COUNT	        (MERR_BASE + 8)
#define ERR_360DEGREE				(MERR_BASE + 9)

// Warning Code
#define MWARN_BASE					0X8000
#define	WARN_MOVE_SLOPE				(MWARN_BASE + 1)
#define	WARN_MOVE_SLOPE_TO_UP		(MWARN_BASE + 1)
#define	WARN_MOVE_SLOPE_TO_DOWN		(MWARN_BASE + (1<<1))
#define	WARN_MOVE_SLOPE_TO_LEFT		(MWARN_BASE + (1<<2))
#define	WARN_MOVE_SLOPE_TO_RIGHT	(MWARN_BASE + (1<<3))
#define	WARN_SHAKE_SMALL			(MWARN_BASE + (1<<4))
#define WARN_SHAKE_TOO_BIG	        (MWARN_BASE + (1<<5))
#define WARN_TRACE_LITTLE_QUICK		(MWARN_BASE + (1<<7))

// Direction
#define DIRECTION_UNKNOW				-1
#define DIRECTION_LEFT2RIGHT			0
#define DIRECTION_RIGHT2LEFT			1
#define DIRECTION_TOP2BOTTOM			2
#define DIRECTION_BOTTOM2TOP			3

// User Command
#define COMMAND_SELECT				0
#define COMMAND_STITCH				1
#define COMMAND_STOP				(1<<1)

// ***************** Data structures ***************** //
// INIT_PARAM
typedef struct tag_APBC_INIT_PARAMETER
{	
	MUInt8	*pu8Buffer;
	MInt32  i32BufferSize;
	/*device move direction*/
	MInt32	i32Direction;
	/*small image for tracking*/
	MUInt32	u32SrcSmallImageFormat;
	MInt32  i32SrcSmallImageWidth;
	MInt32  i32SrcSmallImageHeight;
	/*full image for stitching*/
	MUInt32 u32SrcFullImageFormat;
	MInt32  i32SrcFullImageWidth;
	MInt32  i32SrcFullImageHeight;
	MInt32	i32FullResultLength;
	/*thumbnail image for UI display*/
	MUInt32 u32ThumbnailFormat;
	MInt32  i32ThumbnailResultWidthH;/*H:horizontal*/
	MInt32  i32ThumbnailResultHeightH;
	MInt32	i32ThumbnailResultWidthV;/*V:vertical*/
	MInt32	i32ThumbnailResultHeightV;
	/*axis orientation for sensor*/
	MInt32	i32SensorOrientation;/*mobile phone:0, tablet:90*/
} APBC_INIT_PARAMETER, *LPAPBC_INIT_PARAMETER;

// VERSION INFO
typedef struct tag_APBC_VERSION_INFO
{
	MInt32 i32Codebase;	
	MInt32 i32Major;		
	MInt32 i32Minor;
	MInt32 i32Build;		
	MTChar Version[50];  
	MTChar BuildDate[50]; 
	MTChar CopyRight[50];
} APBC_VERSION_INFO, *LPAPBC_VERSION_INFO;
// *************************************************** //

// ****************** API Functions ****************** //
APBC_API MRESULT APBC_InitialEngine(
		IN	LPAPBC_INIT_PARAMETER	pInitParam,
		OUT MHandle					*phEngine		
);

APBC_API MRESULT APBC_ProcessImage(
		IN	MHandle				hEngine,
		IN	LPASVLOFFSCREEN		pSmallSrcImage,
		IN	LPASVLOFFSCREEN		pFullSrcImage,
		IN	MPOINT				*pptSensorOffset,
		IN  MInt32				i32UserCommand,
		OUT LPASVLOFFSCREEN		*ppSmallResultImage,
		OUT	MRECT				*prcUpdateSmallImage,
		OUT LPASVLOFFSCREEN		*ppFullResultImage,
		OUT	MPOINT				*pptOutputOffset,
		OUT	MInt32				*pi32Direction,
		OUT	MInt32				*pi32Progress
);

APBC_API MRESULT APBC_ReleaseEngine(
		OUT MHandle				*phEngine
);

APBC_API MRESULT APBC_GetVersionInfo(
		OUT   LPAPBC_VERSION_INFO	pVersionInfo
);

// *************************************************** //


#ifdef __cplusplus
}
#endif

#endif // ARCSOFT_PANORAMA_BURSTCAPTURE_H

