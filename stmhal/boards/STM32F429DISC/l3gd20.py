"""
Driver for accelerometer on STM32F429 Discover board.

Sets accelerometer range at +-2g.
Returns list containing X,Y,Z axis acceleration values in 'g' units (9.8m/s^2).

See:
    STM32Cube_FW_F4_V1.1.0/Drivers/BSP/Components/l3gd20/l3gd20.h
    STM32Cube_FW_F4_V1.1.0/Drivers/BSP/Components/l3gd20/l3gd20.c
    STM32Cube_FW_F4_V1.1.0/Drivers/BSP/STM32F4-Discovery/stm32f4_discovery.c
    STM32Cube_FW_F4_V1.1.0/Drivers/BSP/STM32F4-Discovery/stm32f4_discovery.h
    STM32Cube_FW_F4_V1.1.0/Drivers/BSP/STM32F4-Discovery/stm32f4_discovery_accelerometer.c
    STM32Cube_FW_F4_V1.1.0/Drivers/BSP/STM32F4-Discovery/stm32f4_discovery_accelerometer.h
    STM32Cube_FW_F4_V1.1.0/Projects/STM32F4-Discovery/Demonstrations/Src/main.c
"""

from pyb import Pin
from pyb import SPI
import struct

#
#    General SPI 
#
L3GD20_READWRITE_CMD = const(0x80) 
L3GD20_MULTIPLEBYTE_CMD = const(0x40)
#
# Constants for the SPI device.
#
L3GD20_WHO_AM_I_ADDR                 = const(0x0F)  # device identification register 
L3GD20_CTRL_REG1_ADDR                = const(0x20)  # Control register 1 
L3GD20_CTRL_REG2_ADDR                = const(0x21)  # Control register 2 
L3GD20_CTRL_REG3_ADDR                = const(0x22)  # Control register 3 
L3GD20_CTRL_REG4_ADDR                = const(0x23)  # Control register 4 
L3GD20_CTRL_REG5_ADDR                = const(0x24)  # Control register 5 
L3GD20_REFERENCE_REG_ADDR            = const(0x25)  # Reference register 
L3GD20_OUT_TEMP_ADDR                 = const(0x26)  # Out temp register 
L3GD20_STATUS_REG_ADDR               = const(0x27)  # Status register 
L3GD20_OUT_X_L_ADDR                  = const(0x28)  # Output Register X 
L3GD20_OUT_X_H_ADDR                  = const(0x29)  # Output Register X 
L3GD20_OUT_Y_L_ADDR                  = const(0x2A)  # Output Register Y 
L3GD20_OUT_Y_H_ADDR                  = const(0x2B)  # Output Register Y 
L3GD20_OUT_Z_L_ADDR                  = const(0x2C)  # Output Register Z 
L3GD20_OUT_Z_H_ADDR                  = const(0x2D)  # Output Register Z  
L3GD20_FIFO_CTRL_REG_ADDR            = const(0x2E)  # Fifo control Register 
L3GD20_FIFO_SRC_REG_ADDR             = const(0x2F)  # Fifo src Register 
L3GD20_INT1_CFG_ADDR                 = const(0x30)  # Interrupt 1 configuration Register 
L3GD20_INT1_SRC_ADDR                 = const(0x31)  # Interrupt 1 source Register 
L3GD20_INT1_TSH_XH_ADDR              = const(0x32)  # Interrupt 1 Threshold X register 
L3GD20_INT1_TSH_XL_ADDR              = const(0x33)  # Interrupt 1 Threshold X register 
L3GD20_INT1_TSH_YH_ADDR              = const(0x34)  # Interrupt 1 Threshold Y register 
L3GD20_INT1_TSH_YL_ADDR              = const(0x35)  # Interrupt 1 Threshold Y register 
L3GD20_INT1_TSH_ZH_ADDR              = const(0x36)  # Interrupt 1 Threshold Z register 
L3GD20_INT1_TSH_ZL_ADDR              = const(0x37)  # Interrupt 1 Threshold Z register 
L3GD20_INT1_DURATION_ADDR            = const(0x38)  # Interrupt 1 DURATION register 
L3GD20_I_AM_L3GD20                   = const(0xD4) 
L3GD20_I_AM_L3GD20_TR                = const(0xD5) 
L3GD20_MODE_POWERDOWN                = const(0x00) 
L3GD20_MODE_ACTIVE                   = const(0x08) 
L3GD20_OUTPUT_DATARATE_1             = const(0x00) 
L3GD20_OUTPUT_DATARATE_2             = const(0x40) 
L3GD20_OUTPUT_DATARATE_3             = const(0x80) 
L3GD20_OUTPUT_DATARATE_4             = const(0xC0) 
L3GD20_X_ENABLE                      = const(0x02) 
L3GD20_Y_ENABLE                      = const(0x01) 
L3GD20_Z_ENABLE                      = const(0x04) 
L3GD20_AXES_ENABLE                   = const(0x07) 
L3GD20_AXES_DISABLE                  = const(0x00) 
L3GD20_BANDWIDTH_1                   = const(0x00) 
L3GD20_BANDWIDTH_2                   = const(0x10) 
L3GD20_BANDWIDTH_3                   = const(0x20) 
L3GD20_BANDWIDTH_4                   = const(0x30) 
L3GD20_FULLSCALE_250                 = const(0x00) 
L3GD20_FULLSCALE_500                 = const(0x10) 
L3GD20_FULLSCALE_2000                = const(0x20)  
L3GD20_FULLSCALE_SELECTION           = const(0x30) 
L3GD20_SENSITIVITY_250DPS            = 8.75        #!< gyroscope sensitivity with 250 dps full scale [DPS/LSB]  
L3GD20_SENSITIVITY_500DPS            = 17.50       #!< gyroscope sensitivity with 500 dps full scale [DPS/LSB]  
L3GD20_SENSITIVITY_2000DPS           = 70.00       #!< gyroscope sensitivity with 2000 dps full scale [DPS/LSB] 
L3GD20_BlockDataUpdate_Continous     = const(0x00) 
L3GD20_BlockDataUpdate_Single        = const(0x80) 
L3GD20_BLE_LSB                       = const(0x00) 
L3GD20_BLE_MSB                       = const(0x40) 
L3GD20_HIGHPASSFILTER_DISABLE        = const(0x00) 
L3GD20_HIGHPASSFILTER_ENABLE         = const(0x10) 
L3GD20_INT1                          = const(0x00) 
L3GD20_INT2                          = const(0x01) 
L3GD20_INT1INTERRUPT_DISABLE         = const(0x00) 
L3GD20_INT1INTERRUPT_ENABLE          = const(0x80) 
L3GD20_INT2INTERRUPT_DISABLE         = const(0x00) 
L3GD20_INT2INTERRUPT_ENABLE          = const(0x08) 
L3GD20_INT1INTERRUPT_LOW_EDGE        = const(0x20) 
L3GD20_INT1INTERRUPT_HIGH_EDGE       = const(0x00) 
L3GD20_BOOT_NORMALMODE               = const(0x00) 
L3GD20_BOOT_REBOOTMEMORY             = const(0x80) 
L3GD20_HPM_NORMAL_MODE_RES           = const(0x00) 
L3GD20_HPM_REF_SIGNAL                = const(0x10) 
L3GD20_HPM_NORMAL_MODE               = const(0x20) 
L3GD20_HPM_AUTORESET_INT             = const(0x30) 
L3GD20_HPFCF_0                       = const(0x00)
L3GD20_HPFCF_1                       = const(0x01)
L3GD20_HPFCF_2                       = const(0x02)
L3GD20_HPFCF_3                       = const(0x03)
L3GD20_HPFCF_4                       = const(0x04)
L3GD20_HPFCF_5                       = const(0x05)
L3GD20_HPFCF_6                       = const(0x06)
L3GD20_HPFCF_7                       = const(0x07)
L3GD20_HPFCF_8                       = const(0x08)
L3GD20_HPFCF_9                       = const(0x09)
#
# Default configuration:
# Output data rate  190 Hz
# Bandwidth/Cut off  50 Hz
L3GD20_CTRL_REG1_VAL = const(0b01101111)
# Normal Mode
# Highpass cut-off 0.018Hz
L3GD20_CTRL_REG2_VAL = const(0b00001001)
# No interrupts
L3GD20_CTRL_REG3_VAL = const(0b00000000)
# Continous block update
# Little endinan data
# Full scale is 250 dps (degrees per second)
# Use 4 wire SPI
L3GD20_CTRL_REG4_VAL = const(0b00000000)
# Normal Mode
# FIFO is enabled
# No Highpass
# Int1/int2:00
#
L3GD20_CTRL_REG5_VAL = const(0b01000000)


