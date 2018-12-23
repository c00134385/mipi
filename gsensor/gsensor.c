/*----------------------------------------------------------------------------*/
/* sim800.c                                                                   */
/* ����:gprsͨѶ����                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* 2014/x/x yihuagang ���� V0.1                                               */
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



#define RECORD_RANGE_NUM 10   ////��¼��XYZ��BUF��С
#define STATIC_RANGE_VALUE  1  ////��ֹ״̬�� XYZ  �仯�ķ�ΧӦ��С�ڸ�ֵ,ȥ����ֹ�����
#define READ_TIME_MS     20      /// �趨����ms ��дһ�Σ���Ϊ20ms


#define SMB223_GET_BITSLICE(regvar, bitname)\
      (regvar & bitname##__MSK) >> bitname##__POS


#define SMB223_SET_BITSLICE(regvar, bitname, val)\
      (regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK)  



//uint8_t  IIC_READ_STATE =0;
 

////////////////////

unsigned long gsensor_task_t_sec = 0;  ////��λ��
unsigned long Gsensor_time_count = 0;
unsigned long INT1_count =0; ////��¼INT1  �жϵĴ���
unsigned long pre_INT1_count_S = 0;///��¼��1S   ���жϴ���
unsigned long pre_INT1_count = 0; //��¼�ϴεĴ���



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

  I2C_WriteByte(0x03, BMA250_FSR_REG,BMA223_ADDR);  /////����Ϊ2G
  
  I2C_WriteByte(BMA250_PWR_AWAKE_MASK, BMA250_PWR_REG,BMA223_ADDR);

  I2C_WriteByte(0x7, 0x16,BMA223_ADDR);/////�жϽſ���������xyz����
  I2C_WriteByte(0x4, 0x19,BMA223_ADDR);/////�жϽſ���
  I2C_WriteByte(0x9, 0x21,BMA223_ADDR);/////�жϽ�����ʱ������Ϊ250us
  I2C_WriteByte(0x8, 0x28,BMA223_ADDR);/////�����𶯷���Ϊ0x5   *2/256G
  
  GPIO_Configuration();
  EXTI_Configuration();
  NVIC_Configuration();

  //��gSensor ID��
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
 
����no_shock_time ʱ������û���𶯣���g_gsensor_info.shock_30s_flag
��־λ����Ϊfalse.

no_shock_time �Ĳ�����������5S��default Ϊ30S
 
 *********************************************************************************/
void GPIO_Configuration(void)
{
    //����GPIOA_0����Ϊ�ⲿ�жϿ�
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
    EXTI_InitStructure.EXTI_Line    = EXTI_Line0;                 //�ⲿ�ж���
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;        //�ж�ģʽ
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;        //�жϴ�����ʽ
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;                     //���ж�
    EXTI_Init(&EXTI_InitStructure);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);   //ѡ�� GPIO�ܽ������ⲿ�ж���·
}

void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel            = EXTI0_IRQn;   //ָ���ж�Դ
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;       //�ж�ռ�ȵȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;              // ָ����Ӧ���ȼ���1
    NVIC_InitStructure.NVIC_IRQChannelCmd         = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

//�жϴ�����
unsigned long last_shock_mark_t;
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        //����жϴ������
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
			  	I2C_WriteByte(0x1,0x28,BMA223_ADDR);/////�����𶯷���Ϊ0x5   *2/256G
				g_gsensor_info.shock_time=0;
        		g_gsensor_info.static_time=0;
        		g_gsensor_info.shock_cnt=0;
			  	g_gsensor_info.gsensor_work_state = GSENSOR_NORMAL;
			  	break;
              case GSENSOR_SLEEP:
			  case GSENSOR_LOW_POWER:
			  	I2C_WriteByte(0x8,0x28,BMA223_ADDR);/////�����𶯷���Ϊ0x5   *2/256G
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
