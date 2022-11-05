#ifndef __INA219_H__
#define __INA219_H__

#include <stm32f1xx.h>

class ina219
{

public:

    ina219(uint32_t pAddress, GPIO_TypeDef* pSDA_Port, uint32_t pSDA_Pin, GPIO_TypeDef *pSCL_Port, uint32_t pSCL_Pin);

    void Init();

    uint32_t getCurrent();

private:

    GPIO_TypeDef *SDA_Port;
    uint32_t SDA_Pin;
    GPIO_TypeDef *SCL_Port;
    uint32_t SCL_Pin;

    // i2c things
    uint8_t i2cAddress;
    I2C_HandleTypeDef i2cHandle;
    uint8_t i2cBuffer[5];

    
};

#endif //__INA219_H__