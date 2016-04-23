/**
    ******************************************************************************
    * @file        stm32f429i_discovery.c
    * @author    MCD Application Team
    * @version V2.1.2
    * @date        02-March-2015
    * @brief     This file provides set of firmware functions to manage Leds and
    *                    push-button available on STM32F429I-Discovery Kit from STMicroelectronics.
    ******************************************************************************
    * @attention
    *
    * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
    *
    * Redistribution and use in source and binary forms, with or without modification,
    * are permitted provided that the following conditions are met:
    *     1. Redistributions of source code must retain the above copyright notice,
    *            this list of conditions and the following disclaimer.
    *     2. Redistributions in binary form must reproduce the above copyright notice,
    *            this list of conditions and the following disclaimer in the documentation
    *            and/or other materials provided with the distribution.
    *     3. Neither the name of STMicroelectronics nor the names of its contributors
    *            may be used to endorse or promote products derived from this software
    *            without specific prior written permission.
    *
    * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
    * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    *
    ******************************************************************************
    */    
    
/* Includes ------------------------------------------------------------------*/
#include <lcdctrl/inc/lcd_io.h>

/** @addtogroup BSP
    * @{
    */ 

/** @addtogroup STM32F429I_DISCOVERY
    * @{
    */
            
/** @defgroup STM32F429I_DISCOVERY_LOW_LEVEL 
    * @brief This file provides set of firmware functions to manage Leds and push-button
    *                available on STM32F429I-Discovery Kit from STMicroelectronics.
    * @{
    */ 

/** @defgroup STM32F429I_DISCOVERY_LOW_LEVEL_Private_TypesDefinitions
    * @{
    */ 
/**
    * @}
    */ 

/** @defgroup STM32F429I_DISCOVERY_LOW_LEVEL_Private_Defines
    * @{
    */ 
    
    /**
    * @brief STM32F429I DISCO BSP Driver version number V2.1.0
    */
#define __STM32F429I_DISCO_BSP_VERSION_MAIN     (0x02) /*!< [31:24] main version */
#define __STM32F429I_DISCO_BSP_VERSION_SUB1     (0x01) /*!< [23:16] sub1 version */
#define __STM32F429I_DISCO_BSP_VERSION_SUB2     (0x02) /*!< [15:8]    sub2 version */
#define __STM32F429I_DISCO_BSP_VERSION_RC         (0x00) /*!< [7:0]    release candidate */ 
#define __STM32F429I_DISCO_BSP_VERSION                ((__STM32F429I_DISCO_BSP_VERSION_MAIN << 24)\
                                                                                         |(__STM32F429I_DISCO_BSP_VERSION_SUB1 << 16)\
                                                                                         |(__STM32F429I_DISCO_BSP_VERSION_SUB2 << 8 )\
                                                                                         |(__STM32F429I_DISCO_BSP_VERSION_RC)) 
/**
    * @}
    */ 

/** @defgroup STM32F429I_DISCOVERY_LOW_LEVEL_Private_Macros
    * @{
    */ 
/**
    * @}
    */ 

/** @defgroup STM32F429I_DISCOVERY_LOW_LEVEL_Private_Variables
    * @{
    */ 
GPIO_TypeDef* GPIO_PORT[LEDn] = {LED3_GPIO_PORT, 
                                                                 LED4_GPIO_PORT};

const uint16_t GPIO_PIN[LEDn] = {LED3_PIN, 
                                                                 LED4_PIN};

GPIO_TypeDef* BUTTON_PORT[BUTTONn] = {KEY_BUTTON_GPIO_PORT}; 
const uint16_t BUTTON_PIN[BUTTONn] = {KEY_BUTTON_PIN}; 
const uint8_t BUTTON_IRQn[BUTTONn] = {KEY_BUTTON_EXTI_IRQn};

uint32_t I2cxTimeout = I2Cx_TIMEOUT_MAX; /*<! Value of Timeout when I2C communication fails */    
uint32_t SpixTimeout = SPIx_TIMEOUT_MAX; /*<! Value of Timeout when SPI communication fails */    

I2C_HandleTypeDef I2cHandle;
static SPI_HandleTypeDef SpiHandle;
static uint8_t Is_LCD_IO_Initialized = 0;

/**
    * @}
    */ 

/** @defgroup STM32F429I_DISCOVERY_LOW_LEVEL_Private_FunctionPrototypes
    * @{
    */ 
