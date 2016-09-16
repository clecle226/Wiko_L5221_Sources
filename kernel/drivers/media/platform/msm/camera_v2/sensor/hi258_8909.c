/* Copyright (c) 2014, The Linux Foundation. All rights reserved.
 *2015-7-3 By paul
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "msm_sensor.h"
#include "msm_cci.h"
#include "msm_camera_io_util.h"
#define HI258_8909_SENSOR_NAME "hi258_8909"
#define PLATFORM_DRIVER_NAME "msm_camera_hi258_8909"
#define hi258_8909_obj hi258_8909_##obj
/*#define CONFIG_MSMB_CAMERA_DEBUG*/
#undef CDBG
#if 1
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif
static int awb_return;
static int ae_return;


DEFINE_MSM_MUTEX(hi258_8909_mut);
static struct msm_sensor_ctrl_t hi258_8909_s_ctrl;

static struct msm_sensor_power_setting hi258_8909_power_setting[] = {
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_VDIG,
    .config_val = GPIO_OUT_LOW,
    .delay = 0,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_VDIG,
    .config_val = GPIO_OUT_HIGH,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_VANA,
    .config_val = GPIO_OUT_LOW,
    .delay = 0,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_VANA,
    .config_val = GPIO_OUT_HIGH,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_VIO,
    .config_val = GPIO_OUT_LOW,
    .delay = 0,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_VIO,
    .config_val = GPIO_OUT_HIGH,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_STANDBY,
    .config_val = GPIO_OUT_HIGH,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_STANDBY,
    .config_val = GPIO_OUT_LOW,
    .delay = 10,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_RESET,
    .config_val = GPIO_OUT_LOW,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = SENSOR_GPIO_RESET,
    .config_val = GPIO_OUT_HIGH,
    .delay = 10,
  },
  {
    .seq_type = SENSOR_CLK,
    .seq_val = SENSOR_CAM_MCLK,
    .config_val = 24000000,
    .delay = 10,
  },
  {
    .seq_type = SENSOR_I2C_MUX,
    .seq_val = 0,
    .config_val = 0,
    .delay = 0,
  },

};


static struct msm_camera_i2c_reg_conf hi258_8909_snapshot_settings[] = {
	/* 2M FULL Mode 15fps*/
	{0x03, 0x00},
	{0x01, 0x01},

	{0x03, 0x22},
	{0x10, 0xe9},

	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},

	{0x03, 0x00},
	{0xd0, 0x05},
	{0xd1, 0x34},
	{0xd2, 0x01},
	{0xd3, 0x20},
	{0xd0, 0x85},
	{0xd0, 0x85},
	{0xd0, 0x85},
	{0xd0, 0x95},

	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},

	{0x03, 0x00},
	{0x10, 0x00},

	{0x20, 0x00},
	{0x21, 0x0b},
	{0x22, 0x00},
	{0x23, 0x08},


	{0x03, 0x18},
	{0x14, 0x00},


	{0x03, 0x48},
	{0x36, 0x01},
	{0x37, 0x05},
	{0x34, 0x04},
	{0x32, 0x15},
	{0x35, 0x04},
	{0x33, 0x0d},

	{0x1c, 0x01},
	{0x1d, 0x0b},
	{0x1e, 0x06},
	{0x1f, 0x09},

	{0x30, 0x0c},
	{0x31, 0x80},

	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},

	{0x03, 0x00},
	{0x01, 0x00},

	{0xff, 0x03},

};


static struct msm_camera_i2c_reg_conf hi258_8909_preview_settings[] = {
	/*800*600 preview 30fps*/
	{0x03, 0x00},
	{0x10, 0x10},
	{0x11, 0x93},

	{0xd2, 0x05},
/*	
	{0x03, 0x20},
    {0x10, 0x1c}, 
    {0x18, 0x38}, 
  
	{0x83, 0x0b}, //EXP Normal 8.33 fps 
	{0x84, 0xe6}, 
	{0x85, 0xe0}, 
	{0x86, 0x01}, //EXPMin 13000.00 fps
	{0x87, 0xf4}, 
	{0x88, 0x10}, //EXP Max 60hz 6.00 fps 
	{0x89, 0x7a}, 
	{0x8a, 0xc0}, 
	{0xa5, 0x0f}, //EXP Max 50hz 6.25 fps 
	{0xa6, 0xde}, 
	{0xa7, 0x80}, 
	{0x8B, 0xfd}, //EXP100 
	{0x8C, 0xe8}, 
	{0x8D, 0xd2}, //EXP120 
	{0x8E, 0xf0}, 
	{0x9c, 0x21}, //EXP Limit 764.71 fps 
	{0x9d, 0x34}, 
	{0x9e, 0x01}, //EXP Unit 
	{0x9f, 0xf4}, 
	{0xa3, 0x00}, //Outdoor Int 
	{0xa4, 0xd2}, 



    
    {0x10, 0x9c}, 
    {0x18, 0x30}, 
	
	*/
	{0x03, 0x48},

	{0x30, 0x06},
	{0x31, 0x40}, //long_packet word count

	{0x03, 0x18},
	{0x10, 0x00},//scaling off    
	{0x18, 0x00},
	//{0x03, 0x20}, //Page 20	
	//{0x10, 0x9c},

	{0xff, 0x03},

};


static struct msm_camera_i2c_reg_conf hi258_8909_recommend_settings[] = {
	{0x01, 0x01},
	{0x01, 0x03},
	{0x01, 0x01},

	{0x03, 0x20},
	{0x10, 0x1c},


