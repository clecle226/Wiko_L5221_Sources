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
 * abstypes.h
 *
 *
 */

#ifndef _ABS_TYPES_H_
#define _ABS_TYPES_H_

#include "amcomdef.h"

#define MAX_FACE_NUM 32
typedef struct _tagFaces {
	MRECT		prtFaces[MAX_FACE_NUM];			// The bounding box of face
	MLong		lFaceNum;						// Number of faces detected
	MLong		plFaceRolls[MAX_FACE_NUM];		// The angle of each face, between [0, 360)
} TFaces;

enum DataType {DataTypeVideo = 0, DataTypeImage = 1};

#endif /* _ABS_TYPES_H_ */
