#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/upmu_common.h>
	#include <platform/mt_gpio.h>
	#include <platform/mt_i2c.h> 
	#include <platform/mt_pmic.h>
	#include <string.h>
#else
	#include <mach/mt_pm_ldo.h>
    #include <mach/mt_gpio.h>
#endif
#include <cust_gpio_usage.h>
#include <cust_i2c.h>
#ifdef BUILD_LK
#define LCD_DEBUG(fmt)  dprintf(CRITICAL,fmt)
#else
#define LCD_DEBUG(fmt)  printk(fmt)
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define FRAME_WIDTH  (1080)
#define FRAME_HEIGHT (1920)

#define LCM_ID_nt35596 (0x9006)

//#define GPIO_SGM3804_ENN   GPIO_LCD_BIAS_ENN_PIN
//#define GPIO_SGM3804_ENP   GPIO_LCD_BIAS_ENP_PIN
#define GPIO_SGM3804_ENN   (GPIO131|0x80000000)
#define GPIO_SGM3804_ENP   (GPIO9  |0x80000000)


// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)				lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)												lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)							lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)												lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   					lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)   


#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>  
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
//#include <linux/jiffies.h>
#include <linux/uaccess.h>
//#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#endif


//#define LCM_DSI_CMD_MODE 1
#define LCM_DSI_CMD_MODE 0


#ifdef BUILD_LK

#define I2C_I2C_LCD_BIAS_CHANNEL  2
#define sgm3804_SLAVE_ADDR_WRITE  0x7C  
static struct mt_i2c_t sgm3804_i2c;

static int sgm3804_write_byte(kal_uint8 addr, kal_uint8 value)
{
    kal_uint32 ret_code = I2C_OK;
    kal_uint8 write_data[2];
    kal_uint16 len;

    write_data[0]= addr;
    write_data[1] = value;

    sgm3804_i2c.id = I2C_I2C_LCD_BIAS_CHANNEL;//I2C2;
    /* Since i2c will left shift 1 bit, we need to set FAN5405 I2C address to >>1 */
    sgm3804_i2c.addr = (sgm3804_SLAVE_ADDR_WRITE >> 1);
    sgm3804_i2c.mode = ST_MODE;
    sgm3804_i2c.speed = 100;
    len = 2;

    ret_code = i2c_write(&sgm3804_i2c, write_data, len);
    printk("%s: i2c_write: ret_code: %d\n", __func__, ret_code);

    return ret_code;
}

#else

	extern int sgm3804_write_bytes(unsigned char addr, unsigned char value);

#endif