	{0x03, 0x22},
	{0x10, 0x69},

	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},

	{0x08, 0x00},
	{0x09, 0x77},
	{0x0a, 0x07},


	{0x03, 0x00},
	{0xd0, 0x05},
	{0xd1, 0x34},
	{0xd2, 0x05},
	{0xd3, 0x20},
	{0xd0, 0x85},
	{0xd0, 0x85},
	{0xd0, 0x85},
	{0xd0, 0x95},

	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},



	{0x03, 0x20},
	{0x10, 0x1c},


	{0x03, 0x22},
	{0x10, 0x69},


	{0x03, 0x00},
	{0x10, 0x10},
	{0x11, 0x93},
	{0x12, 0x04},
	{0x14, 0x05},

	{0x20, 0x00},
	{0x21, 0x0b},
	{0x22, 0x00},
	{0x23, 0x08},


	{0x24, 0x04},
	{0x25, 0xb0},
	{0x26, 0x06},
	{0x27, 0x40},

	{0x28, 0x04},
	{0x29, 0x01},
	{0x2a, 0x02},

	{0x2b, 0x04},
	{0x2c, 0x04},
	{0x2d, 0x02},

	{0x40, 0x01}, //Hblank 376
	{0x41, 0x78}, 
	{0x42, 0x00}, //Vblank 20
	{0x43, 0x14},


	{0x50, 0x00},


	{0x80, 0x2e},
	{0x81, 0x7e},
	{0x82, 0x90},
	{0x83, 0x00},
	{0x84, 0xcc},
	{0x85, 0x00},
	{0x86, 0x00},
	{0x87, 0x0f},
	{0x88, 0x34},
	{0x8a, 0x0b},
	{0x8e, 0x80}, //Pga Blc Hold

	{0x90, 0x10}, //BLC_TIME_TH_ON
	{0x91, 0x10}, //BLC_TIME_TH_OFF 
	{0x92, 0x78}, //BLC_AG_TH_ON
	{0x93, 0x70}, //BLC_AG_TH_OFF

	{0x96, 0xdc}, //BLC Outdoor Th On
	{0x97, 0xfe}, //BLC Outdoor Th Off
	{0x98, 0x38},

	//OutDoor  BLC
	{0x99, 0x43}, //R,Gr,B,Gb Offset

	//Dark BLC
	{0xa0, 0x43}, //R,Gr,B,Gb Offset


	{0xa8, 0x43},

	{0x03, 0x02},
	{0x10, 0x00},
	{0x13, 0x00},
	{0x14, 0x00},
	{0x18, 0xcc},
	{0x19, 0x01},
	{0x1A, 0x39},
	{0x1B, 0x00},
	{0x1C, 0x1a},
	{0x1D, 0x14},
	{0x1E, 0x30},
	{0x1F, 0x10},

	{0x20, 0x77},
	{0x21, 0xde},
	{0x22, 0xa7},
	{0x23, 0x30},
	{0x24, 0x77},
	{0x25, 0x10},
	{0x26, 0x10},
	{0x27, 0x3c},
	{0x2b, 0x80},
	{0x2c, 0x02},
	{0x2d, 0x58},
	{0x2e, 0x11},
	{0x2f, 0x11},

	{0x30, 0x00},
	{0x31, 0x99},
	{0x32, 0x00},
	{0x33, 0x00},
	{0x34, 0x22},
	{0x36, 0x75},
	{0x38, 0x88},
	{0x39, 0x88},
	{0x3d, 0x03},
	{0x3f, 0x02},

	{0x49, 0xc1},
	{0x4a, 0x10},

	{0x50, 0x21},
	{0x53, 0xb1},
	{0x54, 0x10},
	{0x55, 0x1c},
	{0x56, 0x11},
	{0x58, 0x3a},
	{0x59, 0x38},
	{0x5d, 0xa2},
	{0x5e, 0x5a},

	{0x60, 0x87},
	{0x61, 0x98},
	{0x62, 0x88},
	{0x63, 0x96},
	{0x64, 0x88},
	{0x65, 0x96},
	{0x67, 0x3f},
	{0x68, 0x3f},
	{0x69, 0x3f},

	{0x72, 0x89},
	{0x73, 0x95},
	{0x74, 0x89},
	{0x75, 0x95},
	{0x7C, 0x84},
	{0x7D, 0xaf},

	{0x80, 0x01},
	{0x81, 0x7a},
	{0x82, 0x13},
	{0x83, 0x24},
	{0x84, 0x78},
	{0x85, 0x7c},

	{0x92, 0x44},
	{0x93, 0x59},
	{0x94, 0x78},
	{0x95, 0x7c},

	{0xA0, 0x02},
	{0xA1, 0x74},
	{0xA4, 0x74},
	{0xA5, 0x02},
	{0xA8, 0x85},
	{0xA9, 0x8c},
	{0xAC, 0x10},
	{0xAD, 0x16},

	{0xB0, 0x99},
	{0xB1, 0xa3},
	{0xB4, 0x9b},
	{0xB5, 0xa2},
	{0xB8, 0x9b},
	{0xB9, 0x9f},
	{0xBC, 0x9b},
	{0xBD, 0x9f},

	{0xc4, 0x29},
	{0xc5, 0x40},
	{0xc6, 0x5c},
	{0xc7, 0x72},
	{0xc8, 0x2a},
	{0xc9, 0x3f},
	{0xcc, 0x5d},
	{0xcd, 0x71},

	{0xd0, 0x10},
	{0xd1, 0x14},
	{0xd2, 0x20},
	{0xd3, 0x00},
	{0xd4, 0x10}, //DCDC_TIME_TH_ON
	{0xd5, 0x10}, //DCDC_TIME_TH_OFF 
	{0xd6, 0x78}, //DCDC_AG_TH_ON
	{0xd7, 0x70}, //DCDC_AG_TH_OFF

	{0xdc, 0x00},
	{0xdd, 0xa3},
	{0xde, 0x00},
	{0xdf, 0x84},

	{0xe0, 0xa4},
	{0xe1, 0xa4},
	{0xe2, 0xa4},
	{0xe3, 0xa4},
	{0xe4, 0xa4},
	{0xe5, 0x01},
	{0xe8, 0x00},
	{0xe9, 0x00},
	{0xea, 0x77},

	{0xF0, 0x00},
	{0xF1, 0x00},
	{0xF2, 0x00},


	{0x03, 0x10},
	{0x10, 0x03},
	{0x11, 0x03},
	{0x12, 0x30},
	{0x13, 0x03},

	{0x20, 0x00},
	{0x21, 0x40},
	{0x22, 0x0f},
	{0x24, 0x20},
	{0x25, 0x10},
	{0x26, 0x01},
	{0x27, 0x02},
	{0x28, 0x11},

	{0x40, 0x88},
	{0x41, 0x00},
	{0x42, 0x00},
	{0x43, 0x00},
	{0x44, 0x80},
	{0x45, 0x80},
	{0x46, 0xf0},
	{0x48, 0x88},
	{0x4a, 0x80},

	{0x50, 0x8f},

	{0x60, 0x4f},
	{0x61, 0x90},//82
	{0x62, 0x88},//86
	{0x63, 0xF0}, //Auto-De Color

	{0x66, 0x42},
	{0x67, 0x22},

	{0x6a, 0xAF}, //White Protection Offset Dark/Indoor
	{0x74, 0x08}, //White Protection Offset Outdoor
	{0x75, 0x76}, //Sat over th
	{0x76, 0x01}, //White Protection Enable
	{0x77, 0x82},
	//{0x78, 0xff}, //Sat over ratio

	{0x03, 0x11},

	{0x20, 0x00},
	{0x21, 0x00},
	{0x26, 0x62}, //pga_dark1_min (on)
	{0x27, 0x60}, //pga_dark1_max (off)
	{0x28, 0x0f},
	{0x29, 0x10},
	{0x2b, 0x30},
	{0x2c, 0x32},


	{0x70, 0x2b},
	{0x74, 0x30},
	{0x75, 0x18},
	{0x76, 0x30},
	{0x77, 0xff},
	{0x78, 0xa0},
	{0x79, 0xff},
	{0x7a, 0x30},
	{0x7b, 0x20},
	{0x7c, 0xf4},
	{0x7d, 0x02},
	{0x7e, 0xb0},
	{0x7f, 0x10},

	{0x03, 0x12},


	{0x10, 0x03},
	{0x11, 0x08},
	{0x12, 0x10},
	{0x20, 0x53},
	{0x21, 0x03},
	{0x22, 0xe6},

	{0x23, 0x14}, //Outdoor Dy Th
	{0x24, 0x20}, //Indoor Dy Th // For reso Limit 0x20
	{0x25, 0x20}, //Dark Dy Th

	//Outdoor LPF Flat
	{0x30, 0xff}, //Y Hi Th
	{0x31, 0x00}, //Y Lo Th
	{0x32, 0xf0}, //Std Hi Th //Reso Improve Th Low //50
	{0x33, 0x00}, //Std Lo Th
	{0x34, 0xff}, //Median ratio

	//Indoor LPF Flat
	{0x35, 0xff}, //Y Hi Th
	{0x36, 0x00}, //Y Lo Th
	{0x37, 0xff}, //Std Hi Th //Reso Improve Th Low //50
	{0x38, 0x00}, //Std Lo Th
	{0x39, 0xff}, //Median ratio

	//Dark LPF Flat
	{0x3a, 0xff}, //Y Hi Th
	{0x3b, 0x00}, //Y Lo Th
	{0x3c, 0xff}, //Std Hi Th //Reso Improve Th Low //50
	{0x3d, 0x00}, //Std Lo Th
	{0x3e, 0x00}, //Median ratio

	//Outdoor Cindition
	{0x46, 0xa0}, //Out Lum Hi
	{0x47, 0x50}, //Out Lum Lo

	//Indoor Cindition
	{0x4c, 0xa0}, //Indoor Lum Hi
	{0x4d, 0x50}, //Indoor Lum Lo

	//Dark Cindition
	{0x52, 0xa0}, //Dark Lum Hi
	{0x53, 0x50}, //Dark Lum Lo

	//C-Filter
	{0x70, 0x10}, //Outdoor(2:1) AWM Th Horizontal
	{0x71, 0x0a}, //Outdoor(2:1) Diff Th Vertical
	{0x72, 0x10}, //Indoor,Dark1 AWM Th Horizontal
	{0x73, 0x0a}, //Indoor,Dark1 Diff Th Vertical
	{0x74, 0x10}, //Dark(2:3) AWM Th Horizontal
	{0x75, 0x0f}, //Dark(2:3) Diff Th Vertical

	//DPC
	{0x90, 0x5d},
	{0x91, 0x34},
	{0x99, 0x28},
	{0x9c, 0x0f},
	{0x9d, 0x15},
	{0x9e, 0x28},
	{0x9f, 0x28},
	{0xb0, 0x0e}, //Zipper noise Detault change (0x75->0x0e)
	{0xb8, 0x44},
	{0xb9, 0x15},
	///// PAGE 12 END /////

	///// PAGE 13 START /////
	{0x03, 0x13}, //page 13
	{0x80, 0xc1}, //Sharp2D enable _ YUYV Order
	{0x81, 0x07}, //Sharp2D Clip/Limit
	{0x82, 0x73}, //Sharp2D Filter
	{0x83, 0x00}, //Sharp2D Low Clip
	{0x85, 0x00},

	{0x92, 0x33},
	{0x93, 0x30},
	{0x94, 0x02},
	{0x95, 0xf0},
	{0x96, 0x1e},
	{0x97, 0x40},
	{0x98, 0x80},
	{0x99, 0x40},

	//Sharp Lclp
	{0xa2, 0x02}, //Outdoor Lclip_N
	{0xa3, 0x02}, //Outdoor Lclip_P
	{0xa4, 0x03}, //Indoor Lclip_N 0x03 For reso Limit 0x0e
	{0xa5, 0x03}, //Indoor Lclip_P 0x0f For reso Limit 0x0f
	{0xa6, 0x05}, //Dark Lclip_N
	{0xa7, 0x05}, //Dark Lclip_P

	//Outdoor Slope
	{0xb6, 0x34}, //Lum negative Hi
	{0xb7, 0x36}, //Lum negative middle
	{0xb8, 0x34}, //Lum negative Low
	{0xb9, 0x34}, //Lum postive Hi
	{0xba, 0x31}, //Lum postive middle
	{0xbb, 0x31}, //Lum postive Low

	//Indoor Slope
	{0xbc, 0x30}, //Lum negative Hi
	{0xbd, 0x40}, //Lum negative middle
	{0xbe, 0x30}, //Lum negative Low
	{0xbf, 0x32}, //Lum postive Hi
	{0xc0, 0x40}, //Lum postive middle
	{0xc1, 0x30}, //Lum postive Low

	//Dark Slope
	{0xc2, 0x30}, //Lum negative Hi
	{0xc3, 0x30}, //Lum negative middle
	{0xc4, 0x30}, //Lum negative Low
	{0xc5, 0x30}, //Lum postive Hi
	{0xc6, 0x30}, //Lum postive middle
	{0xc7, 0x30}, //Lum postive Low
	///// PAGE 13 END /////

	///// PAGE 14 START /////
	{0x03, 0x14}, //page 14
	{0x10, 0x0f},

	{0x20, 0x50}, //X-Center
	{0x21, 0x80}, //Y-Center

	{0x22, 0x38}, //LSC R 1b->15 20130125
	{0x23, 0x31}, //LSC G
	{0x24, 0x31}, //LSC B

	{0x25, 0x7F}, //LSC Off
	{0x26, 0x7B}, //LSC On
	///// PAGE 14 END /////

	/////// PAGE 15 START ///////
	{0x03, 0x15}, //15 Page
	{0x10, 0x21},
	{0x14, 0x42}, //CMCOFSGH
	{0x15, 0x32}, //CMCOFSGM
	{0x16, 0x22}, //CMCOFSGL
	{0x17, 0x2f},

	//CMC
	{0x30, 0xdc},
	{0x31, 0x5d},
	{0x32, 0x01},
	{0x33, 0x39},
	{0x34, 0xd4},
	{0x35, 0x0f},
	{0x36, 0x17},
	{0x37, 0x46},
	{0x38, 0xd7},
	//CMC OFS
	{0x40, 0x90},
	{0x41, 0x10},
	{0x42, 0x00},
	{0x43, 0x0f},
	{0x44, 0x0b},
	{0x45, 0x9a},
	{0x46, 0x9f},
	{0x47, 0x09},
	{0x48, 0x16},
	//CMC POFS
	{0x50, 0x00},
	{0x51, 0x98},
	{0x52, 0x18},
	{0x53, 0x04},
	{0x54, 0x00},
	{0x55, 0x84},
	{0x56, 0x02},
	{0x57, 0x00},
	{0x58, 0x82},

	///// PAGE 15 END /////

	///// PAGE 16 START /////
	{0x03, 0x16}, //page 16 Gamma
	{0x10, 0x31},
	{0x18, 0x80},// Double_AG 5e->37
	{0x19, 0x7c},// Double_AG 5e->36
	{0x1a, 0x0e},
	{0x1b, 0x01},
	{0x1c, 0xdc},
	{0x1d, 0xfe},

	//Indoor
	{0x30, 0x01},
	{0x31, 0x08},
	{0x32, 0x11},
	{0x33, 0x22},
	{0x34, 0x52},
	{0x35, 0x74},
	{0x36, 0x8e},
	{0x37, 0xa4},
	{0x38, 0xb6},
	{0x39, 0xc5},
	{0x3a, 0xd1},
	{0x3b, 0xd9},
	{0x3c, 0xe2},
	{0x3d, 0xe7},
	{0x3e, 0xed},
	{0x3f, 0xf1},
	{0x40, 0xf8},
	{0x41, 0xfb},
	{0x42, 0xff},

	//Outdoor
	{0x50, 0x01},
	{0x51, 0x08},
	{0x52, 0x11},
	{0x53, 0x22},
	{0x54, 0x52},
	{0x55, 0x74},
	{0x56, 0x8e},
	{0x57, 0xa4},
	{0x58, 0xb6},
	{0x59, 0xc5},
	{0x5a, 0xd1},
	{0x5b, 0xd9},
	{0x5c, 0xe2},
	{0x5d, 0xe7},
	{0x5e, 0xed},
	{0x5f, 0xf1},
	{0x60, 0xf8},
	{0x61, 0xfb},
	{0x62, 0xff},

	//Dark
	{0x70, 0x01},
	{0x71, 0x08},
	{0x72, 0x11},
	{0x73, 0x22},
	{0x74, 0x52},
	{0x75, 0x74},
	{0x76, 0x8e},
	{0x77, 0xa4},
	{0x78, 0xb6},
	{0x79, 0xc5},
	{0x7a, 0xd1},
	{0x7b, 0xd9},
	{0x7c, 0xe2},
	{0x7d, 0xe7},
	{0x7e, 0xed},
	{0x7f, 0xf1},
	{0x80, 0xf8},
	{0x81, 0xfb},
	{0x82, 0xff},
	///// PAGE 16 END /////

	///// PAGE 17 START /////
	{0x03, 0x17}, //page 17
	{0xc1, 0x00},
	{0xC4, 0x41}, //FLK200 
	{0xC5, 0x36}, //FLK240 
	{0xc6, 0x02},
	{0xc7, 0x20},
	///// PAGE 17 END /////

	///// PAGE 18 START /////
	{0x03, 0x18}, //page 18
	{0x10, 0x00},//Scale Off
	{0x11, 0x00},
	{0x12, 0x58},
	{0x13, 0x01},
	{0x14, 0x00}, //Sawtooth
	{0x15, 0x00},
	{0x16, 0x00},
	{0x17, 0x00},
	{0x18, 0x00},
	{0x19, 0x00},
	{0x1a, 0x00},
	{0x1b, 0x00},
	{0x1c, 0x00},
	{0x1d, 0x00},
	{0x1e, 0x00},
	{0x1f, 0x00},
	{0x20, 0x05},//zoom wid
	{0x21, 0x00},
	{0x22, 0x01},//zoom hgt
	{0x23, 0xe0},
	{0x24, 0x00},//zoom start x
	{0x25, 0x00},
	{0x26, 0x00},//zoom start y
	{0x27, 0x00},
	{0x28, 0x05},//zoom end x
	{0x29, 0x00},
	{0x2a, 0x01},//zoom end y
	{0x2b, 0xe0},
	{0x2c, 0x0a},//zoom step vert
	{0x2d, 0x00},
	{0x2e, 0x0a},//zoom step horz
	{0x2f, 0x00},
	{0x30, 0x44},//zoom fifo

	///// PAGE 18 END /////

	{0x03, 0x19}, //Page 0x19
	{0x10, 0x7f}, //mcmc_ctl1
	{0x11, 0x7f}, //mcmc_ctl2
	{0x12, 0x1e}, //mcmc_delta1
	{0x13, 0x48}, //mcmc_center1
	{0x14, 0x1e}, //mcmc_delta2
	{0x15, 0x80}, //mcmc_center2
	{0x16, 0x1e}, //mcmc_delta3
	{0x17, 0xb8}, //mcmc_center3
	{0x18, 0x1e}, //mcmc_delta4
	{0x19, 0xf0}, //mcmc_center4
	{0x1a, 0x9e}, //mcmc_delta5
	{0x1b, 0x22}, //mcmc_center5
	{0x1c, 0x9e}, //mcmc_delta6
	{0x1d, 0x5e}, //mcmc_center6
	{0x1e, 0x40}, //mcmc_sat_gain1
	{0x1f, 0x38}, //mcmc_sat_gain2
	{0x20, 0x5a}, //mcmc_sat_gain3
	{0x21, 0x5a}, //mcmc_sat_gain4
	{0x22, 0x40}, //mcmc_sat_gain5
	{0x23, 0x37}, //mcmc_sat_gain6
	{0x24, 0x00}, //mcmc_hue_angle1
	{0x25, 0x05}, //mcmc_hue_angle2
	{0x26, 0x00}, //mcmc_hue_angle3
	{0x27, 0x92}, //mcmc_hue_angle4
	{0x28, 0x00}, //mcmc_hue_angle5
	{0x29, 0x8a}, //mcmc_hue_angle6

	{0x53, 0x10}, //mcmc_ctl3
	{0x6c, 0xff}, //mcmc_lum_ctl1
	{0x6d, 0x3f}, //mcmc_lum_ctl2
	{0x6e, 0x00}, //mcmc_lum_ctl3
	{0x6f, 0x00}, //mcmc_lum_ctl4
	{0x70, 0x00}, //mcmc_lum_ctl5
	{0x71, 0x3f}, //rg1_lum_gain_wgt_th1
	{0x72, 0x3f}, //rg1_lum_gain_wgt_th2
	{0x73, 0x3f}, //rg1_lum_gain_wgt_th3
	{0x74, 0x3f}, //rg1_lum_gain_wgt_th4
	{0x75, 0x30}, //rg1_lum_sp1
	{0x76, 0x50}, //rg1_lum_sp2
	{0x77, 0x80}, //rg1_lum_sp3
	{0x78, 0xb0}, //rg1_lum_sp4
	{0x79, 0x3f}, //rg2_gain_wgt_th1
	{0x7a, 0x3f}, //rg2_gain_wgt_th2
	{0x7b, 0x3f}, //rg2_gain_wgt_th3
	{0x7c, 0x3f},
	{0x7d, 0x28},
	{0x7e, 0x50},
	{0x7f, 0x80},
	{0x80, 0xb0},
	{0x81, 0x28},
	{0x82, 0x3f},
	{0x83, 0x3f},
	{0x84, 0x3f},
	{0x85, 0x28},
	{0x86, 0x50},
	{0x87, 0x80},
	{0x88, 0xb0},
	{0x89, 0x1a},
	{0x8a, 0x28},
	{0x8b, 0x3f},
	{0x8c, 0x3f},
	{0x8d, 0x10},
	{0x8e, 0x30},
	{0x8f, 0x60},
	{0x90, 0x90},
	{0x91, 0x1a},
	{0x92, 0x28},
	{0x93, 0x3f},
	{0x94, 0x3f},
	{0x95, 0x28},
	{0x96, 0x50},
	{0x97, 0x80},
	{0x98, 0xb0},
	{0x99, 0x1a},
	{0x9a, 0x28},
	{0x9b, 0x3f},
	{0x9c, 0x3f},
	{0x9d, 0x28},
	{0x9e, 0x50}, //rg6_lum_sp2
	{0x9f, 0x80}, //rg6_lum_sp3
	{0xa0, 0xb0}, //rg6_lum_sp4

	{0xe5, 0x80}, //add 20120709 Bit[7] On MCMC --> YC2D_LPF

	/////// PAGE 20 START ///////
	{0x03, 0x20},
	{0x10, 0x1c},
	{0x11, 0x0c},//14
	{0x18, 0x30},
	{0x20, 0x65}, //8x8 Ae weight 0~7 Outdoor / Weight Outdoor On B[5]
	{0x21, 0x30},
	{0x22, 0x10},
	{0x23, 0x00},

	{0x28, 0xf7},
	{0x29, 0x0d},
	{0x2a, 0xff},
	{0x2b, 0x04}, //Adaptive Off,1/100 Flicker

	{0x2c, 0x83}, //AE After CI
	{0x2d, 0xe3}, 
	{0x2e, 0x13},
	{0x2f, 0x0b},

	{0x30, 0x78},
	{0x31, 0xd7},
	{0x32, 0x10},
	{0x33, 0x2e},
	{0x34, 0x20},
	{0x35, 0xd4},
	{0x36, 0xfe},
	{0x37, 0x32},
	{0x38, 0x04},
	{0x39, 0x22},
	{0x3a, 0xde},
	{0x3b, 0x22},
	{0x3c, 0xde},
	{0x3d, 0xe1},

	{0x3e, 0xc9}, //Option of changing Exp max
	{0x41, 0x23}, //Option of changing Exp max

	{0x50, 0x45},
	{0x51, 0x88},
	{0x53, 0x11},

	{0x56, 0x03}, // for tracking
	{0x57, 0xf7}, // for tracking
	{0x58, 0x14}, // for tracking
	{0x59, 0x88}, // for tracking

	{0x5a, 0x04},
	{0x5b, 0x04},

	{0x5e, 0xc7},
	{0x5f, 0x95},

	{0x62, 0x10},
	{0x63, 0xc0},
	{0x64, 0x10},
	{0x65, 0x8a},
	{0x66, 0x58},
	{0x67, 0x58},

	{0x70, 0x48}, //6c
	{0x71, 0x80}, //81(+4),89(-4)

	{0x76, 0x21},
	{0x77, 0x71},
	{0x78, 0x22}, //24
	{0x79, 0x22}, // Y Target 70 => 25, 72 => 26 //
	{0x7a, 0x23}, //23
	{0x7b, 0x22}, //22
	{0x7d, 0x23},

	{0x03, 0x20}, //Page 20

	{0x83, 0x0a}, //EXP Normal 9.09 fps 
	{0x84, 0xe8}, 
	{0x85, 0xf8}, 
	{0x86, 0x01}, //EXPMin 13000.00 fps
	{0x87, 0xf4}, 
	{0x88, 0x0a}, //EXP Max 60hz 9.23 fps 
	{0x89, 0xb6}, 
	{0x8a, 0x30}, 
	{0xa5, 0x0a}, //EXP Max 50hz 9.09 fps 
	{0xa6, 0xe8}, 
	{0xa7, 0xf8}, 
	{0x8B, 0xfd}, //EXP100 
	{0x8C, 0xe8}, 
	{0x8D, 0xd2}, //EXP120 
	{0x8E, 0xf0}, 
	{0x9c, 0x1d}, //EXP Limit 866.67 fps 
	{0x9d, 0x4c}, 
	{0x9e, 0x01}, //EXP Unit 
	{0x9f, 0xf4}, 
	{0xa3, 0x00}, //Outdoor Int 
	{0xa4, 0xd2}, 



	{0xb0, 0x50},
	{0xb1, 0x14},
	{0xb2, 0x80},
	{0xb3, 0x15},
	{0xb4, 0x16},
	{0xb5, 0x3c},
	{0xb6, 0x29},
	{0xb7, 0x23},
	{0xb8, 0x20},
	{0xb9, 0x1e},
	{0xba, 0x1c},
	{0xbb, 0x1b},
	{0xbc, 0x1b},
	{0xbd, 0x1a},

	{0xc0, 0x10},
	{0xc1, 0x40},
	{0xc2, 0x40},
	{0xc3, 0x40},
	{0xc4, 0x06},

	{0xc6, 0x80},

	{0xc8, 0x80},
	{0xc9, 0x80},

	///// PAGE 21 START /////
	{0x03, 0x21},
	{0x20, 0x11},
	{0x21, 0x11},
	{0x22, 0x11},
	{0x23, 0x11},
	{0x24, 0x11},
	{0x25, 0x11},
	{0x26, 0x11},
	{0x27, 0x11},
	{0x28, 0x12},
	{0x29, 0x22},
	{0x2a, 0x22},
	{0x2b, 0x21},
	{0x2c, 0x12},
	{0x2d, 0x33},
	{0x2e, 0x32},
	{0x2f, 0x21},
	{0x30, 0x12},
	{0x31, 0x33},
	{0x32, 0x32},
	{0x33, 0x21},
	{0x34, 0x12},
	{0x35, 0x22},
	{0x36, 0x22},
	{0x37, 0x21},
	{0x38, 0x11},
	{0x39, 0x11},
	{0x3a, 0x11},
	{0x3b, 0x11},
	{0x3c, 0x11},
	{0x3d, 0x11},
	{0x3e, 0x11},
	{0x3f, 0x11},
	{0x40, 0x11},
	{0x41, 0x11},
	{0x42, 0x11},
	{0x43, 0x11},
	{0x44, 0x11},
	{0x45, 0x11},
	{0x46, 0x11},
	{0x47, 0x11},
	{0x48, 0x11},
	{0x49, 0x33},
	{0x4a, 0x33},
	{0x4b, 0x11},
	{0x4c, 0x11},
	{0x4d, 0x33},
	{0x4e, 0x33},
	{0x4f, 0x11},
	{0x50, 0x11},
	{0x51, 0x33},
	{0x52, 0x33},
	{0x53, 0x11},
	{0x54, 0x11},
	{0x55, 0x33},
	{0x56, 0x33},
	{0x57, 0x11},
	{0x58, 0x11},
	{0x59, 0x11},
	{0x5a, 0x11},
	{0x5b, 0x11},
	{0x5c, 0x11},
	{0x5d, 0x11},
	{0x5e, 0x11},
	{0x5f, 0x11},



	///// PAGE 22 START /////
	{0x03, 0x22}, //page 22
	{0x10, 0xfd},
	{0x11, 0x2e},
	{0x19, 0x00},//0x03->0x00
	{0x20, 0x30}, //For AWB Speed
	{0x21, 0x80},
	{0x22, 0x00},
	{0x23, 0x00},
	{0x24, 0x01},
	{0x25, 0x4f}, //2013-09-13 AWB Hunting

	{0x30, 0x80},
	{0x31, 0x80},
	{0x38, 0x11},
	{0x39, 0x34},
	{0x40, 0xe4}, //Stb Yth
	{0x41, 0x53}, //Stb cdiff
	{0x42, 0x55}, //Stb csum
	{0x43, 0xf3}, //Unstb Yth
	{0x44, 0x53}, //Unstb cdiff55
	{0x45, 0x33}, //Unstb csum
	{0x46, 0x00},
	{0x47, 0x09}, //2013-09-13 AWB Hunting
	{0x48, 0x00}, //2013-09-13 AWB Hunting
	{0x49, 0x0a},

	{0x60, 0x04},
	{0x61, 0xc4},
	{0x62, 0x04},
	{0x63, 0x92},
	{0x66, 0x04},
	{0x67, 0xc4},
	{0x68, 0x04},
	{0x69, 0x92},

	{0x80, 0x3e},
	{0x81, 0x20},
	{0x82, 0x30},

	{0x83, 0x5a},
	{0x84, 0x18},
	{0x85, 0x48},
	{0x86, 0x1d},


	{0x87, 0x3e}, //3b->46 awb_r_gain_max middle
	{0x88, 0x1e}, //30->3b awb_r_gain_min middle
	{0x89, 0x28}, //29->2c awb_b_gain_max middle
	{0x8a, 0x12}, //18->1b awb_b_gain_min middle

	{0x8b, 0x3a}, //3c->45 awb_r_gain_max outdoor
	{0x8c, 0x1f}, //32->3b awb_r_gain_min outdoor
	{0x8d, 0x28}, //2a->2c awb_b_gain_max outdoor
	{0x8e, 0x13}, //1b->1b awb_b_gain_min outdoor

	{0x8f, 0x56},
	{0x90, 0x4f},
	{0x91, 0x49},
	{0x92, 0x43},
	{0x93, 0x38},
	{0x94, 0x2a},
	{0x95, 0x22},
	{0x96, 0x1f},
	{0x97, 0x1c},
	{0x98, 0x1b},
	{0x99, 0x1a},
	{0x9a, 0x19},
	{0x9b, 0x99},
	{0x9c, 0x99},

	{0x9d, 0x48},
	{0x9e, 0x38},
	{0x9f, 0x30},

	{0xa0, 0x70},
	{0xa1, 0x54},
	{0xa2, 0x6f},
	{0xa3, 0xff},

	{0xa4, 0x14},
	{0xa5, 0x2c},
	{0xa6, 0xcf},

	{0xad, 0x2e},
	{0xae, 0x2a},

	{0xaf, 0x28},
	{0xb0, 0x26},

	{0xb1, 0x08},
	{0xb4, 0xbf}, //For Tracking AWB Weight
	{0xb8, 0x02}, //low(0+,1-)High Cb , (0+,1-)Low Cr
	{0xb9, 0x00},//high
	/////// PAGE 22 END ///////

	{0x03, 0x48},
	{0x39, 0x4f},
	{0x10, 0x1c},
	{0x11, 0x10},

	{0x16, 0x00},
	{0x18, 0x80},
	{0x19, 0x00},
	{0x1a, 0xf0},
	{0x24, 0x1e},

	{0x36, 0x01},
	{0x37, 0x05},
	{0x34, 0x04},
	{0x32, 0x15},
	{0x35, 0x04},
	{0x33, 0x0d},

	{0x1c, 0x01},
	{0x1d, 0x0b},
	{0x1e, 0x06},
	{0x1f, 0x09},

	{0x30, 0x06},
	{0x31, 0x40},

	{0x03, 0x20},
	{0x10, 0x9c},

	{0x03, 0x22},
	{0x10, 0xe9},

	{0x03, 0x00},
	{0x01, 0x00},
};

