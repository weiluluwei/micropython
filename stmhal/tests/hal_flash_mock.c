/*
 * hal_flash_mock.c
 *
 *  Created on: 09.02.2016
 *      Author: badi
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "helper.h"

#include STM32_HAL_H

#define HAL_FLASHSIZE (1<<8)
static uint8_t buffer[HAL_FLASHSIZE];

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
    if (TypeProgram == FLASH_TYPEPROGRAM_DOUBLEWORD)
    {
        printf("Write 0x%016lx to 0x%04x\n", Data, Address);
        write_le_64(Data, &buffer[Address], HAL_FLASHSIZE);
    }
    else
    {
        printf("Programm type %d not supported for test!.\n", TypeProgram);
    }
    return HAL_OK;
}

HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *SectorError)
{
    printf("Erase\n");
    return HAL_OK;
}

uint32_t hal_flash_mock_buffersize(void)
{
    return HAL_FLASHSIZE;
}


uint8_t hal_flash_mock_get8(uint32_t byteAddr)
{
    if (byteAddr<HAL_FLASHSIZE)
    {
        uint8_t val = *((uint8_t *)&buffer[byteAddr]);
        return val;
    }
    return -1;
}

uint16_t hal_flash_mock_get16(uint32_t byteAddr)
{
    if (byteAddr<HAL_FLASHSIZE)
    {
        uint16_t val = *((uint16_t *)&buffer[byteAddr]);
        return val;
    }
    return -1;
}

uint64_t hal_flash_mock_get64(uint32_t byteAddr)
{
    if (byteAddr<HAL_FLASHSIZE)
    {
        uint64_t val = *((uint64_t *)&buffer[byteAddr]);
        return val;
    }
    return -1;
}

void hal_flash_mock_erase(uint8_t val)
{
    memset(buffer, val, HAL_FLASHSIZE*sizeof(buffer[0]));
}

