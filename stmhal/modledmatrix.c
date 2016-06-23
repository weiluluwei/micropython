/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Tobias Badertscher
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
#include <stdbool.h>

#include STM32_HAL_H
#include "py/nlr.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "pin.h"
#include "timer.h"

#include "measure_cycles.c"

//#define MICROPY_PY_LEDMATRIX 1

#if defined(MICROPY_PY_LEDMATRIX) && (MICROPY_PY_LEDMATRIX == 1)
#include "font_petme128_8x8.h"

#define FRAMERATE 60
#define GPIO_PORT_COUNT 10

#define SET_PIN_VALUE(x, value) HAL_GPIO_WritePin((x)->gpio, (x)->pin_mask, (value))
/**/
 #define SET_PIN_HIGH(x) SET_PIN_VALUE(x, 1)
 #define SET_PIN_LOW(x) SET_PIN_VALUE(x, 0)
 /**/
/*
#define SET_PIN_HIGH(x) ((x)->gpio->BSRRL = (x)->pin_mask)
#define SET_PIN_LOW(x) ((x)->gpio->BSRRH = (x)->pin_mask)
*/

#define CONFIG_PIN_OUTPUT(x)

// Frame buffer organized in a efficient way for output to
// the r0/r1, g0/g1 and b0/b1 pins. Depth is the count of bits per color.
typedef struct _mp_obj_ledmatrix_t {
    mp_obj_base_t base;
    uint8_t *buf;
    uint16_t width, height, depth;
    uint16_t bwidth;
    uint16_t bit_per_weight;
    uint16_t line_sel_cnt;
    const pin_obj_t *pin_line_sel[4];
    uint16_t col_line_cnt;
    const pin_obj_t *pin_col[6]; /* r0, r1, g0, g1, b0, b1 */
    const pin_obj_t *pin_clk;
    const pin_obj_t *pin_le;
    const pin_obj_t *pin_oe;
    uint16_t next_linenr;
    uint16_t next_ln2w;
    uint16_t weight_cnt;   /* Counter to control the duration of the LED illumination in a certain ln2w */
    pyb_timer_obj_t *timer;
    bool show;
} mp_obj_ledmatrix_t;

static uint8_t BUFFER[2048];

STATIC void ledmatrix_set_pixel(mp_obj_ledmatrix_t * self, uint16_t x, uint16_t y, uint8_t col[3]) {
    bool lower = y<16;
    uint16_t x_col = (x>>2)*3;
    if (x%4 != 3) {
        x_col+= x & 0x03;
    }
    for (uint8_t ln2w=0;ln2w<self->depth; ln2w++) {
        uint32_t addr = ln2w*self->bit_per_weight + (y & 0x0F)*self->bwidth+x_col;
        if ( (x&0x03) != 0x03)
        {
            uint8_t val = self->buf[addr];
            if (lower) {
                val &= 0xEA;
                val |= col[0] & (1<<ln2w)? 0x01: 0x00;
                val |= col[1] & (1<<ln2w)? 0x04: 0x00;
                val |= col[2] & (1<<ln2w)? 0x10: 0x00;
            } else {
                val &= 0xD5;
                val |= col[0] & (1<<ln2w)? 0x02: 0x00;
                val |= col[1] & (1<<ln2w)? 0x08: 0x00;
                val |= col[2] & (1<<ln2w)? 0x20: 0x00;
            }
            self->buf[addr] = val;
        } else {
            uint8_t mask = lower?0x40: 0x80;
            for (uint8_t i=0;i<3;i++) {
                self->buf[addr+i] &= ~mask;
            }
            self->buf[addr]   |= col[0] & (1<<ln2w) ? mask : 0x00;
            self->buf[addr+1] |= col[1] & (1<<ln2w) ? mask : 0x00;
            self->buf[addr+2] |= col[2] & (1<<ln2w) ? mask : 0x00;
        }
    }
}

STATIC void ledmatrix_get_pixel(mp_obj_ledmatrix_t * self, uint16_t x, uint16_t y, uint8_t col[3]) {
    bool lower = y<16;
    uint16_t x_col = (x>>2)*3;
    if (x%4 != 3) {
        x_col+= x & 0x03;
    }
    col[0] = 0;
    col[1] = 0;
    col[2] = 0;
    for (uint8_t ln2w=0;ln2w<self->depth; ln2w++) {
        uint32_t addr = ln2w*self->bit_per_weight + (y & 0x0F)*self->bwidth+x_col;
        if ((x&0x03) != 0x03)
        {
            uint8_t val = self->buf[addr];
            if (lower) {
                col[0] += ((val   ) & 0x01)*(1<<ln2w);
                col[1] += ((val>>2) & 0x01)*(1<<ln2w);
                col[2] += ((val>>4) & 0x01)*(1<<ln2w);
            } else {
                col[0] += ((val>>1) & 0x01)*(1<<ln2w);
                col[1] += ((val>>3) & 0x01)*(1<<ln2w);
                col[2] += ((val>>5) & 0x01)*(1<<ln2w);
            }
        } else {
            uint8_t shift = lower? 6: 7;
            col[0] += ((self->buf[addr  ]>>shift) & 0x01)*(1<<ln2w);
            col[1] += ((self->buf[addr+1]>>shift) & 0x01)*(1<<ln2w);
            col[2] += ((self->buf[addr+2]>>shift) & 0x01)*(1<<ln2w);
        }
    }
}