static struct v4l2_subdev_info hi258_8909_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_YUYV8_2X8,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};
static struct msm_camera_i2c_reg_conf hi258_8909_start_settings[] = {
	{0x03, 0x00},
	{0x01, 0x00},
};

static struct msm_camera_i2c_reg_conf hi258_8909_stop_settings[] = {
	{0x03, 0x00},
	{0x01, 0x01},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_saturation[11][3] = {
	{
		{0x03, 0x10},
		{0x61, 0x32},
		{0x62, 0x36},
	},
	{
		{0x03, 0x10},
		{0x61, 0x42},
		{0x62, 0x46},

	},
	{
		{0x03, 0x10},
		{0x61, 0x52},
		{0x62, 0x56},

	},
	{
		{0x03, 0x10},
		{0x61, 0x62},
		{0x62, 0x66},

	},
	{
		{0x03, 0x10},
		{0x61, 0x6a},
		{0x62, 0x6a},

	},
	{
		{0x03, 0x10},
		{0x61, 0x90},//82
		{0x62, 0x88},//86
	},
	{
		{0x03, 0x10},
		{0x61, 0x98},
		{0x62, 0x90},

	},
	{
		{0x03, 0x10},
		{0x61, 0xA0},
		{0x62, 0xA0},

	},
	{
		{0x03, 0x10},
		{0x61, 0xb2},
		{0x62, 0xb6},

	},
	{
		{0x03, 0x10},
		{0x61, 0xc2},
		{0x62, 0xc6},

	},
	{
		{0x03, 0x10},
		{0x61, 0xd2},
		{0x62, 0xd6},

	},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_contrast[11][3] = {
	{
		{0x03, 0x10},
		{0x13, 0x03},
		{0x48, 0x30},
	},
	{
		{0x03, 0x10},
		{0x13, 0x03},
		{0x48, 0x40},
	},
	{
		{0x03, 0x10},
		{0x13, 0x03},
		{0x48, 0x50},
	},
	{
		{0x03, 0x10},
		{0x13, 0x03},
		{0x48, 0x60},
	},
	{
		{0x03, 0x10},
		{0x13, 0x03},
		{0x48, 0x70},
	},
	{
		{0x03, 0x10},
		{0x13, 0x03},
		{0x48, 0x88},
	},
	{
		{0x03, 0x10},
		{0x13, 0x03},
		{0x48, 0x90},
	},
	{
		{0x03, 0x10},
		{0x13, 0x03},
		{0x48, 0xa0},
	},
	{
		{0x03, 0x10},
		{0x13, 0x03},
		{0x48, 0xb0},
	},
	{
		{0x03, 0x10},
		{0x13, 0x03},
		{0x48, 0xc8},
	},
	{
		{0x03, 0x10},
		{0x13, 0x03},
		{0x48, 0xc8},
	},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_sharpness[7][9] = {
	{
		{0x03, 0x13},
		{0xbc, 0x00},
		{0xbd, 0x00},
		{0xbe, 0x00},
		{0xbf, 0x00},
		{0xc0, 0x00},
		{0xc1, 0x00},
	}, /* SHARPNESS LEVEL 0*/
	{
		{0x03, 0x13},
		{0xbc, 0x08},
		{0xbd, 0x0a},
		{0xbe, 0x0a},
		{0xbf, 0x0a},
		{0xc0, 0x0c},
		{0xc1, 0x0a},
	}, /* SHARPNESS LEVEL 1*/
	{
		{0x03, 0x13},
		{0xbc, 0x10},
		{0xbd, 0x13},
		{0xbe, 0x13},
		{0xbf, 0x13},
		{0xc0, 0x15},
		{0xc1, 0x13},
	}, /* SHARPNESS LEVEL 2(Default)*/
	{
		{0x03, 0x13},
		{0xbc, 0x30}, //Lum negative Hi
		{0xbd, 0x32}, //Lum negative middle
		{0xbe, 0x30}, //Lum negative Low
		{0xbf, 0x32}, //Lum postive Hi
		{0xc0, 0x2e}, //Lum postive middle
		{0xc1, 0x2e}, //Lum postive Low

	}, /* SHARPNESS LEVEL 3*/
	{
		{0x03, 0x13},
		{0xbc, 0x1a},
		{0xbd, 0x1d},
		{0xbe, 0x1a},
		{0xbf, 0x1a},
		{0xc0, 0x1d},
		{0xc1, 0x1a},
	}, /* SHARPNESS LEVEL 4*/
	{
		{0x03, 0x13},
		{0xbc, 0x20},
		{0xbd, 0x20},
		{0xbe, 0x20},
		{0xbf, 0x20},
		{0xc0, 0x20},
		{0xc1, 0x20},
	}, /* SHARPNESS LEVEL 5*/
	{
		{0x03, 0x13},
		{0xbc, 0x30},
		{0xbd, 0x30},
		{0xbe, 0x30},
		{0xbf, 0x30},
		{0xc0, 0x30},
		{0xc1, 0x30},
	}, /* SHARPNESS LEVEL 6*/
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_iso[7][6] = {
	/* auto */
	{
		{0x03, 0x10},
		{0x4a, 0x80},
	},
	/* auto hjt */
	{
		{0x03, 0x10},
		{0x4a, 0x88},
	},
	/* iso 100 */
	{
		{0x03, 0x10},
		{0x4a, 0x90},
	},
	/* iso 200 */
	{
		{0x03, 0x10},
		{0x4a, 0x98},
	},
	/* iso 400 */
	{
		{0x03, 0x10},
		{0x4a, 0x9c},
	},
	/* iso 800 */
	{
		{0x03, 0x10},
		{0x4a, 0xa0},
	},
	/* iso 1600 */
	{
		{0x03, 0x10},
		{0x4a, 0xa5},
	},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_exposure_compensation[5][2] = {
	/* -2 */
	{
		{0x03, 0x10},
		{0x40, 0xae},
	},
	/* -1 */
	{
		{0x03, 0x10},
		{0x40, 0x9e},
	},
	/* 0 */
	{
		{0x03, 0x10},
		{0x40, 0x8e},
	},
	/* 1 */
	{
		{0x03, 0x10},
		{0x40, 0x05},
	},
	/* 2 */
	{
		{0x03, 0x10},
		{0x40, 0x15},
	},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_antibanding[][50] = {
	/* OFF */
	{
		{0x03, 0x00},
		{0x01, 0x08},
		{0xff, 0xff},
		{0xff, 0xff},
		{0xff, 0xff},
		{0x03, 0x20},
		{0x03, 0x00},
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	

	},
	/* 60Hz */
	{

		{0x03, 0x00},
		{0x01, 0x08},
        {0xff, 0xff},
        {0xff, 0xff},
        {0xff, 0xff},
      

		{0x03, 0x20}, //Page 20
		{0x10, 0x1c},
		{0x18, 0x38},
		
		{0x83, 0x0a}, //EXP Normal 9.23 fps 
		{0x84, 0xb6}, 
		{0x85, 0x30}, 
		{0x86, 0x01}, //EXPMin 13000.00 fps
		{0x87, 0xf4}, 
		{0x88, 0x0a}, //EXP Max 60hz 9.23 fps 
		{0x89, 0xb6}, 
		{0x8a, 0x30}, 
		{0xa5, 0x0a}, //EXP Max 50hz 9.09 fps 
		{0xa6, 0xe8}, 
		{0xa7, 0xf8}, 
		{0x8B, 0xfd}, //EXP100 
		{0x8C, 0xe8}, 
		{0x8D, 0xd2}, //EXP120 
		{0x8E, 0xf0}, 
		{0x9c, 0x1d}, //EXP Limit 866.67 fps 
		{0x9d, 0x4c}, 
		{0x9e, 0x01}, //EXP Unit 
		{0x9f, 0xf4}, 
		{0xa3, 0x00}, //Outdoor Int 
		{0xa4, 0xd2}, 




		{0x18, 0x30},
		{0x10, 0x8c},
		
		{0x03, 0x00},
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	



	

	},
	/* 50Hz */
	{
		{0x03, 0x00},
		{0x01, 0x08},
        {0xff, 0xff},
        {0xff, 0xff},
        {0xff, 0xff},
      

		{0x03, 0x20}, //Page 20
		{0x10, 0x1c},
		{0x18, 0x38},

		{0x83, 0x0a}, //EXP Normal 9.09 fps 
		{0x84, 0xe8}, 
		{0x85, 0xf8}, 
		{0x86, 0x01}, //EXPMin 13000.00 fps
		{0x87, 0xf4}, 
		{0x88, 0x0a}, //EXP Max 60hz 9.23 fps 
		{0x89, 0xb6}, 
		{0x8a, 0x30}, 
		{0xa5, 0x0a}, //EXP Max 50hz 9.09 fps 
		{0xa6, 0xe8}, 
		{0xa7, 0xf8}, 
		{0x8B, 0xfd}, //EXP100 
		{0x8C, 0xe8}, 
		{0x8D, 0xd2}, //EXP120 
		{0x8E, 0xf0}, 
		{0x9c, 0x1d}, //EXP Limit 866.67 fps 
		{0x9d, 0x4c}, 
		{0x9e, 0x01}, //EXP Unit 
		{0x9f, 0xf4}, 
		{0xa3, 0x00}, //Outdoor Int 
		{0xa4, 0xd2}, 



		{0x18, 0x30},			
		{0x10, 0x9c},
		
		{0x03, 0x00},
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},

	},
	/* AUTO */
	{
		{0x03, 0x00},
		{0x01, 0x08},
		{0xff, 0xff},
		{0xff, 0xff},
		{0xff, 0xff},
		
		
		{0x03, 0x20}, //Page 20
		{0x18, 0x38},

		
		{0x10, 0xec},
		
		{0x03, 0x20}, //Page 20
		{0x18, 0x30},

		{0x03, 0x00},
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	
		{0x01, 0x00},	


	},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_effect_normal[] = {
	/* normal: */

	{0x03, 0x10},
	{0x11, 0x03},
	{0x12, 0x30},
	{0x13, 0x03},
	{0x44, 0x80},
	{0x45, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_effect_black_white[] = {
	/* B&W: */

	{0x03, 0x10},
	{0x11, 0x03},
	{0x12, 0x33},
	{0x13, 0x02},
	{0x44, 0x80},
	{0x45, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_effect_negative[] = {
	/* Negative: */
	{0x03, 0x10},
	{0x11, 0x03},
	{0x12, 0x38},
	{0x13, 0x03},
	{0x44, 0x80},
	{0x45, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_effect_old_movie[] = {
	/* Sepia(antique): */

	{0x03, 0x10},
	{0x11, 0x03},
	{0x12, 0x33},
	{0x13, 0x03},
	{0x44, 0x60},
	{0x45, 0xa3},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_effect_bluecarving[] = {

	{0x03, 0x10},
	{0x11, 0x03},
	{0x12, 0x03},
	{0x13, 0x02},
	{0x44, 0xb0},
	{0x45, 0x40},
};


static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_auto[] = {
	/* <SCENE_auto> */
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x83, 0x5a},
	{0x84, 0x21},
	{0x85, 0x44},
	{0x86, 0x1d},
	{0x03, 0x10},
    {0x41, 0x00},
	{0x4a, 0x80},

};
static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_portrait[] = {
	/* <CAMTUNING_SCENE_PORTRAIT> */
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x83, 0x42},
	{0x84, 0x1a},
	{0x85, 0x40},
	{0x86, 0x20},
	{0x03, 0x10},
	//{0x40, 0x8e},
	{0x4a, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_landscape[] = {
	/* <CAMTUNING_SCENE_LANDSCAPE> */
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x83, 0x42},
	{0x84, 0x1a},
	{0x85, 0x40},
	{0x86, 0x20},
	{0x03, 0x10},
	{0x41, 0x05},
	{0x4a, 0x80},

};
static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_night[] = {
	/* <SCENE_NIGHT> */
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x83, 0x4f},
	{0x84, 0x1a},
	{0x85, 0x47},
	{0x86, 0x20},
	{0x03, 0x10},
	//{0x40, 0x8e},
	{0x4a, 0x90},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_snow[] = {
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x83, 0x42},
	{0x84, 0x1a},
	{0x85, 0x40},
	{0x86, 0x20},
	{0x03, 0x10},
	{0x41, 0x05},
	{0x4a, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_beach[] = {
	{0x03, 0x22},
	{0x10, 0x69},
	{0x80, 0x48},
	{0x82, 0x23},
	{0x83, 0x49},
	{0x84, 0x47},
	{0x85, 0x24},
	{0x86, 0x22},
	{0x03, 0x10},
	{0x41, 0x00},
	{0x4a, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_sunset[] = {
	{0x03, 0x22},
	{0x10, 0x69},
	{0x80, 0x36},
	{0x82, 0x28},
	{0x83, 0x37},
	{0x84, 0x35},
	{0x85, 0x29},
	{0x86, 0x27},
	{0x03, 0x10},
	{0x41, 0x00},
	{0x4a, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_backlight[] = {
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x83, 0x42},
	{0x84, 0x1a},
	{0x85, 0x40},
	{0x86, 0x20},
	{0x03, 0x10},
	{0x41, 0x00},
	{0x4a, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_sports[] = {
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x83, 0x42},
	{0x84, 0x1a},
	{0x85, 0x40},
	{0x86, 0x20},
	{0x03, 0x10},
	{0x41, 0x00},
	{0x4a, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_antishake[] = {
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x83, 0x42},
	{0x84, 0x1a},
	{0x85, 0x40},
	{0x86, 0x20},
	{0x03, 0x10},
	{0x41, 0x00},
	{0x4a, 0x80},
};
static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_flowers[] = {
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x83, 0x42},
	{0x84, 0x1a},
	{0x85, 0x40},
	{0x86, 0x20},
	{0x03, 0x10},
	{0x41, 0x00},
	{0x4a, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_candlelight[] = {
	{0x03, 0x22},
	{0x10, 0x69},
	{0x80, 0x25},
	{0x82, 0x45},
	{0x83, 0x26},
	{0x84, 0x24},
	{0x85, 0x46},
	{0x86, 0x44},
	{0x03, 0x10},
	{0x41, 0x00},
	{0x4a, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_fireworks[] = {
	{0x03, 0x22},
	{0x10, 0x69},
	{0x80, 0x41},
	{0x82, 0x22},
	{0x83, 0x42},
	{0x84, 0x40},
	{0x85, 0x23},
	{0x86, 0x21},
	{0x03, 0x10},
	{0x41, 0x00},
	{0x4a, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_party[] = {
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x83, 0x42},
	{0x84, 0x1a},
	{0x85, 0x40},
	{0x86, 0x20},
	{0x03, 0x10},
	{0x41, 0x00},
	{0x4a, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_night_portrait[] = {
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x83, 0x42},
	{0x84, 0x1a},
	{0x85, 0x40},
	{0x86, 0x20},
	{0x03, 0x10},
	{0x41, 0x15},
	{0x4a, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_theatre[] = {
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x83, 0x42},
	{0x84, 0x1a},
	{0x85, 0x40},
	{0x86, 0x20},
	{0x03, 0x10},
	{0x41, 0x00},
	{0x4a, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_action[] = {
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x83, 0x42},
	{0x84, 0x1a},
	{0x85, 0x40},
	{0x86, 0x20},
	{0x03, 0x10},
	{0x41, 0x00},
	{0x4a, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_ar[] = {
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x83, 0x42},
	{0x84, 0x1a},
	{0x85, 0x40},
	{0x86, 0x20},
	{0x03, 0x10},
	{0x41, 0x00},
	{0x4a, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_face_priority[] = {
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x83, 0x42},
	{0x84, 0x1a},
	{0x85, 0x40},
	{0x86, 0x20},
	{0x03, 0x10},
	{0x41, 0x00},
	{0x4a, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_barcode[] = {
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x83, 0x42},
	{0x84, 0x1a},
	{0x85, 0x40},
	{0x86, 0x20},
	{0x03, 0x10},
	{0x41, 0x00},
	{0x4a, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_scene_hdr[] = {
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x83, 0x42},
	{0x84, 0x1a},
	{0x85, 0x40},
	{0x86, 0x20},
	{0x03, 0x10},
	{0x41, 0x00},
	{0x4a, 0x80},
};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_wb_auto[] = {
	/* Auto: */
	{0x03, 0x22},
	{0x10, 0xe9},
	{0x11, 0x2e},
	{0x80, 0x3e},
	{0x81, 0x20},
	{0x82, 0x32},
	{0x83, 0x53},
	{0x84, 0x18},
	{0x85, 0x50},
	{0x86, 0x28},
	{0x10, 0xe9},

};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_wb_sunny[] = {
	/* Sunny: */
	{0x03, 0x22},
	{0x11, 0x28},
	//{0x80, 0x59},
	//{0x82, 0x29},
	{0x83, 0x36},
	{0x84, 0x34},
	{0x85, 0x26},
	{0x86, 0x24},
	{0x10, 0xe9},

};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_wb_cloudy[] = {
	/* Cloudy: */
	{0x03, 0x22},
	{0x11, 0x2c},
	//{0x80, 0x50},
	//{0x81, 0x20},
	//{0x82, 0x18},
	{0x83, 0x43},
	{0x84, 0x40},
	{0x85, 0x23},
	{0x86, 0x21},
	{0x10, 0xe9},

};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_wb_office[] = {
	/* Office: */
	{0x03, 0x22},
	{0x11, 0x28},
	{0x80, 0x40},
	{0x82, 0x30},
	{0x83, 0x41},
	{0x84, 0x31},
	{0x85, 0x35},
	{0x86, 0x27},
	{0x10, 0xe9},

};

static struct msm_camera_i2c_reg_conf HI258_8909_reg_wb_home[] = {
	/* Home: */
	{0x03, 0x22},
	{0x11, 0x28},
	{0x80, 0x30},
	{0x82, 0x48},
	{0x83, 0x31},
	{0x84, 0x2d},
	{0x85, 0x4a},
	{0x86, 0x46},
	{0x10, 0xe9},

};
static const struct i2c_device_id hi258_8909_i2c_id[] = {
	{HI258_8909_SENSOR_NAME, (kernel_ulong_t)&hi258_8909_s_ctrl},
	{ }
};

static int32_t msm_hi258_8909_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &hi258_8909_s_ctrl);
}

static struct i2c_driver hi258_8909_i2c_driver = {
	.id_table = hi258_8909_i2c_id,
	.probe  = msm_hi258_8909_i2c_probe,
	.driver = {
		.name = HI258_8909_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client hi258_8909_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
};

static const struct of_device_id hi258_8909_dt_match[] = {
	{.compatible = "qcom,hi258_8909", .data = &hi258_8909_s_ctrl},
	{}
};

MODULE_DEVICE_TABLE(of, hi258_8909_dt_match);

static void hi258_8909_i2c_write_table(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_camera_i2c_reg_conf *table,
		int num)
{
	int i = 0;
	int rc = 0;
	for (i = 0; i < num; ++i) 
	{
        if(0xff == table->reg_addr)
        {
        	msleep(50);
			printk("[TSP] delay 100ms.(%s,%d)\n",__func__,__LINE__);
        }		
		else
		{
			rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
				i2c_write(s_ctrl->sensor_i2c_client, table->reg_addr,
				table->reg_data,MSM_CAMERA_I2C_BYTE_DATA);
			//CDBG("camera reg addr = 0x%x, 0x%x\n", table->reg_addr,
			//	table->reg_data);
			if (rc < 0) 
			{
				msleep(100);
				rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
				i2c_write(
					s_ctrl->sensor_i2c_client, table->reg_addr,
					table->reg_data,MSM_CAMERA_I2C_BYTE_DATA);
			}
			//s_ctrl->sensor_i2c_client->i2c_func_tbl->
			//	i2c_read(
			//		s_ctrl->sensor_i2c_client, table->reg_addr,
			//		&temp,
			//		MSM_CAMERA_I2C_BYTE_DATA);
			//printk("hi258_i2c_write_table reg_addr:%x reg_data:%x\n",table->reg_addr,temp);
		}
		
		table++;
	}
}
static int32_t hi258_8909_platform_probe(struct platform_device *pdev)
{
	int32_t rc;
	const struct of_device_id *match;
	match = of_match_device(hi258_8909_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);
	return rc;
}

static struct platform_driver hi258_8909_platform_driver = {
	.driver = {
		.name = "qcom,hi258_8909",
		.owner = THIS_MODULE,
		.of_match_table = hi258_8909_dt_match,
	},
	.probe = hi258_8909_platform_probe,
};

static int __init hi258_8909_init_module(void)
{
	int32_t rc;
	pr_err("%s:%d\n", __func__, __LINE__);
	rc = platform_driver_probe(
		&hi258_8909_platform_driver , hi258_8909_platform_probe);
	if (!rc)
		return rc;
	pr_err("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(&hi258_8909_i2c_driver);
}

static void __exit hi258_8909_exit_module(void)
{
	pr_err("%s:%d\n", __func__, __LINE__);
	if (hi258_8909_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&hi258_8909_s_ctrl);
		platform_driver_unregister(&hi258_8909_platform_driver);
	} else
		i2c_del_driver(&hi258_8909_i2c_driver);
	return;
}

static void hi258_8909_set_saturation(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	pr_err("%s %d\n", __func__, value);
	hi258_8909_i2c_write_table(s_ctrl, &HI258_8909_reg_saturation[value][0],
		ARRAY_SIZE(HI258_8909_reg_saturation[value]));
}
static void hi258_8909_set_contrast(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	pr_err("%s %d\n", __func__, value);
	hi258_8909_i2c_write_table(s_ctrl, &HI258_8909_reg_contrast[value][0],
		ARRAY_SIZE(HI258_8909_reg_contrast[value]));
}

static void hi258_8909_set_sharpness(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	int val = value / 6;
	pr_err("%s %d\n", __func__, val);
	hi258_8909_i2c_write_table(s_ctrl, &HI258_8909_reg_sharpness[val][0],
		ARRAY_SIZE(HI258_8909_reg_sharpness[val]));
}

static void hi258_8909_set_iso(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	pr_err("%s %d\n", __func__, value);
	hi258_8909_i2c_write_table(s_ctrl, &HI258_8909_reg_iso[value][0],
		ARRAY_SIZE(HI258_8909_reg_iso[value]));
}

static void hi258_8909_set_exposure_compensation(struct msm_sensor_ctrl_t *s_ctrl,
	int value)
{
	int val = (value + 12) / 6;
	if (val == 2)
		ae_return = 0;
	else
		ae_return = 1;
	pr_err("%s %d\n", __func__, val);
	hi258_8909_i2c_write_table(s_ctrl,
		&HI258_8909_reg_exposure_compensation[val][0],
		ARRAY_SIZE(HI258_8909_reg_exposure_compensation[val]));
}

static void hi258_8909_set_effect(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	pr_err("%s %d\n", __func__, value);
	switch (value) {
	case MSM_CAMERA_EFFECT_MODE_OFF: {
		pr_err("set effect MSM_CAMERA_EFFECT_MODE_OFF %d\n",
				MSM_CAMERA_EFFECT_MODE_OFF);
		hi258_8909_i2c_write_table(s_ctrl, &HI258_8909_reg_effect_normal[0],
			ARRAY_SIZE(HI258_8909_reg_effect_normal));
		break;
	}
	case MSM_CAMERA_EFFECT_MODE_MONO: {
		pr_err("set effect MSM_CAMERA_EFFECT_MODE_MONO %d\n",
				MSM_CAMERA_EFFECT_MODE_MONO);
		hi258_8909_i2c_write_table(s_ctrl, &HI258_8909_reg_effect_black_white[0],
			ARRAY_SIZE(HI258_8909_reg_effect_black_white));
		break;
	}
	case MSM_CAMERA_EFFECT_MODE_NEGATIVE: {
		pr_err("set effect MSM_CAMERA_EFFECT_MODE_NEGATIVE %d\n",
				MSM_CAMERA_EFFECT_MODE_NEGATIVE);
		hi258_8909_i2c_write_table(s_ctrl, &HI258_8909_reg_effect_negative[0],
			ARRAY_SIZE(HI258_8909_reg_effect_negative));
		break;
	}
	case MSM_CAMERA_EFFECT_MODE_SEPIA: {
		pr_err("set effect MSM_CAMERA_EFFECT_MODE_SEPIA %d\n",
				MSM_CAMERA_EFFECT_MODE_SEPIA);
		hi258_8909_i2c_write_table(s_ctrl, &HI258_8909_reg_effect_old_movie[0],
			ARRAY_SIZE(HI258_8909_reg_effect_old_movie));
		break;
	}
	case MSM_CAMERA_EFFECT_MODE_AQUA: {
		pr_err("set effect MSM_CAMERA_EFFECT_MODE_AQUA %d\n",
				MSM_CAMERA_EFFECT_MODE_AQUA);
		hi258_8909_i2c_write_table(s_ctrl, &HI258_8909_reg_effect_bluecarving[0],
			ARRAY_SIZE(HI258_8909_reg_effect_bluecarving));
		break;
	}
	default:
		hi258_8909_i2c_write_table(s_ctrl, &HI258_8909_reg_effect_normal[0],
			ARRAY_SIZE(HI258_8909_reg_effect_normal));
	}
}
static void hi258_8909_set_antibanding(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	pr_err("%s %d\n", __func__, value);
	hi258_8909_i2c_write_table(s_ctrl, &HI258_8909_reg_antibanding[value][0],
		ARRAY_SIZE(HI258_8909_reg_antibanding[value]));
}
static void hi258_8909_set_scene_mode(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	pr_err("%s %d\n", __func__, value);
	if (!awb_return && !ae_return) {
		switch (value) {
		case MSM_CAMERA_SCENE_MODE_AUTO:
		case MSM_CAMERA_SCENE_MODE_OFF: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_auto[0],
			ARRAY_SIZE(HI258_8909_reg_scene_auto));
		break;
		}
		case MSM_CAMERA_SCENE_MODE_NIGHT: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_night[0],
			ARRAY_SIZE(HI258_8909_reg_scene_night));
					break;
		}
		case MSM_CAMERA_SCENE_MODE_LANDSCAPE: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_landscape[0],
			ARRAY_SIZE(HI258_8909_reg_scene_landscape));
			break;
		}
		case MSM_CAMERA_SCENE_MODE_PORTRAIT: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_portrait[0],
			ARRAY_SIZE(HI258_8909_reg_scene_portrait));
			break;
		}
		case MSM_CAMERA_SCENE_MODE_SNOW: {
				hi258_8909_i2c_write_table(s_ctrl,
				&HI258_8909_reg_scene_snow[0],
				ARRAY_SIZE(HI258_8909_reg_scene_snow));
			break;
		}

		case MSM_CAMERA_SCENE_MODE_BEACH: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_beach[0],
			ARRAY_SIZE(HI258_8909_reg_scene_beach));
			break;
		}

		case MSM_CAMERA_SCENE_MODE_SUNSET: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_sunset[0],
			ARRAY_SIZE(HI258_8909_reg_scene_sunset));
			break;
		}

		case MSM_CAMERA_SCENE_MODE_BACKLIGHT: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_backlight[0],
			ARRAY_SIZE(HI258_8909_reg_scene_backlight));
			break;
		}

		case MSM_CAMERA_SCENE_MODE_SPORTS: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_sports[0],
			ARRAY_SIZE(HI258_8909_reg_scene_sports));
			break;
		}


		case MSM_CAMERA_SCENE_MODE_ANTISHAKE: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_antishake[0],
			ARRAY_SIZE(HI258_8909_reg_scene_antishake));
			break;
		}


		case MSM_CAMERA_SCENE_MODE_FLOWERS: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_flowers[0],
			ARRAY_SIZE(HI258_8909_reg_scene_flowers));
			break;
		}


		case MSM_CAMERA_SCENE_MODE_CANDLELIGHT: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_candlelight[0],
			ARRAY_SIZE(HI258_8909_reg_scene_candlelight));
			break;
		}

		case MSM_CAMERA_SCENE_MODE_FIREWORKS: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_fireworks[0],
			ARRAY_SIZE(HI258_8909_reg_scene_fireworks));
			break;
		}

		case MSM_CAMERA_SCENE_MODE_PARTY: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_party[0],
			ARRAY_SIZE(HI258_8909_reg_scene_party));
			break;
		}

		case MSM_CAMERA_SCENE_MODE_NIGHT_PORTRAIT: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_night_portrait[0],
			ARRAY_SIZE(HI258_8909_reg_scene_night_portrait));
			break;
		}

		case MSM_CAMERA_SCENE_MODE_THEATRE: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_theatre[0],
			ARRAY_SIZE(HI258_8909_reg_scene_theatre));
			break;
		}

