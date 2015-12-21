/**
 * This file is a copy of lcd_log.c from
 * STM32Cube_FW_F4_V1.7.0/Utilities/Log/lcd_log.c
 * function names were fixed with following sed command:
 * 's/\(LCD_LOG_\)\([A-Z]\)/lcd_log_\l\2/g'
 ******************************************************************************
 * @file        lcd_log.c
 * @author    MCD Application Team
 * @version V1.0.0
 * @date        18-February-2014
 * @brief     This file provides all the LCD Log firmware functions.
 *
 *                    The LCD Log module allows to automatically set a header and footer
 *                    on any application using the LCD display and allows to dump user,
 *                    debug and error messages by using the following macros: LCD_ErrLog(),
 *                    LCD_UsrLog() and LCD_DbgLog().
 *
 *                    It supports also the scroll feature by embedding an internal software
 *                    cache for display. This feature allows to dump message sequentially
 *                    on the display even if the number of displayed lines is bigger than
 *                    the total number of line allowed by the display.
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
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

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx.h"
#include "lcd_drv.h"
#include "lcd_log.h"

/** @addtogroup Utilities
 * @{
 */

/** @addtogroup STM32_EVAL
 * @{
 */

/** @addtogroup Common
 * @{
 */

/** @defgroup LCD_LOG 
 * @brief LCD Log LCD_Application module
 * @{
 */ 

/** @defgroup lcd_log_private_Types
 * @{
 */ 
/**
 * @}
 */ 


/** @defgroup lcd_log_private_Defines
 * @{
 */ 

/**
 * @}
 */ 

/* Define the display window settings */
#define         YWINDOW_MIN                 (LCD_LOG_HEADER_SIZE)

/** @defgroup lcd_log_private_Macros
 * @{
 */ 
/**
 * @}
 */ 


/** @defgroup lcd_log_private_Variables
 * @{
 */ 
bool LCD_is_initialized = false;
LCD_LOG_line LCD_CacheBuffer [LCD_CACHE_DEPTH]; 
uint32_t LCD_LineColor;
uint16_t LCD_CacheBuffer_xptr;
uint16_t LCD_CacheBuffer_yptr_top;
uint16_t LCD_CacheBuffer_yptr_bottom;

uint16_t LCD_CacheBuffer_yptr_top_bak;
uint16_t LCD_CacheBuffer_yptr_bottom_bak;

FunctionalState LCD_CacheBuffer_yptr_invert;
FunctionalState LCD_ScrollActive;
FunctionalState LCD_Lock;
FunctionalState LCD_Scrolled;
uint16_t LCD_ScrollBackStep;

/**
 * @}
 */ 


/** @defgroup lcd_log_private_FunctionPrototypes
 * @{
 */ 

/**
 * @}
 */ 


/** @defgroup lcd_log_private_Functions
 * @{
 */ 


/**
 * @brief    Initializes the LCD Log module 
 * @param    None
 * @retval None
 */

void lcd_log_init ( void)
{
    /* Deinit LCD cache */
    lcd_log_deInit();

    /* Clear the LCD */
    lcd_ctrl_clear(LCD_LOG_BACKGROUND_COLOR);

    LCD_is_initialized = true;
}

/**
 * @brief DeInitializes the LCD Log module. 
 * @param    None
 * @retval None
 */
void lcd_log_deInit(void)
{
    LCD_LineColor = LCD_LOG_TEXT_COLOR;
    LCD_CacheBuffer_xptr = 0;
    LCD_CacheBuffer_yptr_top = 0;
    LCD_CacheBuffer_yptr_bottom = 0;

    LCD_CacheBuffer_yptr_top_bak = 0;
    LCD_CacheBuffer_yptr_bottom_bak = 0;

    LCD_CacheBuffer_yptr_invert= ENABLE;
    LCD_ScrollActive = DISABLE;
    LCD_Lock = DISABLE;
    LCD_Scrolled = DISABLE;
    LCD_ScrollBackStep = 0;
}

/**
 * @brief    Display the application header on the LCD screen
 * @param    header: pointer to the string to be displayed
 * @retval None
 */
