/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "9x5shield.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG,"Starting app_main()");
    char message[] = "HELLO NATE, IT'S DAD.";
    while (1) {
        showMessage((byte *)message,strlen(message));
        vTaskDelay(pdMS_TO_TICKS(1000)*strlen(message));
        //ESP_LOGI(TAG,"Repeating from top of main while loop" );
    }

}
