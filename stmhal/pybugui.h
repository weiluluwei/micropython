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
#ifndef __PYBUGUI_H__
#define __PYBUGUI_H__ __PYBUGUI_H__

extern const mp_obj_type_t pyb_ugui_type;

typedef struct _mem_acc
{
	uint8_t *pMem1;	/* Layer 1 base memory address */
	uint8_t *pMem2;	/* Layer 2 base memory address */
} mem_acc_t;


typedef struct _ugui_drv {
	char *    descr;
	uint16_t  x_size;
	uint16_t  y_size;
	uint16_t  pixel_size;
	uint8_t   (*init0)(mem_acc_t *mem_acc);
	uint8_t   (*init2)(void);
	void	  (*display_on)(void);
	void	  (*display_off)(void);
	UG_RESULT (*pset)(UG_S16 x, UG_S16 y, UG_COLOR col);
	void      (*update)(void);
	UG_RESULT (*draw_line_hwaccel)( UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR c );
	UG_RESULT (*fill_frame_hwaccel)( UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR c );
} ugui_drv_t;

#endif
