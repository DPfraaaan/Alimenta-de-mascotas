#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "unistd.h"
#include "rom/ets_sys.h"
#include "math.h"

#include "esp_timer.h"
#include "esp_log.h"
#include "esp_err.h"

#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "driver/i2c.h"

#include "ultrasonic.h"
#include "ultrasonic.c"
#include "brasuca.h"


// Definir pines y variables
#define RED_LED_PIN GPIO_NUM_26
#define GREEN_LED_PIN GPIO_NUM_32
#define BUZZER_PIN GPIO_NUM_25

uint32_t duty;
#define servopin 4

#define BUTTON_PIN GPIO_NUM_13
#define MAX_ANIMAL 2
int TAMANO_ANIMAL = 0;
uint32_t presion_boton = 0;

//ultrasonido
#define PIN_ECHO GPIO_NUM_14
#define PIN_TRIGGER GPIO_NUM_12
#define MAX_DISTANCE_CM 400 //esto para config
#define MAX_CM 17
#define MIN_CM 4
uint32_t distancia;

// Variables de tiempo
unsigned long prevMillisUltrasonido = 0;
unsigned long prevMillisServo = 0;
unsigned long intervalUltrasonido = 60000; // 1 minuto
unsigned long intervalServo = 500; // 500 ms

ultrasonic_sensor_t sensor; // Variable global


//inicio LDR -----------------------------
int rawLDR;
bool servobreaker = true;

#define ADC_1 1
#define ADC_2 2

typedef struct {
  uint8_t adc_device;
  uint8_t adc_channel;
  float k1;
  float k2;
} ldr_handle_t;

ldr_handle_t ldr; // Declaración global

float ldr_get_illuminance(ldr_handle_t * ldr) {
  if (ldr->adc_device == ADC_2) {
    int buffer;
    adc2_get_raw(ldr->adc_channel, ADC_WIDTH_BIT_12, &buffer);
    return ldr->k1 + ldr->k2 * log((4096.0 / buffer) - 1);
  }

  else {
    return ldr->k1 + ldr->k2 * log((4096.0 / adc1_get_raw(ldr->adc_channel)) - 1);
  }
}

#define THRESHOLD 3600 // valor max del rawLDR

// rawLDR = (ldr_get_illuminance(&ldr)/100)*4096-1

// fin de LDR--------------------------------------

// inicio i2c--------------------------------------

lcd_i2c_handle_t display;

void i2c_init() 
{
  i2c_config_t i2c_config = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = 22,
      .sda_pullup_en = 1,
      .scl_io_num = 21,
      .scl_pullup_en = 1,
      .master.clk_speed = 100000,
    };

    i2c_param_config(I2C_NUM_1, &i2c_config);

    i2c_driver_install(I2C_NUM_1, I2C_MODE_MASTER, 0, 0, 0);
}

bool actualizacion_lcd_necesaria = false;

void lcd_i2c_clear(lcd_i2c_handle_t * lcd) {
  lcd_i2c_write(lcd, 0, CLEAR_DISPLAY); // Limpia la pantalla
  vTaskDelay(10 / portTICK_PERIOD_MS);   // Espera para que el LCD procese el comando
}
// fin i2c-----------------------------------------

void servoconfig()
{
  ledc_timer_config_t timer = {
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_13_BIT,
    .timer_num = LEDC_TIMER_0,
    .freq_hz = 50,
    .clk_cfg = LEDC_AUTO_CLK
  };
  ledc_timer_config(&timer);

  ledc_channel_config_t channel = {
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .timer_sel = LEDC_TIMER_0,
    .intr_type = LEDC_INTR_DISABLE,
    .gpio_num = servopin,
    .duty = 0,
    .hpoint = 0
  };
  ledc_channel_config(&channel);
}

void ultrasonico_init() 
{
    sensor.trigger_pin = PIN_TRIGGER;
    sensor.echo_pin = PIN_ECHO;
    ultrasonic_init(&sensor);
}

uint32_t medir_dis()
{
  uint32_t distance;
  esp_err_t res = ultrasonic_measure_cm(&sensor, MAX_DISTANCE_CM, &distance);
  return distance;  // Retorna la distancia medida
}

