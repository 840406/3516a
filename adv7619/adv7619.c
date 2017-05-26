

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
//#include <linux/smp_lock.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/system.h>

#include <linux/delay.h>

#include <linux/proc_fs.h>
#include <linux/poll.h>

#include <mach/hardware.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/irq.h>

#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/reboot.h>
#include <linux/notifier.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/miscdevice.h>

#define dbg_print(fmt,args...)     printk("[%s:%d]: "fmt,__func__, __LINE__, ##args)


#define DEV_NAME "adv7619"
#define MISC_DYNAMIC_MINOR	255

#define IO_I2C_addr		0x98	/* same of the ADV7619_I2C */
#define CEC_I2C_addr		0x80
#define INFO_I2C_addr		0x7c
#define DPLL_I2C_addr		0x4c
#define KSV_I2C_addr		0x64
#define EDID_I2C_addr		0x6c
#define HDMI_I2C_addr		0x68
#define CP_I2C_addr		0x44


#define    CEC_I2C_Programmed_addr	0xF4
#define 	INFO_I2C_Programmed_addr 0xF5
#define 	DPLL_I2C_Programmed_addr  0xF8
#define 	KSV_I2C_Programmed_addr  0xF9
#define 	EDID_I2C_Programmed_addr 0xFA
#define 	HDMI_I2C_Programmed_addr 0xFB
#define 	CP_I2C_Programmed_addr    0xFD

#define      VOL	 2
#define    IO_RAW     11  
#define    CP_RAW      2   //  1//  
#define    KSV_RAW     3 
#define    DPLL_RAW     1 
#define    HDMI_RAW     27 

#define  EDID_LEN  256

unsigned char io_addr_val[IO_RAW][VOL] = {
	{0x0,  0x1e},
	{0x01, 0x15},   // Prim_Mode =110b HDMI-CP
	{0x02, 0xF1},   // Auto CSC, YCrCb out, Set op_656 bit
	{0x03, 0x80},   // 24 bit SDR 422 Mode 0
	{0x05, 0x28},   // AV Codes Off
	{0x06, 0xA6},   // Invert VS,HS pins
	{0x0C, 0x42},   // Power up part
	{0x15, 0x80},   // Disable Tristate of Pins
	{0x19, 0xc3},   // LLC DLL phase
	{0x20, 0xb0},	//0 V applied to HPA_B pin
	{0x33, 0x40},   // LLC DLL MUX enable
}; 

unsigned char cp_addr_val[CP_RAW][VOL] = {
	{0xBA, 0x01},   // Set HDMI FreeRun
	{0x6C, 0x00},   // Required ADI write
}; 

unsigned char ksv_addr_val[KSV_RAW][VOL] = {
	{0x40, 0x81},   // Disable HDCP 1.1 features
	{0x74, 0x02},	//EDID_B_ENABLE
	{0x70, 0x80},	//SPA_LOCATION[7:0]
}; 

unsigned char dpll_addr_val[DPLL_RAW][VOL] = {
	{0xB5, 0x01},   // Setting MCLK to 256Fs
}; 

unsigned char hdmi_addr_val[HDMI_RAW][VOL] = {
	{0x00, 0x01},   // Set HDMI Input Port A (BG_MEAS_PORT_SEL = 001b)
	{0x02, 0x03},   // ALL BG Ports enabled
	{0x03, 0x98},   // ADI Required Write
	{0x10, 0xA5},   // ADI Required Write
	{0x1B, 0x08},   // ADI Required Write
	{0x45, 0x04},   // ADI Required Write
	{0x97, 0xC0},   // ADI Required Write
	{0x3D, 0x10},   // ADI Required Write
	{0x3E, 0x69},   // ADI Required Write
	{0x3F, 0x46},   // ADI Required Write
	{0x4E, 0xFE},   // ADI Required Write 
	{0x4F, 0x08},   // ADI Required Write
	{0x50, 0x00},   // ADI Required Write
	{0x57, 0xA3},   // ADI Required Write
	{0x58, 0x07},   // ADI Required Write
	{0x6F, 0x08},   // ADI Required Write
	{0x83, 0xFC},   // Enable clock terminators for port A & B
	{0x84, 0x03},   // ADI Required Write
	{0x85, 0x10},   // ADI Required Write
	{0x86, 0x9B},   // ADI Required Write 
	{0x89, 0x03},   // ADI Required Write 
	{0x9B, 0x03},   // ADI Required Write
	{0x93, 0x03},   // ADI Required Write
	{0x5A, 0x80},   // ADI Required Write
	{0x9C, 0x80},   // ADI Required Write
	{0x9C, 0xC0},   // ADI Required Write
	{0x9C, 0x00},   // ADI Required Write
}; 

