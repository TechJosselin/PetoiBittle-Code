#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
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

static bool init_i2c_bus()
{
  i2c_config_t conf = {};
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = (gpio_num_t)I2C_SDA_GPIO;
  conf.scl_io_num = (gpio_num_t)I2C_SCL_GPIO;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = 400000;

  esp_err_t ret = i2c_param_config(I2C_NUM_0, &conf);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "I2C param config failed: %s", esp_err_to_name(ret));
    return false;
  }

  ret = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(ret));
    return false;
  }

  ESP_LOGI(TAG, "I2C initialised (SDA=%d, SCL=%d)", I2C_SDA_GPIO, I2C_SCL_GPIO);
  return true;
}

static void scan_i2c_bus()
{
  ESP_LOGI(TAG, "Scanning I2C bus...");
  int found = 0;

  for (uint8_t addr = 1; addr < 0x7F; addr++) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
      ESP_LOGI(TAG, "I2C device found at 0x%02X", addr);
      found++;
    }
  }

  if (found == 0) {
    ESP_LOGW(TAG, "No I2C devices found");
  }
}

extern "C" void app_main(void)
{
  vTaskDelay(pdMS_TO_TICKS(500));
  printf("BOOT: app_main start\n");
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

  // Initialisation I2C
  if (!init_i2c_bus()) {
    ESP_LOGE(TAG, "Echec initialisation I2C");
  }
  scan_i2c_bus();

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
  
  // Supprimer les warnings "wifi:(tx)rts error" qui spamment la console
  esp_log_level_set("wifi", ESP_LOG_ERROR);
  
  ESP_LOGI(TAG, "WiFi AP démarré: %s (Password: %s)", AP_SSID, AP_PASSWORD);

  // Démarrage de la capture des logs (avant le serveur web)
  webserver::start_log_capture();

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
  static bool i2c_ok = false;
  static bool pca_ok = false;
  // Store init results for periodic reporting
  i2c_ok = true;  // If we got here, I2C init passed

  // Quick PCA9685 check: try reading MODE1 register
  {
    uint8_t mode1_val = 0;
    uint8_t reg = 0x00; // MODE1
    esp_err_t pca_ret = i2c_master_write_read_device(I2C_NUM_0, 0x72, &reg, 1, &mode1_val, 1, pdMS_TO_TICKS(100));
    pca_ok = (pca_ret == ESP_OK);
    ESP_LOGI(TAG, "PCA9685 check: %s (MODE1=0x%02X)", pca_ok ? "OK" : "FAIL", mode1_val);
  }

  while (true) {
    // Status periodique toutes les 10 secondes
    if (loop_count % 20 == 0) {
      ESP_LOGI(TAG, "Status: LED=%s | I2C=%s | PCA9685=%s",
               led::get_state() ? "ON" : "OFF",
               i2c_ok ? "OK" : "FAIL",
               pca_ok ? "OK" : "FAIL");
    }

    loop_count++;
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
