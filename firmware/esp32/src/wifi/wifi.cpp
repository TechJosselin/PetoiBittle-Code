#include "wifi.h"

#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
//ZIZI
namespace wifi {
//ZIZI
static const char* TAG = "WiFi-AP";

// Variables globales du module
static bool s_ap_running = false;
static esp_netif_t* s_ap_netif = nullptr;
static char s_ap_ip[16] = "0.0.0.0";
static uint8_t s_client_count = 0;

/**
 * @brief Gestionnaire d'événements WiFi AP
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "✓ Client connecté - MAC: " MACSTR " | AID: %d",
                 MAC2STR(event->mac), event->aid);
        s_client_count++;
        
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "✗ Client déconnecté - MAC: " MACSTR " | AID: %d",
                 MAC2STR(event->mac), event->aid);
        if (s_client_count > 0) {
            s_client_count--;
        }
        
    } else if (event_id == WIFI_EVENT_AP_START) {
        ESP_LOGI(TAG, "✓ Access Point démarré");
        s_ap_running = true;
        
    } else if (event_id == WIFI_EVENT_AP_STOP) {
        ESP_LOGI(TAG, "✗ Access Point arrêté");
        s_ap_running = false;
        s_client_count = 0;
    }
}

bool start_ap(const ApConfig& config)
{
    ESP_LOGI(TAG, "=== Démarrage WiFi Access Point ===");
    
    // Validation configuration
    if (config.ssid == nullptr || strlen(config.ssid) == 0) {
        ESP_LOGE(TAG, "SSID invalide");
        return false;
    }
    
    if (config.password != nullptr && strlen(config.password) > 0 && strlen(config.password) < 8) {
        ESP_LOGE(TAG, "Mot de passe trop court (min 8 caractères)");
        return false;
    }
    
    // Initialisation NVS (requis pour WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS corrompue, effacement...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialisation TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());
    
    // Création event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Création interface WiFi AP
    s_ap_netif = esp_netif_create_default_wifi_ap();
    if (s_ap_netif == nullptr) {
        ESP_LOGE(TAG, "Échec création netif AP");
        return false;
    }
    
    // Configuration IP statique de l'AP
    esp_netif_dhcps_stop(s_ap_netif);
    
    esp_netif_ip_info_t ip_info;
    memset(&ip_info, 0, sizeof(ip_info));
    
    ip_info.ip.addr = esp_ip4addr_aton(config.ip_addr);
    ip_info.gw.addr = esp_ip4addr_aton(config.gateway);
    ip_info.netmask.addr = esp_ip4addr_aton(config.netmask);
    
    ESP_ERROR_CHECK(esp_netif_set_ip_info(s_ap_netif, &ip_info));
    ESP_ERROR_CHECK(esp_netif_dhcps_start(s_ap_netif));
    
    strncpy(s_ap_ip, config.ip_addr, sizeof(s_ap_ip) - 1);
    
    // Configuration WiFi par défaut
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // Enregistrement des handlers d'événements
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &wifi_event_handler,
        nullptr,
        nullptr));
    
    // Configuration WiFi AP
    wifi_config_t wifi_config = {};
    
    strncpy((char*)wifi_config.ap.ssid, config.ssid, sizeof(wifi_config.ap.ssid) - 1);
    wifi_config.ap.ssid_len = strlen(config.ssid);
    wifi_config.ap.channel = config.channel;
    wifi_config.ap.max_connection = config.max_connections;
    wifi_config.ap.beacon_interval = 100;
    
    // Sécurité
    if (config.password != nullptr && strlen(config.password) >= 8) {
        strncpy((char*)wifi_config.ap.password, config.password, sizeof(wifi_config.ap.password) - 1);
        wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
        ESP_LOGI(TAG, "Mode: WPA2-PSK (réseau sécurisé)");
    } else {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        ESP_LOGW(TAG, "Mode: OUVERT (pas de mot de passe)");
    }
    
    wifi_config.ap.pmf_cfg.required = false;
    
    // Démarrage WiFi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "✓ AP configuré");
    ESP_LOGI(TAG, "  SSID: %s", config.ssid);
    ESP_LOGI(TAG, "  IP: %s", config.ip_addr);
    ESP_LOGI(TAG, "  Canal: %d", config.channel);
    ESP_LOGI(TAG, "  Max clients: %d", config.max_connections);
    
    return true;
}

void stop_ap()
{
    if (!s_ap_running) {
        ESP_LOGW(TAG, "AP déjà arrêté");
        return;
    }
    
    ESP_LOGI(TAG, "Arrêt Access Point...");
    esp_wifi_stop();
    s_ap_running = false;
    s_client_count = 0;
}

bool is_ap_running()
{
    return s_ap_running;
}

bool get_ap_ip(char* ip_str, size_t len)
{
    if (!s_ap_running || len < 16) {
        return false;
    }
    
    strncpy(ip_str, s_ap_ip, len - 1);
    ip_str[len - 1] = '\0';
    return true;
}

uint8_t get_client_count()
{
    return s_client_count;
}

uint8_t get_clients_info(void* clients_info, uint8_t max_clients)
{
    if (!s_ap_running || clients_info == nullptr || max_clients == 0) {
        return 0;
    }
    
    wifi_sta_list_t sta_list;
    ESP_ERROR_CHECK(esp_wifi_ap_get_sta_list(&sta_list));
    
    uint8_t count = (sta_list.num < max_clients) ? sta_list.num : max_clients;
    memcpy(clients_info, &sta_list, sizeof(wifi_sta_list_t));
    
    return count;
}

} // namespace wifi
