#include "webserver.h"
#include "web_content.h"
#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <cstring>
#include <cstdio>
#include "led.h"
#include "pca9685.h"

static const char* TAG = "WebServer";

// Instance du serveur HTTP
static httpd_handle_t server = nullptr;
static bool is_running = false;

// État global du robot
static struct {
    bool led_state;
    uint16_t servo_angles[8];  // 8 servos (4 pattes x 2 servos)
} robot_state = {
    .led_state = false,
    .servo_angles = {90, 90, 90, 90, 90, 90, 90, 90}
};

/**
 * Handler GET / - retourne index.html
 */
static esp_err_t handle_root(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_index, strlen(html_index));
    return ESP_OK;
}

/**
 * Handler GET /styles.css
 */
static esp_err_t handle_styles(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, css_styles, strlen(css_styles));
    return ESP_OK;
}

/**
 * Handler GET /js/config.js
 */
static esp_err_t handle_js_config(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/javascript");
    httpd_resp_send(req, js_config, strlen(js_config));
    return ESP_OK;
}

/**
 * Handler GET /js/ik.js
 */
static esp_err_t handle_js_ik(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/javascript");
    httpd_resp_send(req, js_ik, strlen(js_ik));
    return ESP_OK;
}

/**
 * Handler GET /js/net.js
 */
static esp_err_t handle_js_net(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/javascript");
    httpd_resp_send(req, js_net, strlen(js_net));
    return ESP_OK;
}

/**
 * Handler GET /js/ui.js
 */
static esp_err_t handle_js_ui(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/javascript");
    httpd_resp_send(req, js_ui, strlen(js_ui));
    return ESP_OK;
}

/**
 * Handler GET /api/status - Récupère l'état du robot
 */
static esp_err_t handle_api_status(httpd_req_t *req) {
    char response[256];
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_AP, mac);
    
    snprintf(response, sizeof(response),
        "{\"led\":%d,\"ip\":\"192.168.4.1\",\"version\":\"1.0.0\",\"mac\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\"battery\":7.4}",
        robot_state.led_state ? 1 : 0,
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
    );
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

/**
 * Handler POST /api/command - Envoie une commande
 */
static esp_err_t handle_api_command(httpd_req_t *req) {
    char content[256];
    int ret = httpd_req_recv(req, content, sizeof(content) - 1);
    
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    
    content[ret] = '\0';
    ESP_LOGI(TAG, "Commande reçue: %s", content);
    
    // Parse JSON simple
    if (strstr(content, "\"command\":\"calibrate\"")) {
        ESP_LOGI(TAG, "Calibration command");
        // Centrer tous les servos
        for (int i = 0; i < 8; i++) {
            pca9685::set_servo_angle(i, 90);
            robot_state.servo_angles[i] = 90;
        }
    }
    else if (strstr(content, "\"command\":\"center\"")) {
        ESP_LOGI(TAG, "Center all command");
        for (int i = 0; i < 8; i++) {
            pca9685::set_servo_angle(i, 90);
            robot_state.servo_angles[i] = 90;
        }
    }
    else if (strstr(content, "\"command\":\"stop\"")) {
        ESP_LOGI(TAG, "Emergency stop");
        // Désactiver tous les servos
        for (int i = 0; i < 8; i++) {
            pca9685::set_servo_angle(i, 90);
        }
    }
    
    // Réponse JSON
    const char* response = "{\"status\":\"ok\"}";
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

/**
 * Handler POST /api/servo - Contrôle direct d'un servo
 */
static esp_err_t handle_api_servo(httpd_req_t *req) {
    char content[128];
    int ret = httpd_req_recv(req, content, sizeof(content) - 1);
    
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    
    content[ret] = '\0';
    
    // Parse JSON simple pour extraire channel et angle
    int channel = -1;
    int angle = -1;
    
    char* channel_str = strstr(content, "\"channel\":");
    if (channel_str) {
        sscanf(channel_str, "\"channel\":%d", &channel);
    }
    
    char* angle_str = strstr(content, "\"angle\":");
    if (angle_str) {
        sscanf(angle_str, "\"angle\":%d", &angle);
    }
    
    if (channel >= 0 && channel < 8 && angle >= 0 && angle <= 180) {
        pca9685::set_servo_angle(channel, angle);
        robot_state.servo_angles[channel] = angle;
        ESP_LOGI(TAG, "Servo %d défini à %d°", channel, angle);
        
        // Réponse JSON
        const char* response = "{\"status\":\"ok\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, response, strlen(response));
        return ESP_OK;
    }
    
    // Erreur
    const char* response = "{\"status\":\"error\",\"message\":\"Invalid parameters\"}";
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_FAIL;
}

namespace webserver {

bool init(uint16_t port) {
    if (is_running) {
        ESP_LOGW(TAG, "Serveur déjà démarré");
        return true;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    config.max_uri_handlers = 16;
    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Démarrage du serveur HTTP sur le port %d", port);

    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Serveur HTTP démarré avec succès");

        // Enregistrement des routes HTML/CSS/JS
        httpd_uri_t uri_root = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = handle_root,
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server, &uri_root);

        httpd_uri_t uri_styles = {
            .uri = "/styles.css",
            .method = HTTP_GET,
            .handler = handle_styles,
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server, &uri_styles);

        httpd_uri_t uri_js_config = {
            .uri = "/js/config.js",
            .method = HTTP_GET,
            .handler = handle_js_config,
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server, &uri_js_config);

        httpd_uri_t uri_js_ik = {
            .uri = "/js/ik.js",
            .method = HTTP_GET,
            .handler = handle_js_ik,
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server, &uri_js_ik);

        httpd_uri_t uri_js_net = {
            .uri = "/js/net.js",
            .method = HTTP_GET,
            .handler = handle_js_net,
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server, &uri_js_net);

        httpd_uri_t uri_js_ui = {
            .uri = "/js/ui.js",
            .method = HTTP_GET,
            .handler = handle_js_ui,
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server, &uri_js_ui);

        // Enregistrement des routes API
        httpd_uri_t uri_api_status = {
            .uri = "/api/status",
            .method = HTTP_GET,
            .handler = handle_api_status,
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server, &uri_api_status);

        httpd_uri_t uri_api_command = {
            .uri = "/api/command",
            .method = HTTP_POST,
            .handler = handle_api_command,
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server, &uri_api_command);

        httpd_uri_t uri_api_servo = {
            .uri = "/api/servo",
            .method = HTTP_POST,
            .handler = handle_api_servo,
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server, &uri_api_servo);

        is_running = true;
        ESP_LOGI(TAG, "Toutes les routes ont été enregistrées");
        return true;
    }

    ESP_LOGE(TAG, "Échec du démarrage du serveur HTTP");
    return false;
}

void stop() {
    if (server && is_running) {
        httpd_stop(server);
        server = nullptr;
        is_running = false;
        ESP_LOGI(TAG, "Serveur HTTP arrêté");
    }
}

bool get_ip(char* buffer, size_t size) {
    if (!buffer || size < 12) {
        return false;
    }
    snprintf(buffer, size, "192.168.4.1");
    return true;
}

} // namespace webserver
