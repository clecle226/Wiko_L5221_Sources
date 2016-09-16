#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/proc_fs.h>

#if defined(CONFIG_FB)
#include <linux/notifier.h>
#include <linux/fb.h>
#elif defined(CONFIG_HAS_EARLYSUSPEND)
    #include <linux/pm.h>
    #include <linux/earlysuspend.h>
#endif
#define TPD_PROBE_MAGIC_NUM  20150323
extern int tpd_has_probe_flag;
int phy_x_resolution;
int phy_y_resolution;

#include <linux/miscdevice.h>


//add for tp fw update
	int fw_file_ver=0;
//end

//fix bug:EBBAL-2538
static int down_flag=0;
//end


//#define TINNO_DEV_INFO

#ifdef CONFIG_TINNO_DEV_INFO
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
DEF_TINNO_DEV_INFO(TouchPanel)
DEF_TINNO_DEV_INFO(TouchPanel_Fw_Ver)
static char des_buf[100];
#endif

#include "elan_ktf.h"
//include <linux/i2c/elan_ts_board.h>

#define VIRTUAL_TOUCH_KEY_INDEX  3
static int boot_mode=0;
#define  FTM_MODE   1


#define COORDS_ARR_SIZE    4
#define MAX_BUTTONS    4

#define VTG_MIN_UV       2600000
#define VTG_MAX_UV       3300000
#define I2C_VTG_MIN_UV   1800000
#define I2C_VTG_MAX_UV   1800000

#define ELAN_ESD_CHECK
#ifdef ELAN_ESD_CHECK
    static int have_interrupts = 0;
    static struct workqueue_struct *esd_wq = NULL;
    static struct delayed_work esd_work;
    static unsigned long delay = 2*HZ;
#endif

//add for pinctl
static struct pinctrl *Touch_pinctrl;
//add end

struct mutex		ts_pm_lock;
static int suspend_flag=0;

/*********************************platform data********************************************/
//must be init first
static unsigned int reset_gpio  = 0;
static unsigned int intr_gpio = 0;
static int elan_irq = 0;

