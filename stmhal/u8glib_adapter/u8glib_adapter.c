/*
 * u8glib_adapter.c
 *
 *  Created on: 10.12.2015
 *      Author: badi
 */
#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "py/obj.h"
#include "spi.h"
#include "systick.h"
#include "u8g.h"
#include "ili9341.h"
#include "stm32f429i_discovery_lcd.h"
#include "u8glib_adapter.h"
/*
 * Definition of constants
 *
 * Copied from stm32f429i_discovery.h in the Board support files from STM
 */
/*################################ LCD #######################################*/
/* Chip Select macro definition */
#define LCD_CS_LOW()       HAL_GPIO_WritePin(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, GPIO_PIN_RESET)
#define LCD_CS_HIGH()      HAL_GPIO_WritePin(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, GPIO_PIN_SET)

/* Set WRX High to send data */
#define LCD_WRX_LOW()      HAL_GPIO_WritePin(LCD_WRX_GPIO_PORT, LCD_WRX_PIN, GPIO_PIN_RESET)
#define LCD_WRX_HIGH()     HAL_GPIO_WritePin(LCD_WRX_GPIO_PORT, LCD_WRX_PIN, GPIO_PIN_SET)

/* Set WRX High to send data */
#define LCD_RDX_LOW()      HAL_GPIO_WritePin(LCD_RDX_GPIO_PORT, LCD_RDX_PIN, GPIO_PIN_RESET)
#define LCD_RDX_HIGH()     HAL_GPIO_WritePin(LCD_RDX_GPIO_PORT, LCD_RDX_PIN, GPIO_PIN_SET)

/**
  * @brief  LCD Control pin
  */
#define LCD_NCS_PIN                             GPIO_PIN_2
#define LCD_NCS_GPIO_PORT                       GPIOC
#define LCD_NCS_GPIO_CLK_ENABLE()               __GPIOC_CLK_ENABLE()
#define LCD_NCS_GPIO_CLK_DISABLE()              __GPIOC_CLK_DISABLE()
/**
  * @}
  */
/**
  * @brief  LCD Command/data pin
  */
#define LCD_WRX_PIN                             GPIO_PIN_13
#define LCD_WRX_GPIO_PORT                       GPIOD
#define LCD_WRX_GPIO_CLK_ENABLE()               __GPIOD_CLK_ENABLE()
#define LCD_WRX_GPIO_CLK_DISABLE()              __GPIOD_CLK_DISABLE()

#define LCD_RDX_PIN                             GPIO_PIN_12
#define LCD_RDX_GPIO_PORT                       GPIOD
#define LCD_RDX_GPIO_CLK_ENABLE()               __GPIOD_CLK_ENABLE()
#define LCD_RDX_GPIO_CLK_DISABLE()              __GPIOD_CLK_DISABLE()
/* Maximum Timeout values for flags waiting loops. These timeouts are not based
   on accurate values, they just guarantee that the application will not remain
   stuck if the SPI communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */
#define SPIx_TIMEOUT_MAX              ((uint32_t)0x1000)
/*
 * LCD configuration
 */

#define WIDTH 240

#if defined(U8G_16BIT)
#define HEIGHT 320
#else
/* if the user tries to compile the 8Bit version of the lib, then restrict the height to something which fits to 8Bit */
#define HEIGHT 240
#endif
#define PAGE_HEIGHT 4

/************************************
 * Static function definitions
 ***********************************/
static void     SPIx_Init(void);
static uint32_t SPIx_Read(uint8_t ReadSize);
static void SPIx_Write(uint16_t Value);
static uint8_t SPIx_WriteRead(uint8_t Byte);
static void SPIx_Error(void);
static void LCD_IO_WriteData(uint16_t RegValue);
static void LCD_IO_WriteReg(uint8_t Reg);
static uint32_t LCD_IO_ReadData(uint16_t RegValue, uint8_t ReadSize);
static uint8_t u8g_com_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr);
static uint8_t u8g_dev_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg);
/***********************************
 * Static variables for module
 ***********************************/
static uint8_t Is_LCD_IO_Initialized = 0;
u8g_t u8g;
static uint32_t SpixTimeout = SPIx_TIMEOUT_MAX; /*<! Value of Timeout when SPI communication fails */
/*
 * Deveice
 */
static uint8_t u8g_ili9341_320x240_8h8_buf[WIDTH*PAGE_HEIGHT];
static u8g_pb_t u8g_ili9341_320x240_pb = { {PAGE_HEIGHT, HEIGHT, 0, 0, 0},  WIDTH, u8g_ili9341_320x240_8h8_buf};
static u8g_dev_t u8g_dev = { u8g_dev_fn, &u8g_ili9341_320x240_pb, u8g_com_fn };


