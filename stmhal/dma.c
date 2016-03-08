/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include STM32_HAL_H

#include "dma.h"
#include "py/obj.h"
#include "irq.h"


typedef enum {
    dma_id_not_defined=-1,
    dma_id_0,
    dma_id_1,
    dma_id_2,
    dma_id_3,
    dma_id_4,
    dma_id_5,
    dma_id_6,
    dma_id_7,
    dma_id_8,
    dma_id_9,
    dma_id_10,
    dma_id_11,
    dma_id_12,
    dma_id_13,
    dma_id_14,
    dma_id_15,
} dma_id_t;

typedef struct
{
    dma_descr_t     dma_descr;
#if defined(MCU_SERIES_F4) || defined(MCU_SERIES_F7)
    DMA_Stream_TypeDef  *instance;
#elif defined(MCU_SERIES_L4)
    DMA_Channel_TypeDef *instance;
#else
#error "Unsupported Processor"
#endif
    uint32_t        sub_instance;
    dma_id_t        id;
    const DMA_InitTypeDef *init;
} dma_p2dma_t;

#define NSTREAMS_PER_CONTROLLER_LOG2 (3)
#define NCONTROLLERS            (2)
#define DMA_TX_TRANSFER      DMA_MEMORY_TO_PERIPH
#define DMA_RX_TRANSFER      DMA_PERIPH_TO_MEMORY




// Default parameters to dma_init() shared by spi and i2c; Channel and Direction
// vary depending on the peripheral instance so they get passed separately
static const DMA_InitTypeDef dma_init_struct_spi_i2c = {
#if defined(MCU_SERIES_F4) || defined(MCU_SERIES_F7)
    .Channel             = 0,
#elif defined(MCU_SERIES_L4)
    .Request             = 0,
#endif
    .Direction           = 0,
    .PeriphInc           = DMA_PINC_DISABLE,
    .MemInc              = DMA_MINC_ENABLE,
    .PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
    .MemDataAlignment    = DMA_MDATAALIGN_BYTE,
    .Mode                = DMA_NORMAL,
    .Priority            = DMA_PRIORITY_LOW,
#if defined(MCU_SERIES_F4) || defined(MCU_SERIES_F7)
    .FIFOMode            = DMA_FIFOMODE_DISABLE,
    .FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL,
    .MemBurst            = DMA_MBURST_INC4,
    .PeriphBurst         = DMA_PBURST_INC4
#endif
};

#if defined(MICROPY_HW_HAS_SDCARD) && MICROPY_HW_HAS_SDCARD
// Parameters to dma_init() for SDIO tx and rx.
static const DMA_InitTypeDef dma_init_struct_sdio = {
#if defined(MCU_SERIES_F4) || defined(MCU_SERIES_F7)
    .Channel             = 0,
#elif defined(MCU_SERIES_L4)
    .Request             = 0,
#endif
    .Direction           = 0,
    .PeriphInc           = DMA_PINC_DISABLE,
    .MemInc              = DMA_MINC_ENABLE,
    .PeriphDataAlignment = DMA_PDATAALIGN_WORD,
    .MemDataAlignment    = DMA_MDATAALIGN_WORD,
#if defined(MCU_SERIES_F4) || defined(MCU_SERIES_F7)
    .Mode                = DMA_PFCTRL,
#elif defined(MCU_SERIES_L4)
    .Mode                = DMA_NORMAL,
#endif
    .Priority            = DMA_PRIORITY_VERY_HIGH,
#if defined(MCU_SERIES_F4) || defined(MCU_SERIES_F7)
    .FIFOMode            = DMA_FIFOMODE_ENABLE,
    .FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL,
    .MemBurst            = DMA_MBURST_INC4,
    .PeriphBurst         = DMA_PBURST_INC4,
#endif
};
#endif


#if defined(MCU_SERIES_F4) || defined(MCU_SERIES_F7)
/*
 * Configuration for STM32F4xx series
 */

