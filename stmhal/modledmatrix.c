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

#include "py/nlr.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "pin.h"

//#define MICROPY_PY_LEDMATRIX 1

#if defined(MICROPY_PY_LEDMATRIX) && (MICROPY_PY_LEDMATRIX == 1)

#define FRAMERATE 30


// Frame buffer organized in a efficient way for output to
// the r0/r1, g0/g1 and b0/b1 pins. Depth is the count of bits per color.
typedef struct _mp_obj_ledmatrix_t {
    mp_obj_base_t base;
    uint8_t *buf;
    uint16_t width, height, depth;
    uint16_t bwidth;
    uint16_t bit_per_weight;
    const pin_obj_t *pin_line_sel[4];
    const pin_obj_t *pin_col[6]; /* r0, r1, g0, g1, b0, b1 */
    const pin_obj_t *pin_clk;
    const pin_obj_t *pin_le;
    const pin_obj_t *pin_oe;
} mp_obj_ledmatrix_t;

static uint8_t BUFFER[1136];

STATIC void ledmatrix_set_pixel(mp_obj_ledmatrix_t * self, uint16_t x, uint16_t y, uint8_t col[3]) {
    bool lower = y<16;
    uint16_t x_col = (x>>2)*3;
    if (x%4 != 3) {
        x_col+= x & 0x03;
    }
    for (uint8_t ln2w=0;ln2w<self->depth; ln2w++) {
        uint32_t addr = ln2w*self->bit_per_weight + (y & 0x0F)*self->bwidth+x_col;
        if (x&0x03)
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
            self->buf[addr]   |= col[0] & (1<<ln2w) ? mask : 0x00;
            self->buf[addr+1] |= col[1] & (1<<ln2w) ? mask : 0x00;
            self->buf[addr+2] |= col[1] & (1<<ln2w) ? mask : 0x00;
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
        if (x&0x03)
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

STATIC mp_obj_t ledmatrix_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 16, 16, false);

    mp_obj_ledmatrix_t *o = m_new_obj(mp_obj_ledmatrix_t);
    o->base.type = type;

    o->width = mp_obj_get_int(args[0]);
    o->height = mp_obj_get_int(args[1]);
    o->depth = mp_obj_get_int(args[2]);
    o->bwidth = (o->width>>2)*3;
    o->bit_per_weight = o->bwidth*o->height>>1;
    o->buf = BUFFER;

    for (uint8_t i=0;i<4;i++) {
        o->pin_line_sel[i] = pin_find(args[3+i]);
    }
    for (uint8_t i=0;i<6;i++) {
        o->pin_col[i] = pin_find(args[7+i]);
    }
    o->pin_clk = pin_find(args[13]);
    o->pin_le = pin_find(args[14]);
    o->pin_oe = pin_find(args[15]);

    return MP_OBJ_FROM_PTR(o);
}

STATIC mp_obj_t ledmatrix_fill(size_t n_args, const mp_obj_t *args) {
    mp_obj_ledmatrix_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t col_mask = (1<<self->depth)-1;
    uint8_t color[3];
    color[0] = mp_obj_get_int(args[1]) & col_mask;
    color[1] = mp_obj_get_int(args[2]) & col_mask;
    color[2] = mp_obj_get_int(args[3]) & col_mask;
    for (uint16_t x=0;x< self->width; x++) {
        for (uint16_t y=0;y< self->height; y++) {
            ledmatrix_set_pixel(self, x, y, color);
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(ledmatrix_fill_obj, 4, 4, ledmatrix_fill);

STATIC mp_obj_t ledmatrix_pixel(size_t n_args, const mp_obj_t *args) {
    mp_obj_ledmatrix_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t x = mp_obj_get_int(args[1]);
    mp_int_t y = mp_obj_get_int(args[2]);
    if (0 <= x && x < self->width && 0 <= y && y < self->height) {
        if (n_args == 3) {
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
            mp_obj_get_array(args[3], &len, &elem);
            if (len==3)
            {
                // set
                uint8_t color[3] = {mp_obj_get_int(elem[0]), mp_obj_get_int(elem[1]), mp_obj_get_int(elem[2])};
                ledmatrix_set_pixel(self, x, y, color);
            }
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(ledmatrix_pixel_obj, 3, 4, ledmatrix_pixel);

STATIC const mp_rom_map_elem_t ledmatrix_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&ledmatrix_fill_obj) },
    { MP_ROM_QSTR(MP_QSTR_pixel), MP_ROM_PTR(&ledmatrix_pixel_obj) },
};
STATIC MP_DEFINE_CONST_DICT(ledmatrix_locals_dict, ledmatrix_locals_dict_table);

STATIC const mp_obj_type_t mp_type_ledmatrix = {
    { &mp_type_type },
    .name = MP_QSTR_LedMatrix,
    .make_new = ledmatrix_make_new,
    .locals_dict = (mp_obj_t)&ledmatrix_locals_dict,
};

STATIC const mp_rom_map_elem_t ledmatrix_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_ledmatrix) },
    { MP_ROM_QSTR(MP_QSTR_LedMatrix), MP_ROM_PTR(&mp_type_ledmatrix) },
};

STATIC MP_DEFINE_CONST_DICT(ledmatrix_module_globals, ledmatrix_module_globals_table);

const mp_obj_module_t mp_module_ledmatrix = {
    .base = { &mp_type_module },
    .name = MP_QSTR_ledmatrix,
    .globals = (mp_obj_dict_t*)&ledmatrix_module_globals,
};

#endif // MICROPY_PY_LEDMATRIX
