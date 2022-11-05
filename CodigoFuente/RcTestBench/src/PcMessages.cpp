#include "PcMessages.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "USB\usbd_cdc_if.h"


PcMessagesController::PcMessagesController(
                            CircularBuffer<uint8_t> *pRecvBuffer)
:recvBuffer(pRecvBuffer)
{

}

void PcMessagesController::sendTestData(float pwmValue,
                      float forceValue,
                      uint32_t currentInMa,
                      uint32_t feedbackVoltage)
{
    
    sprintf(sendBuffer, "[TEST_DATA]:%d:%d:%d:%d\n",
            (int)(pwmValue * 1000.0f),
            (int32_t)forceValue,
            currentInMa,
            feedbackVoltage);

    CDC_Transmit_FS((uint8_t *)sendBuffer, strlen(sendBuffer));

}


void PcMessagesController::sendDebug(const char *format, ...)
{
    char format2[200];
    va_list args;
    va_start(args, format);
    sprintf(format2, "[DEBUG]:%s", format);
    vsprintf(sendBuffer, format2, args);
    CDC_Transmit_FS((uint8_t *)sendBuffer, strlen(sendBuffer));
    va_end(args);

}


enum PcMessagesController::MESSAGE_RET 
PcMessagesController::waitForPing(uint32_t msToWait){

    uint32_t waitTimeStart = HAL_GetTick();
    while(recvBuffer->isEmpty())
    {
        if((HAL_GetTick() - waitTimeStart) > msToWait)
        {
            return MESSAGE_TIMEOUT;
        }
    }

    


}