static char reg_init[][2] =
{
0xFF,0x20,
0xFB,0x01,                    
0x00,0x01,
0x01,0x55,
0x02,0x40,
0x05,0x50,  
0x06,0x19,
0x07,0x23,
0x08,0x0C,           
//0x11,0x19,
//0x12,0x19,
//0x13,0x03,          
0x14,0x4A,   
0x15,0x12,
0x16,0x12,   
0x0B,0xAF, 
0x0C,0xAF, 
0x0E,0xB0,//AB
0x0F,0xAE,//A4
0x58,0x02,
0x59,0x00,
0x5A,0x00,
0x5B,0x00,
0x5C,0x02,
0x5D,0x00,
0x5E,0x00,
0x5F,0x00,
0x30,0x0F,
//0x44,0x12,
//0x45,0x34,
//0x46,0x56,                    
0x75,0x00,
0x76,0x00,
0x77,0x00,
0x78,0x12,
0x79,0x00,
0x7A,0x34,
0x7B,0x00,
0x7C,0x53,
0x7D,0x00,
0x7E,0x6A,
0x7F,0x00,
0x80,0x82,
0x81,0x00,
0x82,0x9A,
0x83,0x00,
0x84,0xAB,
0x85,0x00,
0x86,0xBC,
0x87,0x00,
0x88,0xED,
0x89,0x01,
0x8A,0x19,
0x8B,0x01,
0x8C,0x58,
0x8D,0x01,
0x8E,0x8A,
0x8F,0x01,
0x90,0xCF,
0x91,0x02,
0x92,0x0C,
0x93,0x02,
0x94,0x0E,
0x95,0x02,
0x96,0x42,
0x97,0x02,
0x98,0x85,
0x99,0x02,
0x9A,0xB0,
0x9B,0x02,
0x9C,0xE3,
0x9D,0x03,
0x9E,0x05,
0x9F,0x03,
0xA0,0x36,
0xA2,0x03,
0xA3,0x45,
0xA4,0x03,
0xA5,0x54,
0xA6,0x03,
0xA7,0x65,
0xA9,0x03,
0xAA,0x78,
0xAB,0x03,
0xAC,0x8F,
0xAD,0x03,
0xAE,0xB5,
0xAF,0x03,
0xB0,0xE5,
0xB1,0x03,
0xB2,0xFF,                  
0xB3,0x00,
0xB4,0x00,
0xB5,0x00,
0xB6,0x12,
0xB7,0x00,
0xB8,0x34,
0xB9,0x00,
0xBA,0x53,
0xBB,0x00,
0xBC,0x6A,
0xBD,0x00,
0xBE,0x82,
0xBF,0x00,
0xC0,0x9A,
0xC1,0x00,
0xC2,0xAB,
0xC3,0x00,
0xC4,0xBC,
0xC5,0x00,
0xC6,0xED,
0xC7,0x01,
0xC8,0x19,
0xC9,0x01,
0xCA,0x58,
0xCB,0x01,
0xCC,0x8A,
0xCD,0x01,
0xCE,0xCF,
0xCF,0x02,
0xD0,0x0C,
0xD1,0x02,
0xD2,0x0E,
0xD3,0x02,
0xD4,0x42,
0xD5,0x02,
0xD6,0x85,
0xD7,0x02,
0xD8,0xB0,
0xD9,0x02,
0xDA,0xE3,
0xDB,0x03,
0xDC,0x05,
0xDD,0x03,
0xDE,0x36,
0xDF,0x03,
0xE0,0x45,
0xE1,0x03,
0xE2,0x54,
0xE3,0x03,
0xE4,0x65,
0xE5,0x03,
0xE6,0x78,
0xE7,0x03,
0xE8,0x8F,
0xE9,0x03,
0xEA,0xB5,
0xEB,0x03,
0xEC,0xE5,
0xED,0x03,
0xEE,0xFF,
0xEF,0x00,
0xF0,0x00,
0xF1,0x00,
0xF2,0x12,
0xF3,0x00,
0xF4,0x34,
0xF5,0x00,
0xF6,0x53,
0xF7,0x00,
0xF8,0x6A,
0xF9,0x00,
0xFA,0x82,
0xFF,0x21,
0xFB,0x01,
0x00,0x00,
0x01,0x9A,
0x02,0x00,
0x03,0xAB,
0x04,0x00,
0x05,0xBC,
0x06,0x00,
0x07,0xED,
0x08,0x01,
0x09,0x19,
0x0A,0x01,
0x0B,0x58,
0x0C,0x01,
0x0D,0x8A,
0x0E,0x01,
0x0F,0xCF,
0x10,0x02,
0x11,0x0C,
0x12,0x02,
0x13,0x0E,
0x14,0x02,
0x15,0x42,
0x16,0x02,
0x17,0x85,
0x18,0x02,
0x19,0xB0,
0x1A,0x02,
0x1B,0xE3,
0x1C,0x03,
0x1D,0x05,
0x1E,0x03,
0x1F,0x36,
0x20,0x03,
0x21,0x45,
0x22,0x03,
0x23,0x54,
0x24,0x03,
0x25,0x65,
0x26,0x03,
0x27,0x78,
0x28,0x03,
0x29,0x8F,
0x2A,0x03,
0x2B,0xB5,
0x2D,0x03,
0x2F,0xE5,
0x30,0x03,
0x31,0xFF,                    
0x32,0x00,
0x33,0x00,
0x34,0x00,
0x35,0x12,
0x36,0x00,
0x37,0x34,
0x38,0x00,
0x39,0x53,
0x3A,0x00,
0x3B,0x6A,
0x3D,0x00,
0x3F,0x82,
0x40,0x00,
0x41,0x9A,
0x42,0x00,
0x43,0xAB,
0x44,0x00,
0x45,0xBC,
0x46,0x00,
0x47,0xED,
0x48,0x01,
0x49,0x19,
0x4A,0x01,
0x4B,0x58,
0x4C,0x01,
0x4D,0x8A,
0x4E,0x01,
0x4F,0xCF,
0x50,0x02,
0x51,0x0C,
0x52,0x02,
0x53,0x0E,
0x54,0x02,
0x55,0x42,
0x56,0x02,
0x58,0x85,
0x59,0x02,
0x5A,0xB0,
0x5B,0x02,
0x5C,0xE3,
0x5D,0x03,
0x5E,0x05,
0x5F,0x03,
0x60,0x36,
0x61,0x03,
0x62,0x45,
0x63,0x03,
0x64,0x54,
0x65,0x03,
0x66,0x65,
0x67,0x03,
0x68,0x78,
0x69,0x03,
0x6A,0x8F,
0x6B,0x03,
0x6C,0xB5,
0x6D,0x03,
0x6E,0xE5,
0x6F,0x03,
0x70,0xFF,                  
0x71,0x00,
0x72,0x00,
0x73,0x00,
0x74,0x12,
0x75,0x00,
0x76,0x34,
0x77,0x00,
0x78,0x53,
0x79,0x00,
0x7A,0x6A,
0x7B,0x00,
0x7C,0x82,
0x7D,0x00,
0x7E,0x9A,
0x7F,0x00,
0x80,0xAB,
0x81,0x00,
0x82,0xBC,
0x83,0x00,
0x84,0xED,
0x85,0x01,
0x86,0x19,
0x87,0x01,
0x88,0x58,
0x89,0x01,
0x8A,0x8A,
0x8B,0x01,
0x8C,0xCF,
0x8D,0x02,
0x8E,0x0C,
0x8F,0x02,
0x90,0x0E,
0x91,0x02,
0x92,0x42,
0x93,0x02,
0x94,0x85,
0x95,0x02,
0x96,0xB0,
0x97,0x02,
0x98,0xE3,
0x99,0x03,
0x9A,0x05,
0x9B,0x03,
0x9C,0x36,
0x9D,0x03,
0x9E,0x45,
0x9F,0x03,
0xA0,0x54,
0xA2,0x03,
0xA3,0x65,
0xA4,0x03,
0xA5,0x78,
0xA6,0x03,
0xA7,0x8F,
0xA9,0x03,
0xAA,0xB5,
0xAB,0x03,
0xAC,0xE5,
0xAD,0x03,
0xAE,0xFF,            
0xAF,0x00,
0xB0,0x00,
0xB1,0x00,
0xB2,0x12,
0xB3,0x00,
0xB4,0x34,
0xB5,0x00,
0xB6,0x53,
0xB7,0x00,
0xB8,0x6A,
0xB9,0x00,
0xBA,0x82,
0xBB,0x00,
0xBC,0x9A,
0xBD,0x00,
0xBE,0xAB,
0xBF,0x00,
0xC0,0xBC,
0xC1,0x00,
0xC2,0xED,
0xC3,0x01,
0xC4,0x19,
0xC5,0x01,
0xC6,0x58,
0xC7,0x01,
0xC8,0x8A,
0xC9,0x01,
0xCA,0xCF,
0xCB,0x02,
0xCC,0x0C,
0xCD,0x02,
0xCE,0x0E,
0xCF,0x02,
0xD0,0x42,
0xD1,0x02,
0xD2,0x85,
0xD3,0x02,
0xD4,0xB0,
0xD5,0x02,
0xD6,0xE3,
0xD7,0x03,
0xD8,0x05,
0xD9,0x03,
0xDA,0x36,
0xDB,0x03,
0xDC,0x45,
0xDD,0x03,
0xDE,0x54,
0xDF,0x03,
0xE0,0x65,
0xE1,0x03,
0xE2,0x78,
0xE3,0x03,
0xE4,0x8F,
0xE5,0x03,
0xE6,0xB5,
0xE7,0x03,
0xE8,0xE5,
0xE9,0x03,
0xEA,0xFF,                            
0xFF,0x24,
0xFB,0x01,           
0x00,0x00,
0x01,0x01,
0x02,0x0B,
0x03,0x0C,
0x04,0x00,
0x05,0x03,
0x06,0x00,
0x07,0x04,
0x08,0x00,
0x09,0x00,
0x0A,0x00,
0x0B,0x00,
0x0C,0x00,
0x0D,0x17,
0x0E,0x15,
0x0F,0x13,
0x10,0x00,
0x11,0x01,
0x12,0x0B,
0x13,0x0C,
0x14,0x00,
0x15,0x03,
0x16,0x00,
0x17,0x04,
0x18,0x00,
0x19,0x00,
0x1A,0x00,
0x1B,0x00,
0x1C,0x00,
0x1D,0x17,
0x1E,0x15,
0x1F,0x13,               
0x20,0x09,
0x21,0x02,
0x22,0x00,
0x23,0x00,
0x24,0x01,
0x25,0x3D,
0x26,0x00,                                                                        
0x27,0x01,                                                                                                                                                                            
0x29,0x38,
0x2A,0x2B,        
0x2F,0x02,
0x30,0x01,                                                                 
0x31,0x49,
0x32,0x23,
0x33,0x01,
0x34,0x03,
0x35,0x4f,
0x36,0x00,
0x37,0x2D,
0x38,0x08,
0x39,0x03,                                                                     
0x3A,0x4f,                                                                     
0x90,0x53,
0x92,0x53,                                                                                                                                                                  
0x7A,0x00,
0x7B,0xA2,
0x7C,0xD8,
0x7D,0x50,
0x7E,0x06,
0x7F,0x15,
0x74,0x02,
0x75,0x16,
0x81,0x03,
0x82,0x00,
0x76,0x03,
0x77,0x00,
0x84,0x01,
0x85,0x01,
0xD3,0x00,
0xD4,0x50,
0x2D,0x08,                                                                                       
0x78,0x00,
0x79,0x00,
0x80,0x00,
0x83,0x00,                                                                                            
0x93,0x04,
0x8E,0x04,
0x94,0x04,
0x8F,0x04,
0x8A,0x33,
0x8B,0xF0,
0x9B,0x0F,                                                                                         
0x9D,0xB0,                                                                                           
0xB3,0x2A,
0xB4,0x2A,
0xB5,0x15,
0xE3,0x02,
0xFF,0x28,
0xFB,0x01,
0x61,0x80,
0xFF,0x2E,
0xFB,0x01,
0x00,0x2B,
0x04,0x06,
0x05,0x25,
0x06,0x30,
0x07,0x01,
0x09,0x00,
0x3F,0x05,
0xF8,0x06,
0x19,0x41,
0x1A,0x00,
0x1B,0x21,
0x0A,0x00,
0x0B,0x0E,
0x0D,0x01,
0xE1,0x0A,
0xE2,0x16,
0xE3,0x0A,
0xE4,0x16,
0x1E,0x9A,
0x20,0x1D,
0x21,0x21,
0x22,0x3E,
0x23,0x00,
0x48,0x1D,
0x49,0x00,
0x41,0x20,
0x42,0x00,
0x44,0x00,
0x46,0x12,
0x4C,0x01,
0x4D,0x1C,
0x4E,0x01,
0x4F,0x1A,
0x25,0x8E,
0x26,0x80,
0x27,0x92,
0x2F,0x04,
0x30,0x5B,
0x32,0xBF,
0x33,0x1D,
0x34,0x20,
0x50,0x09,
0x51,0x0E,
0x53,0x1F,
0x54,0x3A,
0x55,0x40,
0x31,0x00,
0x52,0x00,
0x91,0x00,
0x96,0x01,
0x97,0x10,
0x98,0x02,
0x99,0x02,
0x9A,0x4f,
0x9B,0x02,
0x9C,0x01,
0x9D,0x02,
0x9E,0x01,
0x9F,0x00,
0xAD,0x66,
0xE0,0xA0,
0xBA,0x13,
0xBB,0x13,
0xBC,0x00,
0xBD,0x02,
0xBE,0x4F,
0xBF,0x50,
0xC0,0x58,
0xC3,0x02,
0xC4,0x01,
0xC5,0x03,
0xC6,0x15,
0xC7,0x49,
0xC8,0x0C,
0xC9,0x56,
0xCA,0x00,
0xCB,0x18,
0xCC,0x57,
0xCE,0x2F,
0xCF,0x00,
0xD0,0xB0,
0xD1,0x0A,
0xD2,0x0A,

#if !(LCM_DSI_CMD_MODE)
//add 
0xFF,0x2E,
0xFB,0x01,
0x00,0xAB, 
0x03,0x0A,
//
#endif

0xFF,0xE0,
0xFB,0x01,
0xA9,0x01,            
0xFF,0x10,
0xFB,0x01,
0x3B,0x03,
0xC9,0x45,
0x35,0x00,

#if (LCM_DSI_CMD_MODE)
//0x44,0x07,
//0x45,0x78,
#endif

0xBB,0x10,
0x51,0xFF,
0x53,0x2C,
0x55,0x01
};