unsigned char hdmi_addr_val0[3][VOL] = {
	{0x00, 0x01},
	{0x02, 0x03},
	{0x83, 0xFC},
	
};

unsigned char edid[EDID_LEN] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x51, 0x20, 0x68, 0x81, 0x01, 0x00, 0x00, 0x00,
	0x01, 0x16, 0x01, 0x03, 0x80, 0x10, 0x09, 0x78, 0x2a, 0x60, 0x41, 0xa6, 0x56, 0x4a, 0x9c, 0x25,
	0x12, 0x50, 0x54, 0x21, 0x08, 0x00, 0xd1, 0x00, 0xa9, 0x40, 0x90, 0x40, 0x81, 0x80, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x0e, 0x1f, 0x00, 0x80, 0x51, 0x00, 0x1e, 0x30, 0x40, 0x80,
	0x37, 0x00, 0xa0, 0x5a, 0x00, 0x00, 0x00, 0x18, 0x66, 0x21, 0x50, 0xb0, 0x51, 0x00, 0x1b, 0x30,
	0x40, 0x70, 0x36, 0x00, 0xa0, 0x5a, 0x00, 0x00, 0x00, 0x1e, 0x28, 0x3c, 0x80, 0xa0, 0x70, 0xb0,
	0x2d, 0x40, 0x88, 0xc8, 0x36, 0x00, 0xa0, 0x64, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0a,
	0x02, 0x03, 0x1a, 0x72, 0x47, 0x84, 0x02, 0x03, 0x05, 0x01, 0x06, 0x07, 0x23, 0x09, 0x07, 0x01, 
	0x83, 0x01, 0x00, 0x00, 0x65, 0x03, 0x0c, 0x00, 0x10, 0x00, 0x01, 0x1d, 0x80, 0x18, 0x71, 0x1c, 
	0x16, 0x20, 0x58, 0x2c, 0x25, 0x00, 0xfe, 0x22, 0x11, 0x00, 0x00, 0x9e, 0xd6, 0x09, 0x80, 0xa0, 
	0x20, 0xe0, 0x2d, 0x10, 0x10, 0x60, 0xa2, 0x00, 0x82, 0x22, 0x11, 0x00, 0x00, 0x18, 0x8c, 0x0a, 
	0xd0, 0x8a, 0x20, 0xe0, 0x2d, 0x10, 0x10, 0x3e, 0x96, 0x00, 0xfe, 0x22, 0x11, 0x00, 0x00, 0x18, 
	0x8c, 0x0a, 0xa0, 0x14, 0x51, 0xf0, 0x16, 0x00, 0x26, 0x7c, 0x43, 0x00, 0x82, 0x22, 0x11, 0x00, 
	0x00, 0x98, 0x8c, 0x0a, 0xa0, 0x14, 0x51, 0xf0, 0x16, 0x00, 0x26, 0x7c, 0x43, 0x00, 0xfe, 0x22, 
	0x11, 0x00, 0x00, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc8

};

static int adv7619_open(struct inode* inode, struct file* file)
{
	return 0;
}

static int adv7619_close(struct inode* inode , struct file* file)
{
    return 0;
}

static struct file_operations adv7619_fops =
{
    .owner		= THIS_MODULE,
   // .unlocked_ioctl		= tlv320aic31_ioctl,
    .open		= adv7619_open,
    .release	= adv7619_close
};

static struct miscdevice adv7619_dev =
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME,
    .fops = &adv7619_fops,
};


static struct i2c_board_info hi_infos[8] =
{
	 [0] = { 
			I2C_BOARD_INFO("adv7619_io", IO_I2C_addr),
		 },
	 [1] = { 
			I2C_BOARD_INFO("adv7619_cec", CEC_I2C_addr),
	 	},
	 [2] = { 
			I2C_BOARD_INFO("adv7619_info", INFO_I2C_addr),
		 },
	 [3] = { 
			I2C_BOARD_INFO("adv7619_dpll", DPLL_I2C_addr),
		 },
	 [4] = { 
			I2C_BOARD_INFO("adv7619_ksv", KSV_I2C_addr),
		 },
	 [5] = { 
			I2C_BOARD_INFO("adv7619_edid", EDID_I2C_addr),
		 },
	 [6] = { 
			I2C_BOARD_INFO("adv7619_hdmi", HDMI_I2C_addr),
	 	},
	 [7] = { 
			I2C_BOARD_INFO("adv7619_cp", CP_I2C_addr),
	 	},
	
};