/* I2Cx bus function */
static void                             I2Cx_Init(void);
static void                             I2Cx_ITConfig(void);
static void                             I2Cx_WriteData(uint8_t Addr, uint8_t Reg, uint8_t Value);
static void                             I2Cx_WriteBuffer(uint8_t Addr, uint8_t Reg,    uint8_t *pBuffer, uint16_t Length);
static uint8_t                        I2Cx_ReadData(uint8_t Addr, uint8_t Reg);
static uint8_t                        I2Cx_ReadBuffer(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length);
static void                             I2Cx_Error(void);
static void                             I2Cx_MspInit(I2C_HandleTypeDef *hi2c);    
#ifdef EE_M24LR64
static HAL_StatusTypeDef    I2Cx_WriteBufferDMA(uint8_t Addr, uint16_t Reg,    uint8_t *pBuffer, uint16_t Length);
static HAL_StatusTypeDef    I2Cx_ReadBufferDMA(uint8_t Addr, uint16_t Reg, uint8_t *pBuffer, uint16_t Length);
static HAL_StatusTypeDef    I2Cx_IsDeviceReady(uint16_t DevAddress, uint32_t Trials);
#endif /* EE_M24LR64 */

/* SPIx bus function */
static void                             SPIx_Init(void);
static void                             SPIx_Write(uint16_t Value);
static uint32_t                     SPIx_Read(uint8_t ReadSize);
static uint8_t                        SPIx_WriteRead(uint8_t Byte);
static void                             SPIx_Error(void);
static void                             SPIx_MspInit(SPI_HandleTypeDef *hspi);

/* Link function for LCD peripheral */
void                                            LCD_IO_Init(void);
void                                            LCD_IO_WriteData(uint16_t RegValue);
void                                            LCD_IO_WriteReg(uint8_t Reg);
uint32_t                                    LCD_IO_ReadData(uint16_t RegValue, uint8_t ReadSize);
void                                            LCD_Delay(uint32_t delay);

/* IOExpander IO functions */
void                                            IOE_Init(void);
void                                            IOE_ITConfig(void);
void                                            IOE_Delay(uint32_t Delay);
void                                            IOE_Write(uint8_t Addr, uint8_t Reg, uint8_t Value);
uint8_t                                     IOE_Read(uint8_t Addr, uint8_t Reg);
uint16_t                                    IOE_ReadMultiple(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length);
void                                            IOE_WriteMultiple(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length);

/* Link function for GYRO peripheral */
void                                            GYRO_IO_Init(void);
void                                            GYRO_IO_Write(uint8_t* pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite);
void                                            GYRO_IO_Read(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead);

#ifdef EE_M24LR64
/* Link function for I2C EEPROM peripheral */
void                                            EEPROM_IO_Init(void);
HAL_StatusTypeDef                 EEPROM_IO_WriteData(uint16_t DevAddress, uint16_t MemAddress, uint8_t* pBuffer, uint32_t BufferSize);
HAL_StatusTypeDef                 EEPROM_IO_ReadData(uint16_t DevAddress, uint16_t MemAddress, uint8_t* pBuffer, uint32_t BufferSize);
HAL_StatusTypeDef                 EEPROM_IO_IsDeviceReady(uint16_t DevAddress, uint32_t Trials);
#endif /* EE_M24LR64 */

/**
    * @}
    */ 


/********************************* LINK LCD ***********************************/

/**
    * @brief    Configures the LCD_SPI interface.
    * @param    None
    * @retval None
    */
void LCD_IO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    if(Is_LCD_IO_Initialized == 0)
    {
        Is_LCD_IO_Initialized = 1; 
        
        /* Configure NCS in Output Push-Pull mode */
        LCD_WRX_GPIO_CLK_ENABLE();
        GPIO_InitStructure.Pin         = LCD_WRX_PIN;
        GPIO_InitStructure.Mode        = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStructure.Pull        = GPIO_NOPULL;
        GPIO_InitStructure.Speed     = GPIO_SPEED_FAST;
        HAL_GPIO_Init(LCD_WRX_GPIO_PORT, &GPIO_InitStructure);
        
        LCD_RDX_GPIO_CLK_ENABLE();
        GPIO_InitStructure.Pin         = LCD_RDX_PIN;
        GPIO_InitStructure.Mode        = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStructure.Pull        = GPIO_NOPULL;
        GPIO_InitStructure.Speed     = GPIO_SPEED_FAST;
        HAL_GPIO_Init(LCD_RDX_GPIO_PORT, &GPIO_InitStructure);
        
        /* Configure the LCD Control pins ----------------------------------------*/
        LCD_NCS_GPIO_CLK_ENABLE();
        
        /* Configure NCS in Output Push-Pull mode */
        GPIO_InitStructure.Pin         = LCD_NCS_PIN;
        GPIO_InitStructure.Mode        = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStructure.Pull        = GPIO_NOPULL;
        GPIO_InitStructure.Speed     = GPIO_SPEED_FAST;
        HAL_GPIO_Init(LCD_NCS_GPIO_PORT, &GPIO_InitStructure);
        
        /* Set or Reset the control line */
        LCD_CS_LOW();
        LCD_CS_HIGH();
        
        SPIx_Init();
    }
}