static const struct i2c_device_id elan_ts_id[] = {
    {ELAN_TS_NAME, 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, elan_ts_id);

#ifdef I2C_BOARD_REGISTER
static struct i2c_board_info __initdata elan_i2c_info={
    I2C_BOARD_INFO(ELAN_TS_NAME, (ELAN_7BITS_ADDR))
};
//#else
static const unsigned short normal_i2c[2] = {
    ELAN_7BITS_ADDR,
    I2C_CLIENT_END
};
#endif

/**********************************elan struct*********************************************/
struct elan_ts_i2c_platform_data {
    const char *name;
    u32 irqflags;
    u32 intr_gpio;
    u32 intr_gpio_flags;
    u32 rst_gpio;
    u32 rst_gpio_flags;
    u32 x_max;
    u32 y_max;
    u32 x_min;
    u32 y_min;
    u32 panel_minx;
    u32 panel_miny;
    u32 panel_maxx;
    u32 panel_maxy;
    int (*init_platform_hw)(void);
    int (*power_init) (bool);
    int (*power_on) (bool);
};

struct elan_ts_data{
    struct i2c_client *client;
    struct elan_ts_i2c_platform_data *pdata;
    struct input_dev *input_dev;
    struct regulator *vdd;
    struct regulator *vcc_i2c;
//queue or thread handler interrupt
    struct task_struct *work_thread;
    struct work_struct  work;
#if defined(CONFIG_FB)
    struct notifier_block fb_notif;
#elif defined(CONFIG_HAS_EARLYSUSPEND)
//used for early_suspend
    struct early_suspend early_suspend;
#endif
//Firmware Information
    int fw_ver;
    int fw_id;
    int fw_bcd;
    int x_resolution;
    int y_resolution;
    int recover;//for iap mod
//for suspend or resum lock
    int power_lock;
    int circuit_ver;
//for button state
    int button_state;
//For Firmare Update
    struct miscdevice firmware;
};

/************************************global elan data*************************************/
static int tpd_flag = 0;
//static int file_fops_addr = ELAN_7BITS_ADDR;

static struct elan_ts_data *private_ts;
static DECLARE_WAIT_QUEUE_HEAD(waiter);

/*********************************global elan function*************************************/
static int __hello_packet_handler(struct i2c_client *client);
static int __fw_packet_handler(struct i2c_client *client);
static int elan_ts_setup(struct i2c_client *client);
static int elan_ts_rough_calibrate(struct i2c_client *client);
static int elan_ts_resume(struct i2c_client *client);

#if defined(CONFIG_FB)
static int fb_notifier_callback(struct notifier_block *self,
                unsigned long event, void *data);
#elif defined(CONFIG_HAS_EARLYSUSPEND)

static void elan_ts_early_suspend(struct early_suspend *h);
static void elan_ts_late_resume(struct early_suspend *h);
#endif

#if defined IAP_PORTION
static int check_update_flage(struct elan_ts_data *ts);
#endif

/************************************** function list**************************************/
void touch_callback(unsigned cable_status)
{
    elan_info("[elan] %s enter cable_status:%d\n", __func__, cable_status);
}
static void elan_reset(void)
{
    elan_info("[elan]:%s enter\n", __func__);
    gpio_direction_output(reset_gpio, 1);
    mdelay(20);	
    gpio_set_value(reset_gpio, 1);
    mdelay(20);
    gpio_set_value(reset_gpio, 0);
    mdelay(20);
    gpio_set_value(reset_gpio, 1);
    mdelay(20);
}

static void elan_switch_irq(int on)
{
    elan_info("[elan] %s enter, irq = %d, on = %d\n", __func__, elan_irq, on);
    if(on){
        enable_irq(elan_irq);
    }
    else{
        disable_irq(elan_irq);
    }
}

static int elan_ts_poll(void)
{
    int status = 0, retry = 20;

    do {
        status = gpio_get_value(intr_gpio);
        elan_info("[elan]: %s: status = %d\n", __func__, status);
        retry--;
        mdelay(50);
    } while (status == 1 && retry > 0);

    elan_info( "[elan]%s: poll interrupt status %s\n", __func__, status == 1 ? "high" : "low");

    return status == 0 ? 0 : -ETIMEDOUT;
}

static int elan_ts_send_cmd(struct i2c_client *client, uint8_t *cmd, size_t size)
{
    elan_info("[elan] dump cmd: %02x, %02x, %02x, %02x\n", cmd[0], cmd[1], cmd[2], cmd[3]);
    if (i2c_master_send(client, cmd, size) != size) {
        elan_info("[elan error]%s: elan_ts_send_cmd failed\n", __func__);
        return -EINVAL;
    }
    else{
        elan_info("[elan] elan_ts_send_cmd ok");
    }
    return size;
}

static int elan_ts_get_data(struct i2c_client *client, uint8_t *cmd, uint8_t *buf, size_t size)
{
    int rc;

    if (buf == NULL)
        return -EINVAL;

    if (elan_ts_send_cmd(client, cmd, size) != size)
        return -EINVAL;

    mdelay(2);
    rc = elan_ts_poll();
    if (rc < 0){
        return -EINVAL;
    }
    else {
        if (i2c_master_recv(client, buf, size) != size ||buf[0] != CMD_S_PKT){
            elan_info("[elan error]%s: i2c_master_recv failed\n", __func__);
            return -EINVAL;
        }
        elan_info("[elan] %s: respone packet %2x:%2X:%2x:%2x\n", __func__, buf[0], buf[1], buf[2], buf[3]);
    }

    return 0;
}


//record the FW ID in IC section
static int write_check_fwid_to_rom(struct i2c_client *client, uint8_t *cmd, size_t size)
{
    int rc;
    uint8_t get_cmd[] = {0x53, 0xD3, 0x00, 0x01}; /* Get CHECK FWID */
    uint8_t buf_recv[4] = { 0 };
    uint8_t retry = 0;
	
    elan_info("[elan] check cmd: %02x, %02x, %02x, %02x\n", cmd[0], cmd[1], cmd[2], cmd[3]);

check_id:
    rc = elan_ts_get_data(client, get_cmd, buf_recv, 4);//get the infomation and show out
    if (rc < 0)
    {
        return rc;
    }
    elan_info("[elan] read SENSOR option: %02x, %02x, %02x, %02x\n", buf_recv[0], buf_recv[1], buf_recv[2], buf_recv[3]);

    if((buf_recv[2] == cmd[2]) && (buf_recv[3] == cmd[3]) )
        return 0;

    if ((i2c_master_send(client, cmd, 4)) != 4)
    {
        elan_info("[elan] %s: i2c_master_send failed\n", __func__);
        return -1;
    }
    msleep(50);
    if((++retry) < 5 )
    {
        elan_info("[elan] %s: retry %d times for sensor option!\n", __func__, retry);
	    goto check_id;
    }
	
    elan_info("[elan] %s: retry %d times failed !\n", __func__, retry);
    return -1;
}

static int __hello_packet_handler(struct i2c_client *client)
{
    struct elan_ts_data *ts = i2c_get_clientdata(client);
    int rc;
    uint8_t buf_recv[8] = { 0 };

    rc = elan_ts_poll();
    if(rc != 0){
        elan_info("[elan] %s: Int poll 55 55 55 55 failed!\n", __func__);
    }

    rc = i2c_master_recv(client, buf_recv, sizeof(buf_recv));
    if(rc != sizeof(buf_recv)){
        elan_info("[elan error] __hello_packet_handler recv error, retry\n");
        rc = i2c_master_recv(client, buf_recv, sizeof(buf_recv));
    }
    elan_info("[elan] %s: hello packet %2x:%2X:%2x:%2x\n", __func__, buf_recv[0], buf_recv[1], buf_recv[2], buf_recv[3]);

    if(buf_recv[0]==0x55 && buf_recv[1]==0x55 && buf_recv[2]==0x80 && buf_recv[3]==0x80){
        elan_info("[elan] %s: boot code packet %2x:%2X:%2x:%2x\n", __func__, buf_recv[4], buf_recv[5], buf_recv[6], buf_recv[7]);
        ts->recover = 0x80;
		ts->fw_id = buf_recv[5] << 8 | buf_recv[4];
    }
    else if(buf_recv[0]==0x55 && buf_recv[1]==0x55 && buf_recv[2]==0x55 && buf_recv[3]==0x55){
        elan_info("[elan] __hello_packet_handler recv ok\n");
		#ifdef ELAN_3K_XX
        msleep(200);
        rc = elan_ts_poll();
        if (rc < 0) {
            elan_info("[elan] %s: 66 66 66 66 failed!\n", __func__);
        }
        rc = i2c_master_recv(client, buf_recv, 4);
        elan_info("[elan] %s: RK packet %2x:%2X:%2x:%2x\n", __func__, buf_recv[0], buf_recv[1], buf_recv[2], buf_recv[3]);
		#endif
        ts->recover = 0;
    }
    else{
        ts->recover = -EINVAL;
    }

    rc = ts->recover;
    return rc;
}

static int __fw_packet_handler(struct i2c_client *client)
{
    struct elan_ts_data *ts = i2c_get_clientdata(client);
    int rc;
    int major, minor;
    uint8_t cmd[]           = {0x53, 0x00, 0x00, 0x01};/* Get Firmware Version*/
    uint8_t cmd_check_fwid[] = { 0x54, 0XD2, 0xFF,0xFF };  /* Get Check FWID */
    uint8_t cmd_id[]        = {0x53, 0xf0, 0x00, 0x01}; /*Get firmware ID*/
    uint8_t cmd_bc[]        = {0x53, 0x10, 0x00, 0x01};/* Get BootCode Version*/
#ifdef ELAN_3K_XX
    int x, y;
    uint8_t cmd_getinfo[] = {0x5B, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t adcinfo_buf[17]={0};
#else
    uint8_t cmd_x[]         = {0x53, 0x60, 0x00, 0x00}; /*Get x resolution*/
    uint8_t cmd_y[]         = {0x53, 0x63, 0x00, 0x00}; /*Get y resolution*/
#endif
    uint8_t buf_recv[4]     = {0};
// Firmware version
    rc = elan_ts_get_data(client, cmd, buf_recv, 4);
    if (rc < 0)
        return rc;
    major = ((buf_recv[1] & 0x0f) << 4) | ((buf_recv[2] & 0xf0) >> 4);
    minor = ((buf_recv[2] & 0x0f) << 4) | ((buf_recv[3] & 0xf0) >> 4);
    ts->fw_ver = major << 8 | minor;

#ifndef ELAN_3K_XX
    // X Resolution
    rc = elan_ts_get_data(client, cmd_x, buf_recv, 4);
    if (rc < 0)
        return rc;
    minor = ((buf_recv[2])) | ((buf_recv[3] & 0xf0) << 4);
    ts->x_resolution = minor;

    // Y Resolution
    rc = elan_ts_get_data(client, cmd_y, buf_recv, 4);
    if (rc < 0)
        return rc;
    minor = ((buf_recv[2])) | ((buf_recv[3] & 0xf0) << 4);
    ts->y_resolution = minor;
#else
    i2c_master_send(client, cmd_getinfo, sizeof(cmd_getinfo));
    msleep(10);
    i2c_master_recv(client, adcinfo_buf, 17);
    x  = adcinfo_buf[2]+adcinfo_buf[6]+adcinfo_buf[10]+adcinfo_buf[14];
    y  = adcinfo_buf[3]+adcinfo_buf[7]+adcinfo_buf[11]+adcinfo_buf[15];

    elan_info( "[elan] %s: x= %d, y=%d\n",__func__,x,y);

    ts->x_resolution=(x-1)*64;
    ts->y_resolution=(y-1)*64;
#endif

phy_x_resolution=ts->x_resolution;
phy_y_resolution=ts->y_resolution;

ts->x_resolution=480;
 ts->y_resolution=854;
// Firmware BC
    rc = elan_ts_get_data(client, cmd_bc, buf_recv, 4);
    if (rc < 0)
        return rc;
    major = ((buf_recv[1] & 0x0f) << 4) | ((buf_recv[2] & 0xf0) >> 4);
    minor = ((buf_recv[2] & 0x0f) << 4) | ((buf_recv[3] & 0xf0) >> 4);
    ts->fw_bcd = major << 8 | minor;

// Firmware ID
	rc = elan_ts_get_data(client, cmd_id, buf_recv, 4);
	if (rc < 0)
		return rc;
	major = ((buf_recv[1] & 0x0f) << 4) | ((buf_recv[2] & 0xf0) >> 4);
	minor = ((buf_recv[2] & 0x0f) << 4) | ((buf_recv[3] & 0xf0) >> 4);
    cmd_check_fwid[2] = major;
    cmd_check_fwid[3] = minor;
    rc = write_check_fwid_to_rom(client, cmd_check_fwid , 4);  // write check fwid info
	ts->fw_id = major << 8 | minor;

    elan_info( "[elan] %s: firmware version: 0x%4.4x\n",__func__, ts->fw_ver);
    elan_info( "[elan] %s: firmware ID: 0x%4.4x\n",__func__, ts->fw_id);
    elan_info( "[elan] %s: firmware BC: 0x%4.4x\n",__func__, ts->fw_bcd);
    elan_info( "[elan] %s: x resolution: %d, y resolution: %d\n",__func__, ts->x_resolution, ts->y_resolution);
#ifdef  CONFIG_TINNO_DEV_INFO
    sprintf(des_buf, "HUARUIC-EKTF2232-L5221-%x",ts->fw_ver);
    SET_DEVINFO_STR(TouchPanel,des_buf);
    sprintf(des_buf, "%x",ts->fw_ver);
    SET_DEVINFO_STR(TouchPanel_Fw_Ver,des_buf);
#endif
    return 0;
}

#if defined IAP_PORTION || defined ELAN_RAM_XX

static void get_vendor_info(struct elan_ts_data *ts)
{
    int i,vendor_num = 0;
    elan_info("KERN_ERR [elan] %s:  FW_ID: 0x%4.4x \n", __func__, ts->fw_id);
    vendor_num = sizeof(g_vendor_map)/sizeof(g_vendor_map[0]);
    for(i=0; i < vendor_num; i++)
    {
        if(ts->fw_id== g_vendor_map[i].vendor_id)
        {
            file_fw_data = g_vendor_map[i].fw_array;
			elan_info("elan vendor_num = %s\n", g_vendor_map[i].vendor_name);
            return;
        }
    }
    elan_info(KERN_ERR "[elan] TP ID is error: no support!\n");
}

int WritePage(struct i2c_client *client, uint8_t * szPage, int byte, int which)
{
    int len = 0;

#if 1//132 mod
    len = i2c_master_send(client, szPage,  byte);
    if (len != byte) {
        elan_info("[elan] ERROR: write the %d th page error, write error. len=%d\n", which, len);
        return -1;
    }
#else//8bit mod
    int i = 0;
    for(i=0; i<16; i++){
        len = i2c_master_send(client, (szPage+i*8),  8);
        if (len != 8) {
            elan_info("[elan] ERROR: write the %d th page error, write error. len=%d\n", which, len);
            return -1;
        }
    }

    len = i2c_master_send(client, (szPage+i*8),  4);
    if (len != 4) {
        elan_info("[elan] ERROR: write the %d th page error, write error. len=%d\n", which, len);
        return -1;
    }
#endif

    return 0;
}

/*every page write to recv 2 bytes ack */
int GetAckData(struct i2c_client *client, uint8_t *ack_buf)
{
    int len = 0;

    len=i2c_master_recv(client, ack_buf, 2);
    if (len != 2) {
        elan_info("[elan] ERROR: GetAckData. len=%d\r\n", len);
        return -1;
    }

    if (ack_buf[0] == 0xaa && ack_buf[1] == 0xaa) {
        return ACK_OK;
    }
    else if (ack_buf[0] == 0x55 && ack_buf[1] == 0x55){
        return ACK_REWRITE;
    }
    else{
        return ACK_Fail;
    }
    return 0;
}

/* Check Master & Slave is "55 aa 33 cc" */
int CheckIapMode(struct i2c_client *client)
{
    char buff[4] = {0};
    int rc = 0;

    rc = i2c_master_recv(client, buff, 4);
    if (rc != 4) {
        elan_info("[elan] ERROR: CheckIapMode. len=%d\r\n", rc);
        return -1;
    }
    else
        elan_info("[elan] Mode= 0x%x 0x%x 0x%x 0x%x\r\n", buff[0], buff[1], buff[2], buff[3]);

    return 0;
}
void update_fw_one(struct i2c_client *client)
{
    uint8_t ack_buf[2] = {0};
    struct elan_ts_data *ts = i2c_get_clientdata(client);

    int res = 0;
    int iPage = 0;

    uint8_t data;

    const int PageSize = 132;
    //const int PageNum = sizeof(file_fw_data)/PageSize;
	int PageNum;

    const int PAGERETRY = 10;
    const int IAPRESTART = 3;

    int restartCnt = 0; // For IAP_RESTART
    int rewriteCnt = 0;// For IAP_REWRITE

    int iap_mod;

    uint8_t *szBuff = NULL;
    int curIndex = 0;

#ifdef ELAN_2K_XX
    uint8_t isp_cmd[] = {0x54, 0x00, 0x12, 0x34};    //54 00 12 34
    iap_mod = 2;
	PageNum = 249;
#endif

#ifdef ELAN_3K_XX
    uint8_t isp_cmd[] = {0x45, 0x49, 0x41, 0x50};    //45 49 41 50
    iap_mod = 3;
	PageNum = 377;
#endif

#ifdef ELAN_RAM_XX
    uint8_t isp_cmd[] = {0x22, 0x22, 0x22, 0x22};    //22 22 22 22
    iap_mod = 1;
#endif

    elan_switch_irq(0);
    ts->power_lock = 1;

    data=ELAN_7BITS_ADDR;
    elan_info( "[elan] %s: address data=0x%x iap_mod=%d PageNum = %d\r\n", __func__, data, iap_mod, PageNum);

IAP_RESTART:
    //reset tp
    if(iap_mod == 3){
        elan_reset();
    }

    if((iap_mod!=2) || (ts->recover != 0x80)){
        elan_info("[elan] Firmware update normal mode !\n");
        //Step 1 enter isp mod
        res = elan_ts_send_cmd(client, isp_cmd, sizeof(isp_cmd));
        //Step 2 Chech IC's status is 55 aa 33 cc
	    msleep(10);
        if(1){
            res = CheckIapMode(client);
        }
    } else{
        elan_info("[elan] Firmware update recovery mode !\n");
    }

    //Step 3 Send Dummy Byte
    res = i2c_master_send(client, &data,  sizeof(data));
    if(res!=sizeof(data)){
        elan_info("[elan] dummy error code = %d\n",res);
        return;
    }
    else{
        elan_info("[elan] send Dummy byte sucess data:%x", data);
    }

    msleep(10);


    //Step 4 Start IAP
    for( iPage = 1; iPage <= PageNum; iPage++ ) {

        szBuff = file_fw_data + curIndex;
        curIndex =  curIndex + PageSize;

PAGE_REWRITE:
        res = WritePage(client, szBuff, PageSize, iPage);

        if(iPage==PageNum || iPage==1){
            msleep(300);
        }
        else{
            msleep(50);
        }

        res = GetAckData(client, ack_buf);
        if (ACK_OK != res) {

            msleep(50);
            elan_info("[elan]: %d page ack error: ack0:%x ack1:%x\n",  iPage, ack_buf[0], ack_buf[1]);

            if ( res == ACK_REWRITE ){
                rewriteCnt = rewriteCnt + 1;
                if (rewriteCnt != PAGERETRY){
                    elan_info("[elan] ---%d--- page ReWrite %d times!\n",  iPage, rewriteCnt);
                    return;
                }
                else{
                    elan_info("[elan] ---%d--- page ReWrite %d times! failed\n",  iPage, rewriteCnt);
                    goto PAGE_REWRITE;
                }
            }
            else{
                restartCnt = restartCnt + 1;
                if (restartCnt != IAPRESTART){
                    elan_info("[elan] try to ReStart %d times !\n", restartCnt);
                    return;
                }
                else{
                    elan_info("[elan] ReStart %d times fails!\n", restartCnt);
                    curIndex = 0;
                    goto IAP_RESTART;
                }
            }
        }
        else{
            elan_info("[elan]---%d--- page flash ok", iPage);
            rewriteCnt=0;
        }
    }

    elan_reset();
	elan_ts_setup(client);
    elan_switch_irq(1);
    ts->power_lock = 0;

    elan_info("[elan] Update ALL Firmware successfully!\n");

    return;
}
static void elan_ts_handler_event(
        struct elan_ts_data *ts/*, uint8_t *buf*/) {
#if defined IAP_PORTION
    int rc = 0;
    if (ts->recover == 0) {
        rc = check_update_flage(ts);
        if(rc == 1) {
            update_fw_one(ts->client);
        }
    } else {
        update_fw_one(ts->client);
    }
#endif
}
#endif
static inline int elan_ts_parse_xy(uint8_t *data, uint16_t *x, uint16_t *y)
{
    *x = *y = 0;

    *x = (data[0] & 0xf0);
    *x <<= 4;
    *x |= data[1];

    *y = (data[0] & 0x0f);
    *y <<= 8;
    *y |= data[2];

    *x=(*x*480)/phy_x_resolution;
    *y=(*y*854)/phy_y_resolution;

    return 0;
}

static int elan_ts_setup(struct i2c_client *client)
{
    int rc;

    elan_reset();
    msleep(200);

    rc = __hello_packet_handler(client);
    if (rc < 0){
        elan_info("[elan error] %s, hello_packet_handler fail, rc = %d\n", __func__, rc);
        return rc;
    }

    // for firmware update
    if(rc == 0x80){
        elan_info("[elan error] %s, fw had bening miss, rc = %d\n", __func__, rc);
        return rc;
    }

    mdelay(10);

    rc = __fw_packet_handler(client);
    if (rc < 0){
        elan_info("[elan error] %s, fw_packet_handler fail, rc = %d\n", __func__, rc);
    }

    return rc;
}

static int elan_ts_rough_calibrate(struct i2c_client *client)
{
    uint8_t cmd[] = {CMD_W_PKT, 0x29, 0x00, 0x01};
    int size = sizeof(cmd);

    if (elan_ts_send_cmd(client, cmd, size) != size)
        return -EINVAL;

    return 0;
}

static void elan_ts_touch_down(struct elan_ts_data* ts,s32 id,s32 x,s32 y,s32 w)
{
    //x = ts->x_resolution-x;
    //y = ts->y_resolution-y;
#ifdef ELAN_ICS_SLOT_REPORT
    input_mt_slot(ts->input_dev, id);
    //input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, id);
    input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, 1);
    input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);
    input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);
    input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 8);
    input_report_abs(ts->input_dev, ABS_MT_PRESSURE, 64);
    input_report_abs(ts->input_dev, ABS_MT_PRESSURE, 32);