STATIC void mp_ob_2_u8(mp_obj_t tuple, uint8_t *array, uint8_t cnt) {
    mp_uint_t len;
    mp_obj_t *elem;
    mp_obj_get_array(tuple, &len, &elem);
    if (len == cnt)
    {
        for (uint8_t i=0;i<cnt;i++)
        {
            array[i] = mp_obj_get_int(elem[i]);
        }
    }
}

STATIC void ledmatrix_select_line(mp_obj_ledmatrix_t * self, uint16_t line_nr) {

    for (uint8_t i = 0, value = line_nr; i<self->line_sel_cnt; ++i, value >>=1)
    {
        SET_PIN_VALUE(self->pin_line_sel[i], value&0x01);
    }
}

STATIC void ledmatrix_set_next_line(mp_obj_ledmatrix_t * self) {
    uint16_t offset = self->next_ln2w*self->bit_per_weight+(self->next_linenr & 0x0F)*self->bwidth;
    uint8_t val_11 = 0;
    uint8_t idx = 0;
    uint8_t* ptr =  &self->buf[offset-1];
    for (uint8_t x=0; x< self->width; ++x) {
        uint8_t s_idx = (x & 0x03);
        uint8_t ser_val = 0;
        if (s_idx == 3) {
            ser_val = val_11;
            val_11 =0;
        } else {
            ser_val = *(++ptr);
            val_11 |= ((ser_val & 0xC0)>>(6-2*s_idx));
            idx += 1;
        }
        for (uint8_t pin_nr=0, value = ser_val; pin_nr < self->col_line_cnt; ++pin_nr, value>>=1) {
            SET_PIN_VALUE(self->pin_col[pin_nr], value & 0x01);
        }
        SET_PIN_HIGH(self->pin_clk);
        SET_PIN_LOW(self->pin_clk);
    }
}


STATIC mp_obj_t ledmatrix_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_uint_t len;
    mp_obj_t *elem;

    mp_arg_check_num(n_args, n_kw, 8, 8, false);

    mp_obj_ledmatrix_t *o = m_new_obj(mp_obj_ledmatrix_t);
    o->base.type = type;

    o->width = mp_obj_get_int(args[0]);
    o->height = mp_obj_get_int(args[1]);
    o->depth = mp_obj_get_int(args[2]);
    o->bwidth = (o->width>>2)*3;
    o->bit_per_weight = o->bwidth*o->height>>1;
    o->buf = BUFFER;
    memset(o->buf, 0, o->bit_per_weight*o->depth);
    mp_obj_get_array(args[3], &len, &elem);
    if (len != 4) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "Line address does not contain 4 elements"));
    }
    o->line_sel_cnt = len;
    for (uint8_t i=0;i<o->line_sel_cnt;i++) {
        o->pin_line_sel[i] = pin_find(elem[i]);
        if (pin_get_mode(o->pin_line_sel[i]) != GPIO_MODE_OUTPUT_PP) {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "Select line must be output"));
        }
    }
    /* Get colors */
    mp_obj_get_array(args[4], &len, &elem);
    if (len != 6) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "Colors does not contain 6 elements"));
    }
    o->col_line_cnt = len;
    for (uint8_t i=0;i<o->col_line_cnt;i++) {
        o->pin_col[i] = pin_find(elem[i]);
        if (pin_get_mode(o->pin_col[i]) != GPIO_MODE_OUTPUT_PP) {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "Color lines must be output"));
        }
    }
    o->pin_clk = pin_find(args[5]);
    if (pin_get_mode(o->pin_clk) != GPIO_MODE_OUTPUT_PP) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "clk must be output"));
    }
    o->pin_le = pin_find(args[6]);
    if (pin_get_mode(o->pin_le) != GPIO_MODE_OUTPUT_PP) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "le must be output"));
    }
    o->pin_oe = pin_find(args[7]);
    if (pin_get_mode(o->pin_oe) != GPIO_MODE_OUTPUT_PP) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "oe must be output"));
    }
    o->next_linenr = 0;
    o->next_ln2w = 0;
    o->weight_cnt = 0;
    o->timer = NULL;
    o->show = false;

    return MP_OBJ_FROM_PTR(o);
}

