/*
 * Copyright (c) 2009, Google Inc.
 * All rights reserved.
 *
 * Copyright (c) 2009-2014, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of The Linux Foundation nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <debug.h>
#include <string.h>
#include <stdlib.h>
#include <video.h>
#include <dev/keys.h>
#include <kernel/timer.h>
#include <platform/timer.h>

//#define FUNC_KEY_POWER
int get_key_status(uint16_t key_code);
static struct timer mtimer;
int timeout = 0;
int select = -1;  // 0=recovery mode, 1=fastboot.  2=normal boot 3=normal boot + ftrace
bool flag_ftm_boot = false;

#ifdef FUNC_KEY_POWER
struct function_menu
{
	const char *menu_str;
	int x_coordinate;
	int y_coordinate;
	bool flag_choosen;
};
/*
	highlight flag "=>" choosen flag "*"
	=> <*> --go            back--	
	   < > --normal        boot--
	   < > --open  kernel  uart--
	   < > --open adb root mode--	   
	   < > --open ramdump mode--
*/

#define TITLE_X      8
#define TITLE_Y      4

#define FUNC_MENU_X  8
#define FUNC_MENU_Y  6

#define HIGHLIGHT_X  FUNC_MENU_X
#define HIGHLIGHT_Y  FUNC_MENU_Y

#define CHOOSEN_X    FUNC_MENU_X+4
#define CHOOSEN_Y    FUNC_MENU_Y

enum menu
{
	MENU_GO_BACK,
	MENU_NORMAL_BOOT,
	MENU_OPEN_KERNEL_UART,
	MENU_OPEN_ADB_ROOT,	
	MENU_OPEN_RAMDUMP,
	MENU_GOTO_FASTBOOT,
	MENU_NUM,
};
bool flag_open_kernel_uart = false;
bool flag_normal_boot = false;
bool flag_goto_fastboot = false;
bool flag_adb_root_enable = false;
bool flag_ramdump_enable = false;

struct function_menu func_menu[MENU_NUM] =
{
	{"   < > --go            back--",FUNC_MENU_X,FUNC_MENU_Y,false},
	{"   < > --normal        boot--",FUNC_MENU_X,FUNC_MENU_Y+1,false},
	{"   < > --open  kernel  uart--",FUNC_MENU_X,FUNC_MENU_Y+2,false},
	{"   < > --open adb root mode--",FUNC_MENU_X,FUNC_MENU_Y+3,false},
	{"   < > --open ramdump  mode--",FUNC_MENU_X,FUNC_MENU_Y+4,false},	
	{"   < > --goto      fastboot--",FUNC_MENU_X,FUNC_MENU_Y+5,false},
};

void function_select(int index,bool state)
{
	const char* title_msg = "Volume up ==> recovery mode  Volume down ==> factory mode  \n";

	switch(index)
	{
		case MENU_OPEN_KERNEL_UART:
			flag_open_kernel_uart = state;
			break;
			
		case MENU_GOTO_FASTBOOT:
			flag_goto_fastboot = true;
			break;
			
		case MENU_OPEN_ADB_ROOT:
			flag_adb_root_enable = state;				
			break;
			
		case MENU_OPEN_RAMDUMP:
			flag_ramdump_enable = state;				
			break;
		
		case MENU_NORMAL_BOOT:
			flag_normal_boot = true;
			break;
			
		case MENU_GO_BACK:
			func_menu[MENU_GO_BACK].flag_choosen = false;
			video_clean_screen();
			video_printf_string(0,video_get_rows()/2,title_msg);
			video_printf("Volume up ==> [Recovery    Mode]         \n");
			video_printf("Volume down ==>[Factory    Mode]         \n");
			break;
		default:
			dprintf(CRITICAL,"function select error!!!!!\n");
			break;
	}
}