void generalconfig()
{
  esp_rom_gpio_pad_select_gpio(RED_LED_PIN);
  gpio_set_direction(RED_LED_PIN, GPIO_MODE_OUTPUT);

  esp_rom_gpio_pad_select_gpio(GREEN_LED_PIN);
  gpio_set_direction(GREEN_LED_PIN, GPIO_MODE_OUTPUT);

  esp_rom_gpio_pad_select_gpio(BUZZER_PIN);
  gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);

  esp_rom_gpio_pad_select_gpio(BUTTON_PIN);
  gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);
  // gpio_pullup_en(BUTTON_PIN);  // Habilitar pull-up interno
  
}


void boton() 
{
    if (gpio_get_level(BUTTON_PIN) == 0) {
        presion_boton = esp_timer_get_time();
        TAMANO_ANIMAL += 1;
        if (TAMANO_ANIMAL > MAX_ANIMAL) {
            TAMANO_ANIMAL = 0;
        }
        // lcd_clear();
        lcd_i2c_clear(&display);
        lcd_i2c_cursor_set(&display, 1, 0); 
        lcd_i2c_print(&display, "Tm: "); 
        lcd_i2c_cursor_set(&display, 1, 3);
        if (TAMANO_ANIMAL == 1) {
            lcd_i2c_print(&display, "G");
        } else if (TAMANO_ANIMAL == 2) {
            lcd_i2c_print(&display, "M");
        } else {
            lcd_i2c_print(&display, "C");
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    // Verificar si pasaron 5 segundos sin presionar el botón
    if (presion_boton > 0 && (esp_timer_get_time() - presion_boton) >= 5000000) {
        presion_boton = 0; // Resetear tiempo de presión
        lcd_i2c_cursor_set(&display, 1, 3);  
        lcd_i2c_print(&display, "seleccionado"); 
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
}

void porongus() 
{
  // Leer el valor del LDR
  rawLDR = (ldr_get_illuminance(&ldr)/100)*4096-1;
  if (rawLDR > THRESHOLD) { // No hay luz (si es de noche)
    // Lo siguiente es para comida de tanque
    uint32_t distancia = medir_dis();
    servobreaker = true;
    if (distancia > MAX_CM) {
        gpio_set_level(RED_LED_PIN, 1);
        gpio_set_level(BUZZER_PIN, 1);
        vTaskDelay(250 / portTICK_PERIOD_MS);
        gpio_set_level(RED_LED_PIN, 0);
        gpio_set_level(BUZZER_PIN, 0);
        vTaskDelay(250 / portTICK_PERIOD_MS);
    } else if (distancia > MIN_CM && distancia < MAX_CM) {
        gpio_set_level(GREEN_LED_PIN, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(GREEN_LED_PIN, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
      }
  } else if (rawLDR < 600 && servobreaker == true) { // De día
    duty = 600; // 90 grados
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    if (TAMANO_ANIMAL == 1) {
        vTaskDelay(2000 / portTICK_PERIOD_MS); // Grande
    } else if (TAMANO_ANIMAL == 2) {
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Mediano
    } else {
        vTaskDelay(500 / portTICK_PERIOD_MS); // Chico
    }
    duty = 200; // 0 grados
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    if (TAMANO_ANIMAL == 1) {
        vTaskDelay(2000 / portTICK_PERIOD_MS); // Grande
    } else if (TAMANO_ANIMAL == 2) {
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Mediano
    } else {
        vTaskDelay(500 / portTICK_PERIOD_MS); // Chico
    }
    servobreaker = false;
  }
}

void app_main(void) 
{
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_7,  ADC_ATTEN_DB_11);

  ldr.adc_device = ADC_1;
  ldr.adc_channel = ADC1_CHANNEL_7;
  ldr.k1 = 49.95095;
  ldr.k2 = 10.34034;
  
  //inicio config
  servoconfig();
  ultrasonico_init();
  generalconfig();

  i2c_init();
  display = (lcd_i2c_handle_t) {
    .address = 0x27,
    .num = I2C_NUM_1,
    .backlight = 1,
    .size = DISPLAY_16X02
  };
  lcd_i2c_init(&display);

  lcd_i2c_cursor_set(&display, 1, 5);  // Mover el cursor a la fila 1, columna 3
  lcd_i2c_print(&display, "Alimentador");  // Escribir en la pantalla

  while (1) 
  {
    boton();
    porongus();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