void lcd_log_setHeader (uint8_t *header)
{
    if (LCD_is_initialized)
    {
        /* Set the LCD Font */
        lcd_ctrl_setFont(&LCD_LOG_HEADER_FONT);

        lcd_ctrl_setTextColor(LCD_LOG_SOLID_BACKGROUND_COLOR);
        lcd_ctrl_fillRect(0, 0, lcd_ctrl_getXSize(), LCD_LOG_HEADER_FONT.Height * LCD_LOG_HEADER_SIZE);

        /* Set the LCD Text Color */
        lcd_ctrl_setTextColor(LCD_LOG_SOLID_TEXT_COLOR);
        lcd_ctrl_setBackColor(LCD_LOG_SOLID_BACKGROUND_COLOR);

        lcd_ctrl_displayStringAt(0, ((LCD_LOG_HEADER_FONT.Height * (LCD_LOG_HEADER_SIZE - 1))>>2), (uint8_t*)header, CENTER_MODE);

        lcd_ctrl_setBackColor(LCD_LOG_BACKGROUND_COLOR);
        lcd_ctrl_setTextColor(LCD_LOG_TEXT_COLOR);
        lcd_ctrl_setFont(&LCD_LOG_TEXT_FONT);
    }
}

/**
 * @brief    Display the application footer on the LCD screen
 * @param    footer: pointer to the string to be displayed
 * @retval None
 */
void lcd_log_setFooter(uint8_t *footer)
{
    if (LCD_is_initialized)
    {
    	uint16_t extra_space = 4;
        /* Set the LCD Font */
        lcd_ctrl_setFont (&LCD_LOG_FOOTER_FONT);

        lcd_ctrl_setTextColor(LCD_LOG_SOLID_BACKGROUND_COLOR);
        lcd_ctrl_fillRect(0, lcd_ctrl_getYSize() - LCD_LOG_FOOTER_FONT.Height - extra_space, lcd_ctrl_getXSize(), LCD_LOG_FOOTER_FONT.Height + 4);

        /* Set the LCD Text Color */
        lcd_ctrl_setTextColor(LCD_LOG_SOLID_TEXT_COLOR);
        lcd_ctrl_setBackColor(LCD_LOG_SOLID_BACKGROUND_COLOR);

        lcd_ctrl_displayStringAt(0, lcd_ctrl_getYSize() - LCD_LOG_FOOTER_FONT.Height - (extra_space>>1), footer, CENTER_MODE);

        lcd_ctrl_setBackColor(LCD_LOG_BACKGROUND_COLOR);
        lcd_ctrl_setTextColor(LCD_LOG_TEXT_COLOR);
        lcd_ctrl_setFont (&LCD_LOG_TEXT_FONT);
    }
}

/**
 * @brief    Clear the Text Zone
 * @param    None
 * @retval None
 */
void lcd_log_clearTextZone(void)
{
    if (LCD_is_initialized)
    {
        uint8_t i=0;

        for (i= 0 ; i < YWINDOW_SIZE; i++)
        {
            lcd_ctrl_clearStringLine(i + YWINDOW_MIN);
        }

        lcd_log_deInit();
    }
}

/**
 * @brief    Write buffer content to LCD
 * @param    fd: File descripto (not used)
 * @param    buf: buffer which should be written to LCD
 * @param    nbyte: Count of bytes to write
 * @retval None
 */
void lcd_log_write(int fd, const char* buf, size_t nbyte)
{
    if (LCD_is_initialized)
    {
        if (buf != NULL)
        {
            size_t i;
        	LCD_LineColor = (fd == 2)?LCD_COLOR_RED:LCD_LOG_TEXT_COLOR;
            for (i=0;i<nbyte;i++)
            {
                lcd_log_putc(buf[i]);
            }
        }
    }
}


/**
 * @brief    Redirect the printf to the LCD
 * @param    c: character to be displayed
 * @retval char
 */
