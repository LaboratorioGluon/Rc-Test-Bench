#include <stm32f1xx.h>

#include <hx711.h>
#include <Servo.h>

#include "basicConfig.h"
#include "Timers.h"
#include "Comms.h"
#include "CircularBuffer.h"
#include "TLVManager.h"

#include "Sensores.h"
#include "PcMessages.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "USB\usb_device.h"
#include "STM32_USB_Device_Library\Class\usbd_cdc.h"
#include "USB\usbd_cdc_if.h"

#include "ina219.h"

#define NUMERO_ITERACIONES (60)

#define EMERGENCY_STOP_TIME (400)

CircularBuffer<uint8_t> uartBuffer(200);
CircularBuffer<uint8_t> usbBuffer(200);
TLV gTlvManager;

Servo ESC;

extern USBD_HandleTypeDef hUsbDeviceFS;
extern "C"
{

    void USART1_IRQHandler(void)
    {
        uint32_t isrflags = READ_REG(huart1.Instance->SR);

        if ((isrflags & USART_SR_RXNE) != RESET)
        {
            uartBuffer.Write(huart1.Instance->DR & 0xFF);
        }

        HAL_NVIC_ClearPendingIRQ(USART1_IRQn);
    }

    void Error_Handler() {}
}

void masterStartMeasuring()
{
    // Connect to ESC and calibrate
    CDC_Transmit_FS((uint8_t *)"  Comienzo calibracion .", 25);
    ESC.setOutMs(2);
    HAL_Delay(1000);

    // Give power to the ESC.

    // Configure current path
    Sensores::currentSensing.setCurrentPath(1);
    Sensores::currentSensing.startMeasuring();

    for (int i = 0; i < 20; i++)
    {
        ESC.setOutMs(2.0f - (i * 1.0f / 20.0f));
        CDC_Transmit_FS((uint8_t *)".", 1);
        HAL_Delay(100);
    }

    CDC_Transmit_FS((uint8_t *)" FIN\n", 5);

    HAL_Delay(500);
}

void masterStopMeasuring()
{
    ESC.setOutMs(1);
    Sensores::currentSensing.stopMeasuring();
}

enum CMD
{
    NOTHING = 0,
    SET_PWM,
    START_RUN,
    READ_CURRENT,
    PING = 0xAA
};

uint8_t USB_RecvBuffer[256];

int main()
{

    HAL_Init();

    SystemClock_Config();

    uartBuffer.Init();
    gTlvManager.Init();

    InitTimers();
    // InitUART();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM1_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    GPIO_InitTypeDef GpioUSBHack;
    /* libopencm3
        gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
              GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
    gpio_clear(GPIOA, GPIO12);
    msleep(5); //delay
    */

    __HAL_RCC_GPIOA_CLK_ENABLE();
    GpioUSBHack.Mode = GPIO_MODE_OUTPUT_PP;
    GpioUSBHack.Pin = GPIO_PIN_12;
    GpioUSBHack.Pull = GPIO_PULLUP;
    GpioUSBHack.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GpioUSBHack);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_Delay(50);


    memset(USB_RecvBuffer, 0, 256);
    MX_USB_DEVICE_Init();

    // Inicializamos sensores
    Sensores::Init();

    // PWM output
    ESC.Init(TIM1, 1, GPIOA, GPIO_PIN_8);

    PcMessagesController PcMessages(&usbBuffer);


    float pwmOut = 1.4;
    float delta_pwm;
    char buf[150];
    volatile float fValue;

    uint8_t bufferlocal[200];

    uint8_t last_data;
    tlv_command last_tlv;
    uint32_t last_received_data_time = 0;
    uint32_t last_received_ping = 0;

    uint32_t swWatchdogTime=0;
    uint8_t isOk;

    while (1)
    {

        if (!usbBuffer.isEmpty())
        {

            if ((HAL_GetTick() - last_received_data_time) > 200)
            {
                gTlvManager.Timeout();
            }

            last_received_data_time = HAL_GetTick();
            memset(bufferlocal, 0, 200);

            while (!usbBuffer.isEmpty())
            {

                usbBuffer.Read(&last_data);
                gTlvManager.addData(last_data);
            }

            if (gTlvManager.getLastTlv(&last_tlv) == TLV_OK)
            {
                switch ((CMD)last_tlv.tag)
                {
                case CMD::NOTHING:
                    break;
                case CMD::SET_PWM:
                    pwmOut = (float)last_tlv.data[0] * 1.0f / 100.0f;
                    ESC.setOutMs(pwmOut);
                    PcMessages.sendDebug("Setting PWM: %d\n", (int)pwmOut);
                    break;

                case CMD::START_RUN:

                    PcMessages.sendDebug("\n===Iniciando Medidas===\n");

                    delta_pwm = (last_tlv.data[0] * 1.0f / 100.0f) / ((float)NUMERO_ITERACIONES);

                    PcMessages.sendDebug("  Maximo: %d\n",(int)last_tlv.data[0]);
                    pwmOut = 1.0f;
                    masterStartMeasuring();


                    Sensores::currentSensing.setConfig(100, 10);

                    isOk = true;
                    last_received_ping = HAL_GetTick();

                    for (uint8_t iteracion = 0; (iteracion < NUMERO_ITERACIONES) && isOk; iteracion++)
                    {

                        // Si llega algo por USB cancelamos.
                        if (!usbBuffer.isEmpty())
                        {
                            while (!usbBuffer.isEmpty())
                            {
                                usbBuffer.Read(&last_data);
                                gTlvManager.addData(last_data);
                            }
                            if(gTlvManager.getLastTlv(&last_tlv) == TLV_OK)
                            {
                                // Tenemos un mensaje pero no es ping
                                if(last_tlv.tag != CMD::PING)
                                {
                                    break;
                                }
                                else
                                {
                                    PcMessages.sendDebug("PING Received!");
                                    last_received_ping = HAL_GetTick();

                                }
                            }
                            else
                            {
                                // Si hay algo en el USB, pero no sabemos que 
                                // es, salir porsiaca.
                                break;
                            }
                            
                        }

                        // Comprobamos tiempo desde ultimo ping
                        if((HAL_GetTick()-last_received_ping) > EMERGENCY_STOP_TIME)
                        {
                            break;
                        }

                        pwmOut = 1.0f + iteracion * delta_pwm;
                        ESC.setOutMs(pwmOut);
                        HAL_Delay(10);
                        fValue = Sensores::hx711.getValue(5);

                        PcMessages.sendTestData(
                            pwmOut,
                            fValue,
                            Sensores::currentSensing.readCurrent(),
                            Sensores::lipoFeedback.GetVoltage(1)
                        );

                        swWatchdogTime = HAL_GetTick();

                        /*while(usbBuffer.isEmpty())
                        {
                            if((HAL_GetTick() - swWatchdogTime) > EMERGENCY_STOP_TIME)
                            {
                                isOk = false;
                                break;
                            }
                        }*/
                    }
                    Sensores::currentSensing.stopMeasuring();
                    ESC.setOutMs(1.0f);
                    break;

                case CMD::READ_CURRENT:
                    while (usbBuffer.isEmpty())
                    {
                        sprintf(buf, "Current: %d\n", Sensores::currentSensing.readCurrent());
                        CDC_Transmit_FS((uint8_t *)buf, strlen(buf));
                    }
                    break;
                }
            }
        }

        memset(buf, 0, 150);

    }

    return 0;
}