#else
 elan_info("report BTN_TOUCH \n");
    input_report_key(ts->input_dev, BTN_TOUCH, 1);
    input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);
 elan_info("report ABS_MT_POSITION_X \n");	
    input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);
 elan_info("report ABS_MT_POSITION_Y \n");	
	
    input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, w);
 elan_info("report ABS_MT_TOUCH_MAJOR \n");	
	
    input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, w);
 elan_info("report ABS_MT_WIDTH_MAJOR \n");	
	
    input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, id);
 elan_info("report ABS_MT_TRACKING_ID \n");	
	
    input_mt_sync(ts->input_dev);
#endif
//fix bug:EBBAL-2538
	down_flag=1;
//end
    elan_info("Touch ID:%d, X:%d, Y:%d, W8:%d down", id, x, y, w);
}

static void elan_ts_touch_up(struct elan_ts_data* ts,s32 id,s32 x,s32 y)
{
#ifdef ELAN_ICS_SLOT_REPORT
    input_mt_slot(ts->input_dev, id);
    //input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, -1);
    //input_report_abs(ts->input_dev, ABS_MT_PRESSURE, 32);
    input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, 0);
    //input_report_abs(ts->input_dev, ABS_MT_PRESSURE, 0);
    elan_info("Touch id[%2d] release==!", id);
#else
    input_report_key(ts->input_dev, BTN_TOUCH, 0);
//    input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
 //   input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
    input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, -1);	
//    input_mt_sync(ts->input_dev);
    elan_info("Touch all release!");
#endif
//fix bug:EBBAL-2538
	down_flag=0;
//end
}
static void elan_ts_report_key(struct elan_ts_data *ts, uint8_t button_data)
{
    unsigned int key_value = 0;
    static unsigned int key_value_pre = 0;	
    static unsigned int x = 0, y = 0;

    switch (button_data) {
        case ELAN_KEY_BACK:
            key_value = KEY_BACK;
	     x=400;
	     y=900;
            break;
        case ELAN_KEY_HOME:
            key_value = KEY_HOMEPAGE;
	     x=240;
	     y=900;
            break;
        case ELAN_KEY_MENU:
            key_value = KEY_MENU;
	     x=80;
	     y=900;
            break;
        default:
	     key_value = 0;
	     x=80;
	     y=900;	
            break;
    }
	    if(boot_mode==FTM_MODE){
		if(key_value!=0)
		{
		     input_report_key(ts->input_dev, key_value, 1);
		     input_sync(ts->input_dev);
		}else{
		    input_report_key(ts->input_dev, key_value_pre, 0);
		    input_sync(ts->input_dev);
		}
           }else{
	           	if(key_value!=0)
			{
  				 elan_ts_touch_down(ts, VIRTUAL_TOUCH_KEY_INDEX, x, y, 60);
	           	}
	    }
           if((key_value==0))
           {
		     elan_ts_touch_up(ts, 0, 0, 0);	
	     }
	     key_value_pre= key_value;
}


#if defined ELAN_RAM_XX
static void elan_ts_iap_ram_continue(struct i2c_client *client)
{
    uint8_t cmd[] = { 0x33, 0x33, 0x33, 0x33 };
    int size = sizeof(cmd);

    elan_ts_send_cmd(client, cmd, size);
}
#endif



#ifdef ELAN_IAP_DEV
int elan_iap_open(struct inode *inode, struct file *filp)
{
    elan_info("%s enter", __func__);
    if (private_ts == NULL)
        elan_info("private_ts is NULL~~~");
    return 0;
}

int elan_iap_release(struct inode *inode, struct file *filp)
{
    elan_info("%s enter", __func__);
    return 0;
}

static ssize_t elan_iap_write(struct file *filp, const char *buff, size_t count, loff_t *offp)
{
    int ret;
    char *tmp;
    struct i2c_client *client = private_ts->client;

    elan_info("%s enter", __func__);
    if (count > 8192)
        count = 8192;

    tmp = kmalloc(count, GFP_KERNEL);
    if (tmp == NULL)
        return -ENOMEM;

    if (copy_from_user(tmp, buff, count)) {
        return -EFAULT;
    }

    ret = i2c_master_send(client, tmp, count);
    if (ret != count)
        elan_info("elan i2c_master_send fail, ret=%d \n", ret);

    kfree(tmp);

    return ret;
}

