/*
 * hal_flash_mock.c
 *
 *  Created on: 09.02.2016
 *      Author: badi
 */
#include STM32_HAL_H


HAL_StatusTypeDef   HAL_FLASH_Unlock(void)
{
  return HAL_OK;
}

HAL_StatusTypeDef   HAL_FLASH_Lock(void)
{
  return HAL_OK;

}

HAL_StatusTypeDef   HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data)
{
  return HAL_OK;
}

HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *SectorError)
{
  return HAL_OK;
}