// These are ordered by DMAx_Stream number, and within a stream by channel
// number. The duplicate streams are ok as long as they aren't used at the
// same time.
//
// Currently I2C and SPI are synchronous and they call dma_init/dma_deinit
// around each transfer.
static const dma_p2dma_t dma_transfer_info[]= {
    // DMA1 streams
    { { dma_I2C,  1, DMA_RX_TRANSFER}, DMA1_Stream0, DMA_CHANNEL_1, dma_id_0  , &dma_init_struct_spi_i2c},
    { { dma_SPI,  3, DMA_RX_TRANSFER}, DMA1_Stream2, DMA_CHANNEL_0, dma_id_2  , &dma_init_struct_spi_i2c},
    { { dma_I2C,  3, DMA_RX_TRANSFER}, DMA1_Stream2, DMA_CHANNEL_3, dma_id_2  , &dma_init_struct_spi_i2c},
    { { dma_I2C,  2, DMA_RX_TRANSFER}, DMA1_Stream2, DMA_CHANNEL_7, dma_id_2  , &dma_init_struct_spi_i2c},
    { { dma_SPI,  2, DMA_RX_TRANSFER}, DMA1_Stream3, DMA_CHANNEL_0, dma_id_3  , &dma_init_struct_spi_i2c},
    { { dma_SPI,  2, DMA_TX_TRANSFER}, DMA1_Stream4, DMA_CHANNEL_0, dma_id_4  , &dma_init_struct_spi_i2c},
    { { dma_I2C,  3, DMA_TX_TRANSFER}, DMA1_Stream4, DMA_CHANNEL_3, dma_id_4  , &dma_init_struct_spi_i2c},
    { { dma_DAC,  1, DMA_TX_TRANSFER}, DMA1_Stream5, DMA_CHANNEL_7, dma_id_5  , NULL},
    { { dma_DAC,  2, DMA_TX_TRANSFER}, DMA1_Stream6, DMA_CHANNEL_7, dma_id_6  , NULL},
    { { dma_SPI,  3, DMA_TX_TRANSFER}, DMA1_Stream7, DMA_CHANNEL_0, dma_id_7  , &dma_init_struct_spi_i2c},
    { { dma_I2C,  1, DMA_TX_TRANSFER}, DMA1_Stream7, DMA_CHANNEL_1, dma_id_7  , &dma_init_struct_spi_i2c},
    { { dma_I2C,  2, DMA_TX_TRANSFER}, DMA1_Stream7, DMA_CHANNEL_7, dma_id_7  , &dma_init_struct_spi_i2c},
    /* Not prefered streams on DMA1
    { { dma_SPI,  3, DMA_RX_TRANSFER}, DMA1_Stream0, DMA_CHANNEL_0, dma_id_0  , &dma_init_struct_spi_i2c},
    { { dma_I2C,  1, DMA_TX_TRANSFER}, DMA1_Stream6, DMA_CHANNEL_1, dma_id_6  , &dma_init_struct_spi_i2c}, */
    // DMA2 streams
    { { dma_SPI,  1, DMA_RX_TRANSFER}, DMA2_Stream2, DMA_CHANNEL_3, dma_id_10 , &dma_init_struct_spi_i2c},
    { { dma_SPI,  5, DMA_RX_TRANSFER}, DMA2_Stream3, DMA_CHANNEL_2, dma_id_11 , &dma_init_struct_spi_i2c},
    { { dma_SPI,  1, DMA_TX_TRANSFER}, DMA2_Stream3, DMA_CHANNEL_3, dma_id_11 , &dma_init_struct_spi_i2c},
#if defined(MICROPY_HW_HAS_SDCARD) && MICROPY_HW_HAS_SDCARD
    { { dma_SDIO, 0, DMA_RX_TRANSFER}, DMA2_Stream3, DMA_CHANNEL_4, dma_id_11 , &dma_init_struct_sdio},
#endif
    { { dma_SPI,  4, DMA_RX_TRANSFER}, DMA2_Stream3, DMA_CHANNEL_5, dma_id_11 , &dma_init_struct_spi_i2c},
    { { dma_SPI,  5, DMA_TX_TRANSFER}, DMA2_Stream4, DMA_CHANNEL_2, dma_id_12 , &dma_init_struct_spi_i2c},
    { { dma_SPI,  4, DMA_TX_TRANSFER}, DMA2_Stream4, DMA_CHANNEL_5, dma_id_12 , &dma_init_struct_spi_i2c},
    { { dma_SPI,  6, DMA_TX_TRANSFER}, DMA2_Stream5, DMA_CHANNEL_1, dma_id_13 , &dma_init_struct_spi_i2c},
    { { dma_SPI,  1, DMA_TX_TRANSFER}, DMA2_Stream5, DMA_CHANNEL_3, dma_id_13 , &dma_init_struct_spi_i2c},
    { { dma_SPI,  6, DMA_RX_TRANSFER}, DMA2_Stream6, DMA_CHANNEL_1, dma_id_14 , &dma_init_struct_spi_i2c},
#if defined(MICROPY_HW_HAS_SDCARD) && MICROPY_HW_HAS_SDCARD
    { { dma_SDIO, 0, DMA_TX_TRANSFER}, DMA2_Stream6, DMA_CHANNEL_4, dma_id_14 , &dma_init_struct_sdio}
#endif
    /* Not prefered streams on DMA2
    { { dma_SPI,  1, DMA_RX_TRANSFER}, DMA2_Stream0, DMA_CHANNEL_3, dma_id_8  , &dma_init_struct_spi_i2c},
    { { dma_SPI,  4, DMA_RX_TRANSFER}, DMA2_Stream0, DMA_CHANNEL_4, dma_id_8  , &dma_init_struct_spi_i2c},
    { { dma_SPI,  4, DMA_TX_TRANSFER}, DMA2_Stream1, DMA_CHANNEL_4, dma_id_9  , &dma_init_struct_spi_i2c},
    { { dma_SPI,  5, DMA_RX_TRANSFER}, DMA2_Stream5, DMA_CHANNEL_7, dma_id_13 , &dma_init_struct_spi_i2c},
    { { dma_SPI,  5, DMA_TX_TRANSFER}, DMA2_Stream6, DMA_CHANNEL_7, dma_id_14 , &dma_init_struct_spi_i2c}, */
};

