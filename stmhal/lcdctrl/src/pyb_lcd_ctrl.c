/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
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
#include <stdint.h>
#include <string.h>
#include STM32_HAL_H

#include "py/nlr.h"
#include "py/runtime.h"

#define MICROPY_PY_LCDCTRL 1

#if MICROPY_PY_LCDCTRL == 1

#include "bufhelper.h"
#include "lcd_drv.h"
#include "lcd_ctrl.h"
#include "pyb_lcd_ctrl.h"
#include "lcd_log.h"
/// \moduleref pyb
/// \class LCD - LCD control for the LCD touch-sensor on the STM32F429i-Disco
///
/// The LCD class is used to control the LCD on the STM32F429i-Disco.
/// The LCD is a 320x240 pixel color screen, part SF-TC240T-9370A-T.
///
/// The pyskin must be connected in either the X or Y positions, and then
/// an LCD object is made using:
///
///     lcd = pyb.LCD('X')      # if pyskin is in the X position
///     lcd = pyb.LCD('Y')      # if pyskin is in the Y position
///
/// Then you can use:
///
///     lcd.light(True)                 # turn the backlight on
///     lcd.write('Hello world!\n')     # print text to the screen
///
/// This driver implements a double buffer for setting/getting pixels.
/// For example, to make a bouncing dot, try:
///
///     x = y = 0
///     dx = dy = 1
///     while True:
///         # update the dot's position
///         x += dx
///         y += dy
///
///         # make the dot bounce of the edges of the screen
///         if x <= 0 or x >= 127: dx = -dx
///         if y <= 0 or y >= 31: dy = -dy
///
///         lcd.fill(0)                 # clear the buffer
///         lcd.pixel(x, y, 1)          # draw the dot
///         lcd.show()                  # show the buffer
///         pyb.delay(50)               # pause for 50ms

typedef struct _pyb_lcd_ctrl_obj_t {
    mp_obj_base_t base;
    /*
     * Size of display
     */
    uint32_t xSize;
    uint32_t ySize;

} pyb_lcd_ctrl_obj_t;

// write a string to the LCD at the current cursor location
// output it straight away (doesn't use the pixel buffer)
STATIC void lcd_write_strn(pyb_lcd_ctrl_obj_t *lcd, const char *str, unsigned int len) {
    lcd_log_write(0, str, len);
}

