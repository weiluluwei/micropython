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
typedef enum {
    dma_NONE,
    dma_SPI,
    dma_QUADSPI,
    dma_I2C,
    dma_DAC,
    dma_ADC,
    dma_SDIO,
} periphery_t;

typedef struct _dmaDescr
{
    periphery_t     periphery_type;     /* Periphery type */
    uint32_t        periphery_inst_nr;  /* Instance of Periphery */
    uint32_t        transfer_direction; /* Transfer direction Periphery to memory or vis-versa */
} dma_descr_t;

#define DMA_TX_TRANSFER      DMA_MEMORY_TO_PERIPH
#define DMA_RX_TRANSFER      DMA_PERIPH_TO_MEMORY

typedef union {
    uint16_t    enabled;    // Used to test if both counters are == 0
    uint8_t     counter[2];
} dma_idle_count_t;
extern volatile dma_idle_count_t dma_idle;
#define DMA_IDLE_ENABLED()  (dma_idle.enabled != 0)

#define DMA_SYSTICK_MASK            0x0e
#define DMA_MSECS_PER_SYSTICK       (DMA_SYSTICK_MASK + 1)
#define DMA_IDLE_TICK_MAX           (8)     // 128 msec
#define DMA_IDLE_TICK(tick)         (((tick) & DMA_SYSTICK_MASK) == 0)

void dma_init(DMA_HandleTypeDef *dma, dma_descr_t * dma_descr, void *data);
int32_t  dma_init_handle(DMA_HandleTypeDef *dma, dma_descr_t * dma_descr, void *data);
void dma_deinit(DMA_HandleTypeDef *dma);
void dma_invalidate_channel(const dma_descr_t * dma_descr);
void dma_idle_handler(int controller);
