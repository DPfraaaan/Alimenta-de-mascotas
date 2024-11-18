// Basicos 
#include <stdio.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"
//configMINIMAL_STACK_SIZE = 1024 y esto es como para una tarea de LED BLINK nada más, en perspectiva

// Drivers & librerías
#include "driver/gpio.h" 
#include "driver/ledc.h"
#include "driver/i2c.h" 
#include "../lib/i2c-lcd.h"
#include "../lib/ultrasonido.h"

// General definiciones-----------------------------------------------------------------------
#define BUTTON_PIN 8
#define BUZZER_PIN 10
#define REDLED_PIN 11
#define GREENLED_PIN 12
#define TRIGGER_GPIO 5
#define ECHO_GPIO 18
#define SERVO_PIN 2

// Referido a sensor ultrasonico & LCD
static const int DISTANCIA_UMBRAL_ALTO = 100;  // Umbral alto (cm)
static const int DISTANCIA_UMBRAL_BAJO = 50;   // Umbral bajo (cm)
static const int MAX_HORAS = 24;
static const int MINUTOS_POR_HORA = 60;
int HORAS_ESPERA_ULTRASONIDO; // variable a modificar
static bool es_ultrasonido_activado = false;

static uint32_t presion_boton = 0;
static uint32_t minutos_transcurridos = 0;

static bool actualizacion_lcd_necesaria = false;
static char status_message[32];  // Mensaje a mostrar en el LCD
static int remaining_time = 0;   // Tiempo restante para mostrar en el LCD

esp_timer_handle_t temporizador_ultrasonido;
esp_timer_handle_t temporizador_servo_90;
esp_timer_handle_t temporizador_servo_0;


// i2c definiciones--------------------------------------------------------------------------- 
static const char *TAG_LCD = "LCD"; 
#define I2C_MASTER_SCL_IO GPIO_NUM_22 //GPIO number used for I2C master clock  
#define I2C_MASTER_SDA_IO GPIO_NUM_21 // GPIO number used for I2C master data  
#define I2C_MASTER_NUM 0 // I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip 
#define I2C_MASTER_FREQ_HZ 400000 // I2C master clock frequency 
#define I2C_MASTER_TX_BUF_DISABLE 0 // I2C master doesn't need buffer 
#define I2C_MASTER_RX_BUF_DISABLE 0 // I2C master doesn't need buffer 
#define I2C_MASTER_TIMEOUT_MS 1000 
char buffer[10]; 
float num = 12.34;
esp_err_t esp_err; 
#define I2C_NUM I2C_NUM_0 

// hx711 definiciones-------------------------------------------------------------------------
#define AVG_SAMPLES   10
#define GPIO_PD_SCK   GPIO_NUM_15
#define GPIO_DOUT   GPIO_NUM_16
#define GAIN 128
unsigned long Pesocomida = 0;
float pesoMAX = 1.5; //suponemos kg
static const char* TAG_HX711 = "HX711";

