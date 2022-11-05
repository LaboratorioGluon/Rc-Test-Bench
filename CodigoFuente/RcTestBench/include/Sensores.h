#ifndef __SENSORES_H__
#define __SENSORES_H__

#include <hx711.h>
#include "CurrentSensing.h"
#include "voltageMeter.h"

namespace Sensores
{

    extern HX711 hx711;
    extern CurrentSensing currentSensing;
    extern voltageMeter lipoFeedback;

    void Init();
    
} // namespace Sensores




#endif //__SENSORES_H__
