/*----------------------------------------------------------------------------*/
/* sim800.c                                                                   */
/* 描述:gprs通讯程序                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* 2014/x/x yihuagang 建立 V0.1                                               */
/*                                                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/



/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "systick.h"
#include "gsensor.h"
#include "sof_iic.h"
#include "integer.h"

void set_gsensor_ctrl(int ctrl , int value);
void GPIO_Configuration(void);
void EXTI_Configuration(void);
void NVIC_Configuration(void);


/* registers */
#define BMA250_STATUS_REG (0x0A)
#define BMA250_FSR_REG    (0x0F)
#define BMA250_ODR_REG    (0x10)
#define BMA250_PWR_REG    (0x11)
#define BMA250_SOFTRESET_REG  (0x14)
#define BMA250_INT_TYPE_REG (0x17)
#define BMA250_INT_DST_REG  (0x1A)
#define BMA250_INT_SRC_REG  (0x1E)

/* masks */
#define BMA250_STATUS_RDY_MASK  (0x80)
#define BMA250_FSR_MASK   (0x0F)
#define BMA250_ODR_MASK   (0x1F)
#define BMA250_PWR_SLEEP_MASK (0x80)
#define BMA250_PWR_AWAKE_MASK (0x00)
#define BMA250_SOFTRESET_MASK   (0xB6)
#define BMA250_INT_TYPE_MASK  (0x10)
#define BMA250_INT_DST_1_MASK (0x01)
#define BMA250_INT_DST_2_MASK (0x80)
#define BMA250_INT_SRC_MASK (0x00)


#define BMA223_ADDR         (0x30)


#define CHIP_ID_REG     0x00
#define VERSION_REG     0x01
#define X_AXIS_LSB_REG    0x02
#define X_AXIS_MSB_REG    0x03
#define Y_AXIS_LSB_REG    0x04
#define Y_AXIS_MSB_REG    0x05
#define Z_AXIS_LSB_REG    0x06
#define Z_AXIS_MSB_REG    0x07

#define NEW_DATA_X__POS   0
#define NEW_DATA_X__LEN   1
#define NEW_DATA_X__MSK   0x01
#define NEW_DATA_X__REG   X_AXIS_LSB_REG

#define ACC_X_LSB__POS    6
#define ACC_X_LSB__LEN    2
#define ACC_X_LSB__MSK    0xC0
#define ACC_X_LSB__REG    X_AXIS_LSB_REG

#define ACC_X_MSB__POS    0
#define ACC_X_MSB__LEN    8
#define ACC_X_MSB__MSK    0xFF
#define ACC_X_MSB__REG    X_AXIS_MSB_REG

#define NEW_DATA_Y__POS   0
#define NEW_DATA_Y__LEN   1
#define NEW_DATA_Y__MSK   0x01
#define NEW_DATA_Y__REG   Y_AXIS_LSB_REG

#define ACC_Y_LSB__POS    6
#define ACC_Y_LSB__LEN    2
#define ACC_Y_LSB__MSK    0xC0
#define ACC_Y_LSB__REG    Y_AXIS_LSB_REG

#define ACC_Y_MSB__POS    0
#define ACC_Y_MSB__LEN    8
#define ACC_Y_MSB__MSK    0xFF
#define ACC_Y_MSB__REG    Y_AXIS_MSB_REG

#define NEW_DATA_Z__POS   0
#define NEW_DATA_Z__LEN   1
#define NEW_DATA_Z__MSK   0x01
#define NEW_DATA_Z__REG   Z_AXIS_LSB_REG

#define ACC_Z_LSB__POS    6
#define ACC_Z_LSB__LEN    2
#define ACC_Z_LSB__MSK    0xC0
#define ACC_Z_LSB__REG    Z_AXIS_LSB_REG

#define ACC_Z_MSB__POS    0
#define ACC_Z_MSB__LEN    8
#define ACC_Z_MSB__MSK    0xFF
#define ACC_Z_MSB__REG    Z_AXIS_MSB_REG



#define RECORD_RANGE_NUM 10   ////记录的XYZ的BUF大小
#define STATIC_RANGE_VALUE  1  ////静止状态下 XYZ  变化的范围应该小于该值,去掉静止震动误差
#define READ_TIME_MS     20      /// 设定多少ms 读写一次，现为20ms


