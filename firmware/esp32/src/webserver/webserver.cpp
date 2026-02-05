#include "webserver.h"

#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_littlefs.h"
#include "led.h"
#include "pca9685.h"
#include "mpu9250.h"
#include "littlefs_helper.h"
#include "ik_page.h"
#include "cJSON.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>

namespace webserver {
static const char* TAG = "WebServer";
static httpd_handle_t s_server = nullptr;

/**
 * Récupère le type MIME pour une extension de fichier
 */
static const char* get_mime_type(const char* filepath) {
    if (strstr(filepath, ".html")) return "text/html";
    if (strstr(filepath, ".css")) return "text/css";
    if (strstr(filepath, ".js")) return "application/javascript";
    if (strstr(filepath, ".json")) return "application/json";
    if (strstr(filepath, ".png")) return "image/png";
    if (strstr(filepath, ".jpg") || strstr(filepath, ".jpeg")) return "image/jpeg";
    if (strstr(filepath, ".svg")) return "image/svg+xml";
    return "text/plain";
}

/**
 * Sert un fichier depuis littleFS ou fallback à ik_page.h
 */
static esp_err_t serve_file_from_littlefs(httpd_req_t *req, const char* filepath) {
    char* content = littlefs::read_file(filepath);
    
    if (!content) {
        ESP_LOGW(TAG, "File not found in littleFS: %s (size: %zu)", filepath, strlen(filepath));
        
        // Fallback: Si c'est la racine ET pas sur littleFS, utiliser ik_page.h compilée
        if (strcmp(req->uri, "/") == 0) {
            ESP_LOGI(TAG, "Fallback to embedded HTML_IK_PAGE");
            httpd_resp_set_type(req, "text/html");
            httpd_resp_send(req, HTML_IK_PAGE, HTTPD_RESP_USE_STRLEN);
            return ESP_OK;
        }
        
        httpd_resp_set_status(req, "404 Not Found");
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_send(req, "File not found", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }
    
    httpd_resp_set_type(req, get_mime_type(filepath));
    httpd_resp_send(req, content, strlen(content));
    free(content);
    return ESP_OK;
}

static esp_err_t ik_home_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET / - Client: %s", req->uri);
    return serve_file_from_littlefs(req, "/littlefs/index.html");
}

// Handler pour l'API de commandes
static esp_err_t api_cmd_handler(httpd_req_t *req)
{
    char query[100];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        ESP_LOGI(TAG, "API query: %s", query);
        
        char action[32];
        if (httpd_query_key_value(query, "action", action, sizeof(action)) == ESP_OK) {
            ESP_LOGI(TAG, "Action: %s", action);
            
            // Traitement des commandes
            if (strcmp(action, "led_on") == 0) {
                led::on();
            } else if (strcmp(action, "led_off") == 0) {
                led::off();
            } else if (strcmp(action, "led_toggle") == 0) {
                led::toggle();
            } else if (strcmp(action, "servo") == 0) {
                char ch_str[8] = {0};
                char angle_str[8] = {0};
                if (httpd_query_key_value(query, "ch", ch_str, sizeof(ch_str)) == ESP_OK &&
                    httpd_query_key_value(query, "angle", angle_str, sizeof(angle_str)) == ESP_OK) {
                    int ch = atoi(ch_str);
                    int angle = atoi(angle_str);
                    if (ch >= 0 && ch <= 15 && angle >= 0 && angle <= 180) {
                        pca9685::set_servo_angle((uint8_t)ch, (uint8_t)angle);
                    }
                }
            }
            
            // Réponse JSON
            char response[200];
            snprintf(response, sizeof(response),
                     "{\"status\":\"ok\",\"action\":\"%s\",\"led_state\":%s,\"message\":\"Commande exécutée\"}",
                     action, led::get_state() ? "true" : "false");
            
            httpd_resp_set_type(req, "application/json");
            httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
            return ESP_OK;
        }
    }
    
    // Erreur
    const char* error = "{\"status\":\"error\",\"message\":\"Paramètre manquant\"}";
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, error, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler pour la page de status JSON
static esp_err_t status_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /status");
    
