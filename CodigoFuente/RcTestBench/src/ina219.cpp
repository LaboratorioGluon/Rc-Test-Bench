#include "ina219.h"
#include "USB\usbd_cdc_if.h"


ina219::ina219(uint32_t pAddress,GPIO_TypeDef *pSDA_Port, uint32_t pSDA_Pin, GPIO_TypeDef *pSCL_Port, uint32_t pSCL_Pin): 
        SDA_Port(pSDA_Port), SDA_Pin(pSDA_Pin), SCL_Port(pSCL_Port), SCL_Pin(pSCL_Pin), i2cAddress(pAddress)
{
}

void ina219::Init()
{

    GPIO_InitTypeDef lSdaInit;
    lSdaInit.Pin = GPIO_PIN_8|GPIO_PIN_9;
    lSdaInit.Mode = GPIO_MODE_AF_OD;
    lSdaInit.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &lSdaInit);

    __HAL_AFIO_REMAP_I2C1_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    i2cHandle.Instance = I2C1;
    i2cHandle.Init.ClockSpeed = 100000;
    i2cHandle.Init.DutyCycle = I2C_DUTYCYCLE_2;
    i2cHandle.Init.OwnAddress1 = 0;
    i2cHandle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    i2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    i2cHandle.Init.OwnAddress2 = 0;
    i2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    i2cHandle.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&i2cHandle);


/*
    uint8_t buffer[10];
    memset(buffer, 0, 10);
    buffer[0] = 0x00;
    HAL_StatusTypeDef ret;
    ret = HAL_I2C_Master_Transmit(&i2cHandle, 0x40<<1, buffer, 1, HAL_MAX_DELAY);
    ret = HAL_I2C_Master_Receive(&i2cHandle, 0x40<<1, buffer, 2, HAL_MAX_DELAY);
    (void)ret;
*/
}

uint32_t ina219::getCurrent()
{
    uint32_t currentValue = 0;
    memset(i2cBuffer, 0, 10);
    i2cBuffer[0] = 0x01; // Voltage register

    HAL_I2C_Master_Transmit(&i2cHandle, i2cAddress<<1, i2cBuffer, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&i2cHandle, i2cAddress<<1, i2cBuffer, 2, HAL_MAX_DELAY);

    currentValue = (((uint32_t)i2cBuffer[0]) << 8 | i2cBuffer[1]);
    return currentValue;
}