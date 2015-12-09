/*
 * u8glib_adapter.h
 *
 *  Created on: 09.12.2015
 *      Author: badi
 */

#ifndef STMHAL_U8GLIB_ADAPTER_H_
#define STMHAL_U8GLIB_ADAPTER_H_

#include "u8g.h"

void u8glib_init(void);
uint8_t u8glib_stm32f429_lcd_com_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr);


#endif /* STMHAL_U8GLIB_ADAPTER_H_ */