#define NSTREAMS_PER_CONTROLLER (1 << NSTREAMS_PER_CONTROLLER_LOG2)
#define NSTREAM                 (NCONTROLLERS * NSTREAMS_PER_CONTROLLER)
static const uint8_t dma_irqn[NSTREAM] = {
    DMA1_Stream0_IRQn,
    DMA1_Stream1_IRQn,
    DMA1_Stream2_IRQn,
    DMA1_Stream3_IRQn,
    DMA1_Stream4_IRQn,
    DMA1_Stream5_IRQn,
    DMA1_Stream6_IRQn,
    DMA1_Stream7_IRQn,
    DMA2_Stream0_IRQn,
    DMA2_Stream1_IRQn,
    DMA2_Stream2_IRQn,
    DMA2_Stream3_IRQn,
    DMA2_Stream4_IRQn,
    DMA2_Stream5_IRQn,
    DMA2_Stream6_IRQn,
    DMA2_Stream7_IRQn,
};
#elif defined(MCU_SERIES_L4)
// These are ordered by DMAx_Channel number, and within a channel by request
// number. The duplicate streams are ok as long as they aren't used at the
// same time.
static const dma_p2dma_t dma_transfer_info[]= {
    // DMA1 streams
    { { dma_ADC,  1, DMA_RX_TRANSFER}, DMA1_Channel1, DMA_REQUEST_0, dma_id_0  , NULL},
    { { dma_ADC,  2, DMA_RX_TRANSFER}, DMA1_Channel2, DMA_REQUEST_0, dma_id_1  , NULL},
    { { dma_SPI,  1, DMA_RX_TRANSFER}, DMA1_Channel2, DMA_REQUEST_1, dma_id_1  , &dma_init_struct_spi_i2c},
    { { dma_I2C,  3, DMA_TX_TRANSFER}, DMA1_Channel2, DMA_REQUEST_3, dma_id_1  , &dma_init_struct_spi_i2c},
    { { dma_ADC,  3, DMA_RX_TRANSFER}, DMA1_Channel3, DMA_REQUEST_0, dma_id_2  , NULL},
    { { dma_SPI,  1, DMA_TX_TRANSFER}, DMA1_Channel3, DMA_REQUEST_1, dma_id_2  , &dma_init_struct_spi_i2c},
    { { dma_I2C,  3, DMA_RX_TRANSFER}, DMA1_Channel3, DMA_REQUEST_3, dma_id_2  , &dma_init_struct_spi_i2c},
    { { dma_DAC,  1, DMA_TX_TRANSFER}, DMA1_Channel3, DMA_REQUEST_6, dma_id_2  , NULL},
    { { dma_SPI,  2, DMA_RX_TRANSFER}, DMA1_Channel4, DMA_REQUEST_1, dma_id_3  , &dma_init_struct_spi_i2c},
    { { dma_I2C,  2, DMA_TX_TRANSFER}, DMA1_Channel4, DMA_REQUEST_3, dma_id_3  , &dma_init_struct_spi_i2c},
    { { dma_DAC,  2, DMA_TX_TRANSFER}, DMA1_Channel4, DMA_REQUEST_5, dma_id_3  , NULL},
    { { dma_SPI,  2, DMA_TX_TRANSFER}, DMA1_Channel5, DMA_REQUEST_1, dma_id_4  , &dma_init_struct_spi_i2c},
    { { dma_I2C,  2, DMA_RX_TRANSFER}, DMA1_Channel5, DMA_REQUEST_3, dma_id_4  , &dma_init_struct_spi_i2c},
    { { dma_I2C,  1, DMA_TX_TRANSFER}, DMA1_Channel6, DMA_REQUEST_3, dma_id_5  , &dma_init_struct_spi_i2c},
    { { dma_I2C,  1, DMA_RX_TRANSFER}, DMA1_Channel7, DMA_REQUEST_3, dma_id_6  , &dma_init_struct_spi_i2c},
    // DMA2 streams
    { { dma_SPI,  3, DMA_RX_TRANSFER}, DMA2_Channel1, DMA_REQUEST_3, dma_id_7  , &dma_init_struct_spi_i2c},
    { { dma_SPI,  3, DMA_TX_TRANSFER}, DMA2_Channel2, DMA_REQUEST_3, dma_id_8  , &dma_init_struct_spi_i2c},
    { { dma_ADC,  1, DMA_RX_TRANSFER}, DMA2_Channel3, DMA_REQUEST_0, dma_id_9  , NULL},
    { { dma_SPI,  1, DMA_RX_TRANSFER}, DMA2_Channel3, DMA_REQUEST_4, dma_id_9  , &dma_init_struct_spi_i2c},
    { { dma_ADC,  2, DMA_RX_TRANSFER}, DMA2_Channel4, DMA_REQUEST_0, dma_id_10 , NULL},
    { { dma_DAC,  1, DMA_TX_TRANSFER}, DMA2_Channel4, DMA_REQUEST_3, dma_id_10 , NULL},
    { { dma_SPI,  1, DMA_TX_TRANSFER}, DMA2_Channel4, DMA_REQUEST_4, dma_id_10 , &dma_init_struct_spi_i2c},
#if MICROPY_HW_HAS_SDCARD
    { { dma_SDIO, 1, DMA_TX_TRANSFER}, DMA2_Channel4, DMA_REQUEST_7, dma_id_10 , &dma_init_struct_sdio}
#endif
    { { dma_ADC,  3, DMA_RX_TRANSFER}, DMA2_Channel5, DMA_REQUEST_0, dma_id_11 , NULL},
    { { dma_DAC,  2, DMA_TX_TRANSFER}, DMA2_Channel5, DMA_REQUEST_3, dma_id_11 , NULL},
#if MICROPY_HW_HAS_SDCARD
    { { dma_SDIO, 1, DMA_TX_TRANSFER}, DMA2_Channel5, DMA_REQUEST_7, dma_id_11 , &dma_init_struct_sdio}
#endif
    { { dma_I2C,  1, DMA_RX_TRANSFER}, DMA2_Channel6, DMA_REQUEST_5, dma_id_12 , &dma_init_struct_spi_i2c},
    { { dma_I2C,  1, DMA_TX_TRANSFER}, DMA2_Channel7, DMA_REQUEST_5, dma_id_13 , &dma_init_struct_spi_i2c}
};