ssize_t elan_iap_read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
    char *tmp;
    int ret;
    long rc;

    struct i2c_client *client = private_ts->client;

    elan_info("%s enter", __func__);

    if (count > 8192)
        count = 8192;

    tmp = kmalloc(count, GFP_KERNEL);
    if (tmp == NULL)
        return -ENOMEM;

    ret = i2c_master_recv(client, tmp, count);
    if (ret != count)
        elan_info("elan i2c_master_recv fail, ret=%d \n", ret);

    if (ret == count)
        rc = copy_to_user(buff, tmp, count);

    kfree(tmp);
    return ret;
}

static long elan_iap_ioctl( struct file *filp, unsigned int cmd, unsigned long arg)
{
    int __user *ip = (int __user *)arg;

    elan_info("%s enter", __func__);
    elan_info("cmd value %x\n",cmd);

    switch (cmd) {
        case IOCTL_I2C_SLAVE:
            elan_info("[elan debug] pre addr is %X\n",  private_ts->client->addr);
            private_ts->client->addr = (int __user)arg;
            elan_info("[elan debug] new addr is %X\n",  private_ts->client->addr);
            return 0;
        case IOCTL_MAJOR_FW_VER:
            break;
        case IOCTL_MINOR_FW_VER:
            break;
        case IOCTL_RESET:
            elan_reset();
            break;
        case IOCTL_IAP_MODE_LOCK:
            if(private_ts->power_lock==0){
                private_ts->power_lock=1;
                elan_switch_irq(0);
            }
            break;
        case IOCTL_IAP_MODE_UNLOCK:
            if(private_ts->power_lock==1){
                private_ts->power_lock=0;
                elan_switch_irq(1);
            }
            break;
        case IOCTL_CHECK_RECOVERY_MODE:
            return private_ts->recover;;
            break;
        case IOCTL_FW_VER:
            __fw_packet_handler(private_ts->client);
            return private_ts->fw_ver;;
            break;
        case IOCTL_X_RESOLUTION:
            __fw_packet_handler(private_ts->client);
            return private_ts->x_resolution;
            break;
        case IOCTL_Y_RESOLUTION:
            __fw_packet_handler(private_ts->client);
            return private_ts->y_resolution;
            break;
        case IOCTL_FW_ID:
            __fw_packet_handler(private_ts->client);
            return private_ts->fw_id;
            break;
        case IOCTL_ROUGH_CALIBRATE:
            return elan_ts_rough_calibrate(private_ts->client);
        case IOCTL_I2C_INT:
            //put_user( tpd_flag, ip);
			put_user(gpio_get_value(intr_gpio), ip);
            break;
        case IOCTL_RESUME:
            break;
        case IOCTL_POWER_LOCK:
            private_ts->power_lock=1;
            break;
        case IOCTL_POWER_UNLOCK:
            private_ts->power_lock=0;
            break;
//add for tp fw update
	case IOCTL_HEX_FW_VER :
		return  fw_file_ver;
		break;
	case IOCTL_START_FW_UPDATE :
		if(file_fw_data!=NULL)
		{
			update_fw_one(private_ts->client);
		}		
		break;
//add end			
        default:
            break;
    }
    return 0;
}

struct file_operations elan_touch_fops = {
    .open = elan_iap_open,
    .write = elan_iap_write,
    .read = elan_iap_read,
    .release =  elan_iap_release,
    .unlocked_ioctl = elan_iap_ioctl,
 };
#endif
#ifdef ELAN_ESD_CHECK
static void elan_touch_esd_func(struct work_struct *work)
{
    int res;
    uint8_t cmd[] = {0x53, 0x00, 0x00, 0x01};
    struct i2c_client *client = private_ts->client;

    elan_info("esd %s: enter.......", __FUNCTION__);

    if(private_ts->power_lock == 1){
        goto out_esd;
    }

	if(suspend_flag==1)
	{
		goto out_esd;
		elan_info("now is in suspend ,not check esd!\n");
	}

    if(have_interrupts == 1){
        elan_info("esd %s: had interrup not need check", __func__);
    }
    else{
        res = elan_ts_send_cmd(client, cmd, sizeof(cmd));
        if (res != sizeof(cmd)){
            elan_info("[elan esd] %s: i2c_master_send failed reset now", __func__);
            //reset here
            elan_reset();
        }
        else{
            elan_info(" esd %s: i2c_master_send successful", __func__);
            msleep(20);
            if(have_interrupts == 1){
                elan_info("esd %s: i2c_master_send successful, had response", __func__);
            }
            else{
                elan_info("[elan esd] %s: i2c_master_send successful, no response need reset", __func__);
                elan_reset();
            }
        }
    }

out_esd:
    have_interrupts = 0;
    queue_delayed_work(esd_wq, &esd_work, delay);
    elan_info("[elan esd] %s: out.......", __FUNCTION__);
}
#endif


#ifdef SYS_ATTR_FILE
static ssize_t elan_debug_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    int ret = 0;
#ifdef PRINT_INT_INFO
    debug_flage = !debug_flage;
    if(debug_flage){
        elan_info("elan debug switch open\n");
	}
    else{
        elan_info("elan debug switch close\n");
    	}
#endif
    return ret;
}
static DEVICE_ATTR(debug, S_IRUGO, elan_debug_show, NULL);


static ssize_t elan_info_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    ssize_t ret = 0;
    struct elan_ts_data *ts = private_ts;
    elan_switch_irq(0);
    __fw_packet_handler(ts->client);
    elan_switch_irq(1);
    sprintf(buf, "elan fw ver:%X,id:%X,x:%d,y:%d\n", ts->fw_ver, ts->fw_id, ts->x_resolution, ts->y_resolution);
    ret = strlen(buf) + 1;
    return ret;
}
static DEVICE_ATTR(info, S_IRUGO, elan_info_show, NULL);


static ssize_t elan_rk_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    ssize_t ret = 0;
    ret = elan_ts_rough_calibrate(private_ts->client);

    return ret;
}
static DEVICE_ATTR(rk, S_IRUGO, elan_rk_show, NULL);

static ssize_t set_cmd_store(struct device *dev,
                  struct device_attribute *attr,
                  const char *buf, size_t size)
{
    char cmd[4] = {0};
    if (size > 4)
        return -EINVAL;

    if (sscanf(buf, "%02x %02x %02x %02x\n", (int *)&cmd[0], (int *)&cmd[1], (int *)&cmd[2], (int *)&cmd[3]) != 4){
        elan_info("elan cmd format error\n");
        return -EINVAL;
    }
    elan_ts_send_cmd(private_ts->client, cmd, 4);
    return size;
}
static DEVICE_ATTR(set_cmd, S_IWUSR | S_IRUGO, NULL, set_cmd_store);

unsigned char cmd_write[6]={0x97, 0x0d, 0x00, 0x00, 0x00, 0x01};
unsigned char cmd_read[6] ={0x96, 0x0d, 0x00, 0x00, 0x00, 0x01};

