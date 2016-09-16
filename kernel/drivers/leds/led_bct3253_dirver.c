/*bct3253 led controller driver*/

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>
#include <linux/leds.h>



#include <linux/sched.h>
#include <linux/kthread.h>


typedef struct {
int led_port;
int led_mode;
int state;
int max_current;
int max_brightness;
int mid_brightness;
int rise_time;
int fall_time;
int rise_speed;
int fall_speed;
} Led_Ctl_Data;

#define LED_WORK_DELAY  100

typedef char  kal_uint8 ;

static struct workqueue_struct *bct3253_wq;
struct delayed_work bct3253_work;

static struct workqueue_struct *bct3253_wq_led;
struct delayed_work bct3253_work_led;

/*DEF REG ADDR*/
#define LED_MODE_STATE_REG            0x01
#define BASE_MAX_CURRENT_REG        0x03
#define BASE_RISE_FALL_REG              0x06
#define BASE_BRIGHTNESS_REG           0x09
#define BASE_RISE_SPEED_REG           0x0b
#define BASE_FALL_SPEED_REG           0x0c
#define LED_PORT1_REG_OFFSET         0
#define LED_PORT2_REG_OFFSET         1
#define LED_PORT3_REG_OFFSET         2


#define LED_CURREN_1MA  0x4
#define RF_TIME_1S   0x2
#define RF_SPEED_4MS  0x1
/*END*/

static Led_Ctl_Data g_led_ctl_data;
static Led_Ctl_Data g_led_ctl_data_red;
static Led_Ctl_Data g_led_ctl_data_green;
static int bct3253_int_flag=0;


struct led_classdev		red_led_cdev;
struct led_classdev		green_led_cdev;
struct led_classdev		red_breath_led_cdev;
struct led_classdev		green_breath_led_cdev;
struct led_classdev		red_blink_led_cdev;
struct led_classdev		green_blink_led_cdev;


enum {
	GREEN_LED_PORT=1,
	RED_LED_PORT,
};

enum {
	LED_MODE_NOR=0,
	LED_MODE_BREATH,
	LED_MODE_BLINK=LED_MODE_BREATH,
};

static void register_leds(struct device *dev);

static DEFINE_MUTEX(bct3253_i2c_access);

static int bct3253_driver_probe(struct i2c_client *client, const struct i2c_device_id *id);
static struct i2c_client *new_client = NULL;
static const struct i2c_device_id bct3253_i2c_id[] = {{"bct3253",0},{}};   

#ifdef GTP_CONFIG_OF
static const struct of_device_id bct_match_table[] = {
		{.compatible ="bct,bct3253",},
		{ },
};
#endif

static struct i2c_driver bct3253_driver = {
    .driver = {
        .name    = "bct3253",
    },
    .probe       = bct3253_driver_probe,
    .id_table    = bct3253_i2c_id,
#ifdef GTP_CONFIG_OF
        .of_match_table = bct_match_table,
#endif    
};

int bct3253_write_byte(kal_uint8 reg_addr, kal_uint8 reg_value)
{
    struct i2c_msg msg;
    s32 ret = -1;
    s32 retries = 0;
    char    write_data[2] = {0};
    printk("bct3253_write_byte!reg_addr =0x%x reg_value=0x%x \n",reg_addr,reg_value);
    if(new_client==NULL)
    {
    printk("bct3253_write_byte!new_client==NULL !\n");
       return ret;
    }	
    mutex_lock(&bct3253_i2c_access);	
	write_data[0]=reg_addr;
	write_data[1]=reg_value;
	msg.flags = !I2C_M_RD;
	msg.addr  = new_client->addr;
	msg.len   = 2;
	msg.buf   = write_data;

    while(retries < 5)
    {
        ret = i2c_transfer(new_client->adapter, &msg, 1);
        if (ret == 1)break;
        retries++;
    }
    if((retries >= 5))
    {
		printk("bct3253_write_byte error!\n ");
	}
        mutex_unlock(&bct3253_i2c_access);
	
    return ret;
}
//EXPORT_SYMBOL(bct3253_write_byte);
int bct3253_read_byte(kal_uint8 reg_addr, kal_uint8 *ret_buf)
{
    struct i2c_msg msgs[2];
    s32 ret=-1;
    s32 retries = 0;
    mutex_lock(&bct3253_i2c_access);	
    msgs[0].flags = !I2C_M_RD;
    msgs[0].addr  = new_client->addr;
    msgs[0].len   = 1;
    msgs[0].buf   = &reg_addr;
    
    msgs[1].flags = I2C_M_RD;
    msgs[1].addr  = new_client->addr;
    msgs[1].len   = 1;
    msgs[1].buf   =ret_buf;

    while(retries < 5)
    {
        ret = i2c_transfer(new_client->adapter, msgs, 2);
        if(ret == 2)break;
        retries++;
    }
    if((retries >= 5))
    {
	 printk("bct3253_read_byte!error!!\n");	
    }
    mutex_unlock(&bct3253_i2c_access);	
    return ret;
}



