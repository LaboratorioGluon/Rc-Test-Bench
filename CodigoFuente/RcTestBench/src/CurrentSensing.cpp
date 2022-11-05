#include "CurrentSensing.h"

#include <stm32f1xx.h>

#define INA219_ADDRESS 0x40


#define ADC_TO_MV (0.8056640625f) //3300 mv / (1<<12) bits
#define ACS_MID_RANGE 2500 // 5000mV /2

const HighCurrentConfig cHighCurrentConfig =
{
    .mAdcPort = GPIOA,
    .mAdcPin =  GPIO_PIN_0,
    .mActivationPort = GPIOB,
    .mActivationPin = GPIO_PIN_1
};

const LowCurrentConfig cLowCurrentConfig = 
{
    .mI2CPortSDA = nullptr,
    .mI2CPinSDA = 0,
    .mI2CPortSCL = nullptr,
    .mI2CPinSCL = 0,
    .mActivationPort = GPIOB,
    .mActivationPin = GPIO_PIN_0
};


CurrentSensing::CurrentSensing():
    lowCurrentSensor(INA219_ADDRESS, GPIOB, GPIO_PIN_9, GPIOB, GPIO_PIN_8),
    mCurrentPath(HIGH_CURRENT_PATH)
{
    setConfig(2000, 100);
}

void CurrentSensing::Init(){
    initHighCurrentPath();
    initLowCurrentPath();
    stopMeasuring();
}

void CurrentSensing::startMeasuring(){
    internalUpdateCurrentPath();
}

void CurrentSensing::stopMeasuring(){
    setHighCurrentPath(0);
    setLowCurrentPath(0);
}

void CurrentSensing::initHighCurrentPath(){

        __HAL_RCC_ADC1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        

        GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.Pin = cHighCurrentConfig.mAdcPin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

        HAL_GPIO_Init(cHighCurrentConfig.mAdcPort, &GPIO_InitStruct);

        ADC_ChannelConfTypeDef sConfig = {0};

        /** Common config */
        hAdcHighPath.Instance = ADC1;
        hAdcHighPath.Init.ScanConvMode = ADC_SCAN_DISABLE;
        hAdcHighPath.Init.ContinuousConvMode = ENABLE;
        hAdcHighPath.Init.DiscontinuousConvMode = DISABLE;
        hAdcHighPath.Init.ExternalTrigConv = ADC_SOFTWARE_START;
        hAdcHighPath.Init.DataAlign = ADC_DATAALIGN_RIGHT;
        hAdcHighPath.Init.NbrOfConversion = 1;
        HAL_ADC_Init(&hAdcHighPath);

        /** Configure Regular Channel */
        sConfig.Channel = ADC_CHANNEL_0;
        sConfig.Rank = ADC_REGULAR_RANK_1;
        sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
        HAL_ADC_ConfigChannel(&hAdcHighPath, &sConfig);

        HAL_ADCEx_Calibration_Start(&hAdcHighPath);

        // Start ADC Conversion
        HAL_ADC_Start(&hAdcHighPath);

        // Activation GPIO
        GPIO_InitStruct.Pin = cHighCurrentConfig.mActivationPin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(cHighCurrentConfig.mActivationPort, &GPIO_InitStruct);
        HAL_GPIO_WritePin(cHighCurrentConfig.mActivationPort, 
                          cHighCurrentConfig.mActivationPin, 
                          GPIO_PIN_RESET);

        
        highCurrentZeroMv = readHighCurrentADCMv(10);
}

void CurrentSensing::initLowCurrentPath()
{
    // Activation GPIO
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = cLowCurrentConfig.mActivationPin;

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(cLowCurrentConfig.mActivationPort, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(cLowCurrentConfig.mActivationPort, 
                      cLowCurrentConfig.mActivationPin, 
                      GPIO_PIN_RESET);

    lowCurrentSensor.Init();

}

void CurrentSensing::setConfig(uint32_t pathCurrentLimit, 
                               uint32_t marginAbs)
{
    pathCurrentLimit_whenLow  = pathCurrentLimit+marginAbs;
    pathCurrentLimit_whenHigh = pathCurrentLimit-marginAbs;
}


uint32_t CurrentSensing::readCurrent()
{
    uint32_t lastRead;
    

    if( mCurrentPath == HIGH_CURRENT_PATH)
    {
        lastRead=readHighCurrentPath(3);
    }
    else
    {
        lastRead=readLowCurrentPath();
    }

    //currentPathLogic(lastRead);


    return lastRead;

}

void CurrentSensing::currentPathLogic(uint32_t lastRead)
{

    if (mCurrentPath == HIGH_CURRENT_PATH)
    {
        if ( lastRead < pathCurrentLimit_whenHigh )
        {
            mCurrentPath = LOW_CURRENT_PATH;
            internalUpdateCurrentPath();
        }
    }
    else
    {
        if ( lastRead > pathCurrentLimit_whenLow )
        {
            mCurrentPath = HIGH_CURRENT_PATH;
            internalUpdateCurrentPath();
        }
    }


}

void CurrentSensing::internalUpdateCurrentPath(){
    if( mCurrentPath == HIGH_CURRENT_PATH )
    {
        setHighCurrentPath(1);
        setLowCurrentPath(0);
    }
    else
    {
        setHighCurrentPath(0);
        setLowCurrentPath(1);
    }
}

uint32_t CurrentSensing::readHighCurrentADCMv(uint8_t muestras){
    uint32_t suma = 0;
    for( uint8_t i =0; i < muestras; i++){

        ADC_ChannelConfTypeDef sConfig = {0};
        sConfig.Channel = ADC_CHANNEL_0;
        sConfig.Rank = ADC_REGULAR_RANK_1;
        sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
        HAL_ADC_ConfigChannel(&hAdcHighPath, &sConfig);

        HAL_ADC_Start(&hAdcHighPath);

        HAL_ADC_PollForConversion(&hAdcHighPath, 1);
        suma += HAL_ADC_GetValue(&hAdcHighPath);
    }
    suma = suma / muestras;

    return ((float)suma*ADC_TO_MV);
}

uint32_t CurrentSensing::readHighCurrentPath(uint8_t muestras)
{

    uint32_t valueInMa = readHighCurrentADCMv(muestras);

    /*valueInMa = (valueInMa > highCurrentZeroMv)? valueInMa - highCurrentZeroMv:0;*/

    /*valueInMa = valueInMa*10;*/

    return valueInMa;
}

uint32_t CurrentSensing::readLowCurrentPath(){
    return lowCurrentSensor.getCurrent()/10;
}


void CurrentSensing::setHighCurrentPath(uint8_t isActivated)
{
    HAL_GPIO_WritePin(cHighCurrentConfig.mActivationPort, cHighCurrentConfig.mActivationPin, (GPIO_PinState)isActivated);
}

void CurrentSensing::setLowCurrentPath(uint8_t isActivated)
{
    HAL_GPIO_WritePin(cLowCurrentConfig.mActivationPort, cLowCurrentConfig.mActivationPin, (GPIO_PinState)isActivated);
}