int hide_menu_power_key()
{
	const char* title_msg = "Volume DOWN/UP select function,POWER confirm\n";
	const char* flag_menu_highlight = "=>";
	const char* flag_menu_choosen = "*";
	int i;
	int highlight_x_coordinate = HIGHLIGHT_X;
	int highlight_y_coordinate = HIGHLIGHT_Y;
	int choosen_x_coordinate = CHOOSEN_X;
	int choosen_y_coordinate = CHOOSEN_Y;
	int pre_menu_index = 0;
	struct function_menu * pre_menu = NULL,*cur_menu =NULL;
 	
	video_clean_screen();
	video_printf_string(TITLE_X,TITLE_Y,title_msg);
	for(i = 0;i < MENU_NUM;i++)
	{
		video_printf_string(func_menu[i].x_coordinate,func_menu[i].y_coordinate,func_menu[i].menu_str);
		if(func_menu[i].flag_choosen)
		{
			video_printf_string(choosen_x_coordinate,func_menu[i].y_coordinate,flag_menu_choosen);
		}
	}
	video_printf_string(highlight_x_coordinate,highlight_y_coordinate,flag_menu_highlight);	
	pre_menu = cur_menu = &func_menu[0];
	while(1)
	{
		if(get_key_status(KEY_VOLUMEUP))//VOL_UP
		{
			while(1)
			{
				if(get_key_status(KEY_VOLUMEUP) == false)
				{
					if(pre_menu_index <= 0) 
					{
						cur_menu = &func_menu[MENU_NUM-1];
						pre_menu_index = MENU_NUM-1;
					}
					else
					{
						cur_menu = &func_menu[--pre_menu_index];
					}
					video_printf_string(pre_menu->x_coordinate,pre_menu->y_coordinate,pre_menu->menu_str);
					if(pre_menu->flag_choosen)
					{
						choosen_y_coordinate = pre_menu->y_coordinate;
						video_printf_string(choosen_x_coordinate,choosen_y_coordinate,flag_menu_choosen);
					}
					highlight_y_coordinate = cur_menu->y_coordinate;
					video_printf_string(highlight_x_coordinate,highlight_y_coordinate,flag_menu_highlight);
					pre_menu = cur_menu;
					break;
				}
			}
		}
		else if(get_key_status(KEY_VOLUMEDOWN))//VOL_DOWN,
		{
			while(1)
			{
				if(get_key_status(KEY_VOLUMEDOWN) == false)
				{
					if(pre_menu_index == (MENU_NUM-1)) 
					{
						cur_menu = &func_menu[0];
						pre_menu_index = 0;
					}
					else
					{
						cur_menu = &func_menu[++pre_menu_index];
					}
					video_printf_string(pre_menu->x_coordinate,pre_menu->y_coordinate,pre_menu->menu_str);
					if(pre_menu->flag_choosen)
					{
						choosen_y_coordinate = pre_menu->y_coordinate;
						video_printf_string(choosen_x_coordinate,choosen_y_coordinate,flag_menu_choosen);
					}
					highlight_y_coordinate = cur_menu->y_coordinate;
					video_printf_string(highlight_x_coordinate,highlight_y_coordinate,flag_menu_highlight);
					pre_menu = cur_menu;
					break;
				}
			}
		}
		else if(get_key_status(KEY_POWER))
		{
			while(1)
			{
				if(get_key_status(KEY_POWER) == false)
				{
					cur_menu->flag_choosen = !cur_menu->flag_choosen;
					if(cur_menu->flag_choosen)
					{
						choosen_y_coordinate = cur_menu->y_coordinate;
						video_printf_string(choosen_x_coordinate,choosen_y_coordinate,flag_menu_choosen);
					}
					else
					{					
						highlight_y_coordinate = cur_menu->y_coordinate;
						video_printf_string(cur_menu->x_coordinate,cur_menu->y_coordinate,cur_menu->menu_str);
						video_printf_string(highlight_x_coordinate,highlight_y_coordinate,flag_menu_highlight);
					}
					break;
				}
			}
	
			function_select(pre_menu_index,cur_menu->flag_choosen);
			if((pre_menu_index == MENU_GO_BACK)||(pre_menu_index == MENU_NORMAL_BOOT)||(pre_menu_index == MENU_GOTO_FASTBOOT))
				break;
		}
	}
}
static int  power_key_cut = 0;
static int  flag_power_key_func = 0;

