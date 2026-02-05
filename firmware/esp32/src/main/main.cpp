#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "wifi.h"
#include "webserver.h"
#include "led.h"
#include "oled.h"
#include "pca9685.h"
#include "mpu9250.h"

// LED integree du Seeed Xiao ESP32-C6
static constexpr gpio_num_t LED_GPIO = GPIO_NUM_15;

// Configuration I2C pour OLED, MPU-9250 et PCA9685
static constexpr int OLED_SCL_GPIO = 23;  // GPIO23 - I2C Clock
static constexpr int OLED_SDA_GPIO = 22;  // GPIO22 - I2C Data

static const char* TAG = "Main";

// Configuration WiFi Access Point
#define AP_SSID           "Bittle-Robot"     // Nom du reseau WiFi
#define AP_PASSWORD       "bittle123"        // Mot de passe (min 8 caracteres)
#define AP_CHANNEL        1                  // Canal WiFi (1-13)
#define AP_MAX_CLIENTS    4                  // Nombre maximum de clients
#define AP_IP             "192.168.4.1"      // Adresse IP du robot
#define AP_GATEWAY        "192.168.4.1"      // Passerelle
#define AP_NETMASK        "255.255.255.0"    // Masque de sous-reseau

extern "C" void app_main(void)
{
  ESP_LOGI(TAG, "=== PetoiBittle - Demarrage ===");
  
  // Configuration LED
  led::init(LED_GPIO);

  // Initialisation OLED
  if (!oled::init(OLED_SCL_GPIO, OLED_SDA_GPIO)) {
    ESP_LOGE(TAG, "Echec initialisation OLED");
  } else {
    oled::clear();
    oled::print("Hello World!", 20, 3);
    oled::print("Bittle Robot", 18, 4);
    oled::update();
  }

  // Initialisation MPU-9250 (sur le meme I2C que OLED)
  if (!mpu9250::init(OLED_SDA_GPIO, OLED_SCL_GPIO, 0x69)) {
    ESP_LOGE(TAG, "Echec initialisation MPU-9250");
  } else {
    ESP_LOGI(TAG, "MPU-9250 initialise");
  }

  // Initialisation PCA9685 (controleur servo) avec adresse 0x72
  if (!pca9685::init(0, 50, 0x72)) {
    ESP_LOGE(TAG, "Echec initialisation PCA9685 (le WiFi continuera)");
  } else {
    // Position initiale du servo 0 a 90 degres
    pca9685::set_servo_angle(0, 90);
  }

  // Configuration WiFi Access Point
  wifi::ApConfig ap_config = {
    .ssid = AP_SSID,
    .password = AP_PASSWORD,
    .channel = AP_CHANNEL,
    .max_connections = AP_MAX_CLIENTS,
    .ip_addr = AP_IP,
    .gateway = AP_GATEWAY,
    .netmask = AP_NETMASK
  };

  // Demarrage Access Point
  if (!wifi::start_ap(ap_config)) {
    ESP_LOGE(TAG, "Echec demarrage Access Point");
    return;
  }

  // Demarrage serveur web
  if (!webserver::start(80)) {
    ESP_LOGE(TAG, "Echec demarrage serveur web");
    return;
  }

  char ip[16];
  if (wifi::get_ap_ip(ip, sizeof(ip))) {
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "============================================");
    ESP_LOGI(TAG, "  BITTLE ROBOT - PRET");
    ESP_LOGI(TAG, "============================================");
    ESP_LOGI(TAG, "  Reseau: %s", AP_SSID);
    ESP_LOGI(TAG, "  Pass:   %s", AP_PASSWORD[0] ? AP_PASSWORD : "(ouvert)");
    ESP_LOGI(TAG, "  IP:     %s", ip);
    ESP_LOGI(TAG, "  Web:    http://%s", ip);
    ESP_LOGI(TAG, "============================================");
    ESP_LOGI(TAG, "");
  }

  // Boucle principale avec monitoring clients
  uint32_t loop_count = 0;

  while (true) {
    // Status periodique toutes les 10 secondes
    if (loop_count % 20 == 0) {
      ESP_LOGI(TAG, "LED %s | AP: %s | Clients: %d",
               led::get_state() ? "ON " : "OFF",
               wifi::is_ap_running() ? "actif" : "arrete",
               wifi::get_client_count());
      
      // Mise a jour OLED avec statut
      char status_line[32];
      snprintf(status_line, sizeof(status_line), "LED: %s", led::get_state() ? "ON " : "OFF");
      oled::clear();
      oled::print("Hello World!", 20, 2);
      oled::print("Bittle Robot", 18, 3);
      oled::print(status_line, 10, 5);
      snprintf(status_line, sizeof(status_line), "Clients: %d", wifi::get_client_count());
      oled::print(status_line, 10, 6);
      oled::update();
    }

    loop_count++;
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