#define NSTREAMS_PER_CONTROLLER ((1 << NSTREAMS_PER_CONTROLLER_LOG2)-1)
#define NSTREAM                 (NCONTROLLERS * NSTREAMS_PER_CONTROLLER)
static const uint8_t dma_irqn[NSTREAM] = {
    DMA1_Channel1_IRQn,
    DMA1_Channel2_IRQn,
    DMA1_Channel3_IRQn,
    DMA1_Channel4_IRQn,
    DMA1_Channel5_IRQn,
    DMA1_Channel6_IRQn,
    DMA1_Channel7_IRQn,
    DMA2_Channel1_IRQn,
    DMA2_Channel2_IRQn,
    DMA2_Channel3_IRQn,
    DMA2_Channel4_IRQn,
    DMA2_Channel5_IRQn,
    DMA2_Channel6_IRQn,
    DMA2_Channel7_IRQn,
};
#endif

static DMA_HandleTypeDef *dma_handle[NSTREAM] = {NULL};
static uint8_t  dma_last_subidx[NSTREAM];
static volatile uint32_t dma_enable_mask = 0;

#if defined(MCU_SERIES_F4) || defined(MCU_SERIES_F7)
#define DMA_CHANNEL_AS_UINT8(dma_channel)   (((dma_channel) & DMA_SxCR_CHSEL) >> 24)