int handle_power_key_up(void)
{
	while(1)
	{
		if(!get_key_status(KEY_POWER))
		{
			power_key_cut ++;
			if(power_key_cut >= 5)
			{
				power_key_cut = 0;
				flag_power_key_func = 1;
				hide_menu_power_key();				
			}
			break;
		}
	}
}
#endif

void flash_splash_image()
{
	extern void display_image_on_screen();
	video_clean_screen();
	display_image_on_screen();
}

void screen_show_info()
{
	const char* title_msg = "Volume up ==> recovery mode  Volume down ==> factory mode  \n";
//	video_clean_screen();
	video_printf_string(0,video_get_rows()/2,title_msg);
	video_printf("Volume up ==> [Recovery    Mode]         \n");
	video_printf("Volume down ==>[Factory    Mode]         \n");
	video_set_cursor(0,(video_get_rows()/2)+10);
}

int boot_mode_menu_select(void)
{
	int i = 0;
	video_clean_screen();
	screen_show_info();
	video_printf("please select boot mode, or will boot into FTM after  %d S\n", (5-timeout));		

	while(get_key_status(KEY_VOLUMEDOWN))
	{	
		if(flag_ftm_boot)
		{
			select=2;//timeout more than 5S, go into FTM mode
			break;
		}
		mdelay(100);
		dprintf(INFO,"key is being pressed!!!\n");
	}
	while(1)
	{
		if(flag_ftm_boot)
		{
			select=2;//timeout more than 5S, go into FTM mode
			break;
		}
		
		if(get_key_status(KEY_VOLUMEUP))//VOL_UP
		{
			select=1;
			break;
		}
		else if(get_key_status(KEY_VOLUMEDOWN))//VOL_DOWN,
		{
			select=2;
			break;
		}
		#ifdef FUNC_KEY_POWER
		else if(get_key_status(KEY_POWER))
		{			
			handle_power_key_up();
			if(flag_normal_boot)
			{
				select = 0;
				flash_splash_image();
				video_printf_string(0,0,"normal boot");	
				break;
			}
			if(flag_goto_fastboot)
			{
				select = 3;
				flash_splash_image();
				video_printf_string(0,0,"fast boot");	
				break;
			}
		}
		#endif
		
	
		#ifdef FUNC_KEY_POWER
		if(flag_power_key_func == 1)
		{
			video_set_cursor(0,(video_get_rows()/2)+10);
			video_printf("power key function\n");		
		}		
		#endif		
		mdelay(100);
	}
	if(select == 1)
	{
		flash_splash_image();
		video_printf_string(0,0,"boot into recovery");		
	}
	else if(select == 2)
	{
		flash_splash_image();
		video_printf_string(0,0,"boot into ffbm");
	}
	
	return select;
}


static enum handler_return ftm_timer_func(struct timer *p_timer,void *arg)
{

	if( 
	#ifdef FUNC_KEY_POWER
	(flag_power_key_func == 1)|| 
	#endif
	(select != -1))
	{
		timer_cancel(&mtimer);
	}
	else
	{
		if(timeout < 5)
		{
			timeout++;
			video_set_cursor(0,(video_get_rows()/2)+10);
			video_printf("please select boot mode, or will boot into FTM after  %d S\n", (5-timeout));		
			timer_set_oneshot(&mtimer, 1000,ftm_timer_func, NULL);
		}
		else
		{			
			timer_cancel(&mtimer);
			flag_ftm_boot = true;			
		}
	}	
	return INT_RESCHEDULE;
}

int boot_mode_select(void)
{
	int select = -1;
	timer_initialize(&mtimer);
	timer_set_oneshot(&mtimer, 1000,ftm_timer_func, NULL);
	select = boot_mode_menu_select();	
	return select;

}

int get_key_status(uint16_t key_code)
{
	extern void target_keystatus();
	target_keystatus();
	return keys_get_state(key_code);
}

