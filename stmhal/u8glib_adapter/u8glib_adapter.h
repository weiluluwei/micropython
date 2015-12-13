/*
 * u8glib_adapter.h
 *
 *  Created on: 10.12.2015
 *      Author: badi
 */

#ifndef STMHAL_U8GLIB_ADAPTER_U8GLIB_ADAPTER_H_
#define STMHAL_U8GLIB_ADAPTER_U8GLIB_ADAPTER_H_
#include <stdint.h>

void u8g_Delay(uint16_t val);
void u8g_MicroDelay(void);
void u8g_10MicroDelay(void);

void u8glib_adapter_init(void);

#endif /* STMHAL_U8GLIB_ADAPTER_U8GLIB_ADAPTER_H_ */