#if  0
int bct3253_read_byte(kal_uint8 reg_addr, kal_uint8 *ret_buf)
{
    char     data_buf[1]={0x00};
    char     readData = 0;
    int      ret=0;

    printk("bct3253_read_byte!\n");
    if(new_client==NULL)
    {
    printk("bct3253_read_byte!new_client==NULL !\n");
       return ret;
    }
	
    mutex_lock(&bct3253_i2c_access);
    
    //new_client->addr = ((new_client->addr) & I2C_MASK_FLAG) | I2C_WR_FLAG;    
    new_client->ext_flag=((new_client->ext_flag ) & I2C_MASK_FLAG ) | I2C_WR_FLAG /*| I2C_DIRECTION_FLAG*/;

    data_buf[0] = reg_addr;
    ret = i2c_master_send(new_client, &data_buf[0], (1<<8 | 1));
    if (ret < 0) 
    {    
        new_client->ext_flag=0;
        mutex_unlock(&bct3253_i2c_access);
	 printk("bct3253_read_byte!ret<0!\n");	
        return ret;
    }

    readData = data_buf[0];
   *ret_buf = readData;

    new_client->ext_flag=0;

    printk("bct3253_read_byte!read ok!!ret_buf=0x%x \n",ret_buf);	
	
    mutex_unlock(&bct3253_i2c_access);    
    return 1;
}

int bct3253_write_byte(kal_uint8 reg_addr, kal_uint8 reg_value)
{
    char    write_data[2] = {0};
    int     ret=-1;

    printk("bct3253_write_byte!reg_addr =0x%x reg_value=0x%x \n",reg_addr,reg_value);
    if(new_client==NULL)
    {
    printk("bct3253_write_byte!new_client==NULL !\n");
       return ret;
    }	
    mutex_lock(&bct3253_i2c_access);
    
    write_data[0] = reg_addr;
    write_data[1] = reg_value;	
    new_client->ext_flag=((new_client->ext_flag ) & I2C_MASK_FLAG ) /*| I2C_DIRECTION_FLAG*/;
    
    ret = i2c_master_send(new_client, write_data, 2);

    new_client->ext_flag=0;
    mutex_unlock(&bct3253_i2c_access);
    return ret;
}
#endif


int bct3253_hw_init(void)
{
      int ret=0;
 	ret=bct3253_write_byte(0,1);/*chip reset*/
	return ret;
}

void bct3253_hw_uninit(void)
{
}

//#define bct3253_BUSNUM 1
//static struct i2c_board_info __initdata i2c_bct3253 = { I2C_BOARD_INFO("bct3253", (0x30))};



/**********************************************************
  *
  *   [platform_driver API] 
  *
  *********************************************************/

#define LED_MODE_STATE_REG  0x01
#define BASE_MAX_CURRENT_REG        0x03
#define BASE_RISE_FALL_REG              0x06
#define BASE_BRIGHTNESS_REG           0x09
#define BASE_RISE_SPEED_REG           0x0b
#define BASE_FALL_SPEED_REG           0x0c
/*END*/


