#include "webserver.h"

#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "led.h"
#include "pca9685.h"
#include "mpu9250.h"
#include "ik_page.h"
#include "cJSON.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
//ZIZI
namespace webserver {
//ZIZI
static const char* TAG = "WebServer";
static httpd_handle_t s_server = nullptr;

// Page HTML d'accueil
static const char* HTML_HOME = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Bittle Robot Control</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
        }
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            max-width: 600px;
            width: 100%;
            padding: 40px;
        }
        h1 {
            color: #667eea;
            font-size: 2.5em;
            margin-bottom: 10px;
            text-align: center;
        }
        .robot-emoji {
            font-size: 4em;
            text-align: center;
            margin: 20px 0;
        }
        .subtitle {
            color: #666;
            text-align: center;
            margin-bottom: 30px;
            font-size: 1.1em;
        }
        .info-box {
            background: #f7f9fc;
            border-left: 4px solid #667eea;
            padding: 20px;
            margin: 20px 0;
            border-radius: 8px;
        }
        .info-item {
            display: flex;
            justify-content: space-between;
            padding: 10px 0;
            border-bottom: 1px solid #e0e0e0;
        }
        .info-item:last-child { border-bottom: none; }
        .label { font-weight: 600; color: #333; }
        .value { color: #667eea; font-family: 'Courier New', monospace; }
        .status {
            display: inline-block;
            padding: 5px 15px;
            border-radius: 20px;
            background: #4ade80;
            color: white;
            font-weight: 600;
            font-size: 0.9em;
        }
        .control-section {
            margin-top: 30px;
            padding-top: 30px;
            border-top: 2px solid #f0f0f0;
        }
        .control-title {
            font-size: 1.5em;
            color: #333;
            margin-bottom: 20px;
            text-align: center;
        }
        .btn {
            width: 100%;
            padding: 15px;
            margin: 10px 0;
            border: none;
            border-radius: 10px;
            font-size: 1.1em;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
        }
        .btn-primary {
            background: #667eea;
            color: white;
        }
        .btn-primary:hover {
            background: #5568d3;
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }
        .btn-secondary {
            background: #f0f0f0;
            color: #333;
        }
        .btn-secondary:hover {
            background: #e0e0e0;
        }
        .input-row {
            display: flex;
            align-items: center;
            justify-content: space-between;
            gap: 10px;
            margin: 10px 0;
        }
        .input-row label {
            font-weight: 600;
            color: #333;
        }
        .input-row select,
        .input-row input {
            flex: 1;
            padding: 8px 10px;
            border: 1px solid #ddd;
            border-radius: 8px;
            font-size: 1em;
        }
        .slider {
            width: 100%;
            height: 8px;
            border-radius: 5px;
            background: #ddd;
            outline: none;
            -webkit-appearance: none;
            appearance: none;
        }
        .slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: #667eea;
            cursor: pointer;
            box-shadow: 0 2px 5px rgba(102, 126, 234, 0.4);
        }
        .slider::-moz-range-thumb {
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: #667eea;
            cursor: pointer;
            border: none;
            box-shadow: 0 2px 5px rgba(102, 126, 234, 0.4);
        }
        .slider-value {
            font-weight: 600;
            color: #667eea;
            font-size: 1.2em;
            min-width: 50px;
            text-align: center;
        }
        .footer {
            margin-top: 30px;
            text-align: center;
            color: #999;
            font-size: 0.9em;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="robot-emoji">🤖</div>
        <h1>Bittle Robot</h1>
        <p class="subtitle">Panneau de contrôle</p>
        
        <div class="info-box">
            <div class="info-item">
                <span class="label">État</span>
                <span class="status">● En ligne</span>
            </div>
            <div class="info-item">
                <span class="label">Réseau WiFi</span>
                <span class="value">Bittle-Robot</span>
            </div>
            <div class="info-item">
                <span class="label">Adresse IP</span>
                <span class="value">192.168.4.1</span>
            </div>
            <div class="info-item">
                <span class="label">Plateforme</span>
                <span class="value">ESP32-C6</span>
            </div>
        </div>

        <div class="control-section">
            <h2 class="control-title">LED</h2>
            <button class="btn btn-primary" onclick="sendCommand('led_on')">💡 LED ON</button>
            <button class="btn btn-primary" onclick="sendCommand('led_off')">🌑 LED OFF</button>
        </div>

        <div class="control-section">
            <h2 class="control-title">PWM / Servo (16 sorties)</h2>
            <div class="input-row">
                <label for="servoChannel">Canal (0-15)</label>
                <select id="servoChannel">
                    <option>0</option><option>1</option><option>2</option><option>3</option>
                    <option>4</option><option>5</option><option>6</option><option>7</option>
                    <option>8</option><option>9</option><option>10</option><option>11</option>
                    <option>12</option><option>13</option><option>14</option><option>15</option>
                </select>
            </div>
            <div class="input-row">
                <label for="servoAngle">Angle</label>
                <span class="slider-value"><span id="angleValue">90</span>°</span>
            </div>
            <input id="servoAngle" class="slider" type="range" min="0" max="180" value="90" oninput="updateServo()" />
        </div>

        <div class="control-section">
            <h2 class="control-title">📊 Capteurs IMU (MPU-9250)</h2>
            <div class="info-box">
                <div class="info-item">
                    <span class="label">Accélération X</span>
                    <span class="value"><span id="imu_accel_x">0.00</span> g</span>
                </div>
                <div class="info-item">
                    <span class="label">Accélération Y</span>
                    <span class="value"><span id="imu_accel_y">0.00</span> g</span>
                </div>
                <div class="info-item">
                    <span class="label">Accélération Z</span>
                    <span class="value"><span id="imu_accel_z">0.00</span> g</span>
                </div>
                <div class="info-item">
                    <span class="label">Gyroscope X</span>
                    <span class="value"><span id="imu_gyro_x">0.00</span> °/s</span>
                </div>
                <div class="info-item">
                    <span class="label">Gyroscope Y</span>
                    <span class="value"><span id="imu_gyro_y">0.00</span> °/s</span>
                </div>
                <div class="info-item">
                    <span class="label">Gyroscope Z</span>
                    <span class="value"><span id="imu_gyro_z">0.00</span> °/s</span>
                </div>
                <div class="info-item">
                    <span class="label">Pitch (X)</span>
                    <span class="value"><span id="imu_pitch">0.00</span>°</span>
                </div>
                <div class="info-item">
                    <span class="label">Roll (Y)</span>
                    <span class="value"><span id="imu_roll">0.00</span>°</span>
                </div>
                <div class="info-item">
                    <span class="label">Yaw (Z)</span>
                    <span class="value"><span id="imu_yaw">0.00</span>°</span>
                </div>
            </div>
        </div>

        <div class="footer">
            PetoiBittle v1.0 - ESP-IDF
        </div>
    </div>

    <script>
        // Mise à jour des données IMU
        function updateIMU() {
            fetch('/api/imu')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('imu_accel_x').textContent = data.accel.x.toFixed(2);
                    document.getElementById('imu_accel_y').textContent = data.accel.y.toFixed(2);
                    document.getElementById('imu_accel_z').textContent = data.accel.z.toFixed(2);
                    document.getElementById('imu_gyro_x').textContent = data.gyro.x.toFixed(2);
                    document.getElementById('imu_gyro_y').textContent = data.gyro.y.toFixed(2);
                    document.getElementById('imu_gyro_z').textContent = data.gyro.z.toFixed(2);
                    document.getElementById('imu_pitch').textContent = data.angles.pitch.toFixed(2);
                    document.getElementById('imu_roll').textContent = data.angles.roll.toFixed(2);
                    document.getElementById('imu_yaw').textContent = data.angles.yaw.toFixed(2);
                })
                .catch(error => {
                    console.error('Erreur IMU:', error);
                });
        }

        // Lancer les mises à jour toutes les 100ms
        setInterval(updateIMU, 100);
        updateIMU(); // Mise à jour initiale

        function sendCommand(cmd) {
            fetch('/api/cmd?action=' + cmd)
                .then(response => response.json())
                .then(data => {
                    console.log('Commande exécutée:', data);
                })
                .catch(error => {
                    console.error('Erreur:', error);
                });
        }

        function updateServo() {
            const ch = document.getElementById('servoChannel').value;
            const angle = document.getElementById('servoAngle').value;
            document.getElementById('angleValue').textContent = angle;
            
            const url = `/api/cmd?action=servo&ch=${ch}&angle=${angle}`;
            fetch(url)
                .then(response => response.json())
                .then(data => {
                    console.log('Servo:', data);
                })
                .catch(error => {
                    console.error('Erreur servo:', error);
                });
        }

    </script>
</body>
</html>
)rawliteral";

// Handler pour la page d'accueil
static esp_err_t home_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET / - Client: %s", req->uri);
    httpd_resp_set_type(req, "text/html");
    // Servir la page IK complète à la racine
    httpd_resp_send(req, HTML_IK_PAGE, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
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

bool start(int port)
{
    ESP_LOGI(TAG, "Démarrage serveur web sur port %d...", port);
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    config.max_uri_handlers = 10;
    config.stack_size = 8192;
    
    if (httpd_start(&s_server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Échec démarrage serveur HTTP");
        return false;
    }
    
    // Enregistrement des handlers
    httpd_uri_t home_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = home_handler,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(s_server, &home_uri);
    
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

