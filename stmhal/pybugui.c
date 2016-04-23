/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Tobias Badertscher
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
#include STM32_HAL_H

#include "py/nlr.h"
#include "py/runtime.h"

#if defined(MICROPY_SW_ENABLE_UGUI) && MICROPY_SW_ENABLE_UGUI == 1

#include <stdint.h>
#include "../ugui/ugui.h"
#include "pybugui.h"
#include "ugui_drv.h"
#include "sdram.h"


#define  LCD_PIXEL_WIDTH         (THD)
#define  LCD_PIXEL_HEIGHT        (TVD)

#define  LCD_COLOR_DEPTH         ((uint16_t)2)       /* Farbtiefe: RGB565 --> 2 Bytes / Pixel */

#define  LCD_FRAME_BUFFER        ((uint32_t)SDRAM_BANK_ADDR)
#define  BUFFER_OFFSET           ((uint32_t)LCD_PIXEL_HEIGHT*LCD_PIXEL_WIDTH*LCD_COLOR_DEPTH)
#define  LAYER_1_OFFSET          (0)
#define  LAYER_2_OFFSET          (BUFFER_OFFSET)

/// \moduleref ugui
/// \class UGUI - LCD control for generic LCDs
///
/// The ugui class provides a generic interface to any LCD.
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

typedef struct _pyb_ugui_obj_t {
    mp_obj_base_t base;
    ugui_drv_t * drv;
    /* GUI structure */
    UG_GUI gui;
} pyb_ugui_obj_t;

/// \classmethod \constructor(skin_position)
///
/// Construct an LCD object in the given skin position.  `skin_position` can be 'X' or 'Y', and
/// should match the position where the LCD pyskin is plugged in.
STATIC mp_obj_t pyb_ugui_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {

	mem_acc_t memAcc;
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    // get LCD position
    //const char *lcd_id = mp_obj_str_get_str(args[0]);

    // create lcd object
    pyb_ugui_obj_t *lcd = m_new_obj(pyb_ugui_obj_t);
    lcd->base.type = &pyb_ugui_type;
    lcd->drv = &ugui_drv;
	memAcc.pMem1	=	(uint8_t*)SDRAM_DEVICE_ADDR;
	memAcc.pMem2	=	(uint8_t*)(SDRAM_DEVICE_ADDR+ugui_drv.x_size*ugui_drv.y_size*ugui_drv.pixel_size);

    lcd->drv->init0(&memAcc);

    /* Init uGUI */
    UG_Init(&lcd->gui,(void(*)(UG_S16,UG_S16,UG_COLOR))lcd->drv->pset,lcd->drv->x_size,lcd->drv->y_size);

    /* Register hardware acceleration */
    if (lcd->drv->draw_line_hwaccel)
    {
    	UG_DriverRegister( DRIVER_DRAW_LINE, (void*)lcd->drv->draw_line_hwaccel );
    	UG_DriverEnable( DRIVER_DRAW_LINE );
    }
    if (lcd->drv->fill_frame_hwaccel)
    {
    	UG_DriverRegister( DRIVER_FILL_FRAME, (void*)lcd->drv->fill_frame_hwaccel );
    	UG_DriverEnable( DRIVER_FILL_FRAME );
    }
    lcd->drv->init1();
    UG_FillScreen( C_BLACK );

    return lcd;
}


/// \method light(value)
///
/// Turn the backlight on/off.  True or 1 turns it on, False or 0 turns it off.
STATIC mp_obj_t pyb_ugui_light(mp_obj_t self_in, mp_obj_t value) {
    pyb_ugui_obj_t *self = self_in;
    if (mp_obj_is_true(value)) {
        self->drv->display_on();
    } else {
        self->drv->display_off();
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_ugui_light_obj, pyb_ugui_light);

/// \method write(str)
///
/// Write the string `str` to the screen.  It will appear immediately.
STATIC mp_obj_t pyb_ugui_write(mp_obj_t self_in, mp_obj_t str) {
   //pyb_ugui_obj_t *self = self_in;
    mp_uint_t len;
    const char *data = mp_obj_str_get_data(str, &len);
    UG_ConsolePutString(data);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_ugui_write_obj, pyb_ugui_write);

/// \method fill(colour)
///
/// Fill the screen with the given RGB colour tuple.
///
/// This method writes to the hidden buffer.  Use `show()` to show the buffer.
STATIC mp_obj_t pyb_ugui_fill(mp_obj_t self_in, mp_obj_t col_in) {
    //pyb_ugui_obj_t *self = self_in;
    int col = mp_obj_get_int(col_in);
    UG_FillScreen(col);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_ugui_fill_obj, pyb_ugui_fill);

/// \method get(x, y)
///
/// Get the pixel at the position `(x, y)`.  Returns 0 or 1.
///
/// This method reads from the visible buffer.
STATIC mp_obj_t pyb_ugui_get(mp_obj_t self_in, mp_obj_t x_in, mp_obj_t y_in) {
    pyb_ugui_obj_t *self = self_in;
    int x = mp_obj_get_int(x_in);
    int y = mp_obj_get_int(y_in);
    if (0 <= x && x < self->drv->x_size && 0 <= y && y < self->drv->y_size) {
    	printf("Pixel at %d %d = \n", x, y);
    }
    return mp_obj_new_int(0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_ugui_get_obj, pyb_ugui_get);

/// \method pixel(x, y, colour)
///
/// Set the pixel at `(x, y)` to the given colour.
///
/// This method writes to the hidden buffer.  Use `show()` to show the buffer.
STATIC mp_obj_t pyb_ugui_pixel(mp_uint_t n_args, const mp_obj_t *args) {
    pyb_ugui_obj_t *self = args[0];
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    int color = mp_obj_get_int(args[3]);
    if (0 <= x && x < self->drv->x_size && 0 <= y && y < self->drv->y_size) {
        UG_DrawPixel(x, y, color);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_ugui_pixel_obj, 4, 4, pyb_ugui_pixel);

/// \method text(str, x, y, colour)
///
/// Draw the given text to the position `(x, y)` using the given colour.
///
/// This method writes to the hidden buffer.  Use `show()` to show the buffer.
STATIC mp_obj_t pyb_ugui_text(mp_uint_t n_args, const mp_obj_t *args) {
    // extract arguments
    mp_uint_t len;
    const char *data = mp_obj_str_get_data(args[1], &len);
    int x0 = mp_obj_get_int(args[2]);
    int y0 = mp_obj_get_int(args[3]);
    //int col = mp_obj_get_int(args[4]);

    UG_PutString(x0, y0, data);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_ugui_text_obj, 5, 5, pyb_ugui_text);


STATIC const mp_map_elem_t pyb_ugui_locals_dict_table[] = {
    // instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_light), (mp_obj_t)&pyb_ugui_light_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write), (mp_obj_t)&pyb_ugui_write_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_fill), (mp_obj_t)&pyb_ugui_fill_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get), (mp_obj_t)&pyb_ugui_get_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pixel), (mp_obj_t)&pyb_ugui_pixel_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_text), (mp_obj_t)&pyb_ugui_text_obj },
};

STATIC MP_DEFINE_CONST_DICT(pyb_ugui_locals_dict, pyb_ugui_locals_dict_table);

const mp_obj_type_t pyb_ugui_type = {
    { &mp_type_type },
    .name = MP_QSTR_UGUI,
    .make_new = pyb_ugui_make_new,
    .locals_dict = (mp_obj_t)&pyb_ugui_locals_dict,
};

#endif // MICROPY_SW_ENABLE_UGUI
