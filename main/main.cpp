#include "HLW8032.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "stdio.h"
#include "Useruart.h"

float current = 0, volTage = 0, power = 0;

HLW8032_Obj hlw8032;

extern "C" void app_main(void)
{

    hlw8032.begin();   // 初始化hlw8032
    Base_Init_uart0(); // 初始化串口0
    while (true)
    {
        if (hlw8032.getVoltageAndCurrent(&volTage, &current, &power) == HLW8032_Obj::Status_t::OK)
        {
            printf("vol : %.2f, Current : %.2f, Power : %.2f\n", volTage, current, power);
        }
        else
        {
            printf("HLW8032 : OK\n");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