    const char* json = R"({
        "robot": "Bittle",
        "platform": "ESP32-C6",
        "wifi_mode": "AP",
        "ip": "192.168.4.1",
        "uptime_ms": %llu,
        "free_heap": %u,
        "led_state": %s
    })";
    
    char response[300];
    snprintf(response, sizeof(response), json,
             esp_timer_get_time() / 1000,
             esp_get_free_heap_size(),
             led::get_state() ? "true" : "false");
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler pour les données IMU
static esp_err_t imu_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/imu");
    
    // Lecture des données MPU-9250
    mpu9250::AccelData accel = mpu9250::read_accel();
    mpu9250::GyroData gyro = mpu9250::read_gyro();
    mpu9250::EulerAngles angles = mpu9250::calculate_angles();
    
    char json_buffer[512];
    snprintf(json_buffer, sizeof(json_buffer),
             "{\"accel\":{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f},"
             "\"gyro\":{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f},"
             "\"angles\":{\"pitch\":%.2f,\"roll\":%.2f,\"yaw\":%.2f},"
             "\"status\":\"ok\"}",
             accel.x, accel.y, accel.z,
             gyro.x, gyro.y, gyro.z,
             angles.pitch, angles.roll, angles.yaw);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_buffer, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler pour la commandes IK Servo POST