void DMA1_Stream0_IRQHandler(void) { IRQ_ENTER(DMA1_Stream0_IRQn); if (dma_handle[0] != NULL) { HAL_DMA_IRQHandler(dma_handle[0]); } IRQ_EXIT(DMA1_Stream0_IRQn); }
void DMA1_Stream1_IRQHandler(void) { IRQ_ENTER(DMA1_Stream1_IRQn); if (dma_handle[1] != NULL) { HAL_DMA_IRQHandler(dma_handle[1]); } IRQ_EXIT(DMA1_Stream1_IRQn); }
void DMA1_Stream2_IRQHandler(void) { IRQ_ENTER(DMA1_Stream2_IRQn); if (dma_handle[2] != NULL) { HAL_DMA_IRQHandler(dma_handle[2]); } IRQ_EXIT(DMA1_Stream2_IRQn); }
void DMA1_Stream3_IRQHandler(void) { IRQ_ENTER(DMA1_Stream3_IRQn); if (dma_handle[3] != NULL) { HAL_DMA_IRQHandler(dma_handle[3]); } IRQ_EXIT(DMA1_Stream3_IRQn); }
void DMA1_Stream4_IRQHandler(void) { IRQ_ENTER(DMA1_Stream4_IRQn); if (dma_handle[4] != NULL) { HAL_DMA_IRQHandler(dma_handle[4]); } IRQ_EXIT(DMA1_Stream4_IRQn); }
void DMA1_Stream5_IRQHandler(void) { IRQ_ENTER(DMA1_Stream5_IRQn); if (dma_handle[5] != NULL) { HAL_DMA_IRQHandler(dma_handle[5]); } IRQ_EXIT(DMA1_Stream5_IRQn); }
void DMA1_Stream6_IRQHandler(void) { IRQ_ENTER(DMA1_Stream6_IRQn); if (dma_handle[6] != NULL) { HAL_DMA_IRQHandler(dma_handle[6]); } IRQ_EXIT(DMA1_Stream6_IRQn); }
void DMA1_Stream7_IRQHandler(void) { IRQ_ENTER(DMA1_Stream7_IRQn); if (dma_handle[7] != NULL) { HAL_DMA_IRQHandler(dma_handle[7]); } IRQ_EXIT(DMA1_Stream7_IRQn); }
void DMA2_Stream0_IRQHandler(void) { IRQ_ENTER(DMA2_Stream0_IRQn); if (dma_handle[8] != NULL) { HAL_DMA_IRQHandler(dma_handle[8]); } IRQ_EXIT(DMA2_Stream0_IRQn); }
void DMA2_Stream1_IRQHandler(void) { IRQ_ENTER(DMA2_Stream1_IRQn); if (dma_handle[9] != NULL) { HAL_DMA_IRQHandler(dma_handle[9]); } IRQ_EXIT(DMA2_Stream1_IRQn); }
void DMA2_Stream2_IRQHandler(void) { IRQ_ENTER(DMA2_Stream2_IRQn); if (dma_handle[10] != NULL) { HAL_DMA_IRQHandler(dma_handle[10]); } IRQ_EXIT(DMA2_Stream2_IRQn); }
void DMA2_Stream3_IRQHandler(void) { IRQ_ENTER(DMA2_Stream3_IRQn); if (dma_handle[11] != NULL) { HAL_DMA_IRQHandler(dma_handle[11]); } IRQ_EXIT(DMA2_Stream3_IRQn); }
void DMA2_Stream4_IRQHandler(void) { IRQ_ENTER(DMA2_Stream4_IRQn); if (dma_handle[12] != NULL) { HAL_DMA_IRQHandler(dma_handle[12]); } IRQ_EXIT(DMA2_Stream4_IRQn); }
void DMA2_Stream5_IRQHandler(void) { IRQ_ENTER(DMA2_Stream5_IRQn); if (dma_handle[13] != NULL) { HAL_DMA_IRQHandler(dma_handle[13]); } IRQ_EXIT(DMA2_Stream5_IRQn); }
void DMA2_Stream6_IRQHandler(void) { IRQ_ENTER(DMA2_Stream6_IRQn); if (dma_handle[14] != NULL) { HAL_DMA_IRQHandler(dma_handle[14]); } IRQ_EXIT(DMA2_Stream6_IRQn); }
void DMA2_Stream7_IRQHandler(void) { IRQ_ENTER(DMA2_Stream7_IRQn); if (dma_handle[15] != NULL) { HAL_DMA_IRQHandler(dma_handle[15]); } IRQ_EXIT(DMA2_Stream7_IRQn); }