STATIC mp_obj_t ledmatrix_fill(mp_obj_t self_in, mp_obj_t tuple) {
    mp_obj_ledmatrix_t *self = self_in;
    mp_int_t col_mask = (1<<self->depth)-1;
    uint8_t color[3];
    mp_ob_2_u8(tuple, color, 3);
    color[0] &= col_mask;
    color[1] &= col_mask;
    color[2] &= col_mask;
    for (uint16_t x=0;x< self->width; x++) {
        for (uint16_t y=0;y< self->height; y++) {
            ledmatrix_set_pixel(self, x, y, color);
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(ledmatrix_fill_obj, ledmatrix_fill);

STATIC mp_obj_t ledmatrix_pixel(size_t n_args, const mp_obj_t *args) {
    mp_obj_ledmatrix_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_uint_t len;
    mp_obj_t *elem;
    mp_obj_get_array(args[1], &len, &elem);
    if (len==2)
    {
        mp_int_t x = mp_obj_get_int(elem[0]);
        mp_int_t y = mp_obj_get_int(elem[1]);
        if (0 <= x && x < self->width && 0 <= y && y < self->height) {
            if (n_args == 2) {
                uint8_t color[3];
                mp_obj_tuple_t *tuple = mp_obj_new_tuple(3, NULL);
                ledmatrix_get_pixel(self, x, y, color);
                for (uint8_t i=0;i<3;i++)
                {
                    tuple->items[i] = MP_OBJ_NEW_SMALL_INT(color[i]);
                }
                return tuple;
            } else {
                mp_uint_t len;
                mp_obj_t *elem;
                mp_obj_get_array(args[2], &len, &elem);
                if (len==3) {
                    // set
                    uint8_t color[3] = {mp_obj_get_int(elem[0]), mp_obj_get_int(elem[1]), mp_obj_get_int(elem[2])};
                    ledmatrix_set_pixel(self, x, y, color);
                } else {
                     nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "Expected color as 3-tuple."));
                }

            }
        }
    } else {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "Expected coordinates as 2-tuple."));
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(ledmatrix_pixel_obj, 3, 4, ledmatrix_pixel);

STATIC void ledmatrix_update_raw(mp_obj_t self_in)
{
    mp_obj_ledmatrix_t *self = self_in;
    ++self->weight_cnt;
    if (self->weight_cnt == (1<<self->next_ln2w))
    {
        self->weight_cnt = 0;
        SET_PIN_HIGH(self->pin_oe);
        SET_PIN_HIGH(self->pin_le);
        ledmatrix_select_line(self, self->next_linenr);
        SET_PIN_LOW(self->pin_le);
        SET_PIN_LOW(self->pin_oe);
        self->next_linenr += 1;
        if (self->next_linenr == (1<< self->line_sel_cnt)) {
            self->next_linenr = 0;
            self->next_ln2w = (self->next_ln2w+1)%self->depth;
        }
        ledmatrix_set_next_line(self);
    }

}

/// \method update()
/// Output next line to the display.
STATIC mp_obj_t ledmatrix_update(mp_obj_t self_in) {
    reset_timer();
    start_timer();
    ledmatrix_update_raw(self_in);
    stop_timer();
    return MP_OBJ_NEW_SMALL_INT(getCycles());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ledmatrix_update_obj, ledmatrix_update);


/// \method show()
/// Start continous interrupt driven update of the display.
STATIC mp_obj_t ledmatrix_show(mp_obj_t self_in) {
    mp_obj_ledmatrix_t *self = self_in;
    if (self->timer == NULL) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "Need timer for showing stuff."));
    } else {
        if (self->show == false) {
            uint32_t updFreq = (1<<self->line_sel_cnt)*((1<<self->depth)-1)*FRAMERATE;
            updFreq = MIN(10000, updFreq);
            printf("Start running display @ %d Hz line update frequency.\n", ((int)updFreq));
            const mp_obj_t args1[4] = {
                (mp_obj_t)&pyb_timer_init_obj,
                self->timer,
                MP_OBJ_NEW_QSTR(MP_QSTR_freq),  MP_OBJ_NEW_SMALL_INT(updFreq),
            };
            mp_call_method_n_kw(0, 1, args1);
            self->show = true;
            timer_register_raw_cb(self->timer, ledmatrix_update_raw, self_in);
        } else {
            printf("Shutting down IRQ driven update.\n");
            timer_register_raw_cb(self->timer, NULL, NULL);
            self->show = false;
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ledmatrix_show_obj, ledmatrix_show);

STATIC mp_obj_t ledmatrix_timer(size_t n_args, const mp_obj_t *args) {
    mp_obj_ledmatrix_t *self = MP_OBJ_TO_PTR(args[0]);
    if (n_args == 2) {
        if (MP_OBJ_IS_TYPE(args[1], &pyb_timer_type)) {
            self->timer = MP_OBJ_TO_PTR(args[1]);
        } else {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "must be a timer type object."));
        }
    } else {
        if (self->timer) {
            return self->timer;
        } else {
            return mp_const_none;
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(ledmatrix_timer_obj, 1, 2, ledmatrix_timer);

/// \method text(str, (x, y), colour)
///
/// Draw the given text to the position `(x, y)` using the given colour (r, g, b).
///
STATIC mp_obj_t ledmatrix_text(mp_uint_t n_args, const mp_obj_t *args) {
    mp_obj_ledmatrix_t *self = MP_OBJ_TO_PTR(args[0]);
    // extract arguments
    mp_uint_t len;
    mp_obj_t *elem;
    mp_obj_get_array(args[2], &len, &elem);
    if (len==2)
    {
        mp_int_t x0 = mp_obj_get_int(elem[0]);
        mp_int_t y0 = mp_obj_get_int(elem[1]);
        mp_obj_get_array(args[3], &len, &elem);

        if (len == 3) {
            uint8_t col[3] = {mp_obj_get_int(elem[0]), mp_obj_get_int(elem[1]), mp_obj_get_int(elem[2])};
            // loop over chars
            mp_uint_t str_len;
            const char *data = mp_obj_str_get_data(args[1], &str_len);
            for (const char *top = data + str_len; data < top; data++) {
                // get char and make sure its in range of font
                uint chr = *(byte*)data;
                if (chr < 32 || chr > 127) {
                    chr = 127;
                }
                // get char data
                const uint8_t *chr_data = &font_petme128_8x8[(chr - 32) * 8];
                // loop over char data
                for (uint j = 0; j < 8; j++, x0++) {
                    if (0 <= x0 && x0 < self->width) { // clip x
                        uint vline_data = chr_data[j]; // each byte of char data is a vertical column of 8 pixels, LSB at top
                        for (int y = y0; vline_data; vline_data >>= 1, y++) { // scan over vertical column
                            if (vline_data & 1) { // only draw if pixel set
                                if (0 <= y && y < self->height) { // clip y
                                    ledmatrix_set_pixel(self, x0, y, col);
                                }
                            }
                        }
                    }
                }
            }
        } else {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "Expected color as 3-tuple."));
        }
    } else {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "Expected coordinates as 2-tuple."));
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(ledmatrix_text_obj, 4, 4, ledmatrix_text);