static struct i2c_client* pclient_io;
static struct i2c_client* pclient_cec;
static struct i2c_client* pclient_info;
static struct i2c_client* pclient_dpll;
static struct i2c_client* pclient_ksv;
static struct i2c_client* pclient_edid;
static struct i2c_client* pclient_hdmi;
static struct i2c_client* pclient_cp;


static int i2c_client_init(void)
{
	struct i2c_adapter* i2c_adap;

	i2c_adap = i2c_get_adapter(2);
	pclient_io = i2c_new_device(i2c_adap, &hi_infos[0]);
	pclient_cec = i2c_new_device(i2c_adap, &hi_infos[1]);
	pclient_info = i2c_new_device(i2c_adap, &hi_infos[2]);
	pclient_dpll = i2c_new_device(i2c_adap, &hi_infos[3]); 
	pclient_ksv = i2c_new_device(i2c_adap, &hi_infos[4]);
	pclient_edid = i2c_new_device(i2c_adap, &hi_infos[5]);
	pclient_hdmi = i2c_new_device(i2c_adap, &hi_infos[6]);
	pclient_cp = i2c_new_device(i2c_adap, &hi_infos[7]);
	
	i2c_put_adapter(i2c_adap);

    return 0;
}

static char adv7619_i2c_read(struct i2c_client* pclient, char addr)
{
	char ret = 0;
	char rcvbuf = addr;

	if(pclient == NULL)
	{
		return -EINVAL;
	}

	ret = i2c_master_recv(pclient, &rcvbuf, 1);
	if (ret >= 0)
	{
	    	return rcvbuf;
	}
	else
	{
		return ret;
	}	
}

static int adv7619_i2c_write(struct i2c_client* pclient, unsigned char addr, unsigned char value)
{
	int ret = 0;
	char sndbuf[2] = {};

	if(pclient == NULL)
	{
		return  -EINVAL;
	}
	
	sndbuf[0] = addr;
	sndbuf[1] = value;

	ret = i2c_master_send(pclient, sndbuf, 2);	
	
	return  ret;	
}

static int config_devices_addr(void)
{
	int ret = 0;
	
	ret = adv7619_i2c_write(pclient_io, CEC_I2C_Programmed_addr, CEC_I2C_addr);	
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(5);

	ret = adv7619_i2c_write(pclient_io, INFO_I2C_Programmed_addr, INFO_I2C_addr);	
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(5);

	ret = adv7619_i2c_write(pclient_io, DPLL_I2C_Programmed_addr, DPLL_I2C_addr);	
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(5);

	ret = adv7619_i2c_write(pclient_io, KSV_I2C_Programmed_addr, KSV_I2C_addr);	
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(5);

	ret = adv7619_i2c_write(pclient_io, EDID_I2C_Programmed_addr, EDID_I2C_addr);	
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(5);

	ret = adv7619_i2c_write(pclient_io, HDMI_I2C_Programmed_addr, HDMI_I2C_addr);	


if(ret < 0)
{
	dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
}
msleep(1);



	ret = adv7619_i2c_write(pclient_io, CP_I2C_Programmed_addr, CP_I2C_addr);	
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(5);

	return ret;
}

static int adv7619_reset(void)
{
	int ret = 0;
	ret = adv7619_i2c_write(pclient_io, 0xFF, 0x80);
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}

	return ret;
}

static int  config_reg(struct i2c_client* pclient, unsigned char (*paddr_val)[VOL] )
{
	int i = 0;
	int num = 0;
	int ret = 0;
	int  raw = 0;
	

	if( pclient_io == pclient)
	{
		raw = IO_RAW;
		num = 1;
	}
	else if( pclient_cp == pclient)
	{
		raw = CP_RAW;
		num = 2;
	}
	else if( pclient_ksv == pclient)
	{
		raw = KSV_RAW;
		num = 3;
	}
	else if( pclient_dpll == pclient)
	{
		raw = DPLL_RAW;	
		num = 4;
	}
	else if( pclient_hdmi == pclient)
	{
		raw =  HDMI_RAW;     //  	3;//
		num = 5;
	}
		
	for(i=0; i<raw; i++)
	{
		ret = adv7619_i2c_write(pclient, paddr_val[i][0],  paddr_val[i][1]); 
		if(ret < 0)
		{
			dbg_print("adv7619_i2c_write pclient_io is failed, errornum == %d, num =%d\n", ret, num);
			goto EXIT;
		}
		msleep(5);
	}
	
EXIT:
	return ret;
	
}

