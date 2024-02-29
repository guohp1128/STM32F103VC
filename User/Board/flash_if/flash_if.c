/**
  ******************************************************************************
  * @file    IAP_Main/Src/flash_if.c 
  * @author  MCD Application Team
  * @brief   This file provides all the memory related operation functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/** @addtogroup STM32F1xx_IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "./flash_if/flash_if.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Unlocks Flash for write access
  * @param  None
  * @retval None
  */
void FLASH_If_Init(void)
{
  /* Unlock the Program memory */
  HAL_FLASH_Unlock();

  /* Clear all FLASH flags */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
  /* Unlock the Program memory */
  HAL_FLASH_Lock();
}

/**
  * @brief  This function does an erase of all user flash area
  * @param  pageStartAddr: start of user flash area
  * @param  pageNum: number of page
  * @retval FLASHIF_OK : user flash area successfully erased
  *         FLASHIF_ERASEKO : error occurred
  */
uint32_t FLASH_If_Erase(uint32_t pageStartAddr, uint32_t pageNum)
{
  uint32_t NbrOfPages = 0;
  uint32_t PageError = 0;
  FLASH_EraseInitTypeDef pEraseInit;
  HAL_StatusTypeDef status = HAL_OK;

  /* Unlock the Flash to enable the flash control register access *************/ 
  HAL_FLASH_Unlock();

  /* Get the sector where start the user flash area */
  NbrOfPages = pageNum;

  pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
  pEraseInit.PageAddress = pageStartAddr;
  pEraseInit.Banks = FLASH_BANK_1;
  pEraseInit.NbPages = NbrOfPages;
  status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  if (status != HAL_OK)
  {
    /* Error occurred while page erase */
    return FLASHIF_ERASEKO;
  }

  return FLASHIF_OK;
}

/**
 *******************************************************************************
 * @brief   This function read data
 * @param   [in] startAddr  - start address for read data buffer
 * @param   [in] *buf       - pointer on data buffer
 * @param   [in] bufLen     - length of data buffer (unit is 32-bit word)    
 * @return  None
 * @note    
 *******************************************************************************
 */
void FLASH_If_Read(uint32_t startAddr, uint32_t* buf ,uint32_t bufLen)
{
    if(bufLen)
    {
        uint32_t endAddr = startAddr + ((bufLen - 1) * 4);
        while(endAddr >= startAddr)
        {
            *buf = *(uint32_t*)startAddr;
            startAddr += 4;
            buf++;
        }
    }
}

/* Public functions ---------------------------------------------------------*/
/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  destination: start address for target location
  * @param  p_source: pointer on buffer with data to write
  * @param  length: length of data buffer (unit is 32-bit word)
  * @retval uint32_t 0: Data successfully written to Flash memory
  *         1: Error occurred while writing data in Flash memory
  *         2: Written Data in flash memory is different from expected one
  */
uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length)
{
  uint32_t i = 0;

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  for (i = 0; (i < length) && (destination <= (USER_FLASH_END_ADDRESS-4)); i++)
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */ 
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, destination, *(uint32_t*)(p_source+i)) == HAL_OK)      
    {
     /* Check the written value */
      if (*(uint32_t*)destination != *(uint32_t*)(p_source+i))
      {
        /* Flash content doesn't match SRAM content */
        return(FLASHIF_WRITINGCTRL_ERROR);
      }
      /* Increment FLASH destination address */
      destination += 4;
    }
    else
    {
      /* Error occurred while writing data in Flash memory */
      return (FLASHIF_WRITING_ERROR);
    }
  }

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  return (FLASHIF_OK);
}

