# Select the board to build for: if not given on the command line,
# then default to PYBV10.
BOARD ?= PYBV10
ifeq ($(wildcard boards/$(BOARD)/.),)
$(error Invalid BOARD specified)
endif

# If the build directory is not given, make it reflect the board name.
BUILD ?= build-$(BOARD)

include ../py/mkenv.mk
-include mpconfigport.mk
include boards/$(BOARD)/mpconfigboard.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h $(BUILD)/pins_qstr.h $(BUILD)/modstm_qstr.h

# include py core make definitions
include ../py/py.mk

CMSIS_DIR=cmsis
HAL_DIR=hal/$(MCU_SERIES)
USBDEV_DIR=usbdev
#USBHOST_DIR=usbhost
FATFS_DIR=lib/fatfs
DFU=../tools/dfu.py
# may need to prefix dfu-util with sudo
USE_PYDFU ?= 0
PYDFU = ../tools/pydfu.py
DFU_UTIL ?= dfu-util
DEVICE=0483:df11

CROSS_COMPILE = arm-none-eabi-

INC += -I.
INC += -I..
INC += -I$(BUILD)
INC += -I$(CMSIS_DIR)/inc
INC += -I$(CMSIS_DIR)/devinc
INC += -I$(HAL_DIR)/inc
INC += -I$(USBDEV_DIR)/core/inc -I$(USBDEV_DIR)/class/inc
#INC += -I$(USBHOST_DIR)
INC += -I../lib/mp-readline
INC += -I../lib/netutils
INC += -I../lib/timeutils

CFLAGS_CORTEX_M = -mthumb -mabi=aapcs-linux -mfpu=fpv4-sp-d16 -mfloat-abi=hard -fsingle-precision-constant -Wdouble-promotion
CFLAGS_MCU_f4 = $(CFLAGS_CORTEX_M) -mtune=cortex-m4 -mcpu=cortex-m4 -DMCU_SERIES_F4
CFLAGS_MCU_f7 = $(CFLAGS_CORTEX_M) -mtune=cortex-m7 -mcpu=cortex-m7 -DMCU_SERIES_F7

#CFLAGS = $(INC) -Wall -Wpointer-arith -Werror -ansi -std=gnu99 -nostdlib $(CFLAGS_MOD)
CFLAGS = $(INC) -Wall -Wpointer-arith -ansi -std=gnu99 -nostdlib $(CFLAGS_MOD)
CFLAGS += -D$(CMSIS_MCU)
CFLAGS += $(CFLAGS_MCU_$(MCU_SERIES))
CFLAGS += $(COPT)
CFLAGS += -Iboards/$(BOARD)
CFLAGS += -DSTM32_HAL_H='<stm32$(MCU_SERIES)xx_hal.h>'

LDFLAGS = -nostdlib -T $(LD_FILE) -Map=$(@:.elf=.map) --cref
LIBS =

# Remove uncalled code from the final image.
CFLAGS += -fdata-sections -ffunction-sections
LDFLAGS += --gc-sections

# Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -g -DPENDSV_DEBUG
COPT = -O0
else
COPT += -Os -DNDEBUG
endif

# uncomment this if you want libgcc
#LIBS += $(shell $(CC) -print-libgcc-file-name)

SRC_LIB = $(addprefix lib/,\
	libc/string0.c \
	libm/math.c \
	libm/asinfacosf.c \
	libm/atanf.c \
	libm/atan2f.c \
	libm/fmodf.c \
	libm/roundf.c \
	libm/log1pf.c \
	libm/acoshf.c \
	libm/asinhf.c \
	libm/atanhf.c \
	libm/kf_rem_pio2.c \
	libm/kf_sin.c \
	libm/kf_cos.c \
	libm/kf_tan.c \
	libm/ef_rem_pio2.c \
	libm/erf_lgamma.c \
	libm/sf_sin.c \
	libm/sf_cos.c \
	libm/sf_tan.c \
	libm/sf_frexp.c \
	libm/sf_modf.c \
	libm/sf_ldexp.c \
	libm/sf_erf.c \
	libm/wf_lgamma.c \
	libm/wf_tgamma.c \
	fatfs/ff.c \
	fatfs/option/ccsbcs.c \
	mp-readline/readline.c \
	netutils/netutils.c \
	timeutils/timeutils.c \
	utils/pyexec.c \
	utils/printf.c \
	)