#define SMB223_GET_BITSLICE(regvar, bitname)\
      (regvar & bitname##__MSK) >> bitname##__POS


#define SMB223_SET_BITSLICE(regvar, bitname, val)\
      (regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK)  



//uint8_t  IIC_READ_STATE =0;
 

////////////////////

unsigned long gsensor_task_t_sec = 0;  ////单位秒
unsigned long Gsensor_time_count = 0;
unsigned long INT1_count =0; ////记录INT1  中断的次数
unsigned long pre_INT1_count_S = 0;///记录上1S   进中断次数
unsigned long pre_INT1_count = 0; //记录上次的次数



GSENSOR_INFO g_gsensor_info;


int gSensor_taskinit()
{ 
	int result;

    gsensor_task_t_sec =GetSysTick_Sec();
	set_gsensor_ctrl(GSENSOR_RESET_CAL,0);
  
  return result;
}


int MBA220init(void)
{
  unsigned char MBA_ID;

  I2C_Configuration();
  I2C_WriteByte(BMA250_SOFTRESET_MASK, BMA250_SOFTRESET_REG,BMA223_ADDR);
  Wait10Ms(10);
  //  
  I2C_WriteByte(0x0C, BMA250_ODR_REG,BMA223_ADDR);

  I2C_WriteByte(0x03, BMA250_FSR_REG,BMA223_ADDR);  /////设置为2G
  
  I2C_WriteByte(BMA250_PWR_AWAKE_MASK, BMA250_PWR_REG,BMA223_ADDR);

  I2C_WriteByte(0x7, 0x16,BMA223_ADDR);/////中断脚开启，开启xyz叠加
  I2C_WriteByte(0x4, 0x19,BMA223_ADDR);/////中断脚开启
  I2C_WriteByte(0x9, 0x21,BMA223_ADDR);/////中断脚亮起时间设置为250us
  I2C_WriteByte(0x8, 0x28,BMA223_ADDR);/////设置震动幅度为0x5   *2/256G
  
  GPIO_Configuration();
  EXTI_Configuration();
  NVIC_Configuration();

  //读gSensor ID号
  I2C_ReadBytes(&MBA_ID,1,0x00,BMA223_ADDR);

  if( MBA_ID==0xf8 )
    return 1;
  else
    return 0;
}



int MBA220_INT(void)
{
    
      
  return (int)GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5);
}


 /*
 **
 *********************************************************************************
   Function Name : set_no_shock_time

 
   Detailed Description:
 
设置no_shock_time 时间后，如果没有震动，将g_gsensor_info.shock_30s_flag
标志位设置为false.

no_shock_time 的参数不能少于5S。default 为30S
 
 *********************************************************************************/
void GPIO_Configuration(void)
{
    //配置GPIOA_0口作为外部中断口
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC->APB2ENR |= 0x09;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode   = GPIO_Mode_IN_FLOATING ;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void EXTI_Configuration(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line    = EXTI_Line0;                 //外部中断线
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;        //中断模式
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;        //中断触发方式
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;                     //打开中断
    EXTI_Init(&EXTI_InitStructure);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);   //选择 GPIO管脚用作外部中断线路
}

void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel            = EXTI0_IRQn;   //指定中断源
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;       //中断占先等级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;              // 指定响应优先级别1
    NVIC_InitStructure.NVIC_IRQChannelCmd         = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

//中断处理函数
unsigned long last_shock_mark_t;
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        //添加中断处理程序
        last_shock_mark_t = GetSysTick_Sec();
        //
        INT1_count ++;
		//
        EXTI_ClearFlag(EXTI_Line0);
    }
}



void set_gsensor_ctrl(int ctrl , int value)
{
    if( ctrl==GSNESOR_WORK_MODE ){
	  	switch(value)
        {
	  	 	  case GSENSOR_NORMAL:
			  	I2C_WriteByte(0x1,0x28,BMA223_ADDR);/////设置震动幅度为0x5   *2/256G
				g_gsensor_info.shock_time=0;
        		g_gsensor_info.static_time=0;
        		g_gsensor_info.shock_cnt=0;
			  	g_gsensor_info.gsensor_work_state = GSENSOR_NORMAL;
			  	break;
              case GSENSOR_SLEEP:
			  case GSENSOR_LOW_POWER:
			  	I2C_WriteByte(0x8,0x28,BMA223_ADDR);/////设置震动幅度为0x5   *2/256G
				g_gsensor_info.shock_time=0;
        		g_gsensor_info.static_time=0;
        		g_gsensor_info.shock_cnt=0;
			  	g_gsensor_info.gsensor_work_state = GSENSOR_LOW_POWER;
			  	break;
			  default:
			  	break;
        }
  	}else if( ctrl==GSENSOR_RESET_CAL ){
  	    g_gsensor_info.stat_t_start = GetSysTick_Sec();
  	    g_gsensor_info.shock_time=0;
        g_gsensor_info.static_time=0;
        g_gsensor_info.shock_cnt=0;
  	}
		
}

GSENSOR_INFO *get_gsensor_state()
{
  return &g_gsensor_info;
}


void gsensor_task(void)  
{
    //wjh add
    if( GetSysTick_Sec() > gsensor_task_t_sec )
    {
        //
        g_gsensor_info.shock_level = (g_gsensor_info.shock_level+g_gsensor_info.shock_cnt)/2;
        //
        g_gsensor_info.new_flg=1;
        if( INT1_count>0 ){
            g_gsensor_info.shock_time++;
            g_gsensor_info.static_time=0;
            g_gsensor_info.shock_cnt=INT1_count;
            INT1_count = 0;
        }else{
            g_gsensor_info.shock_time=0;
            g_gsensor_info.static_time++;
            g_gsensor_info.shock_cnt = 0;
        }
        gsensor_task_t_sec = GetSysTick_Sec();
        g_gsensor_info.stat_period = GetSysTick_Sec()-g_gsensor_info.stat_t_start;
    }

}


/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