/**
    * @brief    Writes register value.
    * @param    None
    * @retval None
    */
void LCD_IO_WriteData(uint16_t RegValue) 
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
    * @brief    Writes register address.
    * @param    None
    * @retval None
    */
void LCD_IO_WriteReg(uint8_t Reg) 
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
    * @brief    Reads register value.
    * @param    RegValue Address of the register to read
    * @param    ReadSize Number of bytes to read
    * @retval Content of the register value
    */
uint32_t LCD_IO_ReadData(uint16_t RegValue, uint8_t ReadSize) 
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

/**
    * @brief    Wait for loop in ms.
    * @param    Delay in ms.
    * @retval None
    */
void LCD_Delay(uint32_t Delay)
{
    HAL_Delay(Delay);
}

/*******************************************************************************
                                                        LINK OPERATIONS
*******************************************************************************/

/********************************* LINK IOE ***********************************/

/**
    * @brief    IOE Low Level Initialization.
    * @param    None
    * @retval None
    */
void IOE_Init(void) 
{
    I2Cx_Init();
}

/**
    * @brief    IOE Low Level Interrupt configuration.
    * @param    None
    * @retval None
    */
void IOE_ITConfig(void)
{
    I2Cx_ITConfig();
}

/**
    * @brief    IOE Writes single data operation.
    * @param    Addr: I2C Address
    * @param    Reg: Reg Address 
    * @param    Value: Data to be written
    * @retval None
    */
void IOE_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
    I2Cx_WriteData(Addr, Reg, Value);
}

/**
    * @brief    IOE Reads single data.
    * @param    Addr: I2C Address
    * @param    Reg: Reg Address 
    * @retval The read data
    */
uint8_t IOE_Read(uint8_t Addr, uint8_t Reg)
{
    return I2Cx_ReadData(Addr, Reg);
}

/**
    * @brief    IOE Writes multiple data.
    * @param    Addr: I2C Address
    * @param    Reg: Reg Address 
    * @param    pBuffer: pointer to data buffer
    * @param    Length: length of the data
    * @retval None
    */
void IOE_WriteMultiple(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length)
{
    I2Cx_WriteBuffer(Addr, Reg, pBuffer, Length);
}

/**
    * @brief    IOE Reads multiple data.
    * @param    Addr: I2C Address
    * @param    Reg: Reg Address 
    * @param    pBuffer: pointer to data buffer
    * @param    Length: length of the data
    * @retval 0 if no problems to read multiple data
    */
uint16_t IOE_ReadMultiple(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length)
{
 return I2Cx_ReadBuffer(Addr, Reg, pBuffer, Length);
}

/**
    * @brief    IOE Delay.
    * @param    Delay in ms
    * @retval None
    */
void IOE_Delay(uint32_t Delay)
{
    HAL_Delay(Delay);
}

/********************************* LINK GYROSCOPE *****************************/

/**
    * @brief    Configures the Gyroscope SPI interface.
    * @param    None
    * @retval None
    */
void GYRO_IO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* Configure the Gyroscope Control pins ------------------------------------*/
    /* Enable CS GPIO clock and Configure GPIO PIN for Gyroscope Chip select */    
    GYRO_CS_GPIO_CLK_ENABLE();    
    GPIO_InitStructure.Pin = GYRO_CS_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull    = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
    HAL_GPIO_Init(GYRO_CS_GPIO_PORT, &GPIO_InitStructure);
    
    /* Deselect: Chip Select high */
    GYRO_CS_HIGH();
    
    /* Enable INT1, INT2 GPIO clock and Configure GPIO PINs to detect Interrupts */
    GYRO_INT_GPIO_CLK_ENABLE();
    GPIO_InitStructure.Pin = GYRO_INT1_PIN | GYRO_INT2_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
    GPIO_InitStructure.Pull= GPIO_NOPULL;
    HAL_GPIO_Init(GYRO_INT_GPIO_PORT, &GPIO_InitStructure);

    SPIx_Init();
}