static esp_err_t i2c_master_init(void) // Configuración del lcd i2c---------------------------
{ 
    
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

static void read_weight(void) // Configuración del HX711 -------------------------------------
{
    HX711_init(GPIO_DOUT, GPIO_PD_SCK, GAIN); 
    HX711_tare();

    unsigned long weight = HX711_get_units(AVG_SAMPLES);
    ESP_LOGI(TAG_HX711, "******* weight = %ld *********\n", weight);
    Pesocomida = weight;  // Guardar el peso en la variable global
}

static void generalconfig()
{
    esp_rom_gpio_pad_select_gpio(BUTTON_PIN);
    esp_rom_gpio_pad_select_gpio(BUZZER_PIN);
    esp_rom_gpio_pad_select_gpio(REDLED_PIN);
    esp_rom_gpio_pad_select_gpio(GREENLED_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(REDLED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(GREENLED_PIN, GPIO_MODE_OUTPUT);

    // Servo config
    ledc_timer_config_t timer = { 
        .speed_mode = LEDC_LOW_SPEED_MODE, 
        .duty_resolution = LEDC_TIMER_8_BIT, // Resolución de 8 bits (0-255) 
        .timer_num = LEDC_TIMER_0, //eleccion del timer 
        .freq_hz = 50, // Frecuencia de 50 Hz para control de servomotor 
        .clk_cfg = LEDC_AUTO_CLK 
        }; 
        ledc_timer_config(&timer);

    // Configuración del canal PWM
    ledc_channel_config_t channel = {
        .gpio_num = SERVO_PIN,  // GPIO donde está conectado el servomotor
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,  // Ciclo de trabajo inicial
        .hpoint = 0
        };
        ledc_channel_config(&channel);
        // ledc_fade_func_install(0); // Instalamos la función de desvanecimiento (fade) para suavizar el control de velocidad
}

static void boton()
{
 while (true)
    {
        if (gpio_get_level(BUTTON_PIN) == 1) {
            presion_boton = xTaskGetTickCount() * portTICK_PERIOD_MS; // Guardar el tiempo de presion del botón

            // Incrementar horas del temporizador
            HORAS_ESPERA_ULTRASONIDO += 2;
            if (HORAS_ESPERA_ULTRASONIDO >= MAX_HORAS) {
                HORAS_ESPERA_ULTRASONIDO = 0;
            }
            ESP_LOGI("TEMPORIZADOR", "Horas del temporizador: %d", HORAS_ESPERA_ULTRASONIDO);

            vTaskDelay(100 / portTICK_PERIOD_MS); // Pequeño delay para evitar bouncing
        }

        // Verificar si pasaron 5 segundos sin presionar el botón
        if (presion_boton > 0 && (xTaskGetTickCount() * portTICK_PERIOD_MS - presion_boton) >= 5000) {
            presion_boton = 0; // Resetear tiempo de presion
            //iniciar temporizador agregar
        }

        vTaskDelay(100 / portTICK_PERIOD_MS); // Delay para evitar alta carga de CPU
    }
}

static void toggle_red_led()
{
    gpio_set_level(REDLED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(500));  // Encender por 500ms
    gpio_set_level(REDLED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(500));  // Apagar por 500ms
}

static void turn_on_green_led()
{
    gpio_set_level(GREENLED_PIN, 1);  // Encender el LED verde
}

static void turn_off_green_led()
{
    gpio_set_level(GREENLED_PIN, 0);  // Apagar el LED verde
}

void temporizador_callback(void* arg) // Función de interrupción para temporizador ------------
{
    minutos_transcurridos++;  // Incrementamos el tiempo transcurrido
    remaining_time = (HORAS_ESPERA_ULTRASONIDO * MINUTOS_POR_HORA) - minutos_transcurridos; // para mostrar en el lcd

    if (minutos_transcurridos >= (HORAS_ESPERA_ULTRASONIDO * MINUTOS_POR_HORA)) 
    {
        uint32_t distance = measure_distance();
        printf("Distancia medida: %ld cm\n", distance);

        // Si la distancia es mayor que el umbral alto, se enciende el LED rojo intermitente
        if (distance > DISTANCIA_UMBRAL_ALTO) 
        {
            printf("Distancia superior al umbral. Encendiendo LED rojo intermitente...\n");
            toggle_red_led();  // LED rojo intermitente
            snprintf(status_message, sizeof(status_message), "Falta comida");  // Mensaje en el LCD
        }
        // Si la distancia está en el rango medio del umbral, se enciende el LED verde
        else if (distance > DISTANCIA_UMBRAL_BAJO && distance <= DISTANCIA_UMBRAL_ALTO) 
        {
            printf("Distancia en el rango medio del umbral. Encendiendo LED verde...\n");
            turn_on_green_led();
            snprintf(status_message, sizeof(status_message), "Comida ok");  // Mensaje en el LCD
        }
        else
        {
            // Apagamos el LED verde si no está en el rango medio
            turn_off_green_led();
        }
        read_weight();
        if (Pesocomida  > pesoMAX) // si ya hay suficiente comida en el plato entonces que no lo rellene
        {
            snprintf(status_message, sizeof(status_message), "Hay comida");  // Mensaje en el LCD
        }
        else
        {
            // Mover el servo a 90 grados
            move_servo_to_position(90);  // Mover el servo a 90 grados
            esp_timer_start_once(temporizador_servo_0, 500000);
        }
        
        // Reiniciar el temporizador
        minutos_transcurridos = 0;
        es_ultrasonido_activado = true;

        // Marcar que se necesita una actualización en el LCD
        actualizacion_lcd_necesaria = true;
    }
}

void configurar_temporizadores() // Configuración de los temporizadores clase hardware --------
{
    esp_timer_create_args_t timer_args = {
        .callback = temporizador_callback,  // Función de callback
        .name = "temporizador_ultrasonido"  // Nombre del temporizador
    };

    esp_timer_create(&timer_args, &temporizador_ultrasonido);  // Crear el temporizador
    esp_timer_start_periodic(temporizador_ultrasonido, 60000000);  // Temporizador cada 1 minuto (60,000,000 us)

    // configurar_temporizadores_servo
    esp_timer_create_args_t args_90 = {
        .callback = mover_servo_90,
        .name = "mover_servo_90"
    };
    esp_timer_create_args_t args_0 = {
        .callback = mover_servo_0,
        .name = "mover_servo_0"
    };
    ESP_ERROR_CHECK(esp_timer_create(&args_90, &temporizador_servo_90));
    ESP_ERROR_CHECK(esp_timer_create(&args_0, &temporizador_servo_0));
}

static void move_servo_to_position(int position)
{   
    // Convertir la posición (en grados) al valor del ciclo de trabajo de PWM
    // Para un servomotor común, 0 grados corresponde a 0% de ciclo de trabajo y 180 grados a 100%
    int duty = (position * 255) / 180;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void lcd_update_display_task(void* pvParameters)
{
    while (true) 
    {
        if (actualizacion_lcd_necesaria) 
        {
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_print(status_message);  // Mostrar mensaje en el LCD

            // Mostrar el tiempo restante (en minutos)
            lcd_set_cursor(0, 1);
            lcd_print("Restante: ");
            lcd_print_number(remaining_time);

            actualizacion_lcd_necesaria = false;  // Resetear flag de actualización
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);  // Retraso para evitar actualización continua
    }
}

static void mover_servo_90(void* arg) {
    move_servo_to_position(90);  // Mover el servo a 90 grados
    // Iniciar temporizador para moverlo a 0 grados después de 500 ms
    esp_timer_start_once(temporizador_servo_0, 500000);  // 500 ms en microsegundos
}

static void mover_servo_0(void* arg) {
    move_servo_to_position(0);  // Mover el servo a 0 grados
}

void monitor_task(void *pvParameters) {
    while (1) {
        UBaseType_t boton_stack = uxTaskGetStackHighWaterMark(NULL);
        UBaseType_t lcd_stack = uxTaskGetStackHighWaterMark(NULL);

        printf("Stack usage for boton task: %d bytes\n", boton_stack);
        printf("Stack usage for LCD update task: %d bytes\n", lcd_stack);

        vTaskDelay(pdMS_TO_TICKS(1000)); // Monitorea cada segundo
    }
}

void app_main(void) 
{
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG_LCD, "I2C initialized successfully");
    lcd_init();
    lcd_clear();

    generalconfig();
    configurar_temporizadores();
    ultrasonic_sensor_init();

    // Crear las tareas
    xTaskCreatePinnedToCore(boton, "Task Boton", configMINIMAL_STACK_SIZE*2, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(lcd_update_display_task, "Task LCD Update", configMINIMAL_STACK_SIZE*4, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(monitor_task, "Task Monitor", 2048, NULL, 5, NULL, tskNO_AFFINITY); // Monitoreo
}