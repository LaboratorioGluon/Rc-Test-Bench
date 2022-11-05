#ifndef __PC_MESSAGES_H__
#define __PC_MESSAGES_H__

#include <stdint.h>

#include "CircularBuffer.h"


class PcMessagesController
{
public:
    enum MESSAGE_RET{
        MESSAGE_FAIL = 1,
        MESSAGE_OK   = 0,
        MESSAGE_TIMEOUT
    };
    
    /**
     * @brief Construct a new Pc Messages Controller object
     * 
     * @param recvBuffer Already allocated buffer, with the received contents
     */
    PcMessagesController(CircularBuffer<uint8_t> *pRecvBuffer);

    void sendTestData(float pwmValue,
                      float forceValue,
                      uint32_t currentInMa,
                      uint32_t feedbackVoltage);

    void sendDebug(const char *format, ...);

    enum MESSAGE_RET waitForPing(uint32_t msToWait);

private:

    

    char sendBuffer[200];

    CircularBuffer<uint8_t> *recvBuffer;
};

#endif //__PC_MESSAGES_H__