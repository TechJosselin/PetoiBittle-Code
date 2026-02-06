#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "led.h"
#include "pca9685.h"
#include "mpu9250.h"
#include "webserver.h"

// LED integree du Seeed Xiao ESP32-C6
static constexpr gpio_num_t LED_GPIO = GPIO_NUM_15;

// Configuration I2C pour MPU-9250 et PCA9685
static constexpr int I2C_SCL_GPIO = 23;  // GPIO23 - I2C Clock
static constexpr int I2C_SDA_GPIO = 22;  // GPIO22 - I2C Data

// Configuration WiFi Access Point
#define AP_SSID           "Bittle-Robot"     // Nom du reseau WiFi
#define AP_PASSWORD       "bittle123"        // Mot de passe (min 8 caracteres)
#define AP_CHANNEL        1                  // Canal WiFi (1-13)
#define AP_MAX_CLIENTS    4                  // Nombre maximum de clients

static const char* TAG = "Main";

extern "C" void app_main(void)
{
  ESP_LOGI(TAG, "=== PetoiBittle - Demarrage ===");
  
  // Initialisation NVS (requis pour WiFi)
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  
  // Initialisation de la pile réseau
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  
  // Configuration LED
  led::init(LED_GPIO);

  // Initialisation MPU-9250
  if (!mpu9250::init(I2C_SDA_GPIO, I2C_SCL_GPIO, 0x69)) {
    ESP_LOGE(TAG, "Echec initialisation MPU-9250");
  } else {
    ESP_LOGI(TAG, "MPU-9250 initialise");
  }

  // Initialisation PCA9685 (controleur servo) avec adresse 0x72
  if (!pca9685::init(0, 50, 0x72)) {
    ESP_LOGE(TAG, "Echec initialisation PCA9685");
  } else {
    // Position initiale du servo 0 a 90 degres
    pca9685::set_servo_angle(0, 90);
  }

  // Configuration WiFi Access Point
  esp_netif_create_default_wifi_ap();
  
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  
  wifi_config_t ap_config = {};
  strcpy((char*)ap_config.ap.ssid, AP_SSID);
  strcpy((char*)ap_config.ap.password, AP_PASSWORD);
  ap_config.ap.ssid_len = strlen(AP_SSID);
  ap_config.ap.channel = AP_CHANNEL;
  ap_config.ap.max_connection = AP_MAX_CLIENTS;
  ap_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
  
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  
  ESP_LOGI(TAG, "WiFi AP démarré: %s (Password: %s)", AP_SSID, AP_PASSWORD);

  // Démarrage du serveur web
  if (!webserver::init(80)) {
    ESP_LOGE(TAG, "Echec démarrage serveur web");
  } else {
    char ip[16];
    if (webserver::get_ip(ip, sizeof(ip))) {
      ESP_LOGI(TAG, "Serveur web accessible sur http://%s", ip);
    }
  }

  ESP_LOGI(TAG, "");
  ESP_LOGI(TAG, "============================================");
  ESP_LOGI(TAG, "  BITTLE ROBOT - PRET");
  ESP_LOGI(TAG, "============================================");
  ESP_LOGI(TAG, "");

  // Boucle principale
  uint32_t loop_count = 0;

  while (true) {
    // Status periodique toutes les 10 secondes
    if (loop_count % 20 == 0) {
      ESP_LOGI(TAG, "LED %s",
               led::get_state() ? "ON " : "OFF");
    }

    loop_count++;
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
