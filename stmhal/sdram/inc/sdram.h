/**
 * sdram.h
 *
 * This file is a copy of stm32f429i_discovery_sdram.h from
 * STM32Cube_FW_F4_V1.7.0/Drivers/BSP/STM32F429I-Discovery/stm32f429i_discovery_sdram.h
 * function names were fixed with following sed command:
 * 's/\(BSP_SDRAM_\)\([A-Z]\)/sdram_\l\2/g'
 *
    ******************************************************************************
    * @file    stm32f429i_discovery_sdram.h
    * @author  MCD Application Team
    * @version V2.1.2
    * @date    02-March-2015
    * @brief   This file contains all the functions prototypes for the
    *          stm32f429i_discovery_sdram.c driver.
    ******************************************************************************
    * @attention
    *
    * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
    *
    * Redistribution and use in source and binary forms, with or without modification,
    * are permitted provided that the following conditions are met:
    *     1. Redistributions of source code must retain the above copyright notice,
    *            this list of conditions and the following disclaimer.
    *     2. Redistributions in binary form must reproduce the above copyright notice,
    *            this list of conditions and the following disclaimer in the documentation
    *            and/or other materials provided with the distribution.
    *     3. Neither the name of STMicroelectronics nor the names of its contributors
    *            may be used to endorse or promote products derived from this software
    *            without specific prior written permission.
    *
    * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
    * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    *
    ******************************************************************************
    */     

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SDRAM_H
#define __SDRAM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/** @addtogroup BSP
    * @{
    */
    
/** @addtogroup STM32F429I_DISCOVERY
    * @{
    */
 
/** @defgroup STM32F429I_DISCOVERY_SDRAM
    * @{
    */
    
/** @defgroup STM32F429I_DISCOVERY_SDRAM_Exported_Types
    * @{
    */
/**
    * @}
    */

/** @defgroup STM32F429I_DISCOVERY_SDRAM_Exported_Constants
    * @{
    */ 
/**
    * @brief    FMC SDRAM Bank address
    */     
#define SDRAM_DEVICE_ADDR                 ((uint32_t)0xD0000000)
#define SDRAM_DEVICE_SIZE                 ((uint32_t)0x800000)    /* SDRAM device size in MBytes */
    
/**
    * @brief    FMC SDRAM Memory Width
    */    
/* #define SDRAM_MEMORY_WIDTH     FMC_SDRAM_MEM_BUS_WIDTH_8 */
#define SDRAM_MEMORY_WIDTH        FMC_SDRAM_MEM_BUS_WIDTH_16

/**
    * @brief    FMC SDRAM CAS Latency
    */    
/* #define SDRAM_CAS_LATENCY      FMC_SDRAM_CAS_LATENCY_2 */
#define SDRAM_CAS_LATENCY         FMC_SDRAM_CAS_LATENCY_3

/**
    * @brief    FMC SDRAM Memory clock period
    */    
#define SDCLOCK_PERIOD            FMC_SDRAM_CLOCK_PERIOD_2        /* Default configuration used with LCD */
/* #define SDCLOCK_PERIOD         FMC_SDRAM_CLOCK_PERIOD_3 */

/**
    * @brief    FMC SDRAM Memory Read Burst feature
    */    
#define SDRAM_READBURST           FMC_SDRAM_RBURST_DISABLE        /* Default configuration used with LCD */
/* #define SDRAM_READBURST        FMC_SDRAM_RBURST_ENABLE */

/**
    * @brief    FMC SDRAM Bank Remap
    */        
/* #define SDRAM_BANK_REMAP */

/* Set the refresh rate counter */
/* (15.62 us x Freq) - 20 */
#define REFRESH_COUNT            ((uint32_t)1386)     /* SDRAM refresh counter */
#define SDRAM_TIMEOUT            ((uint32_t)0xFFFF)

/* DMA definitions for SDRAM DMA transfer */
#define __DMAx_CLK_ENABLE        __DMA2_CLK_ENABLE
#define SDRAM_DMAx_CHANNEL       DMA_CHANNEL_0
#define SDRAM_DMAx_STREAM        DMA2_Stream0
#define SDRAM_DMAx_IRQn          DMA2_Stream0_IRQn
#define SDRAM_DMAx_IRQHandler    DMA2_Stream0_IRQHandler

/**
    * @brief    FMC SDRAM Mode definition register defines
    */
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)
/**
    * @}
    */
    
/** @defgroup STM32F429I_DISCOVERY_SDRAM_Exported_Macro
    * @{
    */
/**
    * @}
    */ 

/** @defgroup STM32F429I_DISCOVERY_SDRAM_Exported_Functions
    * @{
    */
void              sdram_init(void);
void              sdram_initialization_sequence(uint32_t RefreshCount);
void              sdram_readData(uint32_t uwStartAddress, uint32_t* pData, uint32_t uwDataSize);
void              sdram_readData_DMA(uint32_t uwStartAddress, uint32_t* pData, uint32_t uwDataSize);
void              sdram_writeData(uint32_t uwStartAddress, uint32_t* pData, uint32_t uwDataSize);
void              sdram_writeData_DMA(uint32_t uwStartAddress, uint32_t* pData, uint32_t uwDataSize);
HAL_StatusTypeDef sdram_sendcmd(FMC_SDRAM_CommandTypeDef *SdramCmd);
void              sdram_DMA_IRQHandler(void);

/* 8 Bit in das SD-RAM schreiben */
void sdram_write_8(uint32_t adr, uint8_t data);

/* 16 Bit in das SD-RAM schreiben */
void sdram_write_16(uint32_t adr, uint16_t data);

/* 32 Bit in das SD-RAM schreiben */
void sdram_write_32(uint32_t adr, uint32_t data);

/* 8 Bit aus dem SD-RAM lesen */
uint8_t sdram_read_8(uint32_t adr);

/* 16 Bit aus dem SD-RAM lesen */
uint16_t sdram_read_16(uint32_t adr);

/* 32 Bit aus dem SD-RAM lesen */
uint32_t sdram_read_32(uint32_t adr);

/**
    * @}
    */

/**
    * @}
    */

/**
    * @}
    */     

/**
    * @}
    */

#ifdef __cplusplus
}
#endif

#endif /* __SDRAM_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
