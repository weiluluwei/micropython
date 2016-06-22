/*
 * measure_cycles.c
 *
 *  Created on: 22.06.2016
 *      Author: badi
 */

#include STM32_HAL_H

void reset_timer(){
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
}

void start_timer(){
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void stop_timer(){
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
}

unsigned int getCycles(){
    return DWT->CYCCNT;
}