static esp_err_t servos_handler(httpd_req_t *req)
{
    // Vérifier que c'est un POST
    if (req->method != HTTP_POST) {
        const char* error = "{\"status\":\"error\",\"message\":\"Method not allowed\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, error, HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    ESP_LOGI(TAG, "POST /api/servos");

    // Lire le body JSON
    char buffer[2048] = {0};
    int recv = httpd_req_recv(req, buffer, sizeof(buffer) - 1);
    
    if (recv <= 0) {
        const char* error = "{\"status\":\"error\",\"message\":\"Body vide\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, error, HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    buffer[recv] = '\0';
    ESP_LOGI(TAG, "Servo command: %s", buffer);

    // Parser le JSON
    cJSON *root = cJSON_Parse(buffer);
    if (!root) {
        const char* error = "{\"status\":\"error\",\"message\":\"JSON invalid\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, error, HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    // Extraire le tableau 'servos'
    cJSON *servos_array = cJSON_GetObjectItem(root, "servos");
    if (!servos_array || !cJSON_IsArray(servos_array)) {
        cJSON_Delete(root);
        const char* error = "{\"status\":\"error\",\"message\":\"No servos array\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, error, HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    // Appliquer les angles à chaque servo
    int servo_count = 0;
    cJSON *servo_item = NULL;
    cJSON_ArrayForEach(servo_item, servos_array) {
        // Accept both 'ch' (legacy) and 'pcaChannel' (current client)
        cJSON *ch_item = cJSON_GetObjectItem(servo_item, "pcaChannel");
        if (!ch_item) ch_item = cJSON_GetObjectItem(servo_item, "ch");
        cJSON *deg_item = cJSON_GetObjectItem(servo_item, "deg");
        cJSON *pulse_item = cJSON_GetObjectItem(servo_item, "pulse_us");
        
        if (ch_item && deg_item) {
            uint8_t channel = 0;
            uint8_t angle = 0;

            // Parser le channel (peut être int ou double)
            if (ch_item->type == cJSON_Number) {
                channel = (uint8_t)ch_item->valueint;
                ESP_LOGD(TAG, "Parsed channel: %d", channel);
            } else {
                ESP_LOGW(TAG, "Channel is not a number (type=%d)", ch_item->type);
                continue;
            }
            
            // Parser l'angle (peut être double)
            if (deg_item->type == cJSON_Number) {
                angle = (uint8_t)round(deg_item->valuedouble);
                ESP_LOGD(TAG, "Parsed angle: %.2f -> %d", deg_item->valuedouble, angle);
            } else {
                ESP_LOGW(TAG, "Angle is not a number (type=%d)", deg_item->type);
                continue;
            }
            
            // If client provided pulse_us, use it (calibration), otherwise use angle
            if (pulse_item && pulse_item->type == cJSON_Number) {
                uint16_t pulse = (uint16_t)round(pulse_item->valuedouble);
                ESP_LOGI(TAG, "Setting Servo[%d] pulse = %d us", channel, pulse);
                pca9685::set_servo_pulse_us(channel, pulse);
                ESP_LOGI(TAG, "Applied Servo[%d] pulse = %d us", channel, pulse);
                servo_count++;
            } else {
                // Vérifier les limites pour angle
                if (channel < 16 && angle <= 180) {
                    ESP_LOGI(TAG, "Setting Servo[%d] = %d°", channel, angle);
                    pca9685::set_servo_angle(channel, angle);
                    ESP_LOGI(TAG, "Applied Servo[%d] = %d°", channel, angle);
                    servo_count++;
                } else {
                    ESP_LOGW(TAG, "Invalid servo params: ch=%d, angle=%d", channel, angle);
                }
            }
        } else {
            ESP_LOGW(TAG, "Missing ch/pcaChannel or deg in servo item");
        }
    }

    cJSON_Delete(root);

    // Répondre avec succès
    char response[256];
    snprintf(response, sizeof(response),
             "{\"status\":\"ok\",\"message\":\"Applied %d servos\",\"count\":%d}",
             servo_count, servo_count);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler pour la page IK complète
static esp_err_t ik_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /ik - Serving IK Control Page");
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, HTML_IK_PAGE, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler générique pour les fichiers statiques (CSS, JS, etc)
static esp_err_t static_file_handler(httpd_req_t *req)
{
    ESP_LOGD(TAG, "GET %s", req->uri);
    
    // Construire le chemin complet dans littleFS
    char filepath[1024];  // Amplement suffisant pour "/littlefs" + URI
    int len = snprintf(filepath, sizeof(filepath), "/littlefs%s", req->uri);
    
    // Vérifier le débordement
    if (len < 0 || len >= (int)sizeof(filepath)) {
        ESP_LOGW(TAG, "URI too long: %s", req->uri);
        httpd_resp_set_status(req, "414 URI Too Long");
        httpd_resp_send(req, "URI too long", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }
    
    return serve_file_from_littlefs(req, filepath);
}

bool start(int port)
{
    // Initialiser littleFS
    if (!littlefs::init()) {
        ESP_LOGW(TAG, "littleFS initialization failed - will use embedded HTML");
    }
    
    ESP_LOGI(TAG, "Démarrage serveur web sur port %d...", port);
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    config.max_uri_handlers = 16;  // Augmenté pour les fichiers statiques
    config.stack_size = 8192;
    
    if (httpd_start(&s_server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Échec démarrage serveur HTTP");
        return false;
    }
    
    // Enregistrement des handlers
    httpd_uri_t home_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = ik_home_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(s_server, &home_uri);
    
    // Handler générique pour les fichiers statiques
    httpd_uri_t static_uri = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = static_file_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(s_server, &static_uri);
    
    httpd_uri_t api_cmd_uri = {
        .uri = "/api/cmd",
        .method = HTTP_GET,
        .handler = api_cmd_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(s_server, &api_cmd_uri);
    
    httpd_uri_t status_uri = {
        .uri = "/status",
        .method = HTTP_GET,
        .handler = status_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(s_server, &status_uri);
    
    httpd_uri_t imu_uri = {
        .uri = "/api/imu",
        .method = HTTP_GET,
        .handler = imu_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(s_server, &imu_uri);

    httpd_uri_t servos_uri = {
        .uri = "/api/servos",
        .method = HTTP_POST,
        .handler = servos_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(s_server, &servos_uri);

    httpd_uri_t ik_uri = {
        .uri = "/ik",
        .method = HTTP_GET,
        .handler = ik_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(s_server, &ik_uri);
    
    ESP_LOGI(TAG, "✓ Serveur web démarré sur http://192.168.4.1:%d", port);
    return true;
}

void stop()
{
    if (s_server != nullptr) {
        ESP_LOGI(TAG, "Arrêt serveur web...");
        httpd_stop(s_server);
        s_server = nullptr;
    }
}

bool is_running()
{
    return s_server != nullptr;
}

} // namespace webserver