unsigned char cmd_close[4]={0x54, 0x50, 0x00, 0x01};
unsigned char cmd_open[4]={0x54, 0x58, 0x00, 0x01};
//static ssize_t elan_ktf2k_set_parameter(struct device *dev,
//      struct device_attribute *attr, char *buf, size_t count)
static ssize_t elan_ktf2k_set_parameter(struct device *dev,
                  struct device_attribute *attr,
                  const char *buf, size_t size)
{
    ssize_t ret = 0;
    char cmd[100];

    //struct elan_ts_data *ts = private_ts;
    disable_irq(private_ts->client->irq);
    sscanf(buf,"%X",(int *)&cmd[0]);

    elan_info("[elan] %x\n",cmd[0]);
    switch (cmd[0]){
        case 0x10:
            cmd_write[2]=0x14;
            cmd_write[3]=0x00;
            cmd_write[4]=0x15;
            i2c_master_send(private_ts->client,cmd_write,6);
            mdelay(2);
            cmd_write[0]=0x96;
            cmd_write[3]=0x00;
            cmd_write[4]=0x00;
            i2c_master_send(private_ts->client,cmd_write,6);

            mdelay(100);
            i2c_master_recv(private_ts->client,cmd_read,6);
            elan_info("[elan] write_ram %2x %2x %2x %2x %2x %2x\n",cmd_read[0],cmd_read[1],cmd_read[2],cmd_read[3],cmd_read[4],cmd_read[5]);

            if(cmd_read[3]!= 0x00 ||cmd_read[4] != 0x15){
                elan_info("[elan] write_ram_err DATA_H=%2x DATA_L=%2x\n",cmd_read[3],cmd_read[4]);
            }

            cmd_write[0]=0x97;
            cmd_write[2]=0x15;
            cmd_write[3]=0x00;
            cmd_write[4]=0x25;
            i2c_master_send(private_ts->client,cmd_write,6);

            elan_info("stylus high sensitive\n");
            enable_irq(private_ts->client->irq);
            break;

        case 0x11:
            cmd_write[2]=0x14;
            cmd_write[3]=0x00;
            cmd_write[4]=0x20;
            i2c_master_send(private_ts->client,cmd_write,6);
            mdelay(2);

            cmd_write[2]=0x15;
            cmd_write[4]=0x40;
            i2c_master_send(private_ts->client,cmd_write,6);
            mdelay(2);

            elan_info("stylus middle sensitive\n");
            enable_irq(private_ts->client->irq);
            break;

        case 0x12:
            cmd_write[2]=0x14;
            cmd_write[3]=0x00;
            cmd_write[4]=0x30;
            i2c_master_send(private_ts->client,cmd_write,6);
            mdelay(2);

            cmd_write[2]=0x15;
            cmd_write[4]=0x50;
            i2c_master_send(private_ts->client,cmd_write,6);
            mdelay(2);

            elan_info("stylus low sensitive\n");
            enable_irq(private_ts->client->irq);
            break;

        case 0x20:
            cmd_write[1]=0x0d;
            cmd_write[2]=0x48;
            cmd_write[3]=0x00;
            cmd_write[4]=0x05;
            i2c_master_send(private_ts->client,cmd_write,6);
            mdelay(2);

            cmd_write[2]=0x49;
            cmd_write[4]=0x06;
            i2c_master_send(private_ts->client, cmd_write,6);
            mdelay(2);

            cmd_write[2]=0x4a;
            cmd_write[4]=0x07;
            i2c_master_send(private_ts->client, cmd_write, 6);
            mdelay(2);

            cmd_write[2]=0x4b;
            cmd_write[4]=0x30;
            i2c_master_send(private_ts->client, cmd_write,6);
            mdelay(2);

            cmd_write[2]=0x4c;
            cmd_write[4]=0x28;
            i2c_master_send(private_ts->client, cmd_write,6);
            mdelay(2);

            cmd_write[2]=0x4f;
            cmd_write[4]=0x48;
            i2c_master_send(private_ts->client, cmd_write,6);

            elan_info("small palm\n");
            enable_irq(private_ts->client->irq);
            break;

        case 0x21:
            cmd_write[1]=0x0d;
            cmd_write[2]=0x48;
            cmd_write[3]=0x00;
            cmd_write[4]=0x06;
            i2c_master_send(private_ts->client,cmd_write,6);
            mdelay(2);

            cmd_write[2]=0x49;
            cmd_write[4]=0x08;
            i2c_master_send(private_ts->client, cmd_write,6);
            mdelay(2);

            cmd_write[2]=0x4a;
            cmd_write[4]=0x09;
            i2c_master_send(private_ts->client, cmd_write, 6);
            mdelay(2);

            cmd_write[2]=0x4b;
            cmd_write[4]=0x30;
            i2c_master_send(private_ts->client, cmd_write,6);
            mdelay(2);

            cmd_write[2]=0x4c;
            cmd_write[4]=0x48;
            i2c_master_send(private_ts->client, cmd_write,6);
            mdelay(2);

            cmd_write[2]=0x4f;
            cmd_write[4]=0x58;
            i2c_master_send(private_ts->client, cmd_write,6);
            elan_info("middle palm\n");
            enable_irq(private_ts->client->irq);
            break;

        case 0x30:
            cmd_write[2]=0x6b;
            cmd_write[3]=0x01;
            cmd_write[4]=0x00;
            i2c_master_send(private_ts->client,cmd_write,6);
            mdelay(2);

            cmd_write[2]=0x6d;
            cmd_write[3]=0x01;
            cmd_write[4]=0x20;
            i2c_master_send(private_ts->client,cmd_write,6);
            mdelay(2);

            elan_info("enable finger\n");
            enable_irq(private_ts->client->irq);
            break;

        case 0x31:
            cmd_write[2]=0x6b;
            cmd_write[3]=0x0f;
            cmd_write[4]=0xff;
            i2c_master_send(private_ts->client,cmd_write,6);
            mdelay(2);

            cmd_write[2]=0x6d;
            cmd_write[3]=0x0f;
            cmd_write[4]=0xff;
            i2c_master_send(private_ts->client,cmd_write,6);
            mdelay(2);

            elan_info("disable finger\n");
            enable_irq(private_ts->client->irq);
            break;

        case 0x38:
            cmd_write[2]=0x6b;
            cmd_write[3]=0x0f;
            cmd_write[4]=0xff;
            i2c_master_send(private_ts->client,cmd_close,4);
            mdelay(2);

            elan_info("cmd_close: %x, %x, %x, %x\n", cmd_close[0],cmd_close[1], cmd_close[2], cmd_close[3]);
            enable_irq(private_ts->client->irq);
            break;

        case 0x39:
            cmd_write[2]=0x6b;
            cmd_write[3]=0x0f;
            cmd_write[4]=0xff;
            i2c_master_send(private_ts->client,cmd_open,4);
            mdelay(2);

            elan_info("cmd_open: %x, %x, %x, %x\n", cmd_open[0],cmd_open[1], cmd_open[2], cmd_open[3]);
            enable_irq(private_ts->client->irq);
            break;
        default:
            enable_irq(private_ts->client->irq);
            break;
  }
    ret = strlen(buf) + 1;
    return ret;
}
static DEVICE_ATTR(set_parameter, S_IRWXUGO, NULL, elan_ktf2k_set_parameter);
//static DEVICE_ATTR(set_cmd, S_IRUGO , NULL, set_cmd_store);
static struct attribute *sysfs_attrs_ctrl[] = {
    &dev_attr_debug.attr,
    &dev_attr_info.attr,
    &dev_attr_rk.attr,
    &dev_attr_set_cmd.attr,
    &dev_attr_set_parameter.attr,
    NULL
};
static struct attribute_group elan_attribute_group[] = {
    {.attrs = sysfs_attrs_ctrl },
};
#endif

static void elan_touch_node_init(void)
{
    int ret ;
    struct elan_ts_data *ts = private_ts;
#ifdef SYS_ATTR_FILE
    android_touch_kobj = kobject_create_and_add("android_touch", NULL) ;
    if (android_touch_kobj == NULL) {
        elan_info(KERN_ERR "[elan]%s: kobject_create_and_add failed\n", __func__);
        return;
    }

    ret = sysfs_create_group(android_touch_kobj, elan_attribute_group);
    if (ret < 0) {
        elan_info(KERN_ERR "[elan]%s: sysfs_create_group failed\n", __func__);
    }
#endif

#ifdef ELAN_IAP_DEV
    ts->firmware.minor = MISC_DYNAMIC_MINOR;
    ts->firmware.name = "elan-iap";
    ts->firmware.fops = &elan_touch_fops;
    ts->firmware.mode = S_IFREG|S_IRWXUGO;

    if (misc_register(&ts->firmware) < 0){
        elan_info("[elan debug] misc_register failed!!\n");
		}
    else{
        elan_info("[elan debug] misc_register ok!!\n");
    	}

	proc_create("elan-iap", 0666, NULL, &elan_touch_fops);
	
#endif
    return;
}

static void elan_touch_node_deinit(void)
{
#ifdef SYS_ATTR_FILE
    sysfs_remove_group(android_touch_kobj, elan_attribute_group);
    kobject_del(android_touch_kobj);
#endif
}