/*
  The following delay procedures must be implemented for u8glib

  void u8g_Delay(uint16_t val)  Delay by "val" milliseconds
  void u8g_MicroDelay(void)     Delay be one microsecond
  void u8g_10MicroDelay(void)   Delay by 10 microseconds

*/
void u8g_Delay(uint16_t val)
{
    HAL_Delay(val);
}

void u8g_MicroDelay(void)
{
    sys_tick_udelay(1);
}

void u8g_10MicroDelay(void)
{
    sys_tick_udelay(10);
}
/********************************* LINK LCD ***********************************/

/**
  * @brief  Configures the LCD_SPI interface.
  * @param  None
  * @retval None
  *
  * SPI must be setup separate
  */
void u8glib_adapter_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    if(Is_LCD_IO_Initialized == 0)
    {
        Is_LCD_IO_Initialized = 1;

        /* Configure NCS in Output Push-Pull mode */
        LCD_WRX_GPIO_CLK_ENABLE();
        GPIO_InitStructure.Pin     = LCD_WRX_PIN;
        GPIO_InitStructure.Mode    = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStructure.Pull    = GPIO_NOPULL;
        GPIO_InitStructure.Speed   = GPIO_SPEED_FAST;
        HAL_GPIO_Init(LCD_WRX_GPIO_PORT, &GPIO_InitStructure);

        LCD_RDX_GPIO_CLK_ENABLE();
        GPIO_InitStructure.Pin     = LCD_RDX_PIN;
        GPIO_InitStructure.Mode    = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStructure.Pull    = GPIO_NOPULL;
        GPIO_InitStructure.Speed   = GPIO_SPEED_FAST;
        HAL_GPIO_Init(LCD_RDX_GPIO_PORT, &GPIO_InitStructure);

        /* Configure the LCD Control pins ----------------------------------------*/
        LCD_NCS_GPIO_CLK_ENABLE();

        /* Configure NCS in Output Push-Pull mode */
        GPIO_InitStructure.Pin     = LCD_NCS_PIN;
        GPIO_InitStructure.Mode    = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStructure.Pull    = GPIO_NOPULL;
        GPIO_InitStructure.Speed   = GPIO_SPEED_FAST;
        HAL_GPIO_Init(LCD_NCS_GPIO_PORT, &GPIO_InitStructure);

        /* Set or Reset the control line */
        LCD_CS_LOW();
        LCD_CS_HIGH();

        SPIx_Init();
    }
    BSP_LCD_Init();
    u8g_InitComFn(&u8g, &u8g_dev, &u8g_com_fn);
}
/******************************* SPI Routines *********************************/

/**
  * @brief  SPIx Bus initialization
  * @param  None
  * @retval None
  */
static void SPIx_Init(void)
{
  if(HAL_SPI_GetState(&SPIHandle5) == HAL_SPI_STATE_RESET)
  {

    /* SPI configuration -----------------------------------------------------*/
    SPIHandle5.Instance = SPI5;
    /* SPI baudrate is set to 5.6 MHz (PCLK2/SPI_BaudRatePrescaler = 90/16 = 5.625 MHz)
       to verify these constraints:
       - ILI9341 LCD SPI interface max baudrate is 10MHz for write and 6.66MHz for read
       - l3gd20 SPI interface max baudrate is 10MHz for write/read
       - PCLK2 frequency is set to 90 MHz
    */
    SPIHandle5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;

    /* On STM32F429I-Discovery, LCD ID cannot be read then keep a common configuration */
    /* for LCD and GYRO (SPI_DIRECTION_2LINES) */
    /* Note: To read a register a LCD, SPI_DIRECTION_1LINE should be set */
    SPIHandle5.Init.Direction      = SPI_DIRECTION_2LINES;
    SPIHandle5.Init.CLKPhase       = SPI_PHASE_1EDGE;
    SPIHandle5.Init.CLKPolarity    = SPI_POLARITY_LOW;
    SPIHandle5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
    SPIHandle5.Init.CRCPolynomial  = 7;
    SPIHandle5.Init.DataSize       = SPI_DATASIZE_8BIT;
    SPIHandle5.Init.FirstBit       = SPI_FIRSTBIT_MSB;
    SPIHandle5.Init.NSS            = SPI_NSS_SOFT;
    SPIHandle5.Init.TIMode         = SPI_TIMODE_DISABLED;
    SPIHandle5.Init.Mode           = SPI_MODE_MASTER;

    spi_init(&SPIHandle5, false);
    HAL_SPI_Init(&SPIHandle5);
  }
}

/**
  * @brief  Reads 4 bytes from device.
  * @param  ReadSize: Number of bytes to read (max 4 bytes)
  * @retval Value read on the SPI
  */
static uint32_t SPIx_Read(uint8_t ReadSize)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t readvalue;

  status = HAL_SPI_Receive(&SPIHandle5, (uint8_t*) &readvalue, ReadSize, SpixTimeout);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Re-Initiaize the BUS */
    SPIx_Error();
  }

  return readvalue;
}

