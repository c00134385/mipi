/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************
* File Name          : mass_mal.c
* Author             : MCD Application Team
* Version            : V3.0.1
* Date               : 04/27/2009
* Description        : Medium Access Layer interface
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "platform_config.h"
#include "stm32f10x.h"
#include "mass_mal.h"
#include <string.h>
#include "w25xx.h"

uint32_t Mass_Memory_Size[3];
uint32_t Mass_Block_Size[3];
uint32_t Mass_Block_Count[3];

/* logic unit count; the first is 0 */
//uint32_t Max_Lun = 0;

uint16_t MAL_Init(uint8_t lun)
{
    uint16_t status = MAL_OK;

    switch (lun)
    {
    case 0:
    {
        uint32_t device_id;

        w25xx_init();
        device_id = w25xx_get_device_id();

        switch(device_id)
        {
        case W25X16:
            Mass_Memory_Size[0] = 2UL*1024*1024;
            Mass_Block_Size[0]  = 4096 ;
            Mass_Block_Count[0] = Mass_Memory_Size[0] / Mass_Block_Size[0];
            break;
        case W25X32:
            Mass_Memory_Size[0] = 4UL*1024*1024;
            Mass_Block_Size[0]  = 4096 ;
            Mass_Block_Count[0] = Mass_Memory_Size[0] / Mass_Block_Size[0];
            break;
        case W25X64:
		case W25Q64:
        case AE25Q64:
            Mass_Memory_Size[0] = 8UL*1024*1024;
            Mass_Block_Size[0]  = 4096 ;
            Mass_Block_Count[0] = Mass_Memory_Size[0] / Mass_Block_Size[0];
            break;
        default:
            Mass_Memory_Size[0] = 0;
            Mass_Block_Size[0]  = 0;
            Mass_Block_Count[0] = 0;
            break;
        } // switch(device_id)
        status = MAL_OK;
    }
    break;
    case 1:
        status = MAL_OK;
        break;
    case 2:
        status = MAL_FAIL;
        break;
    default:
        return MAL_FAIL;
    }
    return status;
}

uint16_t MAL_Write(uint8_t lun, uint32_t Memory_Offset, uint8_t *Writebuff, uint16_t Transfer_Length)
{
    switch (lun)
    {
    case 0:
    {
        w25xx_page_write(Memory_Offset,(uint8_t*)Writebuff,Transfer_Length);
    }
    break;
    case 1:
    {
//        dev_spi_flash->write(dev_spi_flash,Memory_Offset,Writebuff,Transfer_Length);
    }
    break;
    default:
        return MAL_FAIL;
    }
    return MAL_OK;
}

uint16_t MAL_Read(uint8_t lun, uint32_t Memory_Offset, uint8_t *Readbuff, uint16_t Transfer_Length)
{
    switch (lun)
    {
    case 0:
    {
        w25xx_read(Memory_Offset,(uint8_t*)Readbuff,Transfer_Length);
    }
    break;
    case 1:
    {
//        dev_spi_flash->read(dev_spi_flash,Memory_Offset,Readbuff,Transfer_Length);
    }
    break;
    default:
        return MAL_FAIL;
    }
    return MAL_OK;
}

uint16_t MAL_GetStatus (uint8_t lun)
{
    switch(lun)
    {
    case 0:
        return MAL_OK;
    case 1:
        return MAL_OK;
    case 2:
        return MAL_FAIL;
    default:
        return MAL_FAIL;
    }
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