void  Update_Led_reg(void)
{
   char data=0x0;
   char mode;
   char state;
   int ret=-1;

 mode=((g_led_ctl_data_red.led_mode<<1)|g_led_ctl_data_green.led_mode);
 mode&=0x0f;
 state=((g_led_ctl_data_red.state<<1)|g_led_ctl_data_green.state);
 state&=0x0f;
 data=(mode<<4)|state;
 ret=bct3253_write_byte(LED_MODE_STATE_REG,data);
}

int set_led_reg(Led_Ctl_Data ctl_data)
{
	char data=0x0;
	int reg_offset=0;
      int ret=-1;
      int step=0;
   if(ctl_data.led_port>3)
   {
	ctl_data.led_port=3;
   }
   if(ctl_data.led_port<1)
   {
	ctl_data.led_port=1;
   }

   reg_offset=ctl_data.led_port-1;
 
  data=(ctl_data.fall_speed&0x00ff);
  ret= bct3253_write_byte(BASE_FALL_SPEED_REG+reg_offset*4,data);
   step++;
   if(ret<0)
   {
	printk("Set_Controller Failed at step %d \n",step);
	goto exit;
   }

   data=ctl_data.rise_speed&0x00ff;
  ret= bct3253_write_byte(BASE_RISE_SPEED_REG+reg_offset*4,data);
   step++;
   if(ret<0)
   {
	printk("Set_Controller Failed at step %d \n",step);
	goto exit;
   }

   data=((ctl_data.rise_time&0x0f)<<4 |(ctl_data.fall_time&0x0f));
   ret=bct3253_write_byte(BASE_RISE_FALL_REG+reg_offset,data);
   step++;
   if(ret<0)
   {
	printk("Set_Controller Failed at step %d \n",step);
	goto exit;
   }

   data=ctl_data.max_current;
   ret=bct3253_write_byte(2,0xc0);
   ret=bct3253_write_byte(BASE_MAX_CURRENT_REG+reg_offset,data);
   step++;
   if(ret<0)
   {
	printk("Set_Controller Failed at step %d \n",step);
	goto exit;
   }

   data=((ctl_data.max_brightness&0x0f)<<4 |(ctl_data.mid_brightness&0x0f));
   ret=bct3253_write_byte(BASE_BRIGHTNESS_REG+reg_offset*4,data);
   step++;
   if(ret<0)
   {
	printk("Set_Controller Failed at step %d \n",step);
	goto exit;
   }

   step++;
   if(ret<0)
   {
	printk("Set_Controller Failed at step %d \n",step);
	goto exit;
   }

exit:
 	
return ret;

}

int Set_Controller_func(Led_Ctl_Data ctl_data)
{
   char data=0x0;
   int reg_offset=0;
   int ret=-1;
   int step=0;
   if(ctl_data.led_port>3)
   {
	ctl_data.led_port=3;
   }
   if(ctl_data.led_port<1)
   {
	ctl_data.led_port=1;
   }

   reg_offset=ctl_data.led_port-1;
 
  data=(ctl_data.fall_speed&0x00ff);
  ret= bct3253_write_byte(BASE_FALL_SPEED_REG+reg_offset*4,data);
   step++;
   if(ret<0)
   {
	printk("Set_Controller Failed at step %d \n",step);
	goto exit;
   }

   data=ctl_data.rise_speed&0x00ff;
  ret= bct3253_write_byte(BASE_RISE_SPEED_REG+reg_offset*4,data);
   step++;
   if(ret<0)
   {
	printk("Set_Controller Failed at step %d \n",step);
	goto exit;
   }

   data=((ctl_data.rise_time&0x0f)<<4 |(ctl_data.fall_time&0x0f));
   ret=bct3253_write_byte(BASE_RISE_FALL_REG+reg_offset,data);
   step++;
   if(ret<0)
   {
	printk("Set_Controller Failed at step %d \n",step);
	goto exit;
   }

   data=ctl_data.max_current;
   ret=bct3253_write_byte(2,0xc0);
   ret=bct3253_write_byte(BASE_MAX_CURRENT_REG+reg_offset,data);
   step++;
   if(ret<0)
   {
	printk("Set_Controller Failed at step %d \n",step);
	goto exit;
   }

   data=((ctl_data.max_brightness&0x0f)<<4 |(ctl_data.mid_brightness&0x0f));
   ret=bct3253_write_byte(BASE_BRIGHTNESS_REG+reg_offset*4,data);
   step++;
   if(ret<0)
   {
	printk("Set_Controller Failed at step %d \n",step);
	goto exit;
   }

   step++;
   if(ret<0)
   {
	printk("Set_Controller Failed at step %d \n",step);
	goto exit;
   }

   data=((ctl_data.led_mode&0x0f)<<4 |(ctl_data.state&0x0f));
   ret=bct3253_write_byte(LED_MODE_STATE_REG,data);
   step++;
   if(ret<0)
   {
	printk("Set_Controller Failed at step %d \n",step);
   }
exit:   
   return ret;
}


