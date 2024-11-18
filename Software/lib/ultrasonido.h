#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

// Definición de pines (ajusta según tu proyecto)
#define TRIGGER_GPIO 23 // Ajusta según tu configuración
#define ECHO_GPIO 22    // Ajusta según tu configuración

// Declaración de funciones
void ultrasonic_sensor_init();
uint32_t measure_distance();
void distance_measurement_task(void *pvParameters);

#endif // ULTRASONIC_SENSOR_H