static void init_lcm_registers(void)
{
	unsigned int data_array[5];
	int i;
	unsigned int data;
	
	data_array[0]=0x00023902;
	for(i = 0; i < (sizeof(reg_init)/(2*sizeof(char))); i++)
	{
		data = 0xff & reg_init[i][1];
		data_array[1]= 0xffff & (data << 8 | 0xff & reg_init[i][0]);
		printk("i = %d,data_array[0] = %x,data_array[1]=%08x\n",i,data_array[0],data_array[1]);	
		dsi_set_cmdq(data_array,2,1);
	}

	//data_array[0]=0x00043902;
	//data_array[1]=0x0A0A033B;
	//dsi_set_cmdq(data_array,2,1);
	//MDELAY(1);
#if (LCM_DSI_CMD_MODE)
	data_array[0]=0x00033902;
	data_array[1]=0x00000544;
	dsi_set_cmdq(data_array,2,1);
#endif

#if (LCM_DSI_CMD_MODE)
	data_array[0]=0x00023902;
	data_array[1]=0x000010BB;	//0x13:Video mode; 0x10:CMD MODE
	dsi_set_cmdq(data_array,2,1);

	data_array[0]=0x000C3902;
	data_array[1]=0x050249C9;	//0x13:Video mode; 0x10:CMD MODE
	data_array[2]=0x67060F00;
	data_array[3]=0xF0102E03;
	dsi_set_cmdq(data_array,4,1);
#else
	data_array[0]=0x00023902;
	data_array[1]=0x000013BB;	//0x13:Video mode; 0x10:CMD MODE
	dsi_set_cmdq(data_array,2,1);
#endif

//***********************************************
//	lcm start display
//***********************************************
	data_array[0] = 0x00110500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);

	data_array[0] = 0x00290500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{

		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;
   		 params->physical_height = 110000;
        params->physical_width = 62000;
		// enable tearing-free
#if (LCM_DSI_CMD_MODE)
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
#else
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
#endif	


        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   = BURST_VDO_MODE;
        #endif
	
		// DSI
		/* Command mode setting */
		//1 Three lane or Four lane
		params->dsi.LANE_NUM				= 4;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		//params->dsi.packet_size=256;

		#if (LCM_DSI_CMD_MODE)
		params->dsi.intermediat_buffer_num = 4;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
		#else
		params->dsi.intermediat_buffer_num = 0;
		#endif
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		//params->dsi.word_count=720*3;	

		
		params->dsi.vertical_sync_active				= 2;//1;
		params->dsi.vertical_backporch					= 6;
		params->dsi.vertical_frontporch					= 13;//2;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 10;//50;
		params->dsi.horizontal_backporch				= 20;//100;
		params->dsi.horizontal_frontporch				= 40;//80;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		params->dsi.esd_check_enable = 1;
		params->dsi.customization_esd_check_enable      = 0;
		params->dsi.lcm_esd_check_table[0].cmd          = 0x0A;
		params->dsi.lcm_esd_check_table[0].count        = 1;
		params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;

//		params->dsi.PLL_CLOCK=590;
		params->dsi.PLL_CLOCK=422;

}