/**
    * @brief    Writes one byte to the Gyroscope.
    * @param    pBuffer: Pointer to the buffer containing the data to be written to the Gyroscope.
    * @param    WriteAdd: Gyroscope's internal address to write to.
    * @param    NumByteToWrite: Number of bytes to write.
    * @retval None
    */
void GYRO_IO_Write(uint8_t* pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite)
{
    /* Configure the MS bit: 
             - When 0, the address will remain unchanged in multiple read/write commands.
             - When 1, the address will be auto incremented in multiple read/write commands.
    */
    if(NumByteToWrite > 0x01)
    {
        WriteAddr |= (uint8_t)MULTIPLEBYTE_CMD;
    }
    /* Set chip select Low at the start of the transmission */
    GYRO_CS_LOW();
    
    /* Send the Address of the indexed register */
    SPIx_WriteRead(WriteAddr);
    
    /* Send the data that will be written into the device (MSB First) */
    while(NumByteToWrite >= 0x01)
    {
        SPIx_WriteRead(*pBuffer);
        NumByteToWrite--;
        pBuffer++;
    }
    
    /* Set chip select High at the end of the transmission */ 
    GYRO_CS_HIGH();
}

/**
    * @brief    Reads a block of data from the Gyroscope.
    * @param    pBuffer: Pointer to the buffer that receives the data read from the Gyroscope.
    * @param    ReadAddr: Gyroscope's internal address to read from.
    * @param    NumByteToRead: Number of bytes to read from the Gyroscope.
    * @retval None
    */
void GYRO_IO_Read(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead)
{    
    if(NumByteToRead > 0x01)
    {
        ReadAddr |= (uint8_t)(READWRITE_CMD | MULTIPLEBYTE_CMD);
    }
    else
    {
        ReadAddr |= (uint8_t)READWRITE_CMD;
    }
    /* Set chip select Low at the start of the transmission */
    GYRO_CS_LOW();
    
    /* Send the Address of the indexed register */
    SPIx_WriteRead(ReadAddr);
    
    /* Receive the data that will be read from the device (MSB First) */
    while(NumByteToRead > 0x00)
    {
        /* Send dummy byte (0x00) to generate the SPI clock to Gyroscope (Slave device) */
        *pBuffer = SPIx_WriteRead(DUMMY_BYTE);
        NumByteToRead--;
        pBuffer++;
    }
    
    /* Set chip select High at the end of the transmission */ 
    GYRO_CS_HIGH();
}    


#ifdef EE_M24LR64

/******************************** LINK I2C EEPROM *****************************/

/**
    * @brief    Initializes peripherals used by the I2C EEPROM driver.
    * @param    None
    * @retval None
    */
void EEPROM_IO_Init(void)
{
    I2Cx_Init();
}

/**
    * @brief    Writes data to I2C EEPROM driver in using DMA channel.
    * @param    DevAddress: Target device address
    * @param    MemAddress: Internal memory address
    * @param    pBuffer: Pointer to data buffer
    * @param    BufferSize: Amount of data to be sent
    * @retval HAL status
    */
HAL_StatusTypeDef EEPROM_IO_WriteData(uint16_t DevAddress, uint16_t MemAddress, uint8_t* pBuffer, uint32_t BufferSize)
{
    return (I2Cx_WriteBufferDMA(DevAddress, MemAddress,    pBuffer, BufferSize));
}

/**
    * @brief    Reads data from I2C EEPROM driver in using DMA channel.
    * @param    DevAddress: Target device address
    * @param    MemAddress: Internal memory address
    * @param    pBuffer: Pointer to data buffer
    * @param    BufferSize: Amount of data to be read
    * @retval HAL status
    */
HAL_StatusTypeDef EEPROM_IO_ReadData(uint16_t DevAddress, uint16_t MemAddress, uint8_t* pBuffer, uint32_t BufferSize)
{
    return (I2Cx_ReadBufferDMA(DevAddress, MemAddress, pBuffer, BufferSize));
}

/**
* @brief    Checks if target device is ready for communication. 
* @note     This function is used with Memory devices
* @param    DevAddress: Target device address
* @param    Trials: Number of trials
* @retval HAL status
*/
HAL_StatusTypeDef EEPROM_IO_IsDeviceReady(uint16_t DevAddress, uint32_t Trials)
{ 
    return (I2Cx_IsDeviceReady(DevAddress, Trials));
}
#endif /* EE_M24LR64 */

/**
    * @}
    */ 

/**
    * @}
    */ 

/**
    * @}
    */     

/**
    * @}
    */ 
            
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
