/*
 * u8glib_adapter.c
 *
 *  Created on: 09.12.2015
 *      Author: badi
 */
#include <stdint.h>
#include "u8g.h"

u8g_t u8g;
#define WIDTH 320
#define HEIGHT 240
#define PAGE_HEIGHT 8


/*========================================================================*/
/*
  The following delay procedures must be implemented for u8glib

  void u8g_Delay(uint16_t val)      Delay by "val" milliseconds
  void u8g_MicroDelay(void)     Delay be one microsecond
  void u8g_10MicroDelay(void)   Delay by 10 microseconds

*/

void u8g_Delay(uint16_t val)
{
    delay_micro_seconds(1000UL*(uint32_t)val);
}

void u8g_MicroDelay(void)
{
    delay_micro_seconds(1);
}

void u8g_10MicroDelay(void)
{
    delay_micro_seconds(10);
}

uint8_t u8glib_stm32f429_lcd_com_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr)
{
    switch(msg)
    {
        case U8G_COM_MSG_STOP:
        break;

        case U8G_COM_MSG_INIT:
        break;

        case U8G_COM_MSG_ADDRESS:
        break;

        case U8G_COM_MSG_CHIP_SELECT:
        break;

        case U8G_COM_MSG_RESET:
        break;

        case U8G_COM_MSG_WRITE_BYTE:
        break;

        case U8G_COM_MSG_WRITE_SEQ:
        case U8G_COM_MSG_WRITE_SEQ_P:
        break;
    }
    return 1;
}


uint8_t u8g_dev_stm32f429_lcd_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg)
{
    switch(msg)
    {
        case U8G_DEV_MSG_INIT:
        break;

        case U8G_DEV_MSG_STOP:
        break;

        case U8G_DEV_MSG_PAGE_NEXT:
        break;

        case U8G_DEV_MSG_CONTRAST:
        break;

        case U8G_DEV_MSG_SLEEP_ON:
        return 1;

        case U8G_DEV_MSG_SLEEP_OFF:
        return 1;
    }
    return u8g_dev_pb8v1_base_fn(u8g, dev, msg, arg);
}

U8G_PB_DEV(u8g_dev_stm32f429_lcd_hw, WIDTH, HEIGHT, PAGE_HEIGHT, u8g_dev_stm32f429_lcd_fn, U8G_COM_SW_SPI);

void u8glib_init(void)
{
  u8g_InitComFn(&u8g, &u8g_dev_stm32f429_lcd_hw, u8glib_stm32f429_lcd_com_fn);
}