int lcd_log_putc(int ch)
{
    if (LCD_is_initialized)
    {
        sFONT *cFont = lcd_ctrl_getFont();
        uint32_t idx;

        if(LCD_Lock == DISABLE)
        {
            if(LCD_ScrollActive == ENABLE)
            {
				LCD_CacheBuffer_yptr_bottom = LCD_CacheBuffer_yptr_bottom_bak;
				LCD_CacheBuffer_yptr_top    = LCD_CacheBuffer_yptr_top_bak;
				LCD_ScrollActive = DISABLE;
				LCD_Scrolled = DISABLE;
				LCD_ScrollBackStep = 0;
            }

            if(( LCD_CacheBuffer_xptr < (lcd_ctrl_getXSize()) /cFont->Width ) &&    ( ch != '\n'))
            {
                LCD_CacheBuffer[LCD_CacheBuffer_yptr_bottom].line[LCD_CacheBuffer_xptr++] = (uint16_t)ch;
            }
            else
            {
                if(LCD_CacheBuffer_yptr_top >= LCD_CacheBuffer_yptr_bottom)
                {

                    if(LCD_CacheBuffer_yptr_invert == DISABLE)
                    {
                        LCD_CacheBuffer_yptr_top++;

                        if(LCD_CacheBuffer_yptr_top == LCD_CACHE_DEPTH)
                        {
                            LCD_CacheBuffer_yptr_top = 0;
                        }
                    }
                    else
                    {
                        LCD_CacheBuffer_yptr_invert= DISABLE;
                    }
                }

                for(idx = LCD_CacheBuffer_xptr ; idx < (lcd_ctrl_getXSize()) /cFont->Width; idx++)
                {
                    LCD_CacheBuffer[LCD_CacheBuffer_yptr_bottom].line[LCD_CacheBuffer_xptr++] = ' ';
                }
                LCD_CacheBuffer[LCD_CacheBuffer_yptr_bottom].color = LCD_LineColor;

                LCD_CacheBuffer_xptr = 0;

                lcd_log_updateDisplay();

                LCD_CacheBuffer_yptr_bottom ++;

                if (LCD_CacheBuffer_yptr_bottom == LCD_CACHE_DEPTH)
                {
                    LCD_CacheBuffer_yptr_bottom = 0;
                    LCD_CacheBuffer_yptr_top = 1;
                    LCD_CacheBuffer_yptr_invert = ENABLE;
                }

                if( ch != '\n')
                {
                    LCD_CacheBuffer[LCD_CacheBuffer_yptr_bottom].line[LCD_CacheBuffer_xptr++] = (uint16_t)ch;
                }

            }
        }
    }
    return ch;
}
    
/**
 * @brief    Update the text area display
 * @param    None
 * @retval None
 */
void lcd_log_updateDisplay (void)
{
    if (LCD_is_initialized)
    {

        uint8_t cnt = 0 ;
        uint16_t length = 0 ;
        uint16_t ptr = 0, index = 0;

        if((LCD_CacheBuffer_yptr_bottom    < (YWINDOW_SIZE -1)) &&
         (LCD_CacheBuffer_yptr_bottom    >= LCD_CacheBuffer_yptr_top))
        {
            lcd_ctrl_setTextColor(LCD_CacheBuffer[cnt + LCD_CacheBuffer_yptr_bottom].color);
            lcd_ctrl_displayStringAtLine ((YWINDOW_MIN + LCD_CacheBuffer_yptr_bottom),
                                     (uint8_t *)(LCD_CacheBuffer[cnt + LCD_CacheBuffer_yptr_bottom].line));
        }
        else
        {

            if(LCD_CacheBuffer_yptr_bottom < LCD_CacheBuffer_yptr_top)
            {
                /* Virtual length for rolling */
                length = LCD_CACHE_DEPTH + LCD_CacheBuffer_yptr_bottom ;
            }
            else
            {
                    length = LCD_CacheBuffer_yptr_bottom;
            }

            ptr = length - YWINDOW_SIZE + 1;

            for    (cnt = 0 ; cnt < YWINDOW_SIZE ; cnt ++)
            {

                    index = (cnt + ptr )% LCD_CACHE_DEPTH ;

                    lcd_ctrl_setTextColor(LCD_CacheBuffer[index].color);
                    lcd_ctrl_displayStringAtLine ((cnt + YWINDOW_MIN),
                                     (uint8_t *)(LCD_CacheBuffer[index].line));

            }
        }
    }
}

#if( LCD_SCROLL_ENABLED == 1)
/**
 * @brief    Display previous text frame
 * @param    None
 * @retval Status
 */