static int elan_ts_recv_data(struct elan_ts_data *ts, uint8_t *buf)
{
    int rc;
    rc = i2c_master_recv(ts->client, buf, PACKET_SIZE);
    if(PACKET_SIZE != rc){
        elan_info("[elan error] elan_ts_recv_data\n");
        return -1;
    }
#ifdef PRINT_INT_INFO
    elan_info("%x %x %x %x %x %x %x %x", buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
    if(PACKET_SIZE >= 18)
        elan_info("%x %x %x %x %x %x %x %x", buf[8],buf[9],buf[10],buf[11],buf[12],buf[13],buf[14],buf[15]);
    if(PACKET_SIZE >= 34){
        elan_info("%x %x %x %x %x %x %x %x", buf[16],buf[17],buf[18],buf[19],buf[20],buf[21],buf[22],buf[23]);
        elan_info("%x %x %x %x %x %x %x %x", buf[24],buf[25],buf[26],buf[27],buf[28],buf[29],buf[30],buf[31]);
    #ifndef ELAN_BUFFER_MODE
        elan_info("%x %x %x", buf[32],buf[33], buf[34]);
    #else
        elan_info("%x %x %x %x %x %x %x %x", buf[32],buf[33],buf[34],buf[35],buf[36],buf[37],buf[38],buf[39]);
        elan_info("%x %x %x %x %x", buf[40],buf[41],buf[42],buf[43],buf[44]);
    #endif
    }
    else if(PACKET_SIZE == 18)
        elan_info("%x %x", buf[16],buf[17]);
#endif

    if(FINGERS_PKT != buf[0]){
        elan_info("[elan] other event packet:%x %x %x %x %x %x %x %x\n", buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
        return -1;
    }

    return 0;
}

static void elan_ts_report_data(struct elan_ts_data *ts, uint8_t *buf)
{
    uint16_t fbits=0;
#ifdef ELAN_ICS_SLOT_REPORT
    static uint16_t pre_fbits=0;
    uint16_t fbits_tmp=0;
#else
    int reported = 0;
#endif
    uint8_t idx;
    int finger_num;
    int num = 0;
    uint16_t x = 0;
    uint16_t y = 0;
    int position = 0;
    uint8_t button_byte = 0;

    finger_num = FINGERS_NUM;
#ifdef TWO_FINGERS
    num = buf[7] & 0x03;
    fbits = buf[7] & 0x03;
    idx=1;
    button_byte = buf[PACKET_SIZE-1];
#endif

#ifdef FIVE_FINGERS
    num = buf[1] & 0x07;
    fbits = buf[1] >>3;
    idx=2;
    button_byte = buf[PACKET_SIZE-1];
#endif

#ifdef TEN_FINGERS
    fbits = buf[2] & 0x30;
    fbits = (fbits << 4) | buf[1];
    num = buf[2] &0x0f;
    idx=3;
    button_byte = buf[PACKET_SIZE-1];
#endif

#ifdef ELAN_ICS_SLOT_REPORT
    fbits_tmp = fbits;
    if(fbits || pre_fbits){
        for(position=0; position<finger_num;position++){
            if(fbits&0x01){
                elan_ts_parse_xy(&buf[idx], &x, &y);
                elan_ts_touch_down(ts, position, x, y, 8);
            }
            else if(pre_fbits&0x01){
                elan_ts_touch_up(ts, position, x, y);
            }
            fbits >>= 1;
            pre_fbits >>= 1;
            idx += 3;
        }
    }
    else{
        elan_ts_report_key(ts, button_byte);
    }
    pre_fbits = fbits_tmp;
#else
    if (num == 0){
        elan_ts_report_key(ts, button_byte);
    }
    else{
        elan_info( "[elan] %d fingers", num);

        for(position=0; (position<finger_num) && (reported < num);position++){
            if((fbits & 0x01)){
                elan_ts_parse_xy(&buf[idx],&x, &y);
                elan_ts_touch_down(ts, position, x, y, 8);
                reported++;
            }
            fbits = fbits >> 1;
            idx += 3;
        }
    }
#endif

    input_sync(ts->input_dev);
    return;
}

static irqreturn_t elan_ts_irq_handler(int irq, void *dev_id)
{
    elan_info("----------elan_ts_irq_handler----------");
    elan_info("[elan] disable_irq_nosync\n");
    disable_irq_nosync(elan_irq);
#ifdef ELAN_ESD_CHECK
    have_interrupts = 1;
#endif
    tpd_flag = 1;
    wake_up_interruptible(&waiter);

    return IRQ_HANDLED;
}

static int elan_ts_register_interrupt(struct elan_ts_data *ts )
{
    int err = 0;
    //err = request_irq(elan_irq, elan_ts_irq_handler, IRQF_TRIGGER_FALLING, ELAN_TS_NAME, ts);
    err = request_irq(elan_irq, elan_ts_irq_handler,IRQF_TRIGGER_FALLING /*IRQF_TRIGGER_LOW*/, ELAN_TS_NAME, ts);
    if (err != 0){
        elan_info("[elan error] %s: request_irq %d failed\n",__func__, ts->client->irq);
    }
    return err;
}

#if defined IAP_PORTION
static int check_update_flage(struct elan_ts_data *ts)
{
    int NEW_FW_VERSION = 0;
    int New_FW_ID = 0;
    int rc = 0;

    if(ts->fw_ver == 0 && ts->fw_id == 0){
        elan_switch_irq(0);
        rc = __fw_packet_handler(ts->client);
        elan_switch_irq(1);
    }

#ifdef ELAN_2K_XX
#if 0
        New_FW_ID = file_fw_data[0x7DB3]<<8  | file_fw_data[0x7DB2];
        NEW_FW_VERSION = file_fw_data[0x7DB1]<<8  | file_fw_data[0x7DB0];
#else
        New_FW_ID = file_fw_data[0x7BD3]<<8 | file_fw_data[0x7BD2];
        NEW_FW_VERSION = file_fw_data[0x7BD1]<<8 | file_fw_data[0x7BD0];
#endif
#endif

#ifdef ELAN_3K_XX
    New_FW_ID  = file_fw_data[0x7D67]<<8  | file_fw_data[0x7D66];
    NEW_FW_VERSION = file_fw_data[0x7D65]<<8  | file_fw_data[0x7D64];
#endif
//add for fw update in ftm
	fw_file_ver=NEW_FW_VERSION;
//add end
    elan_info("[elan] FW_ID=0x%x, New_FW_ID=0x%x \n",ts->fw_id, New_FW_ID);
    elan_info("[elan] FW_VERSION=0x%x,New_FW_VER=0x%x \n",ts->fw_ver,NEW_FW_VERSION);

if((ts->fw_id) != (New_FW_ID)){
        elan_info("[elan] fw id is different, can not update !\n");
      //  return 0; do not check FW ID.
    }
    else{
        elan_info("[elan] fw id is same !\n");
    }

    if((ts->fw_ver) < (NEW_FW_VERSION)){
        return 1;
} else{
        elan_info("[elan] fw version is same !\n");
    }
    return 0;
}
#endif

static int touch_event_handler(void *unused)
{
    uint8_t buf[PACKET_SIZE] = {0};
#ifdef  ELAN_BUFFER_MODE
    uint8_t head[4] = {0};
    int cnt = 0;
#endif
    int rc = 0;
    struct elan_ts_data *ts = private_ts;
    struct sched_param param = { .sched_priority = 4};
    sched_setscheduler(current, SCHED_RR, &param);

    do{
        elan_info("[elan] enable_irq\n");
        enable_irq(elan_irq);
        set_current_state(TASK_INTERRUPTIBLE);
        wait_event_interruptible(waiter, tpd_flag != 0);
        tpd_flag = 0;
        set_current_state(TASK_RUNNING);
#ifdef  ELAN_BUFFER_MODE
        rc = i2c_master_recv(ts->client, head, 4);
        if(4 != rc){
            elan_info("[elan error] elan_ts_recv_data\n");
            continue;
        }
        if(head[0] == BUFF_PKT){
            cnt = head[1] & 0x03;
            while(cnt--){
                rc = elan_ts_recv_data(ts, buf);
                if(rc < 0){
                    continue;
                }
                elan_ts_report_data(ts, buf);
            }
        }
        else{
            elan_info("[elan] %02x %02x %02x %02x\n", head[0], head[1], head[2], head[3]);
        }
#else
        rc = elan_ts_recv_data(ts, buf);
        if(rc < 0){
            continue;
        }
        elan_ts_report_data(ts, buf);
#endif

    }while(!kthread_should_stop());

    return 0;
}

static int elan_request_input_dev(struct elan_ts_data *ts)
{
    int err = 0;
    ts->input_dev = input_allocate_device();
    if (ts->input_dev == NULL) {
        err = -ENOMEM;
        elan_info("[elan error] Failed to allocate input device\n");
        return err;
    }
//    ts->input_dev->evbit[0] = BIT(EV_KEY)|BIT_MASK(EV_REP);

#ifdef TPD_HAVE_BUTTON
    for (int i = 0; i < ARRAY_SIZE(button); i++){
        set_bit(button[i] & KEY_MAX, ts->input_dev->keybit);
    }
#endif
if(boot_mode==FTM_MODE){
	input_set_capability(ts->input_dev, EV_KEY, KEY_MENU);
	input_set_capability(ts->input_dev, EV_KEY, KEY_HOMEPAGE);
	input_set_capability(ts->input_dev, EV_KEY, KEY_BACK);
	}
    ts->input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
#ifdef ELAN_ICS_SLOT_REPORT
    ts->input_dev->absbit[0] = BIT(ABS_X) | BIT(ABS_Y) | BIT(ABS_PRESSURE);
   // __set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);
    input_mt_init_slots(ts->input_dev, FINGERS_NUM, 0);
#else
    ts->input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
#endif

    __set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);

    elan_info( "[elan] %s: x resolution: %d, y resolution: %d\n",__func__, ts->x_resolution, ts->y_resolution);
    input_set_abs_params(ts->input_dev,ABS_MT_POSITION_X,  0, ts->x_resolution, 0, 0);
    input_set_abs_params(ts->input_dev,ABS_MT_POSITION_Y,  0, ts->y_resolution, 0, 0);
    //input_set_abs_params(ts->input_dev,ABS_MT_POSITION_X,  0, 1344, 0, 0);
    //input_set_abs_params(ts->input_dev,ABS_MT_POSITION_Y,  0, 2240, 0, 0);

    input_set_abs_params(ts->input_dev, ABS_PRESSURE, 0, 255, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
    //input_set_abs_params(ts->input_dev, ABS_MT_PRESSURE, 0, 255, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, 255, 0, 0);
    ts->input_dev->name = "elan_ts";
    ts->input_dev->phys = "input/ts";
    ts->input_dev->id.bustype = BUS_I2C;
    ts->input_dev->id.vendor = 0xDEAD;
    ts->input_dev->id.product = 0xBEEF;
    ts->input_dev->id.version = 2013;

    err = input_register_device(ts->input_dev);
    if (err) {
        input_free_device(ts->input_dev);
        elan_info("[elan error]%s: unable to register %s input device\n", __func__, ts->input_dev->name);
        return err;
    }
    return 0;
}

static int elan_power_on(struct elan_ts_data *data, bool on)
{
    int rc = 0;
/*
    if (!on) {
        rc = regulator_disable(data->vdd);
        if (rc) {
            dev_err(&data->client->dev,
                "Regulator vdd disable failed rc=%d\n", rc);
            return rc;
        }

        rc = regulator_disable(data->vcc_i2c);
        if (rc) {
            dev_err(&data->client->dev,
                "Regulator vcc_i2c disable failed rc=%d\n", rc);
            rc = regulator_enable(data->vdd);
        }
    } else {
        rc = regulator_enable(data->vdd);
        if (rc) {
            dev_err(&data->client->dev,
                "Regulator vdd enable failed rc=%d\n", rc);
            return rc;
        }

        rc = regulator_enable(data->vcc_i2c);
        if (rc) {
            dev_err(&data->client->dev,
                "Regulator vcc_i2c enable failed rc=%d\n", rc);
           rc = regulator_disable(data->vdd);
        }

    }
*/
    return rc;

}

static void set_int_pin_state(int state)
{
      struct pinctrl_state *set_state;
      int ret=0; 
      if (IS_ERR(Touch_pinctrl))
      	{
		elan_info("gtp_parse_dt : not find pinctl!\n");
		return;
	 }
	else{
		     if(state)
			{
				set_state = pinctrl_lookup_state(Touch_pinctrl,
				"gt9xx_int_pin_active");
			}else{
				set_state = pinctrl_lookup_state(Touch_pinctrl,
				"gt9xx_int_pin_suspend");
			}
			if (IS_ERR(set_state)) {
				elan_info("cannot get  pinctrl active state\n");
				return ;
				}
		}
		ret=pinctrl_select_state(Touch_pinctrl, set_state);
		if(ret)
			{
			elan_info("set_int_pin_state ret:%d \n",ret);
		}else{
			elan_info("set_int_pin_state ok! ret:%d \n",ret);
		}
}



static int elan_power_init(struct elan_ts_data *data, bool on)
{
    int rc = 0;
/*
    if (!on) {
        if (regulator_count_voltages(data->vdd) > 0) {
            regulator_set_voltage(data->vdd, 0, VTG_MAX_UV);
        }
        regulator_put(data->vdd);

        if (regulator_count_voltages(data->vcc_i2c) > 0) {
            regulator_set_voltage(data->vcc_i2c, 0, I2C_VTG_MAX_UV);
        }
        regulator_put(data->vcc_i2c);

    } else {
        data->vdd = regulator_get(&data->client->dev, "vdd");
        if (IS_ERR(data->vdd)) {
            rc = PTR_ERR(data->vdd);
            dev_err(&data->client->dev,
                "Regulator get failed vdd rc=%d\n", rc);
            return rc;
        }

        if (regulator_count_voltages(data->vdd) > 0) {
            rc = regulator_set_voltage(data->vdd, VTG_MIN_UV,
                           VTG_MAX_UV);
            if (rc) {
                dev_err(&data->client->dev,
                    "Regulator set_vtg failed vdd rc=%d\n", rc);
                goto reg_vdd_put;
            }
        }

        data->vcc_i2c = regulator_get(&data->client->dev, "vcc_i2c");
        if (IS_ERR(data->vcc_i2c)) {
            rc = PTR_ERR(data->vcc_i2c);
            dev_err(&data->client->dev,
                "Regulator get failed vcc_i2c rc=%d\n", rc);
            goto reg_vdd_set_vtg;
        }

        if (regulator_count_voltages(data->vcc_i2c) > 0) {
            rc = regulator_set_voltage(data->vcc_i2c, I2C_VTG_MIN_UV,
                           I2C_VTG_MAX_UV);
            if (rc) {
                dev_err(&data->client->dev,
                "Regulator set_vtg failed vcc_i2c rc=%d\n", rc);
                goto reg_vcc_i2c_put;
            }
        }
    }

    return 0;

reg_vcc_i2c_put:
    regulator_put(data->vcc_i2c);
reg_vdd_set_vtg:
    if (regulator_count_voltages(data->vdd) > 0)
        regulator_set_voltage(data->vdd, 0, VTG_MAX_UV);
reg_vdd_put:
    regulator_put(data->vdd);
    return rc;
*/
return rc;
}


#ifdef CONFIG_OF
#if 0
static int elan_get_dt_coords(struct device *dev, char *name,
                struct elan_ts_i2c_platform_data *pdata)
{
    u32 coords[COORDS_ARR_SIZE];
    struct property *prop;
    struct device_node *np = dev->of_node;
    int coords_size, rc;

    prop = of_find_property(np, name, NULL);
    if (!prop)
        return -EINVAL;
    if (!prop->value)
        return -ENODATA;

    coords_size = prop->length / sizeof(u32);
    if (coords_size != COORDS_ARR_SIZE) {
        dev_err(dev, "invalid %s\n", name);
        return -EINVAL;
    }

    rc = of_property_read_u32_array(np, name, coords, coords_size);
    if (rc && (rc != -EINVAL)) {
        dev_err(dev, "Unable to read %s\n", name);
        return rc;
    }

    if (!strcmp(name, "elan,panel-coords")) {
        pdata->panel_minx = coords[0];
        pdata->panel_miny = coords[1];
        pdata->panel_maxx = coords[2];
        pdata->panel_maxy = coords[3];
    } else if (!strcmp(name, "elan,display-coords")) {
        pdata->x_min = coords[0];
        pdata->y_min = coords[1];
        pdata->x_max = coords[2];
        pdata->y_max = coords[3];
    } else {
        dev_err(dev, "unsupported property %s\n", name);
        return -EINVAL;
    }

    return 0;
}
#endif

static int elan_parse_dt(struct device *dev)
{

	struct device_node *np = dev->of_node;

	intr_gpio = of_get_named_gpio(np, "elan,irq-gpio", 0);
	reset_gpio = of_get_named_gpio(np, "elan,rst-gpio", 0);
	Touch_pinctrl= devm_pinctrl_get(dev);
	set_int_pin_state(1);
#if 0
    int rc;
    struct device_node *np = dev->of_node;
    struct property *prop;
    u32 temp_val, num_buttons;
    u32 button_map[MAX_BUTTONS];

    rc = of_property_read_string(np, "elan,name", &pdata->name);
    if (rc && (rc != -EINVAL)) {
        dev_err(dev, "Unable to read name\n");
        return rc;
    }

    rc = elan_get_dt_coords(dev, "elan,panel-coords", pdata);
    if (rc && (rc != -EINVAL))
        return rc;

    rc = elan_get_dt_coords(dev, "elan,display-coords", pdata);
    if (rc)
        return rc;

    /* reset, irq gpio info */
    pdata->rst_gpio = of_get_named_gpio_flags(np, "elan,rst-gpio",
                0, &pdata->rst_gpio_flags);
    if (pdata->rst_gpio < 0)
        return pdata->rst_gpio;

    pdata->intr_gpio = of_get_named_gpio_flags(np, "elan,intr-gpio",
                0, &pdata->intr_gpio_flags);
    if (pdata->intr_gpio < 0)
        return pdata->intr_gpio;

    prop = of_find_property(np, "elan,button-map", NULL);
    if (prop) {
        num_buttons = prop->length / sizeof(temp_val);
        if (num_buttons > MAX_BUTTONS)
            return -EINVAL;

        rc = of_property_read_u32_array(np,
            "elan,button-map", button_map,
            num_buttons);
        if (rc) {
            dev_err(dev, "Unable to read key codes\n");
            return rc;
        }
    }
#endif
    return 0;
}
#else
static int elan_parse_dt(struct device *dev)
{
    return -ENODEV;
}
#endif

static int elan_gpio_init(void)
{
    int err;

    //set reset output
    if (gpio_is_valid(reset_gpio)) {
        err = gpio_request(reset_gpio, "elan_rst_gpio");
        if (err) {
            elan_info("[elan error] reset gpio request failed\n");
            return err;
        }
        err = gpio_direction_output(reset_gpio, 1);
        if (err) {
            elan_info("[elan error] set_direction for reset gpio failed\n");
            return err;
        }
    }
    //gpio_request(pdata->rst_gpio, "tp_reset");
    //gpio_direction_output(pdata->rst_gpio, 1);

    //set int pin
    if (gpio_is_valid(intr_gpio)) {
        err = gpio_request(intr_gpio, "elan_irq_gpio");
        if (err) {
            elan_info("[elan error] irq gpio request failed\n");
            return err;
        }
        err = gpio_direction_input(intr_gpio);
        if (err) {
            elan_info("[elan error] set_direction for irq gpio failed\n");
            return err;
        }
    }
    //gpio_request(pdata->intr_gpio, "tp_irq");
    //gpio_direction_input(pdata->intr_gpio);
    return 0;
}

/**
 * elan_power_switch - power switch .
 * @on: 1-switch on, 0-switch off.
 * return: 0-succeed, -1-faileds
 */
static int elan_power_switch(struct i2c_client *client, int on)
{
	static struct regulator *vdd_ana;
	static struct regulator *vcc_i2c;
	int ret;
	
	if (!vdd_ana) {
		vdd_ana = regulator_get(&client->dev, "vdd_ana");
		if (IS_ERR(vdd_ana)) {
			elan_info("regulator get of vdd_ana failed\n");
			ret = PTR_ERR(vdd_ana);
			vdd_ana = NULL;
			return ret;
		}
	}

	if (!vcc_i2c) {
		vcc_i2c = regulator_get(&client->dev, "vcc_i2c");
		if (IS_ERR(vcc_i2c)) {
			elan_info("regulator get of vcc_i2c failed");
			ret = PTR_ERR(vcc_i2c);
			vcc_i2c = NULL;
			goto ERR_GET_VCC;
		}
	}

	if (on) {
		elan_info("elan power on.\n");
		ret = regulator_enable(vdd_ana);
		udelay(2);
		ret = regulator_enable(vcc_i2c);
	} else {
		elan_info("elan power off.\n");
		ret = regulator_disable(vcc_i2c);
		udelay(2);
		ret = regulator_disable(vdd_ana);
	}
	return ret;
	
ERR_GET_VCC:
	regulator_put(vdd_ana);
	return ret;
}

static int elan_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret = 0;
    long retval;
    struct elan_ts_data *ts;

    elan_info("[elan] %s enter i2c addr %x\n", __func__, client->addr);

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        elan_info("[elan error] %s: i2c check functionality error\n", __func__);
        return -ENODEV;
    }

    ts = devm_kzalloc(&client->dev, sizeof(struct elan_ts_data), GFP_KERNEL);
    if (ts == NULL) {
        elan_info("[elan error] %s: allocate elan_ts_data failed\n", __func__);
        return ENOMEM;
    }

    /***********platform gpio&irq**********/
    if (client->dev.of_node) {
        ret = elan_parse_dt(&client->dev);
        if (ret) {
            dev_err(&client->dev, "DT parsing failed\n");
        }
	    ret = elan_power_switch(client, 1);
		if (ret) {
			elan_info("GTP power on failed.\n");
		//	return -EINVAL;
		}

		
    } 

    ts->client = client;
	
    i2c_set_clientdata(client, ts);
    private_ts = ts;

	elan_irq   =    client->irq;

    /***********platform gpio&irq**********/
    ret = elan_gpio_init();
    if (ret) {
        dev_err(&client->dev, "gpio init failed");
        goto pwr_off;
    }
        
    ret = elan_ts_setup(client);
    if (ret < 0) {
        elan_info("[elan error]: %s No Elan chip inside, return now\n", __func__);
        ret = -ENODEV;
        goto free_platform_hw;
    }

/*    ret = elan_request_input_dev(ts);
    if (ret < 0) {
        elan_info("[elan error]: %s elan_request_input_dev\n", __func__);
        ret =  -EINVAL;
        goto free_platform_hw;
    }
*/

    ret = elan_ts_register_interrupt(ts);
    if (ret < 0) {
        elan_info("[elan error]: %s elan_ts_register_interrupt\n", __func__);
        ret =  -EINVAL;
        goto unreg_inputdev_l;
    }

    ts->work_thread = kthread_run(touch_event_handler, 0, ELAN_TS_NAME);
    if(IS_ERR(ts->work_thread)) {
        retval = PTR_ERR(ts->work_thread);
        elan_info("[elan error] failed to create kernel thread: %ld\n", retval);
        ret = -EINVAL;
        goto unreg_inputdev;
    }

#if defined(CONFIG_FB)
    ts->fb_notif.notifier_call = fb_notifier_callback;
    ret = fb_register_client(&ts->fb_notif);
    if (ret)
        dev_err(&ts->client->dev,
            "Unable to register fb_notifier: %d\n",
            ret);
#elif defined(CONFIG_HAS_EARLYSUSPEND)
    ts->early_suspend.level = EARLY_SUSPEND_LEVEL_STOP_DRAWING - 1;
    ts->early_suspend.suspend = elan_ts_early_suspend;
    ts->early_suspend.resume = elan_ts_late_resume;
    register_early_suspend(&ts->early_suspend);
#endif

#ifdef ELAN_ESD_CHECK
    INIT_DELAYED_WORK(&esd_work, elan_touch_esd_func);
    esd_wq = create_singlethread_workqueue("esd_wq");
    if (!esd_wq) {
        dev_err(&client->dev, "create workqueue esd_wq failed");
        ret = -ENOMEM;
        goto unreg_inputdev;
    }
    queue_delayed_work(esd_wq, &esd_work, delay);
#endif

    elan_touch_node_init();
#if defined IAP_PORTION
	get_vendor_info(ts);
    if (file_fw_data != NULL){
		elan_ts_handler_event(ts);
    }

#endif

    ret = elan_request_input_dev(ts);
    if (ret < 0) {
        elan_info("[elan error]: %s elan_request_input_dev\n", __func__);
        ret =  -EINVAL;
        goto unreg_inputdev;
    }

    elan_info("[elan]+++++++++end porbe+++++++++!\n");
#ifdef CONFIG_TINNO_DEV_INFO
    CAREAT_TINNO_DEV_INFO(TouchPanel);
    CAREAT_TINNO_DEV_INFO(TouchPanel_Fw_Ver);
#endif
    tpd_has_probe_flag=TPD_PROBE_MAGIC_NUM;

	mutex_init(&ts_pm_lock);

    return 0;

unreg_inputdev:
    free_irq(client->irq, ts);
unreg_inputdev_l:	
    input_unregister_device(ts->input_dev);
    ts->input_dev = NULL;
free_platform_hw:
    if (gpio_is_valid(reset_gpio))
        gpio_free(reset_gpio);

    if (gpio_is_valid(intr_gpio))
        gpio_free(intr_gpio);
pwr_off:
        elan_power_on(ts, false);

    return ret;
}

