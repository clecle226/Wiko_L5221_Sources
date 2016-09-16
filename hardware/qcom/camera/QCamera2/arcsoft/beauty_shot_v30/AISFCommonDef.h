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
* AISFCommonDef.h
*
* Reference:
*
* Description:
*
* Create by xhliu 2014-10-28
*
*/

#ifndef __AISFCOMMONDEF_H_20141028__
#define __AISFCOMMONDEF_H_20141028__

//define the AISF_ABI_CODE
//armeabi,armeabi-v7a,arm64-v8a,x86,x86_64,mips,mips64
//if abi = armeabi , AISF_ABI_CODE = 1
//if abi = armeabi-v7a , AISF_ABI_CODE = 2
//if abi = arm64-v8a , AISF_ABI_CODE = 3
//if abi = x86 , AISF_ABI_CODE = 4
//if abi = x86_64 , AISF_ABI_CODE = 5
//if abi = mips , AISF_ABI_CODE = 6
//if abi = mips64 , AISF_ABI_CODE = 7
class AISF_ABI
{
public:
	static const int unknown		=	0;
	static const int armeabi 		=	1;
	static const int arm64_v8a 		=	3;
	static const int armeabi_v7a 	=	2;
	static const int x86			=	4;
	static const int x86_64			=	5;
	static const int mips			=	6;
	static const int mips64			=	7;

	static const int armeabi_v7a_neon 		=	8;
	static const int armeabi_v7a_neon_vfp	=	9;
	static const int armeabi_v7a_vfp		=	10;

};

#if defined(__arm__)
  #if defined(__ARM_ARCH_7A__)//if armv7a
	#if defined(__ARM_NEON__)
	  #if defined(__ARM_PCS_VFP)
		#define AISF_ABI_CODE (AISF_ABI::armeabi_v7a_neon_vfp)
	  #else
		#define AISF_ABI_CODE (AISF_ABI::armeabi_v7a_neon)
	  #endif
	#else
	  #if defined(__ARM_PCS_VFP)
		#define AISF_ABI_CODE (AISF_ABI::armeabi_v7a_vfp)
	  #else
		#define AISF_ABI_CODE (AISF_ABI::armeabi_v7a)
	  #endif
	#endif //armv7a
  #else //arm common
    #define AISF_ABI_CODE (AISF_ABI::armeabi) //armeabi
  #endif
#elif defined(__aarch64__)
  #define AISF_ABI_CODE (AISF_ABI::arm64_v8a)
#elif defined(__i386__)
  #define AISF_ABI_CODE (AISF_ABI::x86)
#elif defined(__x86_64__)
  #define AISF_ABI_CODE (AISF_ABI::x86_64)
#elif defined(__mips__)
  #define AISF_ABI_CODE (AISF_ABI::mips)
#elif defined(__mips64)  /* mips64el-* toolchain defines __mips__ too */
  #define AISF_ABI_CODE (AISF_ABI::mips64)
#else
  #define AISF_ABI_CODE (AISF_ABI::unknown)
#endif



//namespace macros
#define START_NAMESPACE_(NS) namespace NS {
#define END_NAMESPACE_(NS) };
#define USING_NAMESPACE_(NS) using namespace NS;

//define AISF namespace

#define AISF_NS ARC_AISF
//for aisfframework namespace
#define START_NAMESPACE_AISF START_NAMESPACE_(AISF_NS)

#define END_NAMESPACE_AISF END_NAMESPACE_(AISF_NS)

#define USING_NAMESPACE_AISF USING_NAMESPACE_(AISF_NS)


#endif
