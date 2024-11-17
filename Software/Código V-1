//basicos
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_err.h"

//drivers
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/i2c.h"
#include "../lib/hx711.h"
#include "../lib/i2c-lcd.h"

//i2c definiciones---------------------------------------------------------------------------
static const char *TAG = "LCD";
#define I2C_MASTER_SCL_IO           GPIO_NUM_22      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           GPIO_NUM_21      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                      /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000
#define SLAVE_ADDRESS_LCD 0x4E //cambiar direccion correspondiente a su lcd

char buffer[10];
float num = 12.34;

esp_err_t esp_err;
#define I2C_NUM I2C_NUM_0
//fin de definciones de i2c LCD--------------------------------------------------------------

// celda de carga hx711 definiciones---------------------------------------------------------
static const char *TAG = "LoadCell";
#define DOUT_PIN GPIO_NUM_16    // Pin para la señal de datos (DOUT)
#define PD_SCK_PIN GPIO_NUM_4  // Pin para la señal de reloj (PD_SCK)
#define CONFIG_AVG_TIMES 3 // Variable que establece la cantidad de valores a promediar
// fin de definiciones de hx711--------------------------------------------------------------

void servoconfig(void)
{
    // Configuración del temporizador para PWM
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT, // Resolución de 8 bits (0-255)
        .timer_num = LEDC_TIMER_0, //eleccion del timer
        .freq_hz = 50,  // Frecuencia de 50 Hz para control de servomotor
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    // Configuración del canal PWM
    ledc_channel_config_t channel = {
        .gpio_num = 2,  // GPIO donde está conectado el servomotor
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,  // Ciclo de trabajo inicial
        .hpoint = 0
    };
    ledc_channel_config(&channel);

    // Instalamos la función de desvanecimiento (fade) para suavizar el control de velocidad
    // ledc_fade_func_install(0);
}

static esp_err_t i2c_master_init(void)
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

void LoadCell(void *pvParameters)
{
    hx711_t dev = {
        .dout = DOUT_PIN,
        .pd_sck = PD_SCK_PIN,
        .gain = HX711_GAIN_A_64
    };
    int32_t offset; // Para calibrarlo
    // initialize device
    ESP_ERROR_CHECK(hx711_init(&dev));
    ESP_ERROR_CHECK(hx711_read_average(&dev, CONFIG_AVG_TIMES, &offset));
    // read from device
    while (true)
    {
        esp_err_t raw = hx711_wait(&dev, 500);
        if (raw != ESP_OK)
        {
            ESP_LOGE(TAG, "Dispositivo no encontrad: %d (%s)\n", raw, esp_err_to_name(raw));
            continue;
        }

        int32_t data;
        raw = hx711_read_average(&dev, CONFIG_AVG_TIMES, &data);
        if (raw != ESP_OK)
        {
            ESP_LOGE(TAG, "No se pudo leer raw: %d (%s)\n", raw, esp_err_to_name(raw));
            continue;
        }
        
        int32_t kg_weight;
        ESP_ERROR_CHECK(hx711_read_average(&dev, CONFIG_AVG_TIMES, &kg_weight));
        float g_factor = 1000.0 / (kg_weight - offset);

        ESP_LOGI(TAG, "Raw data: %" PRIi32, data);

        ESP_ERROR_CHECK(hx711_read_average(&dev, CONFIG_AVG_TIMES, &raw));
        float weight_g = (raw - offset) * g_factor; //trabajamos con weight_g para condicionales
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    servoconfig();  

    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");
    
    lcd_init();
    lcd_clear();

    while (true)
    {
        // Movimiento del servomotor de 0° a 90° (puedes ajustar estos valores)
        for (int i = 25; i <= 63; i++) // Ajusta el rango de 25 a 125 para variar el ángulo (aproximadamente 0-180°)
        {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, i);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            vTaskDelay(10 / portTICK_PERIOD_MS);  // Espera 10 ms para permitir el movimiento
        }

        // Movimiento del servomotor de 90° a 0°
        for (int i = 63; i >= 25; i--) // Ajusta el rango de 125 a 25 para volver al ángulo inicial
        {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, i);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            vTaskDelay(10 / portTICK_PERIOD_MS);  // Espera 10 ms
        }
    }
    
//    lcd_put_cur(0, 0);
//    lcd_send_string("Hello world!");
//
//    lcd_put_cur(1, 0);
//    lcd_send_string("from ESP32");
    xTaskCreatePinnedToCore(LoadCell, "LoadCell", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL, 1);
}