		case MSM_CAMERA_SCENE_MODE_ACTION: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_action[0],
			ARRAY_SIZE(HI258_8909_reg_scene_action));
			break;
		}

		case MSM_CAMERA_SCENE_MODE_AR: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_ar[0],
			ARRAY_SIZE(HI258_8909_reg_scene_ar));
			break;
		}

		case MSM_CAMERA_SCENE_MODE_FACE_PRIORITY: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_face_priority[0],
			ARRAY_SIZE(HI258_8909_reg_scene_face_priority));
			break;
		}

		case MSM_CAMERA_SCENE_MODE_BARCODE: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_barcode[0],
			ARRAY_SIZE(HI258_8909_reg_scene_barcode));
			break;
		}

		case MSM_CAMERA_SCENE_MODE_HDR: {
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_hdr[0],
			ARRAY_SIZE(HI258_8909_reg_scene_hdr));
			break;
		}
		default:
			hi258_8909_i2c_write_table(s_ctrl,
			&HI258_8909_reg_scene_auto[0],
			ARRAY_SIZE(HI258_8909_reg_scene_auto));
		}
	}
}

static void hi258_8909_set_white_balance_mode(struct msm_sensor_ctrl_t *s_ctrl,
	int value)
{
	pr_err("%s %d\n", __func__, value);
	pr_err("%s %d\n", __func__, value);
	switch (value) {
	case MSM_CAMERA_WB_MODE_AUTO: {
		awb_return = 0;
		hi258_8909_i2c_write_table(s_ctrl, &HI258_8909_reg_wb_auto[0],
			ARRAY_SIZE(HI258_8909_reg_wb_auto));
		break;
	}
	case MSM_CAMERA_WB_MODE_INCANDESCENT: {
		awb_return = 1;
		hi258_8909_i2c_write_table(s_ctrl, &HI258_8909_reg_wb_home[0],
			ARRAY_SIZE(HI258_8909_reg_wb_home));
		break;
	}
	case MSM_CAMERA_WB_MODE_DAYLIGHT: {
		awb_return = 1;
		hi258_8909_i2c_write_table(s_ctrl, &HI258_8909_reg_wb_sunny[0],
			ARRAY_SIZE(HI258_8909_reg_wb_sunny));
					break;
	}
	case MSM_CAMERA_WB_MODE_FLUORESCENT: {
		awb_return = 1;
		hi258_8909_i2c_write_table(s_ctrl, &HI258_8909_reg_wb_office[0],
			ARRAY_SIZE(HI258_8909_reg_wb_office));
					break;
	}
	case MSM_CAMERA_WB_MODE_CLOUDY_DAYLIGHT: {
		awb_return = 1;
		hi258_8909_i2c_write_table(s_ctrl, &HI258_8909_reg_wb_cloudy[0],
			ARRAY_SIZE(HI258_8909_reg_wb_cloudy));
					break;
	}
	default:
		hi258_8909_i2c_write_table(s_ctrl, &HI258_8909_reg_wb_auto[0],
		ARRAY_SIZE(HI258_8909_reg_wb_auto));
	}
}

