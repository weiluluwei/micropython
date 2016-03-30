MCU_SERIES = l4
CMSIS_MCU = STM32L476xx
AF_FILE = boards/stm32l476_af.csv
LD_FILE = boards/stm32l476xe.ld

deploy2b: $(BUILD)/firmware.dfu
	$(ECHO) "Writing $(BUILD)/firmware0.bin to the board via ST-LINK"
	$(Q)$(STFLASH) write $(BUILD)/firmware0.bin 0x08000000
	$(ECHO) "Writing $(BUILD)/firmware1.bin to the board via ST-LINK"
	$(Q)$(STFLASH) --reset write $(BUILD)/firmware1.bin 0x08008800
