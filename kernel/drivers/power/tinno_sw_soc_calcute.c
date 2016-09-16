#include <linux/init.h>        /* For init/exit macros */
#include <linux/module.h>      /* For MODULE_ marcros  */
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <linux/slab.h>


#ifdef CONFIG_TINNO_L5251
#define MAX_BATTERY_TYPE  2
#include "battery_meter_table_tinno_L5251_vk.h"
#include "battery_meter_table_tinno_L5251.h"
extern char *saved_command_line;
#define BAT_TYPE_STR   "tinno.battype="
#ifdef CONFIG_TINNO_BATTERY_VK
int battype=1;
#else
int battype=0;
#endif
#else
#include "battery_meter_table_tinno.h"
#endif

//end 
// ============================================================ //
// define
// ============================================================ //


//#define TINNO_DEBUG
#if defined(TINNO_DEBUG)
#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif	
#define tinno_pr_debug(fmt, ...) \
	printk(KERN_DEBUG "Tinno_calcute: "pr_fmt(fmt), ##__VA_ARGS__)
#else
#define tinno_pr_debug(fmt, ...) \
	do{}while(0)
#endif

int fgauge_read_r_bat_by_v_by_table(R_PROFILE_STRUC_P profile_p,int voltage);
int get_q_cost_by_ocv_and_temp(int ocv,int T_temp);
int fgauge_read_r_bat_by_v(int voltage);
int fgauge_read_Q_cost_by_v(int voltage);
int fgauge_get_Q_max_match_Current_and_temp(int t_current,int temp);
int fgauge_read_Q_cost_by_v_by_temp(int voltage,int temp);
static void table_init(void);
int fgauge_read_v_by_Qcost(int q_cost);

extern int get_bat_voltage(void);
extern int force_get_tbat(void);

int g_table_temp=-225;
int g_battery_temp=-225;
static long  elapse_columb=0;
static int  current_temp=0;
static int bat_init_soc=0;

#ifdef CONFIG_TINNO_L5251 
static int bat_totals_columb_st=2500;
static int gFG_columb_standard= 2500; //标称电量
#else
static int bat_totals_columb_st=2000;
static int gFG_columb_standard= 2000; //标称电量
#endif

static long Q_left=0;
static int last_init_soc=50;
static int last_vm_ocv=0;
struct mutex		tinno_calcute_lock;
int mutex_flag_init=0;
#define POWER_OFF_VOLTAGE 3300

int fgauge_get_Q_max_match_Current_and_temp(int t_current,int temp)
{
	int bat_r=0;
	int v=4350;
	int Q_max=0;
       R_PROFILE_STRUC_P profile_p;
	profile_p=fgauge_get_profile_r_table(TEMPERATURE_T2);
	table_init();
	for(;v>=3400;)
	{
		bat_r=fgauge_read_r_bat_by_v_by_table(profile_p,v);
		if((v-bat_r*t_current/1000)<=3400)
			break;
		v=v-10;
	}
    Q_max= fgauge_read_Q_cost_by_v_by_temp(v,temp);

    tinno_pr_debug("fgauge_get_Q_max_match_Current_and_temp : current=%d ,v=%d Q_max=%d , bat_r =%d \n",t_current,v,Q_max,bat_r);	
    return Q_max;
}

int fgauge_get_Q_max_match_Current(int t_current)
{
	int bat_r=0;
	int v=4350;
	int Q_max=0;
  	table_init();
	for(;v>=3400;)
	{
		bat_r=fgauge_read_r_bat_by_v(v);
		if((v-bat_r*t_current/1000)<=3400)
			break;
		v=v-10;
	}
    Q_max= fgauge_read_Q_cost_by_v(v);

    tinno_pr_debug("fgauge_get_Q_max_match_Current : current=%d ,v=%d Q_max=%d , bat_r=%d \n",t_current,v,Q_max,bat_r);	
    return Q_max;
}


#ifdef CONFIG_TINNO_L5251
int fgauge_get_saddles(void)
{	
#ifdef TINNO_DEBUG
	int t0_len, t1_len,t2_len,t3_len,t_len;
	if(battype==0)
	{
		t0_len =sizeof(battery_q_cost_profile_t0) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
		t1_len =sizeof(battery_q_cost_profile_t1) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
		t2_len =sizeof(battery_q_cost_profile_t2) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
		t3_len =sizeof(battery_q_cost_profile_t3) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
		t_len   =sizeof(battery_q_cost_profile_temperature) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
	}else if(battype==1)
	{
		t0_len =sizeof(vk_battery_q_cost_profile_t0) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
		t1_len =sizeof(vk_battery_q_cost_profile_t1) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
		t2_len =sizeof(vk_battery_q_cost_profile_t2) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
		t3_len =sizeof(vk_battery_q_cost_profile_t3) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
		t_len   =sizeof(vk_battery_q_cost_profile_temperature) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
	}
      if((t0_len !=t1_len)||(t0_len !=t2_len)||(t0_len !=t3_len)||(t0_len !=t_len) )
      	{
      		 tinno_pr_debug("tinno FATAL error !!, q_cost table len invalid!!!!\r\n");
      	}
#endif
	if(battype==0)
	{
	    return sizeof(battery_q_cost_profile_t2) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
	}else if(battype==1){
	    return sizeof(vk_battery_q_cost_profile_t2) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
	}else{
	   return sizeof(battery_q_cost_profile_t2) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
	}
}

int fgauge_get_saddles_r_table(void)
{

#ifdef TINNO_DEBUG
	int t0_len, t1_len,t2_len,t3_len,t_len;
	if(battype==0)
	{
		t0_len =sizeof(tinno_r_profile_t0) / sizeof(R_PROFILE_STRUC);
		t1_len =sizeof(tinno_r_profile_t1) / sizeof(R_PROFILE_STRUC);
		t2_len =sizeof(tinno_r_profile_t2) / sizeof(R_PROFILE_STRUC);
		t3_len =sizeof(tinno_r_profile_t3) / sizeof(R_PROFILE_STRUC);
		t_len   =sizeof(tinno_r_profile_temperature) / sizeof(R_PROFILE_STRUC);
	}else if(battype==1)
	{
		t0_len =sizeof(vk_tinno_r_profile_t0) / sizeof(R_PROFILE_STRUC);
		t1_len =sizeof(vk_tinno_r_profile_t1) / sizeof(R_PROFILE_STRUC);
		t2_len =sizeof(vk_tinno_r_profile_t2) / sizeof(R_PROFILE_STRUC);
		t3_len =sizeof(vk_tinno_r_profile_t3) / sizeof(R_PROFILE_STRUC);
		t_len   =sizeof(vk_tinno_r_profile_temperature) / sizeof(R_PROFILE_STRUC);
	}


      if((t0_len !=t1_len)||(t0_len !=t2_len)||(t0_len !=t3_len)||(t0_len !=t_len) )
      	{
      		 tinno_pr_debug("tinno FATAL error !!, r_profile table len invalid!!!!\r\n");
      	}
	
#endif
	
	if(battype==0)
	{
		 	 return sizeof(tinno_r_profile_t2) / sizeof(R_PROFILE_STRUC);
	}else if(battype==1)
	{
		 	 return sizeof(vk_tinno_r_profile_t2) / sizeof(R_PROFILE_STRUC);
	}else{
		 	 return sizeof(tinno_r_profile_t2) / sizeof(R_PROFILE_STRUC);
	}
}

BATTERY_Q_COST_PROFILE_STRUC_P fgauge_get_profile(int temperature)
{

	if(battype==0)
	{
	    switch (temperature)
	    {
	        case TEMPERATURE_T0:
	            return &battery_q_cost_profile_t0[0];
	     
	        case TEMPERATURE_T1:
	            return &battery_q_cost_profile_t1[0];
	          
	        case TEMPERATURE_T2:
	            return &battery_q_cost_profile_t2[0];
	          
	        case TEMPERATURE_T3:
	            return &battery_q_cost_profile_t3[0];
	          
	        case TEMPERATURE_T:
	            return &battery_q_cost_profile_temperature[0];
	       
	        default:
	            return NULL;
	    	}
	   }else if(battype==1){
		switch (temperature)
	    {
	        case TEMPERATURE_T0:
	            return &vk_battery_q_cost_profile_t0[0];
	     
	        case TEMPERATURE_T1:
	            return &vk_battery_q_cost_profile_t1[0];
	          
	        case TEMPERATURE_T2:
	            return &vk_battery_q_cost_profile_t2[0];
	          
	        case TEMPERATURE_T3:
	            return &vk_battery_q_cost_profile_t3[0];
	          
	        case TEMPERATURE_T:
	            return &vk_battery_q_cost_profile_temperature[0];
	       
	        default:
	            return NULL;
	   }
	}else 
	{
	    switch (temperature)
	    {
	        case TEMPERATURE_T0:
	            return &battery_q_cost_profile_t0[0];
	     
	        case TEMPERATURE_T1:
	            return &battery_q_cost_profile_t1[0];
	          
	        case TEMPERATURE_T2:
	            return &battery_q_cost_profile_t2[0];
	          
	        case TEMPERATURE_T3:
	            return &battery_q_cost_profile_t3[0];
	          
	        case TEMPERATURE_T:
	            return &battery_q_cost_profile_temperature[0];
	       
	        default:
	            return NULL;
	    	}
	   }
          
}

R_PROFILE_STRUC_P fgauge_get_profile_r_table(int temperature)
{
	if(battype==0)
	{
		switch (temperature)
		{
		case TEMPERATURE_T0:
		    return &tinno_r_profile_t0[0];

		case TEMPERATURE_T1:
		    return &tinno_r_profile_t1[0];

		case TEMPERATURE_T2:
		    return &tinno_r_profile_t2[0];
		   
		case TEMPERATURE_T3:
		    return &tinno_r_profile_t3[0];

		case TEMPERATURE_T:
		    return &tinno_r_profile_temperature[0];
		 
		default:
		    return NULL;
		  
		}
	}else if(battype==1)
	{
		switch (temperature)
		{
		case TEMPERATURE_T0:
		    return &vk_tinno_r_profile_t0[0];

		case TEMPERATURE_T1:
		    return &vk_tinno_r_profile_t1[0];

		case TEMPERATURE_T2:
		    return &vk_tinno_r_profile_t2[0];
		   
		case TEMPERATURE_T3:
		    return &vk_tinno_r_profile_t3[0];

		case TEMPERATURE_T:
		    return &vk_tinno_r_profile_temperature[0];
		 
		default:
		    return NULL;
		  
		}

	}else 
	{
		switch (temperature)
		{
		case TEMPERATURE_T0:
		    return &tinno_r_profile_t0[0];

		case TEMPERATURE_T1:
		    return &tinno_r_profile_t1[0];

		case TEMPERATURE_T2:
		    return &tinno_r_profile_t2[0];
		   
		case TEMPERATURE_T3:
		    return &tinno_r_profile_t3[0];

		case TEMPERATURE_T:
		    return &tinno_r_profile_temperature[0];
		 
		default:
		    return NULL;
		  
		}
	}
	
}
#else
int fgauge_get_saddles(void)
{	
#ifdef TINNO_DEBUG
	int t0_len, t1_len,t2_len,t3_len,t_len;

		t0_len =sizeof(battery_q_cost_profile_t0) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
		t1_len =sizeof(battery_q_cost_profile_t1) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
		t2_len =sizeof(battery_q_cost_profile_t2) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
		t3_len =sizeof(battery_q_cost_profile_t3) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
		t_len   =sizeof(battery_q_cost_profile_temperature) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
		
      if((t0_len !=t1_len)||(t0_len !=t2_len)||(t0_len !=t3_len)||(t0_len !=t_len) )
      	{
      		 tinno_pr_debug("tinno FATAL error !!, q_cost table len invalid!!!!\r\n");
      	}
#endif
	   return sizeof(battery_q_cost_profile_t2) / sizeof(BATTERY_Q_COST_PROFILE_STRUC);
}

int fgauge_get_saddles_r_table(void)
{

#ifdef TINNO_DEBUG
	int t0_len, t1_len,t2_len,t3_len,t_len;
		t0_len =sizeof(tinno_r_profile_t0) / sizeof(R_PROFILE_STRUC);
		t1_len =sizeof(tinno_r_profile_t1) / sizeof(R_PROFILE_STRUC);
		t2_len =sizeof(tinno_r_profile_t2) / sizeof(R_PROFILE_STRUC);
		t3_len =sizeof(tinno_r_profile_t3) / sizeof(R_PROFILE_STRUC);
		t_len   =sizeof(tinno_r_profile_temperature) / sizeof(R_PROFILE_STRUC);

      if((t0_len !=t1_len)||(t0_len !=t2_len)||(t0_len !=t3_len)||(t0_len !=t_len) )
      	{
      		 tinno_pr_debug("tinno FATAL error !!, r_profile table len invalid!!!!\r\n");
      	}
	
#endif
	return sizeof(tinno_r_profile_t2) / sizeof(R_PROFILE_STRUC);
}

BATTERY_Q_COST_PROFILE_STRUC_P fgauge_get_profile(int temperature)
{
    switch (temperature)
    {
        case TEMPERATURE_T0:
            return &battery_q_cost_profile_t0[0];
     
        case TEMPERATURE_T1:
            return &battery_q_cost_profile_t1[0];
          
        case TEMPERATURE_T2:
            return &battery_q_cost_profile_t2[0];
          
        case TEMPERATURE_T3:
            return &battery_q_cost_profile_t3[0];
          
        case TEMPERATURE_T:
            return &battery_q_cost_profile_temperature[0];
       
        default:
            return NULL;
    	}
          
}

R_PROFILE_STRUC_P fgauge_get_profile_r_table(int temperature)
{
	switch (temperature)
	{
	case TEMPERATURE_T0:
	    return &tinno_r_profile_t0[0];

	case TEMPERATURE_T1:
	    return &tinno_r_profile_t1[0];

	case TEMPERATURE_T2:
	    return &tinno_r_profile_t2[0];
	   
	case TEMPERATURE_T3:
	    return &tinno_r_profile_t3[0];

	case TEMPERATURE_T:
	    return &tinno_r_profile_temperature[0];
	 
	default:
	    return NULL;
	  
	}
}

#endif
int fgauge_read_Q_cost_by_v_by_temp(int voltage,int temp)
{    
    int i = 0, saddles = 0;
    BATTERY_Q_COST_PROFILE_STRUC_P profile_p;
    int ret_q_cost= 0;

    profile_p = fgauge_get_profile(temp);
    if (profile_p == NULL)
    {
        tinno_pr_debug("[FGADC] fgauge get ZCV profile T=%d: fail !\r\n",temp);
	profile_p = fgauge_get_profile(TEMPERATURE_T);	
    }

    saddles = fgauge_get_saddles();

    if (voltage > (profile_p+0)->voltage)
    {
        return 0; // battery capacity, not dod
    }    
    if (voltage <= (profile_p+saddles-1)->voltage)
    {
		return gFG_columb_standard;
    }

    for (i = 0; i < saddles - 1; i++)
    {
        if ((voltage <= (profile_p+i)->voltage) && (voltage > (profile_p+i+1)->voltage))
        {
            ret_q_cost = (profile_p+i)->q_cost +
                (
                    (
                        ( ((profile_p+i)->voltage) - voltage ) * 
                        ( ((profile_p+i+1)->q_cost) - ((profile_p + i)->q_cost) ) 
                    ) /
                    ( ((profile_p+i)->voltage) - ((profile_p+i+1)->voltage) )
                );         
            
            break;
        }
        
    }
    return ret_q_cost;
}


int fgauge_read_Q_cost_by_v(int voltage)
{    
    int i = 0, saddles = 0;
    BATTERY_Q_COST_PROFILE_STRUC_P profile_p;
    int ret_q_cost= 0;

    profile_p = fgauge_get_profile(TEMPERATURE_T);
    if (profile_p == NULL)
    {
        tinno_pr_debug("[FGADC] fgauge get ZCV profile : fail !\r\n");
		return gFG_columb_standard;
    }

    saddles = fgauge_get_saddles();

    tinno_pr_debug("saddles = %d, 3 =%d (profile_p+saddles-2)->voltage =%d, (profile_p+saddles-1)->voltage =%d, %d,%d,%d\n", 
		saddles,(profile_p+saddles-3)->voltage,(profile_p+saddles-2)->voltage, (profile_p+saddles-1)->voltage, 
		(profile_p+saddles-3)->q_cost,(profile_p+saddles-2)->q_cost, (profile_p+saddles-1)->q_cost);	

    if (voltage > (profile_p+0)->voltage)
    {
        return 0; // battery capacity, not dod
    }    
    if (voltage <=(profile_p+saddles-1)->voltage)
    {
		return gFG_columb_standard;
    }

    for (i = 0; i < saddles - 1; i++)
    {
        if ((voltage <= (profile_p+i)->voltage) && (voltage > (profile_p+i+1)->voltage))
        {
            ret_q_cost = (profile_p+i)->q_cost +
                (
                    (
                        ( ((profile_p+i)->voltage) - voltage ) * 
                        ( ((profile_p+i+1)->q_cost) - ((profile_p + i)->q_cost) ) 
                    ) /
                    ( ((profile_p+i)->voltage) - ((profile_p+i+1)->voltage) )
                );         
            
            break;
        }
        
    }
    return ret_q_cost;
}

int fgauge_read_q_cost_by_v_by_table(BATTERY_Q_COST_PROFILE_STRUC_P profile_p,int volt_bat)
{
    int i = 0, saddles = 0;
    int ret_d = 0;
    if (profile_p == NULL)
    {
        tinno_pr_debug("[FGADC] fgauge get ZCV profile : fail !\r\n");
        return 0;
    }

    saddles = fgauge_get_saddles();

    if (volt_bat > (profile_p+0)->voltage)
    {
        return (profile_p+0)->q_cost; 
    }    
    if (volt_bat <= (profile_p+saddles-1)->voltage)
    {

	return gFG_columb_standard;
    }


    for (i = 0; i < saddles - 1; i++)
    {
        if (volt_bat == (profile_p+i)->voltage)
        {
            ret_d = (profile_p+i)->q_cost;
           return ret_d;
        }
    }
	

    for (i = 0; i < saddles - 1; i++)
    {
        if ((volt_bat <= (profile_p+i)->voltage) && (volt_bat > (profile_p+i+1)->voltage))
        {
            ret_d = (profile_p+i)->q_cost +
                (
                    (
                        ( ((profile_p+i)->voltage) - volt_bat ) * 
                        ( ((profile_p+i+1)->q_cost) - ((profile_p + i)->q_cost) ) 
                    ) /
                    ( ((profile_p+i)->voltage) - ((profile_p+i+1)->voltage) )
                );         
            
            break;
        }
        
    }

    return ret_d;
}


int fgauge_read_r_bat_by_v_by_table(R_PROFILE_STRUC_P profile_p,int voltage){
 int i = 0, saddles = 0;
    int ret_r = 0;

    if (profile_p == NULL)
    {
        tinno_pr_debug("[FGADC] fgauge get R-Table profile : fail !\r\n");
        return 0;
    }

    saddles = fgauge_get_saddles_r_table();

    if (voltage > (profile_p+0)->voltage)
    {
        return (profile_p+0)->resistance; 
    }    
    if (voltage <= (profile_p+saddles-1)->voltage)
    {
        return (profile_p+saddles-1)->resistance; 
    }

    for (i = 0; i < saddles - 1; i++)
    {
        if ((voltage <= (profile_p+i)->voltage) && (voltage > (profile_p+i+1)->voltage))
        {
            ret_r = (profile_p+i)->resistance +
                (
                    (
                        ( ((profile_p+i)->voltage) - voltage ) * 
                        ( ((profile_p+i+1)->resistance) - ((profile_p + i)->resistance) ) 
                    ) /
                    ( ((profile_p+i)->voltage) - ((profile_p+i+1)->voltage) )
                );
            break;
        }
    }

	
    return ret_r;
}

int  fgauge_read_r_bat_by_v(int voltage)
{    
    int i = 0, saddles = 0;
    R_PROFILE_STRUC_P profile_p;
    int ret_r = 0;

    profile_p = fgauge_get_profile_r_table(TEMPERATURE_T);
    if (profile_p == NULL)
    {
        tinno_pr_debug("[FGADC] fgauge get R-Table profile : fail !\r\n");
        return (profile_p+0)->resistance;
    }

    saddles = fgauge_get_saddles_r_table();

    if (voltage > (profile_p+0)->voltage)
    {
        return (profile_p+0)->resistance; 
    }    
    if (voltage <= (profile_p+saddles-1)->voltage)
    {
        return (profile_p+saddles-1)->resistance; 
    }

    for (i = 0; i < saddles - 1; i++)
    {
        if ((voltage <= (profile_p+i)->voltage) && (voltage > (profile_p+i+1)->voltage))
        {
            ret_r = (profile_p+i)->resistance +
                (
                    (
                        ( ((profile_p+i)->voltage) - voltage ) * 
                        ( ((profile_p+i+1)->resistance) - ((profile_p + i)->resistance) ) 
                    ) /
                    ( ((profile_p+i)->voltage) - ((profile_p+i+1)->voltage) )
                );
            break;
        }
    }

    return ret_r;
}



void fgauge_construct_battery_profile(int temperature, BATTERY_Q_COST_PROFILE_STRUC_P temp_profile_p)
{
    BATTERY_Q_COST_PROFILE_STRUC_P low_profile_p, high_profile_p;
    int low_temperature, high_temperature;
    int i, saddles;
    int temp_d_1 = 0, temp_d_2 = 0;
    int high_profile_d = 0, low_profile_d = 0;

    tinno_pr_debug("fgauge_construct_battery_profile temperature=%d \n",temperature);
	

    if (temperature <= TEMPERATURE_T1)
    {
        low_profile_p    = fgauge_get_profile(TEMPERATURE_T0);
        high_profile_p   = fgauge_get_profile(TEMPERATURE_T1);
        low_temperature  = (-10);
        high_temperature = TEMPERATURE_T1;
        
        if(temperature < low_temperature)
        {
            temperature = low_temperature;
        }
    }
    else if (temperature <= TEMPERATURE_T2)
    {
        low_profile_p    = fgauge_get_profile(TEMPERATURE_T1);
        high_profile_p   = fgauge_get_profile(TEMPERATURE_T2);
        low_temperature  = TEMPERATURE_T1;
        high_temperature = TEMPERATURE_T2;
        
        if(temperature < low_temperature)
        {
            temperature = low_temperature;
        }
    }
    else
    {
        low_profile_p    = fgauge_get_profile(TEMPERATURE_T2);
        high_profile_p   = fgauge_get_profile(TEMPERATURE_T3);
        low_temperature  = TEMPERATURE_T2;
        high_temperature = TEMPERATURE_T3;
        
        if(temperature > high_temperature)
        {
            temperature = high_temperature;
        }
    }

    saddles = fgauge_get_saddles();

    for (i = 0; i < saddles; i++)
    {
            (temp_profile_p + i)->voltage =(high_profile_p + i)->voltage;

     }



    /* Interpolation for R_BAT */
    for (i = 0; i < saddles; i++)
    {
	high_profile_d=fgauge_read_q_cost_by_v_by_table(high_profile_p,(temp_profile_p + i)->voltage);
	low_profile_d=fgauge_read_q_cost_by_v_by_table(low_profile_p,(temp_profile_p + i)->voltage);

	if((temp_profile_p + i)->voltage==3075)
	{
		tinno_pr_debug("high_profile_d=%d , low_profile_d=%d \n",high_profile_d,low_profile_d);
	}

	
        if( high_profile_d>low_profile_d)
        {
            temp_d_1 = high_profile_d;
            temp_d_2 =low_profile_d;    
		if((temp_profile_p + i)->voltage==3075)
		{
			tinno_pr_debug("temp_d_1=%d , temp_d_2=%d \n",temp_d_1,temp_d_2);
		}			
            (temp_profile_p + i)->q_cost = temp_d_2 +
            (
                (
                    (temperature - low_temperature) * 
                    (temp_d_1 - temp_d_2)
                ) / 
                (high_temperature - low_temperature)                
            );
        }
        else
        {
            temp_d_1 =low_profile_d;
            temp_d_2 =high_profile_d;
		if((temp_profile_p + i)->voltage==3075)
		{
			tinno_pr_debug("temp_d_1=%d , temp_d_2=%d \n",temp_d_1,temp_d_2);
		}		
            (temp_profile_p + i)->q_cost= temp_d_2 +
            (
                (
                    (high_temperature - temperature) * 
                    (temp_d_1 - temp_d_2)
                ) / 
                (high_temperature - low_temperature)                
            );
        }
    }

  #if 0 
    // Dumpt new battery profile
    for (i = 0; i < saddles ; i++)
    {
        tinno_pr_debug("<Qcost,Voltage> at %d = <%d,%d>\r\n",
            temperature, (temp_profile_p+i)->q_cost, (temp_profile_p+i)->voltage);
    }
#endif
    
}


#define SHUTDOWN_VOLTAGE 3400

int fgauge_read_v_by_Qcost(int q_cost)
{    
    int i = 0, saddles = 0;
    BATTERY_Q_COST_PROFILE_STRUC_P profile_p;
    int ret_volt = 0;

    profile_p = fgauge_get_profile(TEMPERATURE_T);
    if (profile_p == NULL)
    {
        tinno_pr_debug("[fgauge_read_v_by_capacity] fgauge get ZCV profile : fail !\r\n");
        return 3700;
    }

   tinno_pr_debug("fgauge_read_v_by_Qcost :%d \n",q_cost);
    saddles = fgauge_get_saddles();

    if (q_cost < (profile_p+0)->q_cost)
    {        
        return (profile_p+0)->voltage;
    }    
    if (q_cost > (profile_p+saddles-1)->q_cost)
    {        
        return SHUTDOWN_VOLTAGE;
    }

    for (i = 0; i < saddles - 1; i++)
    {
        if ((q_cost >= (profile_p+i)->q_cost) && (q_cost <= (profile_p+i+1)->q_cost))
        {
            ret_volt = (profile_p+i)->voltage -
                (
                    (
                        ( q_cost - ((profile_p+i)->q_cost) ) * 
                        ( ((profile_p+i)->voltage) - ((profile_p+i+1)->voltage) ) 
                    ) /
                    ( ((profile_p+i+1)->q_cost) - ((profile_p+i)->q_cost) )
                );         
            
            break;
        }        
    }    

    return ret_volt;
}

void fgauge_construct_r_table_profile(int temperature, R_PROFILE_STRUC_P temp_profile_p)
{
    R_PROFILE_STRUC_P low_profile_p, high_profile_p;
    int low_temperature, high_temperature;
    int i, saddles;
    int high_profile_r = 0, low_profile_r = 0;
    int temp_r_1 = 0, temp_r_2 = 0;
    

    if (temperature <= TEMPERATURE_T1)
    {
        low_profile_p    = fgauge_get_profile_r_table(TEMPERATURE_T0);
        high_profile_p   = fgauge_get_profile_r_table(TEMPERATURE_T1);
        low_temperature  = (-10);
        high_temperature = TEMPERATURE_T1;
        
        if(temperature < low_temperature)
        {
            temperature = low_temperature;
        }
    }
    else if (temperature <= TEMPERATURE_T2)
    {
        low_profile_p    = fgauge_get_profile_r_table(TEMPERATURE_T1);
        high_profile_p   = fgauge_get_profile_r_table(TEMPERATURE_T2);
        low_temperature  = TEMPERATURE_T1;
        high_temperature = TEMPERATURE_T2;
        
        if(temperature < low_temperature)
        {
            temperature = low_temperature;
        }
    }
    else
    {
        low_profile_p    = fgauge_get_profile_r_table(TEMPERATURE_T2);
        high_profile_p   = fgauge_get_profile_r_table(TEMPERATURE_T3);
        low_temperature  = TEMPERATURE_T2;
        high_temperature = TEMPERATURE_T3;
        
        if(temperature > high_temperature)
        {
            temperature = high_temperature;
        }
    }

    saddles = fgauge_get_saddles_r_table();

/*use the high temp voltage*/
 for (i = 0; i < saddles; i++){
	 (temp_profile_p + i)->voltage =(high_profile_p + i)->voltage;
 }


    /* Interpolation for R_BAT */
    for (i = 0; i < saddles; i++)
    {
	high_profile_r=fgauge_read_r_bat_by_v_by_table(high_profile_p,(temp_profile_p + i)->voltage);
	low_profile_r=fgauge_read_r_bat_by_v_by_table(low_profile_p,(temp_profile_p + i)->voltage);
	
        if( high_profile_r>low_profile_r)
        {
            temp_r_1 = high_profile_r;
            temp_r_2 =low_profile_r;    

            (temp_profile_p + i)->resistance = temp_r_2 +
            (
                (
                    (temperature - low_temperature) * 
                    (temp_r_1 - temp_r_2)
                ) / 
                (high_temperature - low_temperature)                
            );
        }
        else
        {
            temp_r_1 =low_profile_r;
            temp_r_2 =high_profile_r;

            (temp_profile_p + i)->resistance = temp_r_2 +
            (
                (
                    (high_temperature - temperature) * 
                    (temp_r_1 - temp_r_2)
                ) / 
                (high_temperature - low_temperature)                
            );
        }

    }

#if 0
    // Dumpt new r-table profile
    for (i = 0; i < saddles ; i++)
    {
        tinno_pr_debug("1<Rbat,VBAT> at %d = <%d,%d>\r\n",
            temperature, (temp_profile_p+i)->resistance, (temp_profile_p+i)->voltage);
    }

	for (i = 0; i < saddles ; i++)
	{
		tinno_pr_debug("2<Rbat,VBAT> at %d = <%d,%d>\r\n",
		temperature, (temp_profile_p+i)->resistance, (temp_profile_p+i)->voltage);
	}

		for (i = 0; i < saddles ; i++)
	{
		tinno_pr_debug("3<Rbat,VBAT> at %d = <%d,%d>\r\n",
		temperature, (temp_profile_p+i)->resistance, (temp_profile_p+i)->voltage);
	}
#endif		
}


/*
int dod_to_standard_columb(int dod)
{
	return  ((dod*gFG_columb_standard)/100);
}

*/


// ============================================================ // SW FG
/*int mtk_imp_tracking(int ori_voltage, int ori_current, int recursion_time)
{
    int ret_compensate_value = 0;
    int temp_voltage_1 = ori_voltage;
    int temp_voltage_2 = temp_voltage_1;
    int i = 0;

    for(i=0 ; i < recursion_time ; i++) 
    {
        gFG_resistance_bat = fgauge_read_r_bat_by_v(temp_voltage_2); 
        ret_compensate_value = ( (ori_current) * (gFG_resistance_bat + R_FG_VALUE)) / 1000;
        ret_compensate_value = (ret_compensate_value+(10/2)) / 10; 
        temp_voltage_2 = temp_voltage_1 + ret_compensate_value;

        Tinno_per_debug("[mtk_imp_tracking] temp_voltage_2=%d,temp_voltage_1=%d,ret_compensate_value=%d,gFG_resistance_bat=%d\n", 
            temp_voltage_2,temp_voltage_1,ret_compensate_value,gFG_resistance_bat);
    }
    
    gFG_resistance_bat = fgauge_read_r_bat_by_v(temp_voltage_2); 
    ret_compensate_value = ( (ori_current) * (gFG_resistance_bat + R_FG_VALUE + FG_METER_RESISTANCE)) / 1000;    
    ret_compensate_value = (ret_compensate_value+(10/2)) / 10; 

    gFG_compensate_value = ret_compensate_value;

    Tinno_per_debug("[mtk_imp_tracking] temp_voltage_2=%d,temp_voltage_1=%d,ret_compensate_value=%d,gFG_resistance_bat=%d\n", 
        temp_voltage_2,temp_voltage_1,ret_compensate_value,gFG_resistance_bat);    

    return ret_compensate_value;
}
*/

// ============================================================ //


static void  table_init(void)
{
    BATTERY_Q_COST_PROFILE_STRUC_P profile_p;
    R_PROFILE_STRUC_P profile_p_r_table;
	
    int temperature = force_get_tbat();
    g_battery_temp=temperature;

    tinno_pr_debug("[table_init] g_battery_temp =%d,g_table_temp =%d\r\n",g_battery_temp,g_table_temp );
	
    if((abs(g_table_temp-temperature)<5)&&(g_table_temp!=-255))
    {
        return;	
    }
     g_table_temp=temperature;

    // Re-constructure r-table profile according to current temperature
    profile_p_r_table = fgauge_get_profile_r_table(TEMPERATURE_T);
    if (profile_p_r_table == NULL)
    {
        tinno_pr_debug("[FGADC] fgauge_get_profile_r_table : create table fail !\r\n");
    }
	
    fgauge_construct_r_table_profile(temperature, profile_p_r_table);

    // Re-constructure battery profile according to current temperature
    profile_p = fgauge_get_profile(TEMPERATURE_T);
    if (profile_p == NULL)
    {
        tinno_pr_debug("[FGADC] fgauge_get_profile : create table fail !\r\n");
    }
    fgauge_construct_battery_profile(temperature, profile_p);
}
	

int calcute_soc(int maintain_times, int ocv_uv);
int get_soc_by_voltage_by_temp(int voltage);
#define DEF_SOC 50
void force_init_soc(int soc, int ocv);


int get_last_vm_ocv(void)
{
	return last_vm_ocv;
}

int get_last_vm_soc(void)
{
	return last_init_soc;
}

int calcute_soc(int maintain_times, int ocv_uv)
{

#if 0
       current_temp=50;
	return current_temp;
#else
	int bat_r=0;
	int voltage_last=0;
	int voltage_current=0;
	int soc_temp, sco_by_ocv;
//	int ret=0;
	int voltage=0;
	if(!mutex_flag_init)
	{
		mutex_init(&tinno_calcute_lock);
		mutex_flag_init=1;
	//	return DEF_SOC;
	}
	
	voltage=get_bat_voltage();
	if(voltage<0)
	{
		tinno_pr_debug("DRIVER NOT INIT FINISH!last_init_soc=%d\n",last_init_soc);
		return last_init_soc;
	}
	
	mutex_lock(&tinno_calcute_lock);
	tinno_pr_debug("maintain_times=%d \n",maintain_times);
	table_init();
	bat_r=fgauge_read_r_bat_by_v(voltage);
    	 tinno_pr_debug("g_table_temp=%d , voltage=%d,bat_r=%d bat_totals_columb_st=%d \n",g_table_temp,voltage,bat_r,bat_totals_columb_st);
	bat_totals_columb_st=fgauge_get_Q_max_match_Current_and_temp(400,25);
	voltage_last=fgauge_read_v_by_Qcost(bat_totals_columb_st-Q_left+elapse_columb/1000);
	
    	current_temp = (((voltage_last-voltage)*1000)*10) /bat_r;    //0.1mA
       elapse_columb = (current_temp*maintain_times/36) + elapse_columb; //0.001mAh
	soc_temp=((Q_left*1000-elapse_columb))/bat_totals_columb_st;

	voltage_current=fgauge_read_v_by_Qcost(bat_totals_columb_st-Q_left+elapse_columb/1000);
	last_vm_ocv=voltage_current*1000;
 	tinno_pr_debug("voltage_temp=%d , current_temp=%d,elapse_columb=%ld Q_left=%ld \n",voltage_current,current_temp,elapse_columb,Q_left);

        tinno_pr_debug("tinno :calcute_soc  voltage =%d,soc_temp =%d\n",voltage,soc_temp);
		
	if((voltage<POWER_OFF_VOLTAGE)&&(soc_temp/10<=5))
	{
		printk("tinno:calcute Froce set soc to 0,soc_voltage=%d,phy_voltage=%d ,soc_temp =%d\n",voltage_current,voltage,soc_temp/10);
		last_vm_ocv=voltage*1000;
		soc_temp=0;
	}
	
	if(soc_temp>993)
	{
		soc_temp=1000;
	}else if(soc_temp<5){
		soc_temp=0;
	}

	last_init_soc=(soc_temp+5)/10;   // four drop, five add 
	
	tinno_pr_debug("soc_temp=%d ,last_init_soc =%d \n",soc_temp, last_init_soc);
	mutex_unlock(&tinno_calcute_lock);

       if(last_init_soc == 0)
   	{
   		sco_by_ocv =get_soc_by_voltage_by_temp(ocv_uv/1000);
		if(sco_by_ocv >0)
			last_init_soc=1;


		printk("tinno :when last_init_soc ==0, recheck  sco_by_ocv =%d,ocv_uv =%d\n",sco_by_ocv,ocv_uv);
   	}
	
	return last_init_soc;
#endif	
}

int track_bat_voltage(int batvol,int bat_i)
{
	int bat_r=0;
	int batvol_temp=0;
       table_init();
	bat_r=fgauge_read_r_bat_by_v(batvol);
	batvol_temp=batvol+(bat_i*bat_r)/1000;
       return batvol_temp;
}

int get_soc_by_voltage_by_temp(int in_voltage)
{
	long soc_temp=0;
	long cap_temp=0;
	//int ret=0;
	int voltage=in_voltage;
	if(!mutex_flag_init)
	{
		mutex_init(&tinno_calcute_lock);
		mutex_flag_init=1;
	//	return DEF_SOC;
	}
	if(voltage<0)
	{
		tinno_pr_debug("DRIVER NOT INIT FINISH!\n");
		return DEF_SOC;
	}

	mutex_lock(&tinno_calcute_lock);
	table_init();
	cap_temp=fgauge_read_Q_cost_by_v(voltage);
	bat_totals_columb_st=fgauge_get_Q_max_match_Current_and_temp(400,25);
	Q_left=bat_totals_columb_st-cap_temp;
	soc_temp=((Q_left*1000)/bat_totals_columb_st);
	tinno_pr_debug("voltage=%d , cap_temp=%lu,bat_totals_columb_st=%d Q_left=%lu \n",voltage,cap_temp,bat_totals_columb_st,Q_left);
	if(soc_temp>993)
	{
		soc_temp=1000;
	}else if(soc_temp<5){
			soc_temp=0;
	}

	last_init_soc=(soc_temp+5)/10;   // four drop, five add 
	tinno_pr_debug("soc_temp=%lu \n",soc_temp);
	mutex_unlock(&tinno_calcute_lock);
	return last_init_soc;
}

#ifdef CONFIG_TINNO_L5251
static int str_to_int(char *str)
{
	int count=0;
	int value=0;
	int t_index=0;
	for(count=0;count<15;count++)
	{
		if((str[count]>='0')&&(str[count]<='9'))
		{
			continue;
		}
		break;
	}

	for(;count>0;count--)
	{
	
		if(count>1)
		{
			value=value*10+(str[t_index]-'0')*10;
		}else{
			value=value+(str[t_index]-'0');
		}
		t_index++;
	}
	return value;
}

int  get_battery_type_in_cmdline(char * start)
{
	char buf[2];
	int ret=0;
	memset(buf,0,2);
	buf[0]=start[0];
	ret=str_to_int(buf);
	return ret;
}
#endif

void force_init_soc(int soc, int ocv)
{
#ifdef CONFIG_TINNO_L5251
	char * start;
	start=strstr(saved_command_line,BAT_TYPE_STR);
	    if(start)
	    {
		battype=get_battery_type_in_cmdline(start+strlen(BAT_TYPE_STR));
	    }else{
		battype=0;
		}
		printk("battype=%d\n",battype);
		if(battype>=MAX_BATTERY_TYPE)
		{
			printk("ERROR battype=%d\n",battype);
			battype=0;
		}
#endif


	if(!mutex_flag_init)
	{
		mutex_init(&tinno_calcute_lock);
		mutex_flag_init=1;
	//	return DEF_SOC;
	}

	printk("tinno: force_init_soc soc =%d, ocv =%d",soc, ocv);
       last_vm_ocv =ocv;
	
	bat_init_soc=soc;
	last_init_soc=bat_init_soc;
	elapse_columb=0;
	
	
	bat_totals_columb_st=fgauge_get_Q_max_match_Current_and_temp(400,25);
	Q_left=(soc*bat_totals_columb_st)/100;
	
	tinno_pr_debug("bat_init_soc=%d , elapse_columb=%ld,bat_totals_columb_st=%d Q_left=%lu \n",bat_init_soc,elapse_columb,bat_totals_columb_st,Q_left);	
}