int32_t hi258_8909_sensor_config(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp)
{
	struct sensorb_cfg_data *cdata = (struct sensorb_cfg_data *)argp;
	long rc = 0;
	int32_t i = 0;
	mutex_lock(s_ctrl->msm_sensor_mutex);
	pr_err("%s:%d %s cfgtype = %d\n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
	switch (cdata->cfgtype) {
	case CFG_GET_SENSOR_INFO:
		pr_err("hi258 CFG_GET_SENSOR_INFO\n");
		memcpy(cdata->cfg.sensor_info.sensor_name,
			s_ctrl->sensordata->sensor_name,
			sizeof(cdata->cfg.sensor_info.sensor_name));
		cdata->cfg.sensor_info.session_id =
			s_ctrl->sensordata->sensor_info->session_id;
		for (i = 0; i < SUB_MODULE_MAX; i++)
			cdata->cfg.sensor_info.subdev_id[i] =
				s_ctrl->sensordata->sensor_info->subdev_id[i];
		cdata->cfg.sensor_info.is_mount_angle_valid =
			s_ctrl->sensordata->sensor_info->is_mount_angle_valid;
		cdata->cfg.sensor_info.sensor_mount_angle =
			s_ctrl->sensordata->sensor_info->sensor_mount_angle;
		pr_err("%s:%d sensor name %s\n", __func__, __LINE__,
			cdata->cfg.sensor_info.sensor_name);
		pr_err("%s:%d session id %d\n", __func__, __LINE__,
			cdata->cfg.sensor_info.session_id);
		for (i = 0; i < SUB_MODULE_MAX; i++)
			pr_err("%s:%d subdev_id[%d] %d\n", __func__, __LINE__, i,
				cdata->cfg.sensor_info.subdev_id[i]);
		pr_err("%s:%d mount angle valid %d value %d\n", __func__,
			__LINE__, cdata->cfg.sensor_info.is_mount_angle_valid,
			cdata->cfg.sensor_info.sensor_mount_angle);

		break;
	case CFG_SET_INIT_SETTING:
		pr_err("hi258 CFG_SET_INIT_SETTING !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		/* 1. Write Recommend settings */
		/* 2. Write change settings */
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(
			s_ctrl->sensor_i2c_client, hi258_8909_recommend_settings,
			ARRAY_SIZE(hi258_8909_recommend_settings),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	case CFG_SET_RESOLUTION:
		{
			/*copy from user the desired resoltuion*/
			enum msm_sensor_resolution_t res =
				MSM_SENSOR_INVALID_RES;
			pr_err("hi258 CFG_SET_RESOLUTION\n");
			if (copy_from_user(&res, (void *)cdata->cfg.setting,
				sizeof(enum msm_sensor_resolution_t))) {
				pr_err("%s:%d failed\n", __func__, __LINE__);
				rc = -EFAULT;
				break;
			}

			if (res == MSM_SENSOR_RES_FULL) {

				rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
					i2c_write_conf_tbl(
					s_ctrl->sensor_i2c_client,
					hi258_8909_snapshot_settings,
					ARRAY_SIZE(hi258_8909_snapshot_settings),
					MSM_CAMERA_I2C_BYTE_DATA);
					pr_err("%s:%d res =%d\n hi258_8909_snapshot_settings !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
					__func__, __LINE__, res);
			//yangchen add for start video fail
			rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
				i2c_write_conf_tbl(
				s_ctrl->sensor_i2c_client, hi258_8909_stop_settings,
				ARRAY_SIZE(hi258_8909_stop_settings),
				MSM_CAMERA_I2C_BYTE_DATA);
			//yangchen add for start video fail
				
			} else if (res == MSM_SENSOR_RES_QTR) {
				rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
					i2c_write_conf_tbl(
					s_ctrl->sensor_i2c_client,
					hi258_8909_preview_settings,
					ARRAY_SIZE(hi258_8909_preview_settings),
					MSM_CAMERA_I2C_BYTE_DATA);
				pr_err("%s:%d res =%d hi258_8909_preview_settings   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1\n",
					 __func__, __LINE__, res);
				
				//yangchen add for start video fail
				//	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
				//	         i2c_write_conf_tbl(
			    //             s_ctrl->sensor_i2c_client, hi258_8909_stop_settings,
			    //             ARRAY_SIZE(hi258_8909_stop_settings),
			    //             MSM_CAMERA_I2C_BYTE_DATA);
			     //yangchen add for start video fail
						
			} else {
				pr_err("%s:%d failed resoultion set\n",
					__func__,
					__LINE__);
				rc = -EFAULT;
			}
		}
		break;
	case CFG_SET_STOP_STREAM:
		pr_err("hi258 CFG_SET_STOP_STREAM\n");
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(
			s_ctrl->sensor_i2c_client, hi258_8909_stop_settings,
			ARRAY_SIZE(hi258_8909_stop_settings),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	case CFG_SET_START_STREAM:
		pr_err("hi258 CFG_SET_START_STREAM\n");
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(
			s_ctrl->sensor_i2c_client, hi258_8909_start_settings,
			ARRAY_SIZE(hi258_8909_start_settings),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	case CFG_GET_SENSOR_INIT_PARAMS:
		pr_err("hi258 CFG_GET_SENSOR_INIT_PARAMS\n");
		cdata->cfg.sensor_init_params.modes_supported =
			s_ctrl->sensordata->sensor_info->modes_supported;
		cdata->cfg.sensor_init_params.position =
			s_ctrl->sensordata->sensor_info->position;
		cdata->cfg.sensor_init_params.sensor_mount_angle =
			s_ctrl->sensordata->sensor_info->sensor_mount_angle;
		pr_err("%s:%d init params mode %d pos %d mount %d\n", __func__,
			__LINE__,
			cdata->cfg.sensor_init_params.modes_supported,
			cdata->cfg.sensor_init_params.position,
			cdata->cfg.sensor_init_params.sensor_mount_angle);
		break;
		case CFG_SET_SLAVE_INFO: {
		struct msm_camera_sensor_slave_info *sensor_slave_info = NULL;
		struct msm_camera_power_ctrl_t *p_ctrl;
		uint16_t size;
		int slave_index = 0;
		sensor_slave_info = kmalloc(sizeof(struct msm_camera_sensor_slave_info)
			*1, GFP_KERNEL);
		if (!sensor_slave_info) {
			rc = -ENOMEM;
			break;	
		}
		if (copy_from_user(sensor_slave_info,
			(void *)cdata->cfg.setting,
		sizeof(struct msm_camera_sensor_slave_info))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		/* Update sensor slave address */
		if (sensor_slave_info->slave_addr)
			s_ctrl->sensor_i2c_client->cci_client->sid =
			sensor_slave_info->slave_addr >> 1;

		/* Update sensor address type */
		s_ctrl->sensor_i2c_client->addr_type =
			sensor_slave_info->addr_type;

		/* Update power up / down sequence */
			p_ctrl = &s_ctrl->sensordata->power_info;
			size = sensor_slave_info->power_setting_array.size;
		if (p_ctrl->power_setting_size < size) {
			struct msm_sensor_power_setting *tmp;
			tmp = kmalloc(sizeof(struct msm_sensor_power_setting)
				      * size, GFP_KERNEL);
			if (!tmp) {
				pr_err("%s: failed to alloc mem\n", __func__);
				rc = -ENOMEM;
				break;
			}
			kfree(p_ctrl->power_setting);
			p_ctrl->power_setting = tmp;
		}
		p_ctrl->power_setting_size = size;

		rc = copy_from_user(p_ctrl->power_setting, (void *)
			sensor_slave_info->power_setting_array.power_setting,
			size * sizeof(struct msm_sensor_power_setting));
		if (rc) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
			pr_err("%s sensor id %x addr type %d sensor reg %x\n"
				"sensor id %x\n", __func__,
				sensor_slave_info->slave_addr,
				sensor_slave_info->addr_type,
				sensor_slave_info->
					sensor_id_info.sensor_id_reg_addr,
				sensor_slave_info->sensor_id_info.sensor_id);
		for (slave_index = 0;
			slave_index < p_ctrl->power_setting_size;
			slave_index++) {
			pr_err("%s i %d power setting %d %d %ld %d\n", __func__,
				slave_index,
				p_ctrl->power_setting[slave_index].seq_type,
				p_ctrl->power_setting[slave_index].seq_val,
				p_ctrl->power_setting[slave_index].config_val,
				p_ctrl->power_setting[slave_index].delay);
		}
		break;
	}
	case CFG_WRITE_I2C_ARRAY: {
		struct msm_camera_i2c_reg_setting conf_array;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;
		CDBG("CFG_WRITE_I2C_ARRAY\n");
		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write_table(
			s_ctrl->sensor_i2c_client, &conf_array);
		kfree(reg_setting);
		break;
	}
	case CFG_WRITE_I2C_SEQ_ARRAY: {

		struct msm_camera_i2c_seq_reg_setting conf_array;
		struct msm_camera_i2c_seq_reg_array *reg_setting = NULL;
		CDBG("CFG_WRITE_I2C_SEQ_ARRAY\n");
		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_seq_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_seq_reg_array)),
			GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_seq_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_seq_table(s_ctrl->sensor_i2c_client,
			&conf_array);
		kfree(reg_setting);
		break;
	}

	case CFG_POWER_UP:
		pr_err("hi258 CFG_POWER_UP\n");
		if (s_ctrl->func_tbl->sensor_power_up)
			rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
		else
			rc = -EFAULT;
		break;

	case CFG_POWER_DOWN:
		pr_err("hi258 CFG_POWER_DOWN\n");
		if (s_ctrl->func_tbl->sensor_power_down)
			rc = s_ctrl->func_tbl->sensor_power_down(s_ctrl);
		else
			rc = -EFAULT;
		break;

	case CFG_SET_STOP_STREAM_SETTING: {

		struct msm_camera_i2c_reg_setting *stop_setting =
			&s_ctrl->stop_setting;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;
		pr_err("hi258 CFG_SET_STOP_STREAM_SETTING\n");
		if (copy_from_user(stop_setting, (void *)cdata->cfg.setting,
		    sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = stop_setting->reg_setting;
		stop_setting->reg_setting = kzalloc(stop_setting->size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!stop_setting->reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(stop_setting->reg_setting,
		    (void *)reg_setting, stop_setting->size *
		    sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(stop_setting->reg_setting);
			stop_setting->reg_setting = NULL;
			stop_setting->size = 0;
			rc = -EFAULT;
			break;
		}
		break;
		}
	case CFG_SET_SATURATION: {
		int32_t sat_lev;
		if (copy_from_user(&sat_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_err("%s: Saturation Value is %d", __func__, sat_lev);
		hi258_8909_set_saturation(s_ctrl, sat_lev);
		break;
	}
	case CFG_SET_CONTRAST: {
		int32_t con_lev;
		if (copy_from_user(&con_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_err("%s: Contrast Value is %d", __func__, con_lev);
		hi258_8909_set_contrast(s_ctrl, con_lev);
		break;
	}
	case CFG_SET_SHARPNESS: {
		int32_t shp_lev;
		if (copy_from_user(&shp_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_err("%s: Sharpness Value is %d", __func__, shp_lev);
		hi258_8909_set_sharpness(s_ctrl, shp_lev);
		break;
	}
	case CFG_SET_ISO: {
		int32_t iso_lev;
		if (copy_from_user(&iso_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_err("%s: ISO Value is %d", __func__, iso_lev);
		hi258_8909_set_iso(s_ctrl, iso_lev);
		break;
	}
	case CFG_SET_EXPOSURE_COMPENSATION: {
		int32_t ec_lev;
		if (copy_from_user(&ec_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_err("%s: Exposure compensation Value is %d",
			__func__, ec_lev);
		hi258_8909_set_exposure_compensation(s_ctrl, ec_lev);
		break;
	}
	case CFG_SET_EFFECT: {
		int32_t effect_mode;
		if (copy_from_user(&effect_mode, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_err("%s: Effect mode is %d", __func__, effect_mode);
		hi258_8909_set_effect(s_ctrl, effect_mode);
		break;
	}
	case CFG_SET_ANTIBANDING: {
		int32_t antibanding_mode;
		if (copy_from_user(&antibanding_mode,
			(void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_err("%s: anti-banding mode is %d", __func__,
			antibanding_mode);
		hi258_8909_set_antibanding(s_ctrl, antibanding_mode);
		break;
	}
	case CFG_SET_BESTSHOT_MODE: {
		int32_t bs_mode;
		if (copy_from_user(&bs_mode, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_err("%s: best shot mode is %d", __func__, bs_mode);
		hi258_8909_set_scene_mode(s_ctrl, bs_mode);
		break;
	}
	case CFG_SET_WHITE_BALANCE: {
		int32_t wb_mode;
		if (copy_from_user(&wb_mode, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_err("%s: white balance is %d", __func__, wb_mode);
		hi258_8909_set_white_balance_mode(s_ctrl, wb_mode);
		break;
	}
	default:
		rc = -EFAULT;
		break;
	}

	mutex_unlock(s_ctrl->msm_sensor_mutex);

	return rc;
}

static struct msm_sensor_fn_t hi258_8909_sensor_func_tbl = {
	.sensor_config = hi258_8909_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_match_id = msm_sensor_match_id,
};

static struct msm_sensor_ctrl_t hi258_8909_s_ctrl = {
	.sensor_i2c_client = &hi258_8909_sensor_i2c_client,
	.power_setting_array.power_setting = hi258_8909_power_setting,
	.power_setting_array.size = ARRAY_SIZE(hi258_8909_power_setting),
	.msm_sensor_mutex = &hi258_8909_mut,
	.sensor_v4l2_subdev_info = hi258_8909_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(hi258_8909_subdev_info),
	.func_tbl = &hi258_8909_sensor_func_tbl,
};

module_init(hi258_8909_init_module);
module_exit(hi258_8909_exit_module);
MODULE_DESCRIPTION("Hi258 2MP YUV sensor driver");
MODULE_LICENSE("GPL v2");