static int elan_ts_remove(struct i2c_client *client)
{
    struct elan_ts_data *ts = i2c_get_clientdata(client);

    elan_touch_node_deinit();

#if defined(CONFIG_FB)
    if (fb_unregister_client(&ts->fb_notif))
        dev_err(&client->dev,
            "Error occurred while unregistering fb_notifier.\n");
#elif defined(CONFIG_HAS_EARLYSUSPEND)
    unregister_early_suspend(&ts->early_suspend);
#endif

#ifdef ELAN_ESD_CHECK
   cancel_delayed_work(&esd_work);
   if (esd_wq)
   {
       destroy_workqueue(esd_wq);
   }
#endif

    input_unregister_device(ts->input_dev);
    ts->input_dev = NULL;

    free_irq(client->irq, ts);

    if (gpio_is_valid(ts->pdata->rst_gpio))
        gpio_free(ts->pdata->rst_gpio);

    if (gpio_is_valid(ts->pdata->intr_gpio))
        gpio_free(ts->pdata->intr_gpio);

    if (ts->pdata->power_on)
        ts->pdata->power_on(false);
    else
        elan_power_on(ts, false);

    if (ts->pdata->power_init)
        ts->pdata->power_init(false);
    else
        elan_power_init(ts, false);

    devm_kfree(&client->dev, ts->pdata);
    devm_kfree(&client->dev, ts);
    return 0;
}