static void bct3253_led_work_func(struct work_struct *work)
{
       printk("bct3253_led_work_func ! \n");
	if(bct3253_int_flag)
	{
		bct3253_write_byte(0,1);
		set_led_reg(g_led_ctl_data_green);
		set_led_reg(g_led_ctl_data_red);
		Update_Led_reg();
	}else{
		printk("bct3253_led_work_func bct3253_int_flag ==0 do nothing!\n");
	}
}


static void bct3253_work_func(struct work_struct *work)
{
       printk("bct3253_work_func ! \n");
	if(bct3253_int_flag)
	{
		Set_Controller_func(g_led_ctl_data);
	}else{
		printk("bct3253_work_func bct3253_int_flag ==0 do nothing!\n");
	}
}
static ssize_t show_bct3253_controller(struct device *dev,struct device_attribute *attr, char *buf)
{
    printk("[show_bct3253_controller] \n");
	if(bct3253_int_flag)
	{
		printk("[show_bct3253_controller]  power on ok\n");
	    return sprintf(buf, "power on ok \n");
	}else{ 
	printk("[show_bct3253_controller]  not find bct3253 \n");
	    return sprintf(buf, "not find bct3253 \n");
	}
}

static ssize_t store_bct3253_controller(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
  Led_Ctl_Data g_led_ctl_data_temp;
   printk("store_bct3253_controller %s \n",buf);
   if(bct3253_int_flag==0)
   {
	printk("store_bct3253_controller not find bct3253,so do nothing !\n");
	  return size;
   }
    sscanf(buf ,"%d %d %d %d %d %d %d %d %d %d",&(g_led_ctl_data_temp.led_port),&(g_led_ctl_data_temp.led_mode),&(g_led_ctl_data_temp.state), \
		&(g_led_ctl_data_temp.max_current),&(g_led_ctl_data_temp.max_brightness),&(g_led_ctl_data_temp.mid_brightness),&(g_led_ctl_data_temp.rise_time), \
		&(g_led_ctl_data_temp.fall_time),&(g_led_ctl_data_temp.rise_speed),&(g_led_ctl_data_temp.fall_speed));

if((g_led_ctl_data_temp.led_port==g_led_ctl_data.led_port)&&(g_led_ctl_data_temp.led_mode==g_led_ctl_data.led_mode) \
	&&(g_led_ctl_data_temp.state==g_led_ctl_data.state) &&(g_led_ctl_data_temp.max_current==g_led_ctl_data.max_current) \
 	&&(g_led_ctl_data_temp.max_current==g_led_ctl_data.max_current) &&(g_led_ctl_data_temp.max_brightness==g_led_ctl_data.max_brightness) \
 	&&(g_led_ctl_data_temp.mid_brightness==g_led_ctl_data.mid_brightness) &&(g_led_ctl_data_temp.rise_time==g_led_ctl_data.rise_time)   \
     &&(g_led_ctl_data_temp.fall_time==g_led_ctl_data.fall_time) &&(g_led_ctl_data_temp.rise_speed==g_led_ctl_data.rise_speed)  \
      &&(g_led_ctl_data_temp.fall_speed==g_led_ctl_data.fall_speed) )
	{
	   printk("store_bct3253_controller nothing changed ,so do nothing !\n");
	  return size;
	}

	//hwPowerOn(MT65XX_POWER_LDO_VGP2,VOL_1800,"bct3253");
	bct3253_write_byte(0,1);
	g_led_ctl_data.led_port=g_led_ctl_data_temp.led_port;
 	g_led_ctl_data.led_mode=g_led_ctl_data_temp.led_mode;
	g_led_ctl_data.state=g_led_ctl_data_temp.state;
	g_led_ctl_data.max_current=g_led_ctl_data_temp.max_current; 
	g_led_ctl_data.max_brightness=g_led_ctl_data_temp.max_brightness; 
 	g_led_ctl_data.mid_brightness=g_led_ctl_data_temp.mid_brightness;
	g_led_ctl_data.rise_time=g_led_ctl_data_temp.rise_time; 
       g_led_ctl_data.fall_time=g_led_ctl_data_temp.fall_time;
	g_led_ctl_data.rise_speed=g_led_ctl_data_temp.rise_speed; 
       g_led_ctl_data.fall_speed=g_led_ctl_data_temp.fall_speed;



    printk("store_bct3253_controller %d, %d, %d ,%d, %d, %d, %d, %d, %d, %d\n",g_led_ctl_data.led_port,g_led_ctl_data.led_mode, \
		g_led_ctl_data.max_current,g_led_ctl_data.max_brightness,g_led_ctl_data.mid_brightness, \
		g_led_ctl_data.rise_time,g_led_ctl_data.fall_time,g_led_ctl_data.rise_speed, \
		g_led_ctl_data.fall_speed,g_led_ctl_data.state);	
    queue_delayed_work(bct3253_wq, &bct3253_work,LED_WORK_DELAY);
    return size;
}
static DEVICE_ATTR(bct3253_access, 0666, show_bct3253_controller, store_bct3253_controller); //664