class L3GD20:
    #
    # Debug 
    #
    DEFAULT_CONF = [ 
    (L3GD20_CTRL_REG1_ADDR, L3GD20_CTRL_REG1_VAL),
    (L3GD20_CTRL_REG2_ADDR, L3GD20_CTRL_REG2_VAL),
    (L3GD20_CTRL_REG3_ADDR, L3GD20_CTRL_REG3_VAL),
    (L3GD20_CTRL_REG4_ADDR, L3GD20_CTRL_REG4_VAL),
    (L3GD20_CTRL_REG5_ADDR, L3GD20_CTRL_REG5_VAL)]
    DEBUG = False
    def __init__(self, spiNr=5):
        self._conf={}
        self.cs_pin = Pin('PC1', Pin.OUT_PP, Pin.PULL_NONE)
        self.cs_pin.high()
        self.spi = SPI(spiNr, SPI.MASTER, baudrate=328125, polarity=1, phase=1, bits=8)
        self.who_am_i = self.read_id()
        if self.who_am_i == L3GD20_I_AM_L3GD20:
            for addr, val in L3GD20.DEFAULT_CONF:
                self.write_bytes(addr, bytearray([val,]))
                self._conf[addr] = val
        else:
            raise Exception('L3GD20 gyro not present')
        self.updateDpsFS()

    def updateDpsFS(self):
        conv = {
            L3GD20_FULLSCALE_250      : 250.0,
            L3GD20_FULLSCALE_500      : 500.0,     
            L3GD20_FULLSCALE_2000     : 2000.0,
            L3GD20_FULLSCALE_SELECTION: 2000.0}
        entry = self._conf[L3GD20_CTRL_REG4_ADDR] & L3GD20_FULLSCALE_SELECTION 
        self._dpsFS= conv[entry]

    def convert_raw_to_dps(self, x):
        x = x[1]*256+x[0]
        if x & 0x8000:
            x = x - 65536
        return x * self._dpsFS / 1000.0

    def read_bytes(self, addr, nbytes):
        addr |= L3GD20_READWRITE_CMD
        if nbytes > 1:
            addr |= L3GD20_MULTIPLEBYTE_CMD
        self.cs_pin.low()
        self.spi.send(addr)
        if self.DEBUG:
            print("SPI read addr: ", addr)
        buf = self.spi.recv(nbytes)
        self.cs_pin.high()
        if self.DEBUG:
            print("SPI read data", buf)
        return buf

    def write_bytes(self, addr, buf):
        if len(buf) > 1:
            addr |= L3GD20_MULTIPLEBYTE_CMD
        self.cs_pin.low()
        self.spi.send(addr)
        if self.DEBUG:
            print("SPI write addr: ", addr)
        for b in buf:
            self.spi.send(b)
            if self.DEBUG:
                print("SPI write data: ", b)
        self.cs_pin.high()

    def read_id(self):
        return self.read_bytes(L3GD20_WHO_AM_I_ADDR, 1)[0]

    def x(self):
        return self.convert_raw_to_dps(self.read_bytes(L3GD20_OUT_X_L_ADDR, 2))

    def y(self):
        return self.convert_raw_to_dps(self.read_bytes(L3GD20_OUT_Y_L_ADDR, 2))

    def z(self):
        return self.convert_raw_to_dps(self.read_bytes(L3GD20_OUT_Z_L_ADDR, 2))

    def xyz(self):
        return (self.x(), self.y(), self.z())

    def temp(self):
        val = self.read_bytes(L3GD20_OUT_TEMP_ADDR, 1)[0]
        if val & 0x80:
            val -= 256
        return val