/**
  * @brief  Writes a byte to device.
  * @param  Value: value to be written
  * @retval None
  */
static void SPIx_Write(uint16_t Value)
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_SPI_Transmit(&SPIHandle5, (uint8_t*) &Value, 1, SpixTimeout);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Re-Initiaize the BUS */
    SPIx_Error();
  }
}

/**
  * @brief  Sends a Byte through the SPI interface and return the Byte received
  *         from the SPI bus.
  * @param  Byte: Byte send.
  * @retval The received byte value
  */
static uint8_t SPIx_WriteRead(uint8_t Byte)
{
  uint8_t receivedbyte = 0;

  /* Send a Byte through the SPI peripheral */
  /* Read byte from the SPI bus */
  if(HAL_SPI_TransmitReceive(&SPIHandle5, (uint8_t*) &Byte, (uint8_t*) &receivedbyte, 1, SpixTimeout) != HAL_OK)
  {
    SPIx_Error();
  }

  return receivedbyte;
}

/**
  * @brief  SPIx error treatment function.
  * @param  None
  * @retval None
  */
static void SPIx_Error(void)
{
  /* De-initialize the SPI comunication BUS */
  HAL_SPI_DeInit(&SPIHandle5);

  /* Re- Initiaize the SPI comunication BUS */
  SPIx_Init();
}

/**
  * @brief  Writes register value.
  * @param  None
  * @retval None
  */
static void LCD_IO_WriteData(uint16_t RegValue)
{
    /* Set WRX to send data */
    LCD_WRX_HIGH();

    /* Reset LCD control line(/CS) and Send data */
    LCD_CS_LOW();
    SPIx_Write(RegValue);

    /* Deselect: Chip Select high */
    LCD_CS_HIGH();
}

/**
  * @brief  Writes register address.
  * @param  None
  * @retval None
  */
static void LCD_IO_WriteReg(uint8_t Reg)
{
    /* Reset WRX to send command */
    LCD_WRX_LOW();

    /* Reset LCD control line(/CS) and Send command */
    LCD_CS_LOW();
    SPIx_Write(Reg);

    /* Deselect: Chip Select high */
    LCD_CS_HIGH();
}

/**
  * @brief  Reads register value.
  * @param  RegValue Address of the register to read
  * @param  ReadSize Number of bytes to read
  * @retval Content of the register value
  */
static uint32_t LCD_IO_ReadData(uint16_t RegValue, uint8_t ReadSize)
{
    uint32_t readvalue = 0;

    /* Select: Chip Select low */
    LCD_CS_LOW();

    /* Reset WRX to send command */
    LCD_WRX_LOW();

    SPIx_Write(RegValue);

    readvalue = SPIx_Read(ReadSize);

    /* Set WRX to send data */
    LCD_WRX_HIGH();

    /* Deselect: Chip Select high */
    LCD_CS_HIGH();

    return readvalue;
}

/*
 * u8glib communication procedure
 * realized for the STM32F429i-Discovery board
 * ILI9341
 */
static uint8_t u8g_com_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr)
{
    switch(msg)
    {
    case U8G_COM_MSG_STOP:
    break;

    case U8G_COM_MSG_INIT:
        if ( arg_val <= U8G_SPI_CLK_CYCLE_50NS )
        {

        }
        else if ( arg_val <= U8G_SPI_CLK_CYCLE_300NS )
        {

        }
        else if ( arg_val <= U8G_SPI_CLK_CYCLE_400NS )
        {

        }
        else
        {

        }

        break;

    case U8G_COM_MSG_ADDRESS:                     /* define cmd (arg_val = 0) or data mode (arg_val = 1) */

    break;

    case U8G_COM_MSG_CHIP_SELECT:
        if ( arg_val == 0 )
        {
            /* disable */

        }
        else
        {
            /* enable */

        }

        break;

    case U8G_COM_MSG_RESET:

        break;

    case U8G_COM_MSG_WRITE_BYTE:

        break;

    case U8G_COM_MSG_WRITE_SEQ:
    case U8G_COM_MSG_WRITE_SEQ_P:
        {

        }
        break;

    }
    return 1;
}


static uint8_t u8g_dev_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg)
{
    switch(msg)
    {
        case U8G_DEV_MSG_INIT:
        break;

        case U8G_DEV_MSG_STOP:
        break;

        case U8G_DEV_MSG_PAGE_NEXT:
        break;

        case U8G_DEV_MSG_CONTRAST:
        break;

        case U8G_DEV_MSG_SLEEP_ON:
        return 1;

        case U8G_DEV_MSG_SLEEP_OFF:
        return 1;
    }
    return u8g_dev_pb8v1_base_fn(u8g, dev, msg, arg);
}

