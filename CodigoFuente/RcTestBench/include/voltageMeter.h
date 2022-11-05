#ifndef __VOLTAGEMETER_H__
#define __VOLTAGEMETER_H__

#include <stm32f1xx.h>
#include <stdint.h>

class voltageMeter
{
public:
    
    voltageMeter(GPIO_TypeDef *pAdcPort, uint32_t pAdcPin);

    void Init();

    uint32_t GetVoltage(uint32_t pNumMeasures);

private:

    ADC_HandleTypeDef mAdcHandle;

    GPIO_TypeDef *mAdcPort;
    uint32_t mAdcPin;

};

#endif // __VOLTAGEMETER_H__