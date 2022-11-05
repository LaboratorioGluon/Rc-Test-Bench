#ifndef __COMMS_H__
#define __COMMS_H__

#include <stm32f1xx_hal.h>

extern UART_HandleTypeDef huart1;

static void MX_USART1_UART_Init(void);
void InitUART();


#endif //__COMMS_H__