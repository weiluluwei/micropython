/**
 * lcd_ctrl.h
 *
 * This file is a copy of stm32f429i_discovery_sdram.h from
 * STM32Cube_FW_F4_V1.7.0/Drivers/BSP/STM32F429I-Discovery/stm32f429i_discovery_lcd.h
 * function names were fixed with following sed command:
 * 's/\(BSP_LCD_\)\([A-Z]\)/lcd_ctrl_\l\2/g'
 *
 ******************************************************************************
 * @file    stm32f429i_discovery_lcd.h
 * @author  MCD Application Team
 * @version V2.1.2
 * @date    02-March-2015
 * @brief   This file contains all the functions prototypes for the
 *          stm32f429i_discovery_lcd.c driver.
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
#ifndef __LCDCTRL_H
#define __LCDCTRL_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "lcd_io.h"
/* Include SDRAM Driver */
#include "sdram.h"
#include "fonts.h"
/* Include LCD component driver */
#include "ili9341.h"

/** @defgroup lcdctrl_Exported_Types
    * @{
    */
typedef enum 
{
    LCD_OK = 0,
    LCD_ERROR = 1,
    LCD_TIMEOUT = 2
}LCD_StatusTypeDef;

typedef struct 
{ 
    uint32_t    TextColor; 
    uint32_t    BackColor;    
    sFONT         *pFont;
}LCD_DrawPropTypeDef;
     
typedef struct 
{
    int16_t X;
    int16_t Y;
} Point, * pPoint;     
     
/** 
* @brief    Line mode structures definition
*/
typedef enum
{
    CENTER_MODE = 0x01,        /* center mode */
    RIGHT_MODE  = 0x02,        /* right mode    */
    LEFT_MODE   = 0x03,        /* left mode     */
}Text_AlignModeTypdef;


/**
* @}
*/

/** @defgroup STM32F429I_DISCOVERY_LCD_Exported_Constants
    * @{
    */ 
#define LCD_LayerCfgTypeDef        LTDC_LayerCfgTypeDef

/** 
* @brief    LCD status structure definition
*/
#define MAX_LAYER_NUMBER          2
#define LCD_FRAME_BUFFER          ((uint32_t)0xD0000000)
#define BUFFER_OFFSET             ((uint32_t)0x50000)

/** 
* @brief    LCD color
*/
#define LCD_COLOR_BLUE            0xFF0000FF
#define LCD_COLOR_GREEN           0xFF00FF00
#define LCD_COLOR_RED             0xFFFF0000
#define LCD_COLOR_CYAN            0xFF00FFFF
#define LCD_COLOR_MAGENTA         0xFFFF00FF
#define LCD_COLOR_YELLOW          0xFFFFFF00
#define LCD_COLOR_LIGHTBLUE       0xFF8080FF
#define LCD_COLOR_LIGHTGREEN      0xFF80FF80
#define LCD_COLOR_LIGHTRED        0xFFFF8080
#define LCD_COLOR_LIGHTCYAN       0xFF80FFFF
#define LCD_COLOR_LIGHTMAGENTA    0xFFFF80FF
#define LCD_COLOR_LIGHTYELLOW     0xFFFFFF80
#define LCD_COLOR_DARKBLUE        0xFF000080
#define LCD_COLOR_DARKGREEN       0xFF008000
#define LCD_COLOR_DARKRED         0xFF800000
#define LCD_COLOR_DARKCYAN        0xFF008080
#define LCD_COLOR_DARKMAGENTA     0xFF800080
#define LCD_COLOR_DARKYELLOW      0xFF808000
#define LCD_COLOR_WHITE           0xFFFFFFFF
#define LCD_COLOR_LIGHTGRAY       0xFFD3D3D3
#define LCD_COLOR_GRAY            0xFF808080
#define LCD_COLOR_DARKGRAY        0xFF404040
#define LCD_COLOR_BLACK           0xFF000000
#define LCD_COLOR_BROWN           0xFFA52A2A
#define LCD_COLOR_ORANGE          0xFFFFA500
#define LCD_COLOR_TRANSPARENT     0xFF000000
/** 
    * @brief LCD default font 
    */ 
#define LCD_DEFAULT_FONT          Font24

/** 
    * @brief    LCD Layer    
    */ 
#define LCD_BACKGROUND_LAYER      0x0000
#define LCD_FOREGROUND_LAYER      0x0001

/**
    * @}
    */ 

/** @defgroup STM32F429I_DISCOVERY_LCD_Exported_Macros
    * @{
    */ 
/** 
    * @brief LCD Pixel format 
    */    
#define LCD_PIXEL_FORMAT_ARGB8888  LTDC_PIXEL_FORMAT_ARGB8888
#define LCD_PIXEL_FORMAT_RGB888    LTDC_PIXEL_FORMAT_RGB888
#define LCD_PIXEL_FORMAT_RGB565    LTDC_PIXEL_FORMAT_RGB565
#define LCD_PIXEL_FORMAT_ARGB1555  LTDC_PIXEL_FORMAT_ARGB1555
#define LCD_PIXEL_FORMAT_ARGB4444  LTDC_PIXEL_FORMAT_ARGB4444
#define LCD_PIXEL_FORMAT_L8        LTDC_PIXEL_FORMAT_L8
#define LCD_PIXEL_FORMAT_AL44      LTDC_PIXEL_FORMAT_AL44
#define LCD_PIXEL_FORMAT_AL88      LTDC_PIXEL_FORMAT_AL88
/**
    * @}
    */ 

/** @defgroup STM32F429I_DISCOVERY_LCD_Exported_Functions
    * @{
    */ 
uint8_t  lcd_ctrl_init(void);
uint32_t lcd_ctrl_getXSize(void);
uint32_t lcd_ctrl_getYSize(void);

/* functions using the LTDC controller */
void     lcd_ctrl_layerDefaultInit(uint16_t LayerIndex, uint32_t FrameBuffer);
void     lcd_ctrl_setTransparency(uint32_t LayerIndex, uint8_t Transparency);
void     lcd_ctrl_setLayerAddress(uint32_t LayerIndex, uint32_t Address);
void     lcd_ctrl_setColorKeying(uint32_t LayerIndex, uint32_t RGBValue);
void     lcd_ctrl_resetColorKeying(uint32_t LayerIndex);
void     lcd_ctrl_setLayerWindow(uint16_t LayerIndex, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void     lcd_ctrl_selectLayer(uint32_t LayerIndex);
void     lcd_ctrl_setLayerVisible(uint32_t LayerIndex, FunctionalState state);

void     lcd_ctrl_setTextColor(uint32_t Color);
void     lcd_ctrl_setBackColor(uint32_t Color);
uint32_t lcd_ctrl_getTextColor(void);
uint32_t lcd_ctrl_getBackColor(void);
void     lcd_ctrl_setFont(sFONT *pFonts);
sFONT    *lcd_ctrl_getFont(void);

uint32_t lcd_ctrl_readPixel(uint16_t Xpos, uint16_t Ypos);
void     lcd_ctrl_drawPixel(uint16_t Xpos, uint16_t Ypos, uint32_t pixel);
void     lcd_ctrl_clear(uint32_t Color);
void     lcd_ctrl_clearStringLine(uint32_t Line);
void     lcd_ctrl_displayStringAtLine(uint16_t Line, uint8_t *ptr);
void     lcd_ctrl_displayStringAt(uint16_t X, uint16_t Y, const uint8_t *pText, Text_AlignModeTypdef mode);
void     lcd_ctrl_displayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii);

void     lcd_ctrl_drawHLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void     lcd_ctrl_drawVLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void     lcd_ctrl_drawLine(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2);
void     lcd_ctrl_drawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void     lcd_ctrl_drawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius);
void     lcd_ctrl_drawPolygon(pPoint Points, uint16_t PointCount);
void     lcd_ctrl_drawEllipse(int Xpos, int Ypos, int XRadius, int YRadius);
void     lcd_ctrl_drawBitmap(uint32_t X, uint32_t Y, uint8_t *pBmp);

void     lcd_ctrl_fillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void     lcd_ctrl_fillCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius);
void     lcd_ctrl_fillTriangle(uint16_t X1, uint16_t X2, uint16_t X3, uint16_t Y1, uint16_t Y2, uint16_t Y3);
void     lcd_ctrl_fillPolygon(pPoint Points, uint16_t PointCount);
void     lcd_ctrl_fillEllipse(int Xpos, int Ypos, int XRadius, int YRadius);

void     lcd_ctrl_displayOff(void);
void     lcd_ctrl_displayOn(void);

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

#endif /* __LCDCTRL_H__LCDCTRL_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