static void lcm_init(void)
{
	unsigned char cmd = 0x0;
	unsigned char data = 0xFF;
	int ret=0;

	mt_set_gpio_mode(GPIO195|0x800000000,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO195|0x80000000,GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO195|0x80000000,GPIO_OUT_ZERO);
	MDELAY(1);

	cmd=0x00;
	data=0x0c;//x3	5.2V
	
	mt_set_gpio_mode(GPIO_SGM3804_ENP, GPIO_MODE_00);	//data sheet 136 page ,the first AVDD power on
	mt_set_gpio_dir(GPIO_SGM3804_ENP, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_SGM3804_ENP, GPIO_OUT_ONE);
	
#ifdef BUILD_LK
	ret=sgm3804_write_byte(cmd,data);
	if(ret)    	
		dprintf(0, "[LK]nt36760----sgm3804----cmd=%0x--i2c write error----\n",cmd);    	
	else
		dprintf(0, "[LK]nt36760----sgm3804----cmd=%0x--i2c write success----\n",cmd);
#else
	ret=sgm3804_write_bytes(cmd,data);
#endif
	MDELAY(5);

	mt_set_gpio_mode(GPIO_SGM3804_ENN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_SGM3804_ENN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_SGM3804_ENN, GPIO_OUT_ONE);
	cmd=0x01;
	data=0x0c;

#ifdef BUILD_LK
	ret=sgm3804_write_byte(cmd,data);
	if(ret)    	
		dprintf(0, "[LK]nt36760----sgm3804----cmd=%0x--i2c write error----\n",cmd);    	
	else
		dprintf(0, "[LK]nt36760----sgm3804----cmd=%0x--i2c write success----\n",cmd);   
#else
	ret=sgm3804_write_bytes(cmd,data); 
#endif

	MDELAY(10);

	SET_RESET_PIN(1);
	MDELAY(5);      

	SET_RESET_PIN(0);
	MDELAY(5);
	SET_RESET_PIN(1);
	MDELAY(20);

	init_lcm_registers();
}


