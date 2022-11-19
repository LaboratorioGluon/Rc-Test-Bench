#include "voltageMeter.h"

#define ADC_TO_MV (0.8056640625f)
    
voltageMeter::voltageMeter(GPIO_TypeDef *pAdcPort, uint32_t pAdcPin):
    mAdcPort(pAdcPort), mAdcPin(pAdcPin)
{

}

void voltageMeter::Init()
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_ADC2_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = mAdcPin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(mAdcPort, &GPIO_InitStruct);

    ADC_ChannelConfTypeDef sConfig = {0};

    /** Common config */
    mAdcHandle.Instance = ADC1;
    mAdcHandle.Init.ScanConvMode = ADC_SCAN_DISABLE;
    mAdcHandle.Init.ContinuousConvMode = ENABLE;
    mAdcHandle.Init.DiscontinuousConvMode = DISABLE;
    mAdcHandle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    mAdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    mAdcHandle.Init.NbrOfConversion = 1;
    HAL_ADC_Init(&mAdcHandle);

    /** Configure Regular Channel */
    sConfig.Channel = ADC_CHANNEL_4;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
    HAL_ADC_ConfigChannel(&mAdcHandle, &sConfig);

    HAL_ADCEx_Calibration_Start(&mAdcHandle);

    // Start ADC Conversion
    HAL_ADC_Start(&mAdcHandle);

}

uint32_t voltageMeter::GetVoltageMv(uint32_t pNumMeasures)
{
    uint32_t suma = 0;
    for( uint8_t i =0; i < pNumMeasures; i++){

        ADC_ChannelConfTypeDef sConfig = {0};
        /** Configure Regular Channel */
        sConfig.Channel = ADC_CHANNEL_4;
        sConfig.Rank = ADC_REGULAR_RANK_1;
        sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
        HAL_ADC_ConfigChannel(&mAdcHandle, &sConfig);

        HAL_ADC_Start(&mAdcHandle);

        HAL_ADC_PollForConversion(&mAdcHandle, 1);
        suma += HAL_ADC_GetValue(&mAdcHandle);
    }
    suma = (suma / pNumMeasures)*ADC_TO_MV*5.7f;
    return suma;
}