/// \classmethod \constructor()
///
/// Construct an LCD object in the given skin position.  `skin_position` can be 'X' or 'Y', and
/// should match the position where the LCD pyskin is plugged in.
STATIC mp_obj_t pyb_lcd_make_new(mp_obj_t type_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {

    // create lcd object
    pyb_lcd_ctrl_obj_t *lcd = m_new_obj(pyb_lcd_ctrl_obj_t);
    lcd->base.type = &pyb_lcd_ctrl_type;

    lcd->xSize = lcd_ctrl_getXSize();
    lcd->ySize = lcd_ctrl_getYSize();

    return lcd;
}

/// \method contrast(value)
///
/// Set the contrast of the LCD.  Valid values are between 0 and 47.
STATIC mp_obj_t pyb_lcd_contrast(mp_obj_t self_in, mp_obj_t contrast_in) {
    int contrast = mp_obj_get_int(contrast_in);
    if (contrast < 0) {
        contrast = 0;
    } else if (contrast > 0x2f) {
        contrast = 0x2f;
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_lcd_contrast_obj, pyb_lcd_contrast);

/// \method light(value)
///
/// Turn the backlight on/off.  True or 1 turns it on, False or 0 turns it off.
STATIC mp_obj_t pyb_lcd_light(mp_obj_t self_in, mp_obj_t value) {
    if (mp_obj_is_true(value)) {
        lcd_ctrl_displayOn(); // set pin high to turn backlight on
    } else {
        lcd_ctrl_displayOff(); // set pin low to turn backlight off
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_lcd_light_obj, pyb_lcd_light);

/// \method write(str)
///
/// Write the string `str` to the screen.  It will appear immediately.
STATIC mp_obj_t pyb_lcd_write(mp_obj_t self_in, mp_obj_t str) {
    pyb_lcd_ctrl_obj_t *self = self_in;
    mp_uint_t len;
    const char *data = mp_obj_str_get_data(str, &len);
    lcd_write_strn(self, data, len);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_lcd_write_obj, pyb_lcd_write);

/// \method fill(colour)
///
/// Fill the screen with the given colour (0 or 1 for white or black).
///
/// This method writes to the hidden buffer.  Use `show()` to show the buffer.
STATIC mp_obj_t pyb_lcd_fill(mp_obj_t self_in, mp_obj_t col_in) {
    pyb_lcd_ctrl_obj_t *self = self_in;
    int col = mp_obj_get_int(col_in)>0?LCD_COLOR_WHITE:LCD_COLOR_BLACK;
    lcd_ctrl_setTextColor(col);
    lcd_ctrl_fillRect(0, 0, self->xSize, self->ySize);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_lcd_fill_obj, pyb_lcd_fill);

/// \method get(x, y)
///
/// Get the pixel at the position `(x, y)`.  Returns 0 or 1.
///
/// This method reads from the visible buffer.
STATIC mp_obj_t pyb_lcd_get(mp_obj_t self_in, mp_obj_t x_in, mp_obj_t y_in) {
    pyb_lcd_ctrl_obj_t *self = self_in;
    int x = mp_obj_get_int(x_in);
    int y = mp_obj_get_int(y_in);
    if (0 <= x && x < self->xSize && 0 <= y && y < self->ySize) {
        uint32_t col = lcd_ctrl_readPixel(x,y);
        return mp_obj_new_int(col);
    }
    return mp_obj_new_int(0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_lcd_get_obj, pyb_lcd_get);

/// \method pixel(x, y, colour)
///
/// Set the pixel at `(x, y)` to the given colour (0 or 1).
///
/// This method writes to the hidden buffer.  Use `show()` to show the buffer.
STATIC mp_obj_t pyb_lcd_pixel(mp_uint_t n_args, const mp_obj_t *args) {
    pyb_lcd_ctrl_obj_t *self = args[0];
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    int col = mp_obj_get_int(args[3])>0?LCD_COLOR_WHITE:LCD_COLOR_BLACK;
    if (0 <= x && x < self->xSize && 0 <= y && y < self->ySize) {
        lcd_ctrl_drawPixel(x,y, col);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_lcd_pixel_obj, 4, 4, pyb_lcd_pixel);

/// \method text(str, x, y, colour)
///
/// Draw the given text to the position `(x, y)` using the given colour (0 or 1).
///
/// This method writes to the hidden buffer.  Use `show()` to show the buffer.
STATIC mp_obj_t pyb_lcd_text(mp_uint_t n_args, const mp_obj_t *args) {
    // extract arguments
    mp_uint_t len;
    const char *data = mp_obj_str_get_data(args[1], &len);
    int x0 = mp_obj_get_int(args[2]);
    int y0 = mp_obj_get_int(args[3]);
    int col = mp_obj_get_int(args[4])>0?LCD_COLOR_WHITE:LCD_COLOR_BLACK;

    lcd_ctrl_setTextColor(col);
    lcd_ctrl_displayStringAt(x0, y0, (uint8_t *)data, LEFT_MODE);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_lcd_text_obj, 5, 5, pyb_lcd_text);


STATIC const mp_map_elem_t pyb_lcd_locals_dict_table[] = {
    // instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_contrast), (mp_obj_t)&pyb_lcd_contrast_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_light), (mp_obj_t)&pyb_lcd_light_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write), (mp_obj_t)&pyb_lcd_write_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_fill), (mp_obj_t)&pyb_lcd_fill_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get), (mp_obj_t)&pyb_lcd_get_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pixel), (mp_obj_t)&pyb_lcd_pixel_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_text), (mp_obj_t)&pyb_lcd_text_obj },
};

STATIC MP_DEFINE_CONST_DICT(pyb_lcd_locals_dict, pyb_lcd_locals_dict_table);

const mp_obj_type_t pyb_lcd_ctrl_type = {
    { &mp_type_type },
    .name = MP_QSTR_LCD,
    .make_new = pyb_lcd_make_new,
    .locals_dict = (mp_obj_t)&pyb_lcd_locals_dict,
};

#endif // MICROPY_HW_HAS_LCD
