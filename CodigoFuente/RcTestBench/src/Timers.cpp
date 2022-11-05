
#include "Timers.h"

#include <stm32f1xx_hal.h>


TIM_HandleTypeDef htim2;



void delay_us(uint32_t us){

  uint32_t Tim2Freq = HAL_RCC_GetPCLK1Freq();

  if ( (RCC->CFGR & RCC_CFGR_PPRE1) != 0){
    Tim2Freq *= 2;
  }

  htim2.Instance->CNT = 0;

  uint32_t match = us*Tim2Freq/1000000;
  if(match > ((1<<15)-1))
    return;
  while(htim2.Instance->CNT <= match);

}


static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0xFFFF;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_Base_Init(&htim2);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);

}

void InitTimers(){

    __HAL_RCC_TIM2_CLK_ENABLE();

    MX_TIM2_Init();
    HAL_TIM_Base_Start(&htim2);
}