static int bct3253_controller_probe(struct platform_device *dev)    
{    
    int ret_device_file = 0;

    printk("******** bct3253_controller_probe!! ********\n" );
    
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_bct3253_access);


    
    return 0;
}

struct platform_device bct3253_controller_device = {
    .name   = "bct3253_controller",
    .id     = -1,
};

static struct platform_driver bct3253_controller_driver = {
    .probe      = bct3253_controller_probe,
    .driver     = {
        .name = "bct3253_controller",
    },
};


static int bct3253_driver_probe(struct i2c_client *client, const struct i2c_device_id *id) 
{             
    int err=0; 
    printk("[bct3253_driver_probe] \n");

    if (!(new_client = kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
        err = -ENOMEM;
        goto exit;
    }    
    memset(new_client, 0, sizeof(struct i2c_client));

    new_client = client;

	/*g_led_ctl_data.led_port=1;
	g_led_ctl_data.led_mode=0;
	g_led_ctl_data.state=1;
	g_led_ctl_data.max_current=80;
	g_led_ctl_data.max_brightness=15;
	g_led_ctl_data.mid_brightness=8;
	g_led_ctl_data.rise_time=9;
	g_led_ctl_data.fall_time=1;
	g_led_ctl_data.rise_speed=17;
	g_led_ctl_data.fall_speed=17;
	*/
    //---------------------
    if(bct3253_hw_init()<0)
    {
    	bct3253_hw_uninit();
   //     kfree(new_client);
	  new_client=NULL;
        err = -ENOMEM;
	bct3253_int_flag=0;
	 printk("[bct3253_driver_probe]  bct3253_hw_init   failed ! \n");
	 goto exit;
    }                   
	bct3253_int_flag=1;
	register_leds(&(client->dev));


	INIT_DELAYED_WORK(&bct3253_work_led, bct3253_led_work_func);
	bct3253_wq_led = create_singlethread_workqueue("bct3253_wq_led");
	
	INIT_DELAYED_WORK(&bct3253_work, bct3253_work_func);
	bct3253_wq = create_singlethread_workqueue("bct3253_wq");
	if(!bct3253_wq)
	{
		printk("bct3253_driver_probe create wq failed! \n");
		bct3253_hw_uninit();
	//	kfree(new_client);
		new_client=NULL;
		err = -ENOMEM;
	}
	Set_Controller_func(g_led_ctl_data);
	
exit:
    return err;

}

static void kernel_set_breath_led( int port , int mode , int state , int mcurrent , int max_b , int mid_b ,int rise_time ,int fall_time , int rspeed , int fspeed)
{
		if(state>0)
		{
			state=1;
		}else{
			state=0;
		}

		if(port==RED_LED_PORT)
		{
			g_led_ctl_data_red.led_port=port;
			g_led_ctl_data_red.led_mode=mode;
			g_led_ctl_data_red.state=state;
			g_led_ctl_data_red.max_current=mcurrent; 
			g_led_ctl_data_red.max_brightness=max_b; 
			g_led_ctl_data_red.mid_brightness=mid_b;
			g_led_ctl_data_red.rise_time=rise_time; 
			g_led_ctl_data_red.fall_time=fall_time;
			g_led_ctl_data_red.rise_speed=rspeed; 
			g_led_ctl_data_red.fall_speed=fspeed;
			
		}else if(port==GREEN_LED_PORT)
		{
			g_led_ctl_data_green.led_port=port;
			g_led_ctl_data_green.led_mode=mode;
			g_led_ctl_data_green.state=state;
			g_led_ctl_data_green.max_current=mcurrent; 
			g_led_ctl_data_green.max_brightness=max_b; 
			g_led_ctl_data_green.mid_brightness=mid_b;
			g_led_ctl_data_green.rise_time=rise_time; 
			g_led_ctl_data_green.fall_time=fall_time;
			g_led_ctl_data_green.rise_speed=rspeed; 
			g_led_ctl_data_green.fall_speed=fspeed;
			
		}
	queue_delayed_work(bct3253_wq_led, &bct3253_work_led,LED_WORK_DELAY);
}

static void red_led_brightness_set(struct led_classdev *cdev,
		enum led_brightness value)
{
		kernel_set_breath_led(RED_LED_PORT,LED_MODE_NOR,value,16,15,8,9,1,17,17);
}

static enum
led_brightness red_led_brightness_get(struct led_classdev *cdev)
{
	if(!(g_led_ctl_data_red.led_mode&0x1))
	{
		if(g_led_ctl_data_red.state&0x01)
		{
			return LED_FULL;
		}
	}
	return LED_OFF;
}


static void green_led_brightness_set(struct led_classdev *cdev,
		enum led_brightness value)
{
		kernel_set_breath_led(GREEN_LED_PORT,LED_MODE_NOR,value,16,15,8,9,1,17,17);
}



static enum
led_brightness green_led_brightness_get(struct led_classdev *cdev)
{
	if(!(g_led_ctl_data_green.led_mode&0x1))
	{
		if(g_led_ctl_data_green.state&0x01)
		{
			return LED_FULL;
		}
	}
	return LED_OFF;
}


static void red_breath_led_brightness_set(struct led_classdev *cdev,
		enum led_brightness value)
{

		kernel_set_breath_led(RED_LED_PORT,LED_MODE_BREATH,value,16,15,8,10,7,88,85);

}


static enum
led_brightness red_breath_led_brightness_get(struct led_classdev *cdev)
{
	if((g_led_ctl_data_red.led_mode&0x1))
	{
		if(g_led_ctl_data_red.state&0x01)
		{
			return LED_FULL;
		}
	}
	return LED_OFF;
}


static void green_breath_led_brightness_set(struct led_classdev *cdev,
		enum led_brightness value)
{
		kernel_set_breath_led(GREEN_LED_PORT,LED_MODE_BREATH,value,16,15,8,10,7,88,85);
}


static enum
led_brightness green_breath_led_brightness_get(struct led_classdev *cdev)
{
	if((g_led_ctl_data_green.led_mode&0x1))
	{
		if(g_led_ctl_data_green.state&0x01)
		{
			return LED_FULL;
		}
	}
	return LED_OFF;
}


static void red_blink_led_brightness_set(struct led_classdev *cdev,
		enum led_brightness value)
{
		kernel_set_breath_led(RED_LED_PORT,LED_MODE_BREATH,value,16,15,8,7,1,16,0);
}

static void green_blink_led_brightness_set(struct led_classdev *cdev,
		enum led_brightness value)
{
		kernel_set_breath_led(GREEN_LED_PORT,LED_MODE_BREATH,value,16,15,8,7,1,16,0);
}



static int bct3253_register_red_led(struct device *dev)
{
	int rc;
	red_led_cdev.name = "red";
	red_led_cdev.brightness_set =red_led_brightness_set;
	red_led_cdev.brightness_get =red_led_brightness_get;

	rc = led_classdev_register(dev, &red_led_cdev);
	if (rc)
		dev_err(dev, "unable to register red led, rc=%d\n",rc);
	return rc;
};


static int bct3253_register_red_breath_led(struct device *dev)
{
	int rc;
	red_breath_led_cdev.name = "red_breath";
	red_breath_led_cdev.brightness_set =red_breath_led_brightness_set;
	red_breath_led_cdev.brightness_get =red_breath_led_brightness_get;

	rc = led_classdev_register(dev, &red_breath_led_cdev);
	if (rc)
	dev_err(dev, "unable to register red_breath led, rc=%d\n",rc);
	return rc;
};


static int bct3253_register_red_blink_led(struct device *dev)
{
	int rc;
	red_blink_led_cdev.name = "red_blink";
	red_blink_led_cdev.brightness_set =red_blink_led_brightness_set;
	red_blink_led_cdev.brightness_get =red_breath_led_brightness_get;

	rc = led_classdev_register(dev, &red_blink_led_cdev);
	if (rc)
		dev_err(dev, "unable to register red_blink led, rc=%d\n",rc);
	return rc;
};


static int bct3253_register_green_led(struct device *dev)
{
	int rc;
	green_led_cdev.name = "green";
	green_led_cdev.brightness_set =green_led_brightness_set;
	green_led_cdev.brightness_get =green_led_brightness_get;

	rc = led_classdev_register(dev, &green_led_cdev);
	if (rc)
		dev_err(dev, "unable to register green led, rc=%d\n",rc);
	return rc;
};


static int bct3253_register_green_breath_led(struct device *dev)
{
	int rc;
	green_breath_led_cdev.name = "green_breath";
	green_breath_led_cdev.brightness_set =green_breath_led_brightness_set;
	green_breath_led_cdev.brightness_get =green_breath_led_brightness_get;

	rc = led_classdev_register(dev, &green_breath_led_cdev);
	if (rc)
		dev_err(dev, "unable to register green_breath led, rc=%d\n",rc);
	return rc;
};


static int bct3253_register_green_blink_led(struct device *dev)
{
	int rc;
	green_blink_led_cdev.name = "green_blink";
	green_blink_led_cdev.brightness_set =green_blink_led_brightness_set;
	green_blink_led_cdev.brightness_get =green_breath_led_brightness_get;

	rc = led_classdev_register(dev, &green_blink_led_cdev);
	if (rc)
		dev_err(dev, "unable to register green_blink led, rc=%d\n",rc);
	return rc;
};

static void register_leds(struct device *dev)
{
	bct3253_register_red_led(dev);
	bct3253_register_green_led(dev);
	bct3253_register_red_breath_led(dev);
	bct3253_register_green_breath_led(dev);
	bct3253_register_red_blink_led(dev);
	bct3253_register_green_blink_led(dev);
}

static int __init bct3253_init(void)
{
    int ret=0;
    
    printk("[bct3253_init] init start\n");
    
//    i2c_register_board_info(bct3253_BUSNUM, &i2c_bct3253, 1);

    if(i2c_add_driver(&bct3253_driver)!=0)
    {
        printk("[bct3253_init] failed to register bct3253 i2c driver.\n");
    }
    else
    {
        printk("[bct3253_init] Success to register bct3253 i2c driver.\n");
    }

    // bq24158 user space access interface
    ret = platform_device_register(&bct3253_controller_device);
    if (ret) {
        printk("****[bct3253_init] Unable to device register(%d)\n", ret);
        return ret;
    }    
    ret = platform_driver_register(&bct3253_controller_driver);
    if (ret) {
        printk("****[bct3253_init] Unable to register driver (%d)\n", ret);
        return ret;
    }
    return 0;   
}

static void __exit bct3253_exit(void)
{
}

module_init(bct3253_init);
module_exit(bct3253_exit);
//MODULE_DESCRIPTION("BCT3253 Driver");
//MODULE_LICENSE("GPL");
