#include "../lib/ultrasonido.h"

void ultrasonic_sensor_init() 
{
    esp_rom_gpio_pad_select_gpio(TRIGGER_GPIO);
    esp_rom_gpio_pad_select_gpio(ECHO_GPIO);
    gpio_set_direction(TRIGGER_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_GPIO, GPIO_MODE_INPUT);
}

uint32_t measure_distance() 
{
    gpio_set_level(TRIGGER_GPIO, 1);
    vTaskDelay(1 / portTICK_PERIOD_MS);
    gpio_set_level(TRIGGER_GPIO, 0);

    while (gpio_get_level(ECHO_GPIO) == 0);

    int64_t start_time = esp_timer_get_time();
    while (gpio_get_level(ECHO_GPIO) == 1);
    int64_t end_time = esp_timer_get_time();

    uint32_t distance = ((end_time - start_time) * 34) / 2000;
    return distance;
}

void distance_measurement_task(void *pvParameters) 
{
    while (1) 
    {
        uint32_t distance = measure_distance();
        printf("Distancia: %ld cm\n", distance);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