static int config_edid(void)
{
	int i;
	int ret;
	for(i=0; i< EDID_LEN; i++)
	{
		ret = adv7619_i2c_write(pclient_edid, i, edid[i]); 
		if(ret < 0)
		{
			dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
			return ret;
		}
	}

	return ret;
}

static void i2c_client_exit(void)
{
	i2c_unregister_device(pclient_io);
	i2c_unregister_device(pclient_cec);
	i2c_unregister_device(pclient_info);
	i2c_unregister_device(pclient_dpll);
	i2c_unregister_device(pclient_ksv);
	i2c_unregister_device(pclient_edid);
	i2c_unregister_device(pclient_hdmi);
	i2c_unregister_device(pclient_cp);
}

static int adv7619_config_device(void)
{
	int ret = 0;

	ret = adv7619_i2c_write(pclient_hdmi, 0xC0, 0x03) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_io, 0x01, 0x06) ; // Prim_Mode =110b HDMI-GR
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_io, 0x02, 0xFD) ; // Auto CSC, YCrCb out, Set op_656 bit  //by cuibo
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_io, 0x03, 0x80) ; // 16 bit SDR 422 Mode 0
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_io, 0x05, 0x28) ; // AV Codes Off
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_io, 0x06, 0xA0) ; //  VS,HS pins
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_io, 0x0C, 0x42) ; // Power up part
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_io, 0x15, 0x80) ; // Disable Tristate of Pins
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_io, 0x19, 0x83) ; // LLC DLL phase
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_io, 0x33, 0x40) ; // LLC DLL MUX enable
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_cp, 0xBA, 0x01) ; // Set HDMI FreeRun
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_cp, 0x6C, 0x00) ; // Required ADI write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_ksv, 0x40, 0x81) ; // Disable HDCP 1.1 features
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_dpll, 0xB5, 0x01) ; // Setting MCLK to 256Fs
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0xC0, 0x03) ; // ADI Required write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x00, 0x01) ; // Set HDMI Input Port B (BG_MEAS_PORT_SEL = 000a)
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x02, 0x03) ; // ALL BG Ports enabled
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x03, 0x98) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x10, 0xA5) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x1B, 0x08) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x45, 0x04) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x97, 0xC0) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x3D, 0x10) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x3E, 0x69) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x3F, 0x46) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x4E, 0xFE) ; // ADI Required Write 
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x4F, 0x08) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x50, 0x00) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x57, 0xA3) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x58, 0x07) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x6F, 0x08) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x83, 0xFC) ; // Enable clock terminators for port A & B
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x84, 0x03) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x85, 0x10) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x86, 0x9B) ; // ADI Required Write 
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x89, 0x03) ; // ADI Required Write 
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x9B, 0x03) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x93, 0x03) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x5A, 0x80) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x9C, 0x80) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x9C, 0xC0) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);

	ret = adv7619_i2c_write(pclient_hdmi, 0x9C, 0x00) ; // ADI Required Write
	if(ret < 0)
	{
		dbg_print("adv7619_i2c_write is failed, errornum == %d\n", ret);
	}
	msleep(1);
	
		
	return ret;
}

static int adv7619_device_init(void)
{
	int ret = 0;

	ret =adv7619_config_device();
	if(ret < 0)
	{
		return ret;
	}
	
/*
	ret = config_edid();
	if(ret < 0)
	{
		return ret;
	}
	
*/
	
	return ret;
}


static int  adv7619_init(void)
{
	int  ret = 0;
	
	ret = misc_register(&adv7619_dev);
	if (ret)
	{
		dbg_print( "could not register adv7619 device");
		return -1;
	}

    	i2c_client_init();
 	dbg_print( "adv7619 i2c_client_init successful!\n");

	ret = config_devices_addr();
	if(ret < 0)
	{
		goto init_fail;
	}	

	ret = adv7619_device_init();
	if(ret < 0)
	{
		goto init_fail;
	}

	
	dbg_print( "adv7619 driver init successful!\n");
	dbg_print("11refrttrery11\n");

    return 0;
	
init_fail:

	i2c_client_exit();
    misc_deregister(&adv7619_dev);

    dbg_print( "adv7619 device init fail,deregister it!");
    return -1;
}



static void  adv7619_exit(void)
{
    
    misc_deregister(&adv7619_dev);

    i2c_client_exit();

    printk("rmmod adv7619.ko for Hi3516A ok!\n");


}

module_init(adv7619_init);
module_exit(adv7619_exit);
MODULE_LICENSE("GPL");