#elif defined(MCU_SERIES_L4)
#define DMA_CHANNEL_AS_UINT8(dma_id)    (dma_id % NSTREAMS_PER_CONTROLLER))

void DMA1_Channel1_IRQHandler(void) { IRQ_ENTER(DMA1_Channel1_IRQn); if (dma_handle[0] != NULL)  { HAL_DMA_IRQHandler(dma_handle[0]); } IRQ_EXIT(DMA1_Channel1_IRQn); }
void DMA1_Channel2_IRQHandler(void) { IRQ_ENTER(DMA1_Channel2_IRQn); if (dma_handle[1] != NULL)  { HAL_DMA_IRQHandler(dma_handle[1]); } IRQ_EXIT(DMA1_Channel2_IRQn); }
void DMA1_Channel3_IRQHandler(void) { IRQ_ENTER(DMA1_Channel3_IRQn); if (dma_handle[2] != NULL)  { HAL_DMA_IRQHandler(dma_handle[2]); } IRQ_EXIT(DMA1_Channel3_IRQn); }
void DMA1_Channel4_IRQHandler(void) { IRQ_ENTER(DMA1_Channel4_IRQn); if (dma_handle[3] != NULL)  { HAL_DMA_IRQHandler(dma_handle[3]); } IRQ_EXIT(DMA1_Channel4_IRQn); }
void DMA1_Channel5_IRQHandler(void) { IRQ_ENTER(DMA1_Channel5_IRQn); if (dma_handle[4] != NULL)  { HAL_DMA_IRQHandler(dma_handle[4]); } IRQ_EXIT(DMA1_Channel5_IRQn); }
void DMA1_Channel6_IRQHandler(void) { IRQ_ENTER(DMA1_Channel6_IRQn); if (dma_handle[5] != NULL)  { HAL_DMA_IRQHandler(dma_handle[5]); } IRQ_EXIT(DMA1_Channel6_IRQn); }
void DMA1_Channel7_IRQHandler(void) { IRQ_ENTER(DMA1_Channel7_IRQn); if (dma_handle[6] != NULL)  { HAL_DMA_IRQHandler(dma_handle[6]); } IRQ_EXIT(DMA1_Channel7_IRQn); }
void DMA2_Channel1_IRQHandler(void) { IRQ_ENTER(DMA2_Channel1_IRQn); if (dma_handle[7] != NULL)  { HAL_DMA_IRQHandler(dma_handle[7]); } IRQ_EXIT(DMA2_Channel1_IRQn); }
void DMA2_Channel2_IRQHandler(void) { IRQ_ENTER(DMA2_Channel2_IRQn); if (dma_handle[8] != NULL)  { HAL_DMA_IRQHandler(dma_handle[8]); } IRQ_EXIT(DMA2_Channel2_IRQn); }
void DMA2_Channel3_IRQHandler(void) { IRQ_ENTER(DMA2_Channel3_IRQn); if (dma_handle[9] != NULL)  { HAL_DMA_IRQHandler(dma_handle[9]); } IRQ_EXIT(DMA2_Channel3_IRQn); }
void DMA2_Channel4_IRQHandler(void) { IRQ_ENTER(DMA2_Channel4_IRQn); if (dma_handle[10] != NULL) { HAL_DMA_IRQHandler(dma_handle[10]);} IRQ_EXIT(DMA2_Channel4_IRQn); }
void DMA2_Channel5_IRQHandler(void) { IRQ_ENTER(DMA2_Channel5_IRQn); if (dma_handle[11] != NULL) { HAL_DMA_IRQHandler(dma_handle[11]);} IRQ_EXIT(DMA2_Channel5_IRQn); }
void DMA2_Channel6_IRQHandler(void) { IRQ_ENTER(DMA2_Channel6_IRQn); if (dma_handle[12] != NULL) { HAL_DMA_IRQHandler(dma_handle[12]);} IRQ_EXIT(DMA2_Channel6_IRQn); }
void DMA2_Channel7_IRQHandler(void) { IRQ_ENTER(DMA2_Channel7_IRQn); if (dma_handle[13] != NULL) { HAL_DMA_IRQHandler(dma_handle[13]);} IRQ_EXIT(DMA2_Channel7_IRQn); }
#endif

