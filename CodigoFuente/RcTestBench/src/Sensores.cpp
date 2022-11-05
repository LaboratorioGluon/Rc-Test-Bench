
#include "Sensores.h"
#include "CurrentSensing.h"
#include "voltageMeter.h"

#include <stm32f1xx.h>

extern UART_HandleTypeDef huart1;

#define CURRENT_ADC_PORT GPIOA
#define CURRENT_ADC_PIN GPIO_PIN_0

namespace Sensores{

    HX711 hx711;
    CurrentSensing currentSensing;
    voltageMeter lipoFeedback(GPIOA, GPIO_PIN_4);

    void Init(){

        // Weight/Force sensor
        hx711.Init(GPIOB, GPIO_PIN_14, GPIOB, GPIO_PIN_15);
        HAL_UART_Transmit(&huart1, (uint8_t*)"=== HX711 ===\n",14, 1000);
        HAL_UART_Transmit(&huart1, (uint8_t*)"  Calibrando ...",16, 1000);
        hx711.Tare(20);
        HAL_UART_Transmit(&huart1, (uint8_t*)"OK \n",4, 1000);
        hx711.setScale(316.0f);
        HAL_UART_Transmit(&huart1, (uint8_t*)"  Scale set: 316.0f\n",20, 1000);

        currentSensing.Init();

        lipoFeedback.Init();

    }


}
