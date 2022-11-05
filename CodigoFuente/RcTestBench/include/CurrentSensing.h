#ifndef __CURRENT_SENSING_H__
#define __CURRENT_SENSING_H__

#include <stdint.h>
#include <stm32f1xx.h>

#include "ina219.h"

struct HighCurrentConfig{
    GPIO_TypeDef *mAdcPort;
    uint32_t     mAdcPin;
    GPIO_TypeDef *mActivationPort;
    uint32_t     mActivationPin;
};

extern const HighCurrentConfig cHighCurrentConfig;

struct LowCurrentConfig{
    GPIO_TypeDef *mI2CPortSDA;
    uint32_t     mI2CPinSDA;
    GPIO_TypeDef *mI2CPortSCL;
    uint32_t     mI2CPinSCL;
    GPIO_TypeDef *mActivationPort;
    uint32_t     mActivationPin;
};

extern const LowCurrentConfig cLowCurrentConfig;


/**
 * @brief Lee los valores de corriente que consume el motor.
 * 
 * 
 */
class CurrentSensing
{
public:

    CurrentSensing();

    void Init();

    /**
     * @brief Set the hysteresis
     * 
     * @param pathCurrentLimit in miliAmps
     * @param marginAbs margin for the hystersis in miliAmps
     */
    void setConfig(uint32_t pathCurrentLimit, uint32_t marginAbs);

    void startMeasuring();
    void stopMeasuring();

    uint32_t readCurrent();


    //@TODO: Borrame, mejor pone en logica
    void setCurrentPath(uint8_t cp){ mCurrentPath = (enum _CURRENT_PATH)cp; }


private:

    // Handles of STM32 hal
    ADC_HandleTypeDef hAdcHighPath;
    I2C_HandleTypeDef hI2cLowPath;

    ina219 lowCurrentSensor;

    enum _CURRENT_PATH{
        LOW_CURRENT_PATH,
        HIGH_CURRENT_PATH
    } mCurrentPath;
     
    uint32_t pathCurrentLimit_whenLow;
    uint32_t pathCurrentLimit_whenHigh;

    void currentPathLogic(uint32_t lastRead);

    uint32_t readLowCurrentPath();
    uint32_t readHighCurrentPath(uint8_t muestras);
    uint32_t readHighCurrentADCMv(uint8_t muestras);

    // 2500mV = 0A in ACS, here we store the "zero"
    uint32_t highCurrentZeroMv;

    void initLowCurrentPath();
    void initHighCurrentPath();

    void internalUpdateCurrentPath();
    void setLowCurrentPath(uint8_t isActivated);
    void setHighCurrentPath(uint8_t isActivated);

};

#endif