ErrorStatus lcd_log_scrollBack(void)
{
    if (LCD_is_initialized)
    {
        
        if(LCD_ScrollActive == DISABLE)
        {

            LCD_CacheBuffer_yptr_bottom_bak = LCD_CacheBuffer_yptr_bottom;
            LCD_CacheBuffer_yptr_top_bak    = LCD_CacheBuffer_yptr_top;


            if(LCD_CacheBuffer_yptr_bottom > LCD_CacheBuffer_yptr_top)
            {

                if ((LCD_CacheBuffer_yptr_bottom - LCD_CacheBuffer_yptr_top) <=    YWINDOW_SIZE)
                {
                    LCD_Lock = DISABLE;
                    return ERROR;
                }
            }
            LCD_ScrollActive = ENABLE;

            if((LCD_CacheBuffer_yptr_bottom    > LCD_CacheBuffer_yptr_top)&&
                 (LCD_Scrolled == DISABLE ))
            {
                LCD_CacheBuffer_yptr_bottom--;
                LCD_Scrolled = ENABLE;
            }

        }

        if(LCD_ScrollActive == ENABLE)
        {
            LCD_Lock = ENABLE;

            if(LCD_CacheBuffer_yptr_bottom > LCD_CacheBuffer_yptr_top)
            {

                if((LCD_CacheBuffer_yptr_bottom    - LCD_CacheBuffer_yptr_top) <    YWINDOW_SIZE )
                {
                    LCD_Lock = DISABLE;
                    return ERROR;
                }

                LCD_CacheBuffer_yptr_bottom --;
            }
            else if(LCD_CacheBuffer_yptr_bottom <= LCD_CacheBuffer_yptr_top)
            {

                if((LCD_CACHE_DEPTH    - LCD_CacheBuffer_yptr_top + LCD_CacheBuffer_yptr_bottom) < YWINDOW_SIZE)
                {
                    LCD_Lock = DISABLE;
                    return ERROR;
                }
                LCD_CacheBuffer_yptr_bottom --;

                if(LCD_CacheBuffer_yptr_bottom == 0xFFFF)
                {
                    LCD_CacheBuffer_yptr_bottom = LCD_CACHE_DEPTH - 2;
                }
            }
            LCD_ScrollBackStep++;
            lcd_log_updateDisplay();
            LCD_Lock = DISABLE;
        }
    }
        return SUCCESS;
}

/**
 * @brief    Display next text frame
 * @param    None
 * @retval Status
 */
ErrorStatus lcd_log_scrollForward(void)
{
    if (LCD_is_initialized)
    {
    
        if(LCD_ScrollBackStep != 0)
        {
            if(LCD_ScrollActive == DISABLE)
            {

                LCD_CacheBuffer_yptr_bottom_bak = LCD_CacheBuffer_yptr_bottom;
                LCD_CacheBuffer_yptr_top_bak        = LCD_CacheBuffer_yptr_top;

                if(LCD_CacheBuffer_yptr_bottom > LCD_CacheBuffer_yptr_top)
                {

                if ((LCD_CacheBuffer_yptr_bottom - LCD_CacheBuffer_yptr_top) <=    YWINDOW_SIZE)
                {
                    LCD_Lock = DISABLE;
                    return ERROR;
                }
                }
                LCD_ScrollActive = ENABLE;

                if((LCD_CacheBuffer_yptr_bottom    > LCD_CacheBuffer_yptr_top)&&
                 (LCD_Scrolled == DISABLE ))
                {
                    LCD_CacheBuffer_yptr_bottom--;
                    LCD_Scrolled = ENABLE;
                }

            }

            if(LCD_ScrollActive == ENABLE)
            {
                LCD_Lock = ENABLE;
                LCD_ScrollBackStep--;

                if(++LCD_CacheBuffer_yptr_bottom == LCD_CACHE_DEPTH)
                {
                    LCD_CacheBuffer_yptr_bottom = 0;
                }

                lcd_log_updateDisplay();
                LCD_Lock = DISABLE;

            }
            return SUCCESS;
        }
        else // LCD_ScrollBackStep == 0
        {
            LCD_Lock = DISABLE;
            return ERROR;
        }
    }
    return ERROR;
}
#endif /* LCD_SCROLL_ENABLED */

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

/**
 * @}
 */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