//从指定地址开始写入指定长度的数据
//WriteAddr:起始地址(此地址必须为4的倍数!!)
//pBuffer:数据指针
//NumToWrite:字(32位)数(就是要写入的32位数据的个数.) 
uint32_t STMFLASH_Write(uint32_t WriteAddr,uint32_t *pBuffer,uint32_t NumToWrite)	
{
	uint32_t addrx=0;
	uint32_t writeEnd=0;	
    uint32_t sectorAddr=0;
    uint32_t writedByte=0;
    uint32_t FLASH_DATA_BACKUP[FLASH_PAGE_SIZE/4] = {0};	// 扇区数据备份
    
    if(WriteAddr<FLASH_BASE||WriteAddr%4)return FLASHIF_WRITINGCTRL_ERROR;	//非法地址
    
    while(writedByte <  NumToWrite*4)
    {
        addrx=WriteAddr + writedByte;				//写入的起始地址
        writeEnd = NumToWrite*4 - writedByte;       //待写入的字节尺寸
        sectorAddr = ((addrx-FLASH_BASE)/FLASH_PAGE_SIZE)*FLASH_PAGE_SIZE+FLASH_BASE;//扇区首地址
        FLASH_If_Read(sectorAddr,FLASH_DATA_BACKUP,FLASH_PAGE_SIZE/4); //读出整个扇区数据

        if(writeEnd <= FLASH_PAGE_SIZE)
        {
            memcpy(((uint8_t*)FLASH_DATA_BACKUP+addrx-sectorAddr),((uint8_t*)pBuffer)+writedByte,writeEnd);
            writedByte += writeEnd;
        }
        else
        {
            memcpy(((uint8_t*)FLASH_DATA_BACKUP+addrx-sectorAddr),((uint8_t*)pBuffer)+writedByte,FLASH_PAGE_SIZE);
            writedByte += FLASH_PAGE_SIZE;
        }

        if(FLASH_If_Erase(sectorAddr, 1) == FLASHIF_OK)//扇区擦除
        {
            if(FLASH_If_Write(sectorAddr, FLASH_DATA_BACKUP, FLASH_PAGE_SIZE/4) != FLASHIF_OK)
            {
                return FLASHIF_WRITING_ERROR;
            }
        }
        else
        {
            return FLASHIF_ERASEKO;
        }
    }
    
    return FLASHIF_OK;
} 



/**
  * @brief  Returns the write protection status of application flash area.
  * @param  None
  * @retval If a sector in application area is write-protected returned value is a combinaison
            of the possible values : FLASHIF_PROTECTION_WRPENABLED, FLASHIF_PROTECTION_PCROPENABLED, ...
  *         If no sector is write-protected FLASHIF_PROTECTION_NONE is returned.
  */
uint32_t FLASH_If_GetWriteProtectionStatus(void)
{
  uint32_t ProtectedPAGE = FLASHIF_PROTECTION_NONE;
  FLASH_OBProgramInitTypeDef OptionsBytesStruct;

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  /* Check if there are write protected sectors inside the user flash area ****/
  HAL_FLASHEx_OBGetConfig(&OptionsBytesStruct);

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  /* Get pages already write protected ****************************************/
  ProtectedPAGE = ~(OptionsBytesStruct.WRPPage) & FLASH_PAGE_TO_BE_PROTECTED;

  /* Check if desired pages are already write protected ***********************/
  if(ProtectedPAGE != 0)
  {
    /* Some sectors inside the user flash area are write protected */
    return FLASHIF_PROTECTION_WRPENABLED;
  }
  else
  { 
    /* No write protected sectors inside the user flash area */
    return FLASHIF_PROTECTION_NONE;
  }
}

/**
  * @brief  Configure the write protection status of user flash area.
  * @param  protectionstate : FLASHIF_WRP_DISABLE or FLASHIF_WRP_ENABLE the protection
  * @retval uint32_t FLASHIF_OK if change is applied.
  */
uint32_t FLASH_If_WriteProtectionConfig(uint32_t protectionstate)
{
  uint32_t ProtectedPAGE = 0x0;
  FLASH_OBProgramInitTypeDef config_new, config_old;
  HAL_StatusTypeDef result = HAL_OK;
  

  /* Get pages write protection status ****************************************/
  HAL_FLASHEx_OBGetConfig(&config_old);

  /* The parameter says whether we turn the protection on or off */
  config_new.WRPState = (protectionstate == FLASHIF_WRP_ENABLE ? OB_WRPSTATE_ENABLE : OB_WRPSTATE_DISABLE);

  /* We want to modify only the Write protection */
  config_new.OptionType = OPTIONBYTE_WRP;
  
  /* No read protection, keep BOR and reset settings */
  config_new.RDPLevel = OB_RDP_LEVEL_0;
  config_new.USERConfig = config_old.USERConfig;  
  /* Get pages already write protected ****************************************/
  ProtectedPAGE = config_old.WRPPage | FLASH_PAGE_TO_BE_PROTECTED;

  /* Unlock the Flash to enable the flash control register access *************/ 
  HAL_FLASH_Unlock();

  /* Unlock the Options Bytes *************************************************/
  HAL_FLASH_OB_Unlock();
  
  /* Erase all the option Bytes ***********************************************/
  result = HAL_FLASHEx_OBErase();
    
  if (result == HAL_OK)
  {
    config_new.WRPPage    = ProtectedPAGE;
    result = HAL_FLASHEx_OBProgram(&config_new);
  }
  
  return (result == HAL_OK ? FLASHIF_OK: FLASHIF_PROTECTION_ERRROR);
}
/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