STATIC const mp_rom_map_elem_t ledmatrix_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&ledmatrix_fill_obj) },
    { MP_ROM_QSTR(MP_QSTR_pixel), MP_ROM_PTR(&ledmatrix_pixel_obj) },
    { MP_ROM_QSTR(MP_QSTR_update), MP_ROM_PTR(&ledmatrix_update_obj) },
    { MP_ROM_QSTR(MP_QSTR_show), MP_ROM_PTR(&ledmatrix_show_obj) },
    { MP_ROM_QSTR(MP_QSTR_timer), MP_ROM_PTR(&ledmatrix_timer_obj) },
    { MP_ROM_QSTR(MP_QSTR_text), MP_ROM_PTR(&ledmatrix_text_obj) },
};
STATIC MP_DEFINE_CONST_DICT(ledmatrix_locals_dict, ledmatrix_locals_dict_table);

STATIC const mp_obj_type_t mp_type_ledmatrix = {
    { &mp_type_type },
    .name = MP_QSTR_ledmatrix,
    .make_new = ledmatrix_make_new,
    .locals_dict = (mp_obj_t)&ledmatrix_locals_dict,
};

STATIC const mp_rom_map_elem_t ledmatrix_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_ledmatrix) },
    { MP_ROM_QSTR(MP_QSTR_ledmatrix), MP_ROM_PTR(&mp_type_ledmatrix) },
};

STATIC MP_DEFINE_CONST_DICT(ledmatrix_module_globals, ledmatrix_module_globals_table);

const mp_obj_module_t mp_module_ledmatrix = {
    .base = { &mp_type_module },
    .name = MP_QSTR_ledmatrix,
    .globals = (mp_obj_dict_t*)&ledmatrix_module_globals,
};

#endif // MICROPY_PY_LEDMATRIX
