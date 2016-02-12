#include STM32_HAL_H

#define MICROPY_HW_BOARD_NAME       "L476-DISCO"
#define MICROPY_HW_MCU_NAME         "STM32L476"

#define MICROPY_HW_HAS_SWITCH       (1)
#define MICROPY_HW_HAS_FLASH        (1)
#define MICROPY_HW_HAS_SDCARD       (0)
#define MICROPY_HW_HAS_MMA7660      (0)
#define MICROPY_HW_HAS_LIS3DSH      (0)
#define MICROPY_HW_HAS_LCD          (0)
#define MICROPY_HW_ENABLE_RNG       (1)
#define MICROPY_HW_ENABLE_RTC       (1)
#define MICROPY_HW_ENABLE_TIMER     (1)
#define MICROPY_HW_ENABLE_SERVO     (0)
#define MICROPY_HW_ENABLE_DAC       (0)
#define MICROPY_HW_ENABLE_CAN       (0)

// HSE is 8MHz
#define MICROPY_HW_CLK_PLLM (8)
#define MICROPY_HW_CLK_PLLN (336)
#define MICROPY_HW_CLK_PLLP (RCC_PLLP_DIV2)
#define MICROPY_HW_CLK_PLLQ (7)

// UART config
#define MICROPY_HW_UART1_PORT (GPIOA)
#define MICROPY_HW_UART1_PINS (GPIO_PIN_9 | GPIO_PIN_10)
#define MICROPY_HW_UART2_PORT (GPIOD)
#define MICROPY_HW_UART2_PINS (GPIO_PIN_8 | GPIO_PIN_9)

// I2C busses
#define MICROPY_HW_I2C1_SCL (pin_B6)
#define MICROPY_HW_I2C1_SDA (pin_B7)

// SPI busses
#define MICROPY_HW_SPI2_NSS     (pin_D7)
#define MICROPY_HW_SPI2_SCK     (pin_D1)
#define MICROPY_HW_SPI2_MISO    (pin_D3)
#define MICROPY_HW_SPI2_MOSI    (pin_D4)

// USRSW is pulled low. Pressing the button makes the input go high.
#define MICROPY_HW_USRSW_PIN        (pin_A0)
#define MICROPY_HW_USRSW_PULL       (GPIO_NOPULL)
#define MICROPY_HW_USRSW_EXTI_MODE  (GPIO_MODE_IT_RISING)
#define MICROPY_HW_USRSW_PRESSED    (1)

// LEDs
#define MICROPY_HW_LED1             (pin_B2) // red
#define MICROPY_HW_LED2             (pin_E8) // green
#define MICROPY_HW_LED_OTYPE        (GPIO_MODE_OUTPUT_PP)
#define MICROPY_HW_LED_ON(pin)      (pin->gpio->BSRR = pin->pin_mask)
#define MICROPY_HW_LED_OFF(pin)     (pin->gpio->BSRR = pin->pin_mask<<16)

// USB config
#define MICROPY_HW_USB_VBUS_DETECT_PIN (pin_B13)
#define MICROPY_HW_USB_OTG_ID_PIN      (pin_B12)