volatile dma_idle_count_t dma_idle;

#define DMA1_ENABLE_MASK    0x00ff  // Bits in dma_enable_mask corresponfing to DMA1
#define DMA2_ENABLE_MASK    0xff00  // Bits in dma_enable_mask corresponding to DMA2
#define DMA_INVALID_CHANNEL 0xff    // Value stored in dma_last_channel which means invalid

#define DMA1_IS_CLK_ENABLED()   ((RCC->AHB1ENR & RCC_AHB1ENR_DMA1EN) != 0)
#define DMA2_IS_CLK_ENABLED()   ((RCC->AHB1ENR & RCC_AHB1ENR_DMA2EN) != 0)

static dma_id_t dma_get_id(void *instance) {
    dma_id_t dma_id=dma_id_not_defined;
    int idx=0;
    for (idx = 0; idx<MP_ARRAY_SIZE(dma_transfer_info);idx++)
    {
        if (dma_transfer_info[idx].instance == instance)
        {
            dma_id = dma_transfer_info[idx].id;
            break;
        }
    }
    return dma_id;
}

static int dma_get_entry(const dma_descr_t *dma_descr) {
    int dma_entry=-1;
    int idx=0;
    for (idx = 0; idx<MP_ARRAY_SIZE(dma_transfer_info);idx++)
    {
        if ((dma_transfer_info[idx].dma_descr.periphery_type == dma_descr->periphery_type) &&
            (dma_transfer_info[idx].dma_descr.periphery_inst_nr == dma_descr->periphery_inst_nr) &&
            (dma_transfer_info[idx].dma_descr.transfer_direction == dma_descr->transfer_direction) )
        {
            dma_entry = idx;
            break;
        }
    }
    return dma_entry;
}

// Resets the idle counter for the DMA controller associated with dma_id.
static void dma_tickle(dma_id_t dma_id) {
    dma_idle.counter[(dma_id >> NSTREAMS_PER_CONTROLLER_LOG2) & 1] = 1;
}

static void dma_enable_clock(dma_id_t dma_id) {
    // We don't want dma_tick_handler() to turn off the clock right after we
    // enable it, so we need to mark the channel in use in an atomic fashion.
    mp_uint_t irq_state = MICROPY_BEGIN_ATOMIC_SECTION();
    uint32_t old_enable_mask = dma_enable_mask;
    dma_enable_mask |= (1 << dma_id);
    MICROPY_END_ATOMIC_SECTION(irq_state);

    if (dma_id <= 7) {
        if (((old_enable_mask & DMA1_ENABLE_MASK) == 0) && !DMA1_IS_CLK_ENABLED()) {
            __DMA1_CLK_ENABLE();

            // We just turned on the clock. This means that anything stored
            // in dma_last_channel (for DMA1) needs to be invalidated.

            for (int channel = 0; channel < NSTREAMS_PER_CONTROLLER; channel++) {
                dma_last_subidx[channel] = DMA_INVALID_CHANNEL;
            }
        }
    } else {
        if (((old_enable_mask & DMA2_ENABLE_MASK) == 0) && !DMA2_IS_CLK_ENABLED()) {
            __DMA2_CLK_ENABLE();

            // We just turned on the clock. This means that anything stored
            // in dma_last_channel (for DMA1) needs to be invalidated.

            for (int channel = NSTREAMS_PER_CONTROLLER; channel < NSTREAM; channel++) {
                dma_last_subidx[channel] = DMA_INVALID_CHANNEL;
            }
        }
    }
}

static void dma_disable_clock(dma_id_t dma_id) {
    // We just mark the clock as disabled here, but we don't actually disable it.
    // We wait for the timer to expire first, which means that back-to-back
    // transfers don't have to initialize as much.
    dma_tickle(dma_id);
    dma_enable_mask &= ~(1 << dma_id);
}

