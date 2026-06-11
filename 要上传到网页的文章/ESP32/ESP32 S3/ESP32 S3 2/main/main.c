#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_PIN 2

void app_main(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    while (1) {
        gpio_set_level(LED_PIN, 1);  /* LED亮 */
        vTaskDelay(pdMS_TO_TICKS(500));  /* 延时500ms */
        gpio_set_level(LED_PIN, 0);  /* LED灭 */
        vTaskDelay(pdMS_TO_TICKS(500));  /* 延时500ms */
    }
}