SRC_C = \
	main.c \
	system_stm32.c \
	stm32_it.c \
	usbd_conf.c \
	usbd_desc.c \
	usbd_cdc_interface.c \
	usbd_msc_storage.c \
	mphalport.c \
	irq.c \
	pendsv.c \
	systick.c  \
	timer.c \
	led.c \
	pin.c \
	pin_defs_stmhal.c \
	pin_named_pins.c \
	bufhelper.c \
	dma.c \
	i2c.c \
	spi.c \
	uart.c \
	can.c \
	usb.c \
	gccollect.c \
	pybstdio.c \
	help.c \
	input.c \
	modmachine.c \
	modpyb.c \
	modstm.c \
	moduos.c \
	modutime.c \
	moduselect.c \
	modusocket.c \
	modnetwork.c \
	import.c \
	lexerfatfs.c \
	extint.c \
	usrsw.c \
	rng.c \
	rtc.c \
	flash.c \
	storage.c \
	file.c \
	sdcard.c \
	diskio.c \
	ffconf.c \
	lcd.c \
	accel.c \
	servo.c \
	dac.c \
	adc.c \
	$(wildcard boards/$(BOARD)/*.c)

SRC_O = \
	startup_stm32.o \
	gchelper.o \

SRC_HAL = $(addprefix $(HAL_DIR)/src/stm32$(MCU_SERIES)xx_,\
	hal.c \
	hal_adc.c \
	hal_adc_ex.c \
	hal_can.c \
	hal_cortex.c \
	hal_dac.c \
	hal_dac_ex.c \
	hal_dma.c \
	hal_flash.c \
	hal_flash_ex.c \
	hal_gpio.c \
	hal_i2c.c \
	hal_pcd.c \
	hal_pcd_ex.c \
	hal_pwr.c \
	hal_pwr_ex.c \
	hal_rcc.c \
	hal_rcc_ex.c \
	hal_rng.c \
	hal_rtc.c \
	hal_rtc_ex.c \
	hal_sd.c \
	hal_spi.c \
	hal_tim.c \
	hal_tim_ex.c \
	hal_uart.c \
	ll_sdmmc.c \
	ll_usb.c \
	)

SRC_USBDEV = $(addprefix $(USBDEV_DIR)/,\
	core/src/usbd_core.c \
	core/src/usbd_ctlreq.c \
	core/src/usbd_ioreq.c \
	class/src/usbd_cdc_msc_hid.c \
	class/src/usbd_msc_bot.c \
	class/src/usbd_msc_scsi.c \
	class/src/usbd_msc_data.c \
	)

ifeq ($(MICROPY_PY_WIZNET5K),1)
WIZNET5K_DIR=drivers/wiznet5k
INC += -I$(TOP)/$(WIZNET5K_DIR)
CFLAGS_MOD += -DMICROPY_PY_WIZNET5K=1
SRC_MOD += modnwwiznet5k.c
SRC_MOD += $(addprefix $(WIZNET5K_DIR)/,\
	ethernet/w5200/w5200.c \
	ethernet/wizchip_conf.c \
	ethernet/socket.c \
	internet/dns/dns.c \
	)
endif

# for CC3000 module
ifeq ($(MICROPY_PY_CC3K),1)
CC3000_DIR=drivers/cc3000
INC += -I$(TOP)/$(CC3000_DIR)/inc
CFLAGS_MOD += -DMICROPY_PY_CC3K=1
SRC_MOD += modnwcc3k.c
SRC_MOD += $(addprefix $(CC3000_DIR)/src/,\
	cc3000_common.c \
	evnt_handler.c \
	hci.c \
	netapp.c \
	nvmem.c \
	security.c \
	socket.c \
	wlan.c \
	ccspi.c \
	inet_ntop.c \
	inet_pton.c \
	patch.c \
	patch_prog.c \
	)
endif

# for u8glib module
ifeq ($(MICROPY_PY_LCDCTRL),1)
U8GLIB_DIR=lib/u8glib
INC += -I$(TOP)/$(U8GLIB_DIR)/csrc
INC += -Iu8glib_adapter
CFLAGS_MOD += -DMICROPY_PY_LCDCTRL=1
SRC_MOD += $(addprefix $(U8GLIB_DIR)/csrc/,\
    chessengine.c \
    u8g_bitmap.c \
    u8g_circle.c \
    u8g_clip.c \
    u8g_com_api_16gr.c \
    u8g_com_api.c \
    u8g_com_arduino_attiny85_hw_spi.c \
    u8g_com_arduino_common.c \
    u8g_com_arduino_fast_parallel.c \
    u8g_com_arduino_hw_spi.c \
    u8g_com_arduino_hw_usart_spi.c \
    u8g_com_arduino_no_en_parallel.c \
    u8g_com_arduino_parallel.c \
    u8g_com_arduino_port_d_wr.c \
    u8g_com_arduino_ssd_i2c.c \
    u8g_com_arduino_st7920_custom.c \
    u8g_com_arduino_st7920_hw_spi.c \
    u8g_com_arduino_st7920_spi.c \
    u8g_com_arduino_std_sw_spi.c \
    u8g_com_arduino_sw_spi.c \
    u8g_com_arduino_t6963.c \
    u8g_com_arduino_uc_i2c.c \
    u8g_com_atmega_hw_spi.c \
    u8g_com_atmega_parallel.c \
    u8g_com_atmega_st7920_hw_spi.c \
    u8g_com_atmega_st7920_spi.c \
    u8g_com_atmega_sw_spi.c \
    u8g_com_atxmega_hw_spi.c \
    u8g_com_atxmega_st7920_hw_spi.c \
    u8g_com_i2c.c \
    u8g_com_io.c \
    u8g_com_linux_ssd_i2c.c \
    u8g_com_msp430_hw_spi.c \
    u8g_com_null.c \
    u8g_com_psoc5_ssd_hw_parallel.c \
    u8g_com_raspberrypi_hw_spi.c \
    u8g_com_raspberrypi_ssd_i2c.c \
    u8g_com_std_sw_spi.c \
    u8g_cursor.c \
    u8g_delay.c \
    u8g_dev_a2_micro_printer.c \
    u8g_dev_flipdisc_2x7.c \
    u8g_dev_gprof.c \
    u8g_dev_ht1632.c \
    u8g_dev_ili9325d_320x240.c \
    u8g_dev_ks0108_128x64.c \
    u8g_dev_lc7981_160x80.c \
    u8g_dev_lc7981_240x128.c \
    u8g_dev_lc7981_240x64.c \
    u8g_dev_lc7981_320x64.c \
    u8g_dev_ld7032_60x32.c \
    u8g_dev_null.c \
    u8g_dev_pcd8544_84x48.c \
    u8g_dev_pcf8812_96x65.c \
    u8g_dev_sbn1661_122x32.c \
    u8g_dev_ssd1306_128x32.c \
    u8g_dev_ssd1306_128x64.c \
    u8g_dev_ssd1306_64x48.c \
    u8g_dev_ssd1309_128x64.c \
    u8g_dev_ssd1322_nhd31oled_bw.c \
    u8g_dev_ssd1322_nhd31oled_gr.c \
    u8g_dev_ssd1325_nhd27oled_bw.c \
    u8g_dev_ssd1325_nhd27oled_bw_new.c \
    u8g_dev_ssd1325_nhd27oled_gr.c \
    u8g_dev_ssd1325_nhd27oled_gr_new.c \
    u8g_dev_ssd1327_96x96_gr.c \
    u8g_dev_ssd1351_128x128.c \
    u8g_dev_ssd1353_160x128.c \
    u8g_dev_st7565_64128n.c \
    u8g_dev_st7565_dogm128.c \
    u8g_dev_st7565_dogm132.c \
    u8g_dev_st7565_lm6059.c \
    u8g_dev_st7565_lm6063.c \
    u8g_dev_st7565_nhd_c12832.c \
    u8g_dev_st7565_nhd_c12864.c \
    u8g_dev_st7687_c144mvgd.c \
    u8g_dev_st7920_128x64.c \
    u8g_dev_st7920_192x32.c \
    u8g_dev_st7920_202x32.c \
    u8g_dev_t6963_128x128.c \
    u8g_dev_t6963_128x64.c \
    u8g_dev_t6963_240x128.c \
    u8g_dev_t6963_240x64.c \
    u8g_dev_tls8204_84x48.c \
    u8g_dev_uc1601_c128032.c \
    u8g_dev_uc1608_240x128.c \
    u8g_dev_uc1608_240x64.c \
    u8g_dev_uc1610_dogxl160.c \
    u8g_dev_uc1611_dogm240.c \
    u8g_dev_uc1611_dogxl240.c \
    u8g_dev_uc1701_dogs102.c \
    u8g_dev_uc1701_mini12864.c \
    u8g_ellipse.c \
    u8g_font.c \
    u8g_line.c \
    u8g_ll_api.c \
    u8g_page.c \
    u8g_pb14v1.c \
    u8g_pb16h1.c \
    u8g_pb16h2.c \
    u8g_pb16v1.c \
    u8g_pb16v2.c \
    u8g_pb32h1.c \
    u8g_pb8h1.c \
    u8g_pb8h1f.c \
    u8g_pb8h2.c \
    u8g_pb8h8.c \
    u8g_pb8v1.c \
    u8g_pb8v2.c \
    u8g_pb.c \
    u8g_pbxh16.c \
    u8g_pbxh24.c \
    u8g_polygon.c \
    u8g_rect.c \
    u8g_rot.c \
    u8g_scale.c \
    u8g_state.c \
    u8g_u16toa.c \
    u8g_u8toa.c \
    u8g_virtual_screen.c \
	)
SRC_MOD += $(addprefix $(U8GLIB_DIR)/fntsrc/,\
    u8g_font_04b_03b.c \
    u8g_font_04b_03bn.c \
    u8g_font_04b_03br.c \
    u8g_font_04b_03.c \
    u8g_font_04b_03n.c \
    u8g_font_04b_03r.c \
    u8g_font_04b_24.c \
    u8g_font_04b_24n.c \
    u8g_font_04b_24r.c \
    u8g_font_10x20_67_75.c \
    u8g_font_10x20_75r.c \
    u8g_font_10x20_78_79.c \
    u8g_font_10x20.c \
    u8g_font_10x20r.c \
    u8g_font_4x6.c \
    u8g_font_4x6r.c \
    u8g_font_5x7.c \
    u8g_font_5x7r.c \
    u8g_font_5x8.c \
    u8g_font_5x8r.c \
    u8g_font_6x10.c \
    u8g_font_6x10r.c \
    u8g_font_6x12_67_75.c \
    u8g_font_6x12_75r.c \
    u8g_font_6x12_78_79.c \
    u8g_font_6x12.c \
    u8g_font_6x12r.c \
    u8g_font_6x13_67_75.c \
    u8g_font_6x13_75r.c \
    u8g_font_6x13_78_79.c \
    u8g_font_6x13B.c \
    u8g_font_6x13Br.c \
    u8g_font_6x13.c \
    u8g_font_6x13O.c \
    u8g_font_6x13Or.c \
    u8g_font_6x13r.c \
    u8g_font_7x13_67_75.c \
    u8g_font_7x13_75r.c \
    u8g_font_7x13B.c \
    u8g_font_7x13Br.c \
    u8g_font_7x13.c \
    u8g_font_7x13O.c \
    u8g_font_7x13Or.c \
    u8g_font_7x13r.c \
    u8g_font_7x14B.c \
    u8g_font_7x14Br.c \
    u8g_font_7x14.c \
    u8g_font_7x14r.c \
    u8g_font_8x13_67_75.c \
    u8g_font_8x13_75r.c \
    u8g_font_8x13B.c \
    u8g_font_8x13Br.c \
    u8g_font_8x13.c \
    u8g_font_8x13O.c \
    u8g_font_8x13Or.c \
    u8g_font_8x13r.c \
    u8g_font_9x15_67_75.c \
    u8g_font_9x15_75r.c \
    u8g_font_9x15_78_79.c \
    u8g_font_9x15B.c \
    u8g_font_9x15Br.c \
    u8g_font_9x15.c \
    u8g_font_9x15r.c \
    u8g_font_9x18_67_75.c \
    u8g_font_9x18_75r.c \
    u8g_font_9x18_78_79.c \
    u8g_font_9x18B.c \
    u8g_font_9x18Br.c \
    u8g_font_9x18.c \
    u8g_font_9x18r.c \
    u8g_font_baby.c \
    u8g_font_babyn.c \
    u8g_font_babyr.c \
    u8g_font_blipfest_07.c \
    u8g_font_blipfest_07n.c \
    u8g_font_blipfest_07r.c \
    u8g_font_chikita.c \
    u8g_font_chikitan.c \
    u8g_font_chikitar.c \
    u8g_font_courb08.c \
    u8g_font_courb08r.c \
    u8g_font_courb10.c \
    u8g_font_courb10r.c \
    u8g_font_courb12.c \
    u8g_font_courb12r.c \
    u8g_font_courb14.c \
    u8g_font_courb14r.c \
    u8g_font_courb18.c \
    u8g_font_courb18r.c \
    u8g_font_courb24.c \
    u8g_font_courb24r.c \
    u8g_font_courb4n.c \
    u8g_font_courr08.c \
    u8g_font_courr08r.c \
    u8g_font_courr10.c \
    u8g_font_courr10r.c \
    u8g_font_courr12.c \
    u8g_font_courr12r.c \
    u8g_font_courr14.c \
    u8g_font_courr14r.c \
    u8g_font_courr18.c \
    u8g_font_courr18r.c \
    u8g_font_courr24.c \
    u8g_font_courr24n.c \
    u8g_font_courr24r.c \
    u8g_font_cu12_67_75.c \
    u8g_font_cu12_75r.c \
    u8g_font_cu12.c \
    u8g_font_cursor.c \
    u8g_font_cursorr.c \
    u8g_font_fixed_v0.c \
    u8g_font_fixed_v0n.c \
    u8g_font_fixed_v0r.c \
    u8g_font_freedoomr10r.c \
    u8g_font_freedoomr25n.c \
    u8g_font_fub11.c \
    u8g_font_fub11n.c \
    u8g_font_fub11r.c \
    u8g_font_fub14.c \
    u8g_font_fub14n.c \
    u8g_font_fub14r.c \
    u8g_font_fub17.c \
    u8g_font_fub17n.c \
    u8g_font_fub17r.c \
    u8g_font_fub20.c \
    u8g_font_fub20n.c \
    u8g_font_fub20r.c \
    u8g_font_fub25.c \
    u8g_font_fub25n.c \
    u8g_font_fub25r.c \
    u8g_font_fub30.c \
    u8g_font_fub30n.c \
    u8g_font_fub30r.c \
    u8g_font_fub35n.c \
    u8g_font_fub42n.c \
    u8g_font_fub49n.c \
    u8g_font_fur11.c \
    u8g_font_fur11n.c \
    u8g_font_fur11r.c \
    u8g_font_fur14.c \
    u8g_font_fur14n.c \
    u8g_font_fur14r.c \
    u8g_font_fur17.c \
    u8g_font_fur17n.c \
    u8g_font_fur17r.c \
    u8g_font_fur20.c \
    u8g_font_fur20n.c \
    u8g_font_fur20r.c \
    u8g_font_fur25.c \
    u8g_font_fur25n.c \
    u8g_font_fur25r.c \
    u8g_font_fur30.c \
    u8g_font_fur30n.c \
    u8g_font_fur30r.c \
    u8g_font_fur35n.c \
    u8g_font_fur42n.c \
    u8g_font_fur49n.c \
    u8g_font_gdb11.c \
    u8g_font_gdb11n.c \
    u8g_font_gdb11r.c \
    u8g_font_gdb12.c \
    u8g_font_gdb12n.c \
    u8g_font_gdb12r.c \
    u8g_font_gdb14.c \
    u8g_font_gdb14n.c \
    u8g_font_gdb14r.c \
    u8g_font_gdb17.c \
    u8g_font_gdb17n.c \
    u8g_font_gdb17r.c \
    u8g_font_gdb20.c \
    u8g_font_gdb20n.c \
    u8g_font_gdb20r.c \
    u8g_font_gdb25.c \
    u8g_font_gdb25n.c \
    u8g_font_gdb25r.c \
    u8g_font_gdb30.c \
    u8g_font_gdb30n.c \
    u8g_font_gdb30r.c \
    u8g_font_gdr10.c \
    u8g_font_gdr10n.c \
    u8g_font_gdr10r.c \
    u8g_font_gdr11.c \
    u8g_font_gdr11n.c \
    u8g_font_gdr11r.c \
    u8g_font_gdr12.c \
    u8g_font_gdr12n.c \
    u8g_font_gdr12r.c \
    u8g_font_gdr14.c \
    u8g_font_gdr14n.c \
    u8g_font_gdr14r.c \
    u8g_font_gdr17.c \
    u8g_font_gdr17n.c \
    u8g_font_gdr17r.c \
    u8g_font_gdr20.c \
    u8g_font_gdr20n.c \
    u8g_font_gdr20r.c \
    u8g_font_gdr25.c \
    u8g_font_gdr25n.c \
    u8g_font_gdr25r.c \
    u8g_font_gdr30.c \
    u8g_font_gdr30n.c \
    u8g_font_gdr30r.c \
    u8g_font_gdr9.c \
    u8g_font_gdr9n.c \
    u8g_font_gdr9r.c \
    u8g_font_helvb08.c \
    u8g_font_helvb08n.c \
    u8g_font_helvb08r.c \
    u8g_font_helvb10.c \
    u8g_font_helvb10n.c \
    u8g_font_helvb10r.c \
    u8g_font_helvb12.c \
    u8g_font_helvb12n.c \
    u8g_font_helvb12r.c \
    u8g_font_helvb14.c \
    u8g_font_helvb14n.c \
    u8g_font_helvb14r.c \
    u8g_font_helvb18.c \
    u8g_font_helvb18n.c \
    u8g_font_helvb18r.c \
    u8g_font_helvb24.c \
    u8g_font_helvb24n.c \
    u8g_font_helvb24r.c \
    u8g_font_helvr08.c \
    u8g_font_helvr08r.c \
    u8g_font_helvr10.c \
    u8g_font_helvr10r.c \
    u8g_font_helvr12.c \
    u8g_font_helvr12r.c \
    u8g_font_helvr14.c \
    u8g_font_helvr14r.c \
    u8g_font_helvr18.c \
    u8g_font_helvr18r.c \
    u8g_font_helvr24.c \
    u8g_font_helvr24n.c \
    u8g_font_helvr24r.c \
    u8g_font_lucasfont_alternate.c \
    u8g_font_lucasfont_alternaten.c \
    u8g_font_lucasfont_alternater.c \
    u8g_font_m2icon_5.c \
    u8g_font_m2icon_7.c \
    u8g_font_m2icon_9.c \
    u8g_font_micro.c \
    u8g_font_ncenb08.c \
    u8g_font_ncenb08r.c \
    u8g_font_ncenb10.c \
    u8g_font_ncenb10r.c \
    u8g_font_ncenb12.c \
    u8g_font_ncenb12r.c \
    u8g_font_ncenb14.c \
    u8g_font_ncenb14r.c \
    u8g_font_ncenb18.c \
    u8g_font_ncenb18r.c \
    u8g_font_ncenb24.c \
    u8g_font_ncenb24n.c \
    u8g_font_ncenb24r.c \
    u8g_font_ncenr08.c \
    u8g_font_ncenr08r.c \
    u8g_font_ncenr10.c \
    u8g_font_ncenr10r.c \
    u8g_font_ncenr12.c \
    u8g_font_ncenr12r.c \
    u8g_font_ncenr14.c \
    u8g_font_ncenr14r.c \
    u8g_font_ncenr18.c \
    u8g_font_ncenr18r.c \
    u8g_font_ncenr24.c \
    u8g_font_ncenr24n.c \
    u8g_font_ncenr24r.c \
    u8g_font_orgv01.c \
    u8g_font_orgv01n.c \
    u8g_font_orgv01r.c \
    u8g_font_osb18.c \
    u8g_font_osb18n.c \
    u8g_font_osb18r.c \
    u8g_font_osb21.c \
    u8g_font_osb21n.c \
    u8g_font_osb21r.c \
    u8g_font_osb26.c \
    u8g_font_osb26n.c \
    u8g_font_osb26r.c \
    u8g_font_osb29.c \
    u8g_font_osb29n.c \
    u8g_font_osb29r.c \
    u8g_font_osb35.c \
    u8g_font_osb35n.c \
    u8g_font_osb35r.c \
    u8g_font_osr18.c \
    u8g_font_osr18n.c \
    u8g_font_osr18r.c \
    u8g_font_osr21.c \
    u8g_font_osr21n.c \
    u8g_font_osr21r.c \
    u8g_font_osr26.c \
    u8g_font_osr26n.c \
    u8g_font_osr26r.c \
    u8g_font_osr29.c \
    u8g_font_osr29n.c \
    u8g_font_osr29r.c \
    u8g_font_osr35.c \
    u8g_font_osr35n.c \
    u8g_font_osr35r.c \
    u8g_font_p01type.c \
    u8g_font_p01typen.c \
    u8g_font_p01typer.c \
    u8g_font_pixelle_micro.c \
    u8g_font_pixelle_micron.c \
    u8g_font_pixelle_micror.c \
    u8g_font_profont10.c \
    u8g_font_profont10r.c \
    u8g_font_profont11.c \
    u8g_font_profont11r.c \
    u8g_font_profont12.c \
    u8g_font_profont12r.c \
    u8g_font_profont15.c \
    u8g_font_profont15r.c \
    u8g_font_profont17.c \
    u8g_font_profont17r.c \
    u8g_font_profont22.c \
    u8g_font_profont22r.c \
    u8g_font_profont29.c \
    u8g_font_profont29r.c \
    u8g_font_robot_de_niro.c \
    u8g_font_robot_de_niron.c \
    u8g_font_robot_de_niror.c \
    u8g_font_symb08.c \
    u8g_font_symb08r.c \
    u8g_font_symb10.c \
    u8g_font_symb10r.c \
    u8g_font_symb12.c \
    u8g_font_symb12r.c \
    u8g_font_symb14.c \
    u8g_font_symb14r.c \
    u8g_font_symb18.c \
    u8g_font_symb18r.c \
    u8g_font_symb24.c \
    u8g_font_symb24r.c \
    u8g_font_timb08.c \
    u8g_font_timb08r.c \
    u8g_font_timb10.c \
    u8g_font_timb10r.c \
    u8g_font_timb12.c \
    u8g_font_timb12r.c \
    u8g_font_timb14.c \
    u8g_font_timb14r.c \
    u8g_font_timb18.c \
    u8g_font_timb18r.c \
    u8g_font_timb24.c \
    u8g_font_timb24n.c \
    u8g_font_timb24r.c \
    u8g_font_timr08.c \
    u8g_font_timr08r.c \
    u8g_font_timr10.c \
    u8g_font_timr10r.c \
    u8g_font_timr12.c \
    u8g_font_timr12r.c \
    u8g_font_timr14.c \
    u8g_font_timr14r.c \
    u8g_font_timr18.c \
    u8g_font_timr18r.c \
    u8g_font_timr24.c \
    u8g_font_timr24n.c \
    u8g_font_timr24r.c \
    u8g_font_tpssb.c \
    u8g_font_tpssbn.c \
    u8g_font_tpssbr.c \
    u8g_font_tpss.c \
    u8g_font_tpssn.c \
    u8g_font_tpssr.c \
    u8g_font_trixel_square.c \
    u8g_font_trixel_squaren.c \
    u8g_font_trixel_squarer.c \
    u8g_font_u8glib_4.c \
    u8g_font_u8glib_4r.c \
    u8g_font_unifont_0_8.c \
    u8g_font_unifont_12_13.c \
    u8g_font_unifont_18_19.c \
    u8g_font_unifont_2_3.c \
    u8g_font_unifont_4_5.c \
    u8g_font_unifont_67_75.c \
    u8g_font_unifont_72_73.c \
    u8g_font_unifont_75r.c \
    u8g_font_unifont_76.c \
    u8g_font_unifont_77.c \
    u8g_font_unifont_78_79.c \
    u8g_font_unifont_86.c \
    u8g_font_unifont_8_9.c \
    u8g_font_unifont.c \
    u8g_font_unifontr.c \
	)
SRC_MOD += $(addprefix u8glib_adapter/,\
    ili9341.c \
    stm32f429i_discovery_lcd.c \
    u8glib_adapter.c \
    )
endif

 
OBJ =
OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_LIB:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_O))
OBJ += $(addprefix $(BUILD)/, $(SRC_HAL:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_USBDEV:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_MOD:.c=.o))
OBJ += $(BUILD)/pins_$(BOARD).o

# We put ff.o and stm32f4xx_hal_sd.o into the first 16K section with the ISRs.
# If we compile these using -O0 then it won't fit. So if you really want these
# to be compiled with -O0, then edit stm32f405.ld (in the .isr_vector section)
# and comment out the following 2 lines.
$(BUILD)/$(FATFS_DIR)/ff.o: COPT += -Os
$(BUILD)/$(HAL_DIR)/src/stm32$(MCU_SERIES)xx_hal_sd.o: COPT += -Os

all: $(BUILD)/firmware.dfu $(BUILD)/firmware.hex

.PHONY: deploy

deploy: $(BUILD)/firmware.dfu
	$(ECHO) "Writing $< to the board"
ifeq ($(USE_PYDFU),1)
	$(Q)$(PYTHON) $(PYDFU) -u $<
else
	$(Q)$(DFU_UTIL) -a 0 -d $(DEVICE) -D $<
endif

$(BUILD)/firmware.dfu: $(BUILD)/firmware.elf
	$(ECHO) "Create $@"
	$(Q)$(OBJCOPY) -O binary -j .isr_vector $^ $(BUILD)/firmware0.bin
	$(Q)$(OBJCOPY) -O binary -j .text -j .data $^ $(BUILD)/firmware1.bin
	$(Q)$(PYTHON) $(DFU) -b 0x08000000:$(BUILD)/firmware0.bin -b 0x08020000:$(BUILD)/firmware1.bin $@

$(BUILD)/firmware.hex: $(BUILD)/firmware.elf
	$(ECHO) "Create $@"
	$(Q)$(OBJCOPY) -O ihex $< $@

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(Q)$(SIZE) $@

MAKE_PINS = boards/make-pins.py
BOARD_PINS = boards/$(BOARD)/pins.csv
PREFIX_FILE = boards/stm32f4xx_prefix.c
GEN_PINS_SRC = $(BUILD)/pins_$(BOARD).c
GEN_PINS_HDR = $(HEADER_BUILD)/pins.h
GEN_PINS_QSTR = $(BUILD)/pins_qstr.h
GEN_PINS_AF_CONST = $(HEADER_BUILD)/pins_af_const.h
GEN_PINS_AF_PY = $(BUILD)/pins_af.py

INSERT_USB_IDS = ../tools/insert-usb-ids.py
FILE2H = ../tools/file2h.py

USB_IDS_FILE = usb.h
CDCINF_TEMPLATE = pybcdc.inf_template
GEN_CDCINF_FILE = $(HEADER_BUILD)/pybcdc.inf
GEN_CDCINF_HEADER = $(HEADER_BUILD)/pybcdc_inf.h

# Making OBJ use an order-only depenedency on the generated pins.h file
# has the side effect of making the pins.h file before we actually compile
# any of the objects. The normal dependency generation will deal with the
# case when pins.h is modified. But when it doesn't exist, we don't know
# which source files might need it.
$(OBJ): | $(HEADER_BUILD)/pins.h

# With conditional pins, we may need to regenerate qstrdefs.h when config
# options change.
$(HEADER_BUILD)/qstrdefs.generated.h: boards/$(BOARD)/mpconfigboard.h

$(BUILD)/main.o: $(GEN_CDCINF_HEADER)

# Use a pattern rule here so that make will only call make-pins.py once to make
# both pins_$(BOARD).c and pins.h
$(BUILD)/%_$(BOARD).c $(HEADER_BUILD)/%.h $(HEADER_BUILD)/%_af_const.h $(BUILD)/%_qstr.h: boards/$(BOARD)/%.csv $(MAKE_PINS) $(AF_FILE) $(PREFIX_FILE) | $(HEADER_BUILD)
	$(ECHO) "Create $@"
	$(Q)$(PYTHON) $(MAKE_PINS) --board $(BOARD_PINS) --af $(AF_FILE) --prefix $(PREFIX_FILE) --hdr $(GEN_PINS_HDR) --qstr $(GEN_PINS_QSTR) --af-const $(GEN_PINS_AF_CONST) --af-py $(GEN_PINS_AF_PY) > $(GEN_PINS_SRC)

$(BUILD)/pins_$(BOARD).o: $(BUILD)/pins_$(BOARD).c
	$(call compile_c)

GEN_STMCONST_HDR = $(HEADER_BUILD)/modstm_const.h
GEN_STMCONST_QSTR = $(BUILD)/modstm_qstr.h
CMSIS_MCU_LOWER = $(shell echo $(CMSIS_MCU) | tr '[:upper:]' '[:lower:]')
CMSIS_MCU_HDR = cmsis/devinc/$(CMSIS_MCU_LOWER).h

$(BUILD)/modstm.o: $(GEN_STMCONST_HDR)
# Use a pattern rule here so that make will only call make-stmconst.py once to
# make both modstm_const.h and modstm_qstr.h
$(HEADER_BUILD)/%_const.h $(BUILD)/%_qstr.h: $(CMSIS_MCU_HDR) make-stmconst.py | $(HEADER_BUILD)
	$(ECHO) "Create stmconst $@"
	$(Q)$(PYTHON) make-stmconst.py --qstr $(GEN_STMCONST_QSTR) $(CMSIS_MCU_HDR) > $(GEN_STMCONST_HDR)

$(GEN_CDCINF_HEADER): $(GEN_CDCINF_FILE) $(FILE2H) | $(HEADER_BUILD)
	$(ECHO) "Create $@"
	$(Q)$(PYTHON) $(FILE2H) $< > $@

$(GEN_CDCINF_FILE): $(CDCINF_TEMPLATE) $(INSERT_USB_IDS) $(USB_IDS_FILE) | $(HEADER_BUILD)
	$(ECHO) "Create $@"
	$(Q)$(PYTHON) $(INSERT_USB_IDS) $(USB_IDS_FILE) $< > $@

include ../py/mkrules.mk