int32_t dma_init_handle(DMA_HandleTypeDef *dma, dma_descr_t * dma_descr, void *data)
{
    int32_t dma_idx =  dma_get_entry(dma_descr);
    if (dma_idx >= 0) {
        // initialise parameters
        dma->Instance = dma_transfer_info[dma_idx].instance;
        dma->Init = *dma_transfer_info[dma_idx].init;
        dma->Init.Direction = dma_transfer_info[dma_idx].dma_descr.transfer_direction;
#if defined(MCU_SERIES_L4)
        dma->Init.Request = dma_transfer_info[dma_idx].sub_instance;
#else
        dma->Init.Channel = dma_transfer_info[dma_idx].sub_instance;
#endif
        // half of __HAL_LINKDMA(data, xxx, *dma)
        // caller must implement other half by doing: data->xxx = dma
        dma->Parent = data;
    } else {
        printf("No DMA with pType %u, pInstNr %u, tDir %u\n", (uint)dma_descr->periphery_type, (uint)dma_descr->periphery_inst_nr, (uint)dma_descr->transfer_direction );
    }

    return dma_idx;
}



void dma_init(DMA_HandleTypeDef *dma, dma_descr_t * dma_descr, void *data){
    int32_t dma_idx;
    //printf("dma_init(%p, %p(%d), 0x%x, 0x%x, %p)\n", dma, dma_stream, dma_id, (uint)dma_channel, (uint)direction, data);
    printf("dma_init( periphery_type=%u, periphery_inst_nr=%u, direction=%u)\n", (uint)dma_descr->periphery_type, (uint)dma_descr->periphery_inst_nr, (uint)dma_descr->transfer_direction );

    // Some drivers allocate the DMA_HandleTypeDef from the stack
    // (i.e. dac, i2c, spi) and for those cases we need to clear the
    // structure so we don't get random values from the stack)
    memset(dma, 0, sizeof(*dma));

    dma_idx = dma_init_handle(dma, dma_descr, data);
    if (dma_idx >= 0)
    {
        dma_id_t dma_id  = dma_transfer_info[dma_idx].id;

        // set global pointer for IRQ handler
        dma_handle[dma_id] = dma;

        dma_enable_clock(dma_id);

        // if this stream was previously configured for this channel then we
        // can skip most of the initialisation
        uint8_t subidx = DMA_CHANNEL_AS_UINT8(dma_id);
        if (dma_last_subidx[dma_id] != subidx) {
            dma_last_subidx[dma_id] = subidx;

            // reset and configure DMA peripheral
            if (HAL_DMA_GetState(dma) != HAL_DMA_STATE_RESET) {
                HAL_DMA_DeInit(dma);
            }
            HAL_DMA_Init(dma);
            HAL_NVIC_SetPriority(dma_irqn[dma_id], IRQ_PRI_DMA, IRQ_SUBPRI_DMA);
        }

        HAL_NVIC_EnableIRQ(dma_irqn[dma_id]);
    } else { printf("DMA not available.\n"); }

}

void dma_deinit(DMA_HandleTypeDef *dma) {
    dma_id_t dma_id = dma_get_id(dma->Instance);
    if (dma_id != dma_id_not_defined)
    {
        HAL_NVIC_DisableIRQ(dma_irqn[dma_id]);
        dma_handle[dma_id] = NULL;

        dma_disable_clock(dma_id);
    }
}

void dma_invalidate_channel(const dma_descr_t * dma_descr) {
    int dma_idx = dma_get_entry(dma_descr);
    if (dma_idx >= 0)
    {
        dma_id_t dma_id = dma_transfer_info[dma_idx].id;
        if (dma_last_subidx[dma_id] == DMA_CHANNEL_AS_UINT8(dma_id) ) {
            dma_last_subidx[dma_id] = DMA_INVALID_CHANNEL;
        }
    }
}

// Called from the SysTick handler
// We use LSB of tick to select which controller to process
void dma_idle_handler(int tick) {
    static const uint32_t   controller_mask[] = {
        DMA1_ENABLE_MASK, DMA2_ENABLE_MASK
    };
    {
        int controller = tick & 1;
        if (dma_idle.counter[controller] == 0) {
            return;
        }
        if (++dma_idle.counter[controller] > DMA_IDLE_TICK_MAX) {
            if ((dma_enable_mask & controller_mask[controller]) == 0) {
                // Nothing is active and we've reached our idle timeout,
                // Now we'll really disable the clock.
                dma_idle.counter[controller] = 0;
                if (controller == 0) {
                    __DMA1_CLK_DISABLE();
                } else {
                    __DMA2_CLK_DISABLE();
                }
            } else {
                // Something is still active, but the counter never got
                // reset, so we'll reset the counter here.
                dma_idle.counter[controller] = 1;
            }
        }
    }
}
