										   

#ifndef _VIDEO_FB_H_
#define _VIDEO_FB_H_
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

#define CONSOLE_BG_COL            0x00
#define CONSOLE_FG_COL            0xff


#define GDF__8BIT_INDEX         0
#define GDF_15BIT_555RGB        1
#define GDF_16BIT_565RGB        2
#define GDF_32BIT_X888RGB       3
#define GDF_24BIT_888RGB        4
#define GDF__8BIT_332RGB        5

typedef struct {
    unsigned int pciBase;
	unsigned int isaBase;
    unsigned int dprBase;
	unsigned int frameAdrs;
    unsigned int vprBase;
    unsigned int cprBase;
    unsigned int memSize;
    unsigned int mode;
	unsigned int fg;
    unsigned int bg;
    unsigned int gdfIndex;
    unsigned int gdfBytesPP;
    unsigned int plnSizeX;
    unsigned int plnSizeY;
    unsigned int winSizeX;
    unsigned int winSizeY;
    char modeIdent[80];
} GraphicDevice;


void *video_hw_init (void);       /* returns GraphicDevice struct or NULL */

#ifdef VIDEO_HW_BITBLT
void video_hw_bitblt (
    unsigned int bpp,             /* bytes per pixel */
    unsigned int src_x,           /* source pos x */
    unsigned int src_y,           /* source pos y */
    unsigned int dst_x,           /* dest pos x */
    unsigned int dst_y,           /* dest pos y */
    unsigned int dim_x,           /* frame width */
    unsigned int dim_y            /* frame height */
    );
#endif

#ifdef VIDEO_HW_RECTFILL
void video_hw_rectfill (
    unsigned int bpp,             /* bytes per pixel */
    unsigned int dst_x,           /* dest pos x */
    unsigned int dst_y,           /* dest pos y */
    unsigned int dim_x,           /* frame width */
    unsigned int dim_y,           /* frame height */
    unsigned int color            /* fill color */
     );
#endif

void video_set_lut (
    unsigned int index,           /* color number */
    unsigned char r,              /* red */
    unsigned char g,              /* green */
    unsigned char b               /* blue */
    );
#ifdef CONFIG_VIDEO_HW_CURSOR
void video_set_hw_cursor(int x, int y); /* x y in pixel */
void video_init_hw_cursor(int font_width, int font_height);
#endif

#endif /*_VIDEO_FB_H_ */
