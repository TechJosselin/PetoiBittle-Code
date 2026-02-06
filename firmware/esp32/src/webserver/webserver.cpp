#include "webserver.h"
#include "web_content.h"
#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <cstring>
#include <cstdio>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "led.h"
#include "pca9685.h"

static const char* TAG = "WebServer";

// Instance du serveur HTTP
static httpd_handle_t server = nullptr;
static bool is_running = false;

// ── Log ring buffer ──
static constexpr int LOG_MAX_LINES = 200;
static constexpr int LOG_MAX_LINE_LEN = 200;
static char log_lines[LOG_MAX_LINES][LOG_MAX_LINE_LEN];
static uint32_t log_write_idx = 0;  // total lines written (monotonic)
static SemaphoreHandle_t log_mutex = nullptr;
static vprintf_like_t original_vprintf = nullptr;

static int custom_log_vprintf(const char* fmt, va_list args) {
    // Forward to original (serial)
    int ret = original_vprintf(fmt, args);

    // Format into temp buffer
    char tmp[LOG_MAX_LINE_LEN];
    vsnprintf(tmp, sizeof(tmp), fmt, args);

    // Strip trailing newlines
    int len = strlen(tmp);
    while (len > 0 && (tmp[len-1] == '\n' || tmp[len-1] == '\r')) tmp[--len] = '\0';
    if (len == 0) return ret;

    // Store in ring buffer
    if (log_mutex && xSemaphoreTake(log_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        strncpy(log_lines[log_write_idx % LOG_MAX_LINES], tmp, LOG_MAX_LINE_LEN - 1);
        log_lines[log_write_idx % LOG_MAX_LINES][LOG_MAX_LINE_LEN - 1] = '\0';
        log_write_idx++;
        xSemaphoreGive(log_mutex);
    }
    return ret;
}

// État global du robot
static struct {
    bool led_state;
    uint16_t servo_angles[8];  // 8 servos (4 pattes x 2 servos)
    bool init_mode;            // Mode initialisation (servos verrouillés)
} robot_state = {
    .led_state = false,
    .servo_angles = {90, 90, 90, 90, 90, 90, 90, 90},
    .init_mode = false
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
 * Lit le body HTTP complet (boucle sur httpd_req_recv)
 */
static int recv_full_body(httpd_req_t *req, char *buf, int buf_size) {
    int total = 0;
    int remaining = req->content_len;
    if (remaining <= 0 || remaining >= buf_size) {
        ESP_LOGW(TAG, "Body size invalid: %d (buf=%d)", remaining, buf_size);
        return -1;
    }
    while (remaining > 0) {
        int ret = httpd_req_recv(req, buf + total, remaining);
        if (ret <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) continue; // retry on timeout
            ESP_LOGE(TAG, "recv error: %d", ret);
            return -1;
        }
        total += ret;
        remaining -= ret;
    }
    buf[total] = '\0';
    return total;
}

/**
 * Handler POST /api/servos - Contrôle multiple de servos (format frontend IK)
 * Payload: {"servos":[{"pcaChannel":0,"deg":90}, ...]}
 */
static esp_err_t handle_api_servos(httpd_req_t *req) {
    // Buffer statique pour ne pas exploser la stack
    static char content[1024];

    int ret = recv_full_body(req, content, sizeof(content));
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Empty or too large body");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Servos recv %d bytes (content_len=%d)", ret, (int)req->content_len);

    // Parcourir chaque occurrence de pcaChannel / deg dans le tableau servos
    int applied = 0;
    int parsed = 0;
    char* cursor = content;
    while ((cursor = strstr(cursor, "\"pcaChannel\":")) != nullptr) {
        int channel = -1;
        sscanf(cursor, "\"pcaChannel\":%d", &channel);

        // Chercher le "deg" associé (juste après dans le même objet)
        char* deg_str = strstr(cursor, "\"deg\":");
        parsed++;
        if (deg_str) {
            float deg_f = -1;
            sscanf(deg_str, "\"deg\":%f", &deg_f);
            int deg = (int)(deg_f + 0.5f);

            ESP_LOGI(TAG, "Parse[%d]: ch=%d deg_f=%.1f deg=%d", parsed, channel, deg_f, deg);

            if (channel >= 0 && channel < 16 && deg >= 0 && deg <= 180) {
                pca9685::set_servo_angle(channel, (uint8_t)deg);
                if (channel < 8) robot_state.servo_angles[channel] = deg;
                applied++;
            } else {
                ESP_LOGW(TAG, "Rejected: ch=%d deg=%d", channel, deg);
            }
        }
        cursor++; // avancer pour trouver le prochain
    }

    ESP_LOGI(TAG, "Servos applied=%d parsed=%d", applied, parsed);

    char response[128];
    snprintf(response, sizeof(response),
        "{\"status\":\"ok\",\"applied\":%d,\"parsed\":%d,\"bytes\":%d}",
        applied, parsed, ret);
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
    config.stack_size = 8192;  // Plus de stack pour I2C + parsing dans les handlers

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

        httpd_uri_t uri_api_servos = {
            .uri = "/api/servos",
            .method = HTTP_POST,
            .handler = handle_api_servos,
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server, &uri_api_servos);

        // POST /api/init-mode - Verrouille ou libère les servos
        httpd_uri_t uri_api_init_mode = {
            .uri = "/api/init-mode",
            .method = HTTP_POST,
            .handler = [](httpd_req_t *req) -> esp_err_t {
                char content[64];
                int ret = httpd_req_recv(req, content, sizeof(content) - 1);
                if (ret <= 0) {
                    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Empty body");
                    return ESP_FAIL;
                }
                content[ret] = '\0';

                bool enabled = (strstr(content, "true") != nullptr);
                robot_state.init_mode = enabled;

                if (enabled) {
                    // Verrouiller: envoyer les angles actuels pour que les servos tiennent
                    ESP_LOGI(TAG, "Init mode ON: verrouillage servos");
                    for (int i = 0; i < 8; i++) {
                        pca9685::set_servo_angle(i, (uint8_t)robot_state.servo_angles[i]);
                    }
                } else {
                    // Libérer: couper le PWM pour que les servos soient libres
                    ESP_LOGI(TAG, "Init mode OFF: servos libérés");
                    pca9685::all_off();
                }

                // Réponse avec les angles actuels
                char resp[256];
                int n = snprintf(resp, sizeof(resp),
                    "{\"status\":\"ok\",\"initMode\":%s,\"angles\":[",
                    enabled ? "true" : "false");
                for (int i = 0; i < 8; i++) {
                    n += snprintf(resp + n, sizeof(resp) - n, "%s%d",
                        i ? "," : "", robot_state.servo_angles[i]);
                }
                snprintf(resp + n, sizeof(resp) - n, "]}");
                httpd_resp_set_type(req, "application/json");
                httpd_resp_send(req, resp, strlen(resp));
                return ESP_OK;
            },
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server, &uri_api_init_mode);

        // GET /api/logs?since=N - retourne les logs depuis l'index N
        httpd_uri_t uri_api_logs = {
            .uri = "/api/logs",
            .method = HTTP_GET,
            .handler = [](httpd_req_t *req) -> esp_err_t {
                // Parse ?since=N
                char query[32] = {};
                uint32_t since = 0;
                if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
                    char val[16];
                    if (httpd_query_key_value(query, "since", val, sizeof(val)) == ESP_OK)
                        since = (uint32_t)atoi(val);
                }

                httpd_resp_set_type(req, "application/json");
                httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

                // Build JSON response
                // Start with {"idx":N,"lines":[
                char header[64];
                uint32_t current_idx;
                if (log_mutex && xSemaphoreTake(log_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                    current_idx = log_write_idx;
                    // Clamp 'since' to available range
                    uint32_t oldest = (current_idx > LOG_MAX_LINES) ? current_idx - LOG_MAX_LINES : 0;
                    if (since < oldest) since = oldest;

                    snprintf(header, sizeof(header), "{\"idx\":%lu,\"lines\":[", (unsigned long)current_idx);
                    httpd_resp_sendstr_chunk(req, header);

                    bool first = true;
                    for (uint32_t i = since; i < current_idx; i++) {
                        const char* line = log_lines[i % LOG_MAX_LINES];
                        if (!first) httpd_resp_sendstr_chunk(req, ",");
                        // JSON-escape the line (simple: escape " and \)
                        httpd_resp_sendstr_chunk(req, "\"");
                        for (const char* p = line; *p; p++) {
                            if (*p == '"') httpd_resp_sendstr_chunk(req, "\\\"");
                            else if (*p == '\\') httpd_resp_sendstr_chunk(req, "\\\\");
                            else {
                                char c[2] = {*p, 0};
                                httpd_resp_sendstr_chunk(req, c);
                            }
                        }
                        httpd_resp_sendstr_chunk(req, "\"");
                        first = false;
                    }
                    xSemaphoreGive(log_mutex);
                } else {
                    snprintf(header, sizeof(header), "{\"idx\":0,\"lines\":[");
                    httpd_resp_sendstr_chunk(req, header);
                }

                httpd_resp_sendstr_chunk(req, "]}");
                httpd_resp_sendstr_chunk(req, nullptr);  // finish chunked response
                return ESP_OK;
            },
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server, &uri_api_logs);

        // Test endpoint: GET /api/test-servo?ch=0&deg=90
        httpd_uri_t uri_test_servo = {
            .uri = "/api/test-servo",
            .method = HTTP_GET,
            .handler = [](httpd_req_t *req) -> esp_err_t {
                // Parse query string
                char query[64] = {};
                httpd_req_get_url_query_str(req, query, sizeof(query));
                int ch = 0, deg = 90;
                char val[8];
                if (httpd_query_key_value(query, "ch", val, sizeof(val)) == ESP_OK) ch = atoi(val);
                if (httpd_query_key_value(query, "deg", val, sizeof(val)) == ESP_OK) deg = atoi(val);

                if (ch >= 0 && ch < 16 && deg >= 0 && deg <= 180) {
                    pca9685::set_servo_angle(ch, (uint8_t)deg);
                    ESP_LOGI(TAG, "TEST servo %d -> %d", ch, deg);
                }

                char resp[128];
                snprintf(resp, sizeof(resp),
                    "{\"test\":true,\"channel\":%d,\"angle\":%d}", ch, deg);
                httpd_resp_set_type(req, "application/json");
                httpd_resp_send(req, resp, strlen(resp));
                return ESP_OK;
            },
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server, &uri_test_servo);

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

void start_log_capture() {
    if (!log_mutex) {
        log_mutex = xSemaphoreCreateMutex();
    }
    original_vprintf = esp_log_set_vprintf(custom_log_vprintf);
    ESP_LOGI(TAG, "Log capture démarrée (buffer: %d lignes)", LOG_MAX_LINES);
}

} // namespace webserver
