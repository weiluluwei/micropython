// ------------------------------------
// �GUI example project
// http://www.embeddedlightning.com/
// ------------------------------------
#include <stdint.h>
#include "pybugui.h"
#include "ugui_drv.h"
#include "touch.h"
#include "ili9341.h"

#define  THS                     10
#define  THB                     20
#define  THD                     240
#define  THF                     10
#define  TVS                     2
#define  TVB                     2
#define  TVD                     320
#define  TVF                     4

#define  HSPOL                   LTDC_HSPolarity_AL
#define  VSPOL                   LTDC_VSPolarity_AL
#define  DEPOL                   LTDC_DEPolarity_AL
#define  PCLKPOL                 LTDC_PCPolarity_IPC

#define  LCD_PIXEL_WIDTH         (THD)
#define  LCD_PIXEL_HEIGHT        (TVD)

#define  LCD_COLOR_DEPTH         ((uint16_t)2)       /* Farbtiefe: RGB565 --> 2 Bytes / Pixel */

#define  LAYER_1                 LAYER_BACKGROUND
#define  LAYER_2                 LAYER_FOREGROUND
#define  LAYER_FOREGROUND        1
#define  LAYER_BACKGROUND        0


void init0(mem_acc_t *mem_acc);
void init1(void);
void init2(void);
void pset(UG_S16 x, UG_S16 y, UG_COLOR col);
void update(void);

mem_acc_t mem_acc
/* Touch structure */
static TP_STATE* TP_State;

ugui_drv_t ugui_drv ={
	.descr = "LTDC/ILI9341 Driver for stm32l429-disco.",
	.x_size = LCD_PIXEL_WIDTH,
	.y_size = LCD_PIXEL_HEIGHT,
	.pixel_size = LCD_COLOR_DEPTH,
	.init0  = init0,
	.init1  = init1,
	.display_on = ili9341_DisplayOn,
	.display_off = ili9341_DisplayOff,
	.pset = pset,
	.update = update,
	.draw_line_hwaccel = draw_line_hwaccel,
	.fill_frame_hwaccel = fill_frame_hwaccel
};


void init0(mem_acc_t *pMem_acc)
{
	mem_acc = *pMem_acc;
	ltdc_init();
	ili9341_init();
    IOE_Config();
}

void init1(void)
{
   /* Clear Screen */
   ltdc_draw_layer(LAYER_1);
   ltdc_show_layer(LAYER_1);
}


void pset(UG_S16 x, UG_S16 y, UG_COLOR col)
{
	uint32_t addr;
	uint8_t r,g,b;

	if ( x<0 ) return;
	if ( y<0 ) return;

	addr = x + y * LCD_PIXEL_WIDTH;
	addr<<=1;

	r = col>>16 & 0xFF;
	g = col>>8 & 0xFF;
	b = col & 0xFF;

	r >>= 3;
	g >>= 2;
	b >>= 3;

	col = b | (g<<5) | (r<<11);

	if ( ltdc_work_layer == LAYER_1 )
	{
		*((uint16_t *)(mem_acc.pMem1+addr)) = col;
	}
	else
	{
		*((uint16_t *)(mem_acc.pMem2+addr)) = col;
	}
}


/* Hardware accelerator for UG_DrawLine (Platform: STM32F4x9) */
UG_RESULT draw_line_hwaccel( UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR c )
{
   DMA2D_InitTypeDef DMA2D_InitStruct;

   RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA2D, ENABLE);
   RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA2D, DISABLE);
   DMA2D_InitStruct.DMA2D_Mode = DMA2D_R2M;
   DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;
   /* Convert UG_COLOR to RGB565 */
   DMA2D_InitStruct.DMA2D_OutputBlue = (c>>3) & 0x1F;
   DMA2D_InitStruct.DMA2D_OutputGreen = (c>>10) & 0x3F;
   DMA2D_InitStruct.DMA2D_OutputRed = (c>>19) & 0x1F;
   DMA2D_InitStruct.DMA2D_OutputAlpha = 0x0F;

   /* horizontal line */
   if ( y1 == y2 )
   {
      DMA2D_InitStruct.DMA2D_OutputOffset = 0;
      DMA2D_InitStruct.DMA2D_NumberOfLine = 1;
      DMA2D_InitStruct.DMA2D_PixelPerLine = x2-x1+1;
   }
   /* vertical line */
   else if ( x1 == x2 )
   {
      DMA2D_InitStruct.DMA2D_OutputOffset = LCD_PIXEL_WIDTH - 1;
      DMA2D_InitStruct.DMA2D_NumberOfLine = y2-y1+1;
      DMA2D_InitStruct.DMA2D_PixelPerLine = 1;
   }
   else
   {
      return UG_RESULT_FAIL;
   }

   if ( ltdc_work_layer == LAYER_1 )
   {
      DMA2D_InitStruct.DMA2D_OutputMemoryAdd = mem_acc.pMem1 + 2*(LCD_PIXEL_WIDTH * y1 + x1);
   }
   else
   {
      DMA2D_InitStruct.DMA2D_OutputMemoryAdd = mem_acc.pMem2 + 2*(LCD_PIXEL_WIDTH * y1 + x1);
   }
   DMA2D_Init(&DMA2D_InitStruct);
   DMA2D_StartTransfer();
   while(DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET){};
   return UG_RESULT_OK;
}

/* Hardware accelerator for UG_FillFrame (Platform: STM32F4x9) */
UG_RESULT fill_frame_hwaccel( UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR c )
{
   DMA2D_InitTypeDef      DMA2D_InitStruct;

   DMA2D_DeInit();
   DMA2D_InitStruct.DMA2D_Mode = DMA2D_R2M;
   DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;
   /* Convert UG_COLOR to RGB565 */
   DMA2D_InitStruct.DMA2D_OutputBlue = (c>>3) & 0x1F;
   DMA2D_InitStruct.DMA2D_OutputGreen = (c>>10) & 0x3F;
   DMA2D_InitStruct.DMA2D_OutputRed = (c>>19) & 0x1F;
   DMA2D_InitStruct.DMA2D_OutputAlpha = 0x0F;
   DMA2D_InitStruct.DMA2D_OutputOffset = (LCD_PIXEL_WIDTH - (x2-x1+1));
   DMA2D_InitStruct.DMA2D_NumberOfLine = y2-y1+1;
   DMA2D_InitStruct.DMA2D_PixelPerLine = x2-x1+1;
   if ( ltdc_work_layer == LAYER_1 )
   {
      DMA2D_InitStruct.DMA2D_OutputMemoryAdd = mem_acc.pMem1 + 2*(LCD_PIXEL_WIDTH * y1 + x1);
   }
   else
   {
      DMA2D_InitStruct.DMA2D_OutputMemoryAdd = mem_acc.pMem1 + 2*(LCD_PIXEL_WIDTH * y1 + x1);
   }
   DMA2D_Init(&DMA2D_InitStruct);

   DMA2D_StartTransfer();
   while(DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET){}
   return UG_RESULT_OK;
}

/* Regularly called update function (best called from systicks */
void update(void)
{
	TP_State = IOE_TP_GetState();
	if( TP_State->TouchDetected )
	{
		if ( (TP_State->X > 0) && (TP_State->X < 239 ) )
		{
			if ( (TP_State->Y > 0) && (TP_State->Y < 319 ) )
			{
			   UG_TouchUpdate(TP_State->X,TP_State->Y,TOUCH_STATE_PRESSED);
			}
		}
	}
	else
	{
		UG_TouchUpdate(-1,-1,TOUCH_STATE_RELEASED);
	}

	UG_Update();
}