static void lcm_suspend(void)
{
	unsigned int data_array[5];

	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20); 

	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);

	/*data_array[0] = 0x00FF1500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);*/

	SET_RESET_PIN(0);
	MDELAY(10);

	mt_set_gpio_mode(GPIO_SGM3804_ENN, GPIO_MODE_00);	//data sheet 136 page, the first AVDD Power off
	mt_set_gpio_dir(GPIO_SGM3804_ENN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_SGM3804_ENN, GPIO_OUT_ZERO);
	MDELAY(5);	

	mt_set_gpio_mode(GPIO_SGM3804_ENP, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_SGM3804_ENP, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_SGM3804_ENP, GPIO_OUT_ZERO);
	MDELAY(10);

	mt_set_gpio_mode(GPIO195|0x80000000,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO195|0x80000000,GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO195|0x80000000,GPIO_OUT_ONE);
	MDELAY(1);
}

static void lcm_resume(void)
{
    lcm_init();
}


#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

//	data_array[0]= 0x00290508; //HW bug, so need send one HS packet
//	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
    
}
#endif


static unsigned int lcm_compare_id(void)
{
#if 1
	unsigned char id=0;
	unsigned char id1=0;
	unsigned char id2=0;
	unsigned char buffer[2];
	unsigned int array[16]; 

	array[0] = 0x00023700;// return byte number
	dsi_set_cmdq(&array, 1, 1);
	MDELAY(10);
	read_reg_v2(0xDA, buffer, 1);
	id = buffer[0]; 

	read_reg_v2(0xDB, buffer, 1);
	id1 = buffer[0]; 

	read_reg_v2(0xDC, buffer, 1);
	id2 = buffer[0]; 
#endif


	#if defined(BUILD_LK)
	printf("%s id(%x)  id1(%x) id2(%x)\n", __func__,id,id1,id2);
	#else
	printk("alvin:%s id(%x)  id1(%x) id2(%x)\n", __func__,id,id1,id2);
	#endif
	if(id == 0xb2 && id1 == 0x68 ) //&& id2 == 0x81
		return 1;
	else
		return 0; 
}

LCM_DRIVER boe_nt36760_dsi_cmd_sgm3804_lcm_drv = 
{
   	.name		= "B2_nt36760_dsi_cmd_sgm3804",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
	//.esd_check   = lcm_esd_check,
        //.esd_recover   = lcm_esd_recover,

#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
};