#if defined(CONFIG_HAS_EARLYSUSPEND) || defined(CONFIG_FB)
static int elan_ts_set_power_state(struct i2c_client *client, int state)
{
    uint8_t cmd[] = {CMD_W_PKT, 0x50, 0x00, 0x01};
    int size = sizeof(cmd);

    cmd[1] |= (state << 3);
    if (elan_ts_send_cmd(client, cmd, size) != size)
        return -EINVAL;

    return 0;
}

static int elan_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
    struct elan_ts_data *ts = private_ts;
    int rc = 0;

    elan_info( "[elan] %s: enter\n", __func__);
	mutex_lock(&ts_pm_lock);

	if(suspend_flag==0)
	{
		suspend_flag=1;
	
		    if(ts->power_lock==0){
		        elan_switch_irq(0);
		        rc = elan_ts_set_power_state(ts->client, PWR_STATE_DEEP_SLEEP);
		    }
#ifdef ELAN_ESD_CHECK
		    cancel_delayed_work_sync(&esd_work);
#endif
		         set_int_pin_state(0); 

	}else{
		elan_info("no need suspend!\n");
	}
    mutex_unlock(&ts_pm_lock);	
    return rc;
}

static int elan_ts_resume(struct i2c_client *client)
{
    struct elan_ts_data *ts = private_ts;
    int rc = 0;
  // char buf[8]; 
    elan_info("[elan] %s: enter\n", __func__);
  //fix bug:EBBAL-2538
	if(down_flag)
	{
		printk("elan_ts_resume up point!\n");
		elan_ts_touch_up(ts,0,0,0);
		input_sync(ts->input_dev);
	}
//end
  mutex_lock(&ts_pm_lock);
           if(suspend_flag==1)
           {
           		suspend_flag=0;
		    if(ts->power_lock==0){
		         elan_info("[elan] reset gpio to resum tp\n");
			  set_int_pin_state(1); 
		         elan_reset();
			  elan_switch_irq(1);		
#if 0 
		//add by alik		
			msleep(150); 
			rc = i2c_master_recv(ts->client, buf, 8); 
			if(8 != rc){ 
			elan_info("[elan error] elan_ts_resume hello error\n"); 
			} 
			else{ 
			elan_info("elan resum %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
			} 
		//add end	
		   #endif   
		    }
#ifdef ELAN_ESD_CHECK
		    queue_delayed_work(esd_wq, &esd_work, delay);
#endif
		}else{
		elan_info("no need resume!\n");
}
mutex_unlock(&ts_pm_lock);
    return rc;
}
#endif

#if defined(CONFIG_FB)
static int fb_notifier_callback(struct notifier_block *self,
                 unsigned long event, void *data)
{
    struct fb_event *evdata = data;
    int *blank;
    struct elan_ts_data *ts =
        container_of(self, struct elan_ts_data, fb_notif);

    if (evdata && evdata->data && event == FB_EVENT_BLANK &&
            ts && ts->client) {
        blank = evdata->data;
        if (*blank == FB_BLANK_UNBLANK)
            elan_ts_resume(ts->client);
        else if (*blank == FB_BLANK_POWERDOWN)
            elan_ts_suspend(ts->client, PMSG_SUSPEND);
    }
    return 0;
}
#elif defined(CONFIG_HAS_EARLYSUSPEND)
static void elan_ts_early_suspend(struct early_suspend *h)
{
    struct elan_ts_data *ts =  private_ts;
    elan_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void elan_ts_late_resume(struct early_suspend *h)
{
    struct elan_ts_data *ts =  private_ts;
    elan_ts_resume(ts->client);
}
#endif

#ifdef CONFIG_OF
static struct of_device_id elan_match_table[] = {
    { .compatible = "elan,ts",},
    { },
};
#else
#define elan_match_table NULL
#endif

static struct i2c_driver elan_ts_driver = {
    .class = I2C_CLASS_HWMON,
    .probe = elan_ts_probe,
    .remove = elan_ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
    .suspend = elan_ts_suspend,
    .resume = elan_ts_resume,
#endif
    .id_table = elan_ts_id,
    .driver = {
        .name           = ELAN_TS_NAME,
        .of_match_table = elan_match_table,
        .owner          = THIS_MODULE,
    },
};


extern char *saved_command_line;
#define CHARGER_MODE_BOOT   "androidboot.mode=charger"
#define FTM_MODE_BOOT            "androidboot.mode=ffbm-01"
static int __init elan_ts_init(void)
{
    int ret = -1;
    if(strstr(saved_command_line,CHARGER_MODE_BOOT))
    {
	    elan_info("elan driver not installing in charger mode! \n");
		return -1;
	}	

    if(strstr(saved_command_line,FTM_MODE_BOOT))
    {
    		 elan_info("in ftm mode! \n");
			boot_mode=FTM_MODE;
    }else{
			boot_mode=0;
	}
	
	if(tpd_has_probe_flag==TPD_PROBE_MAGIC_NUM)
	{
		elan_info("GTP driver has probe ok!! \n");
		return -1;
	}
    elan_info("[elan] %s driver 004 version : auto-mapping resolution\n", __func__);
    ret = i2c_add_driver(&elan_ts_driver);
    elan_info("[elan]: %s add do i2c_add_driver and the return value=%d\n",__func__,ret);
    return ret;
}

static void __exit elan_ts_exit(void)
{
    elan_info("[elan]: %s remove driver\n", __func__);
    i2c_del_driver(&elan_ts_driver);
    return;
}

module_init(elan_ts_init);
module_exit(elan_ts_exit);

MODULE_DESCRIPTION("elan KTF2K Touchscreen Driver");
MODULE_LICENSE("GPL");
