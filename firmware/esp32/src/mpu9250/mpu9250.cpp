#include "mpu9250.h"

#include <cmath>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"

namespace mpu9250 {

static const char* TAG = "MPU9250";

// Registres MPU-9250
#define MPU9250_ADDR                0x68
#define I2C_PORT                    I2C_NUM_0

// Registres USER BANK 0
#define REG_WHOAMI                  0x00
#define REG_ACCEL_XOUT_H            0x3B
#define REG_ACCEL_XOUT_L            0x3C
#define REG_ACCEL_YOUT_H            0x3D
#define REG_ACCEL_YOUT_L            0x3E
#define REG_ACCEL_ZOUT_H            0x3F
#define REG_ACCEL_ZOUT_L            0x40

#define REG_GYRO_XOUT_H             0x43
#define REG_GYRO_XOUT_L             0x44
#define REG_GYRO_YOUT_H             0x45
#define REG_GYRO_YOUT_L             0x46
#define REG_GYRO_ZOUT_H             0x47
#define REG_GYRO_ZOUT_L             0x48

#define REG_PWR_MGMT_1              0x6B
#define REG_CONFIG                  0x1A
#define REG_GYRO_CONFIG             0x1B
#define REG_ACCEL_CONFIG            0x1C

#define WHOAMI_ID                   0x71

// Variables globales
static bool s_initialized = false;
static float accel_scale = 1.0f / 2048.0f;  // +-16g
static float gyro_scale = 1.0f / 16.4f;     // +-2000deg/s

// Donnees globales pour le calcul d'angles
static AccelData last_accel = {0, 0, 0};
static GyroData last_gyro = {0, 0, 0};
static EulerAngles euler_angles = {0, 0, 0};

// Ecrit un byte dans un registre
static bool write_reg(uint8_t reg, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU9250_ADDR << 1) | 0, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret == ESP_OK;
}

// Lit un byte depuis un registre
static bool read_reg(uint8_t reg, uint8_t* out_value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU9250_ADDR << 1) | 0, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU9250_ADDR << 1) | 1, true);
    i2c_master_read_byte(cmd, out_value, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret == ESP_OK;
}

// Lit N bytes en continu a partir d'un registre
static bool read_regs(uint8_t reg, uint8_t* buffer, uint8_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU9250_ADDR << 1) | 0, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU9250_ADDR << 1) | 1, true);
    for (int i = 0; i < len - 1; i++) {
        i2c_master_read_byte(cmd, &buffer[i], I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, &buffer[len - 1], I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret == ESP_OK;
}

bool init(int sda_gpio, int scl_gpio, uint8_t addr)
{
    ESP_LOGI(TAG, "Initialisation MPU-9250...");

    vTaskDelay(pdMS_TO_TICKS(100));

    // Verification du capteur (WHOAMI)
    uint8_t whoami = 0;
    if (!read_reg(REG_WHOAMI, &whoami)) {
        ESP_LOGE(TAG, "Impossible de lire WHOAMI");
        return false;
    }

    if (whoami != WHOAMI_ID) {
        ESP_LOGE(TAG, "WHOAMI invalide: 0x%02X (attendu 0x%02X)", whoami, WHOAMI_ID);
        return false;
    }

    ESP_LOGI(TAG, "MPU-9250 detecte (WHOAMI=0x%02X)", whoami);

    // Reset du capteur
    write_reg(REG_PWR_MGMT_1, 0x80);
    vTaskDelay(pdMS_TO_TICKS(100));

    // Configuration PWR_MGMT_1 (sortie du sleep mode)
    write_reg(REG_PWR_MGMT_1, 0x00);

    // Configuration CONFIG (filtre passe-bas DLPF)
    write_reg(REG_CONFIG, 0x04);  // DLPF = 20Hz

    // Configuration GYRO_CONFIG (pleine echelle +-2000deg/s)
    write_reg(REG_GYRO_CONFIG, 0x00);

    // Configuration ACCEL_CONFIG (pleine echelle +-16g)
    write_reg(REG_ACCEL_CONFIG, 0x03);

    vTaskDelay(pdMS_TO_TICKS(50));

    s_initialized = true;
    ESP_LOGI(TAG, "MPU-9250 initialise avec succes");
    ESP_LOGI(TAG, "  Accel: +-16g (scale: %f)", accel_scale);
    ESP_LOGI(TAG, "  Gyro: +-2000deg/s (scale: %f)", gyro_scale);

    return true;
}

AccelData read_accel()
{
    AccelData data = {0, 0, 0};

    uint8_t buffer[6];
    if (!read_regs(REG_ACCEL_XOUT_H, buffer, 6)) {
        return data;
    }

    int16_t ax = (int16_t)((buffer[0] << 8) | buffer[1]);
    int16_t ay = (int16_t)((buffer[2] << 8) | buffer[3]);
    int16_t az = (int16_t)((buffer[4] << 8) | buffer[5]);

    data.x = ax * accel_scale;
    data.y = ay * accel_scale;
    data.z = az * accel_scale;

    last_accel = data;
    return data;
}

GyroData read_gyro()
{
    GyroData data = {0, 0, 0};

    uint8_t buffer[6];
    if (!read_regs(REG_GYRO_XOUT_H, buffer, 6)) {
        return data;
    }

    int16_t gx = (int16_t)((buffer[0] << 8) | buffer[1]);
    int16_t gy = (int16_t)((buffer[2] << 8) | buffer[3]);
    int16_t gz = (int16_t)((buffer[4] << 8) | buffer[5]);

    data.x = gx * gyro_scale;
    data.y = gy * gyro_scale;
    data.z = gz * gyro_scale;

    last_gyro = data;
    return data;
}

MagnetoData read_magneto()
{
    MagnetoData data = {0, 0, 0};
    // TODO: Integration magnetometre AK8963 (I2C secondaire)
    return data;
}

EulerAngles calculate_angles()
{
    // Lecture des capteurs
    AccelData accel = read_accel();
    GyroData gyro = read_gyro();

    // Calcul des angles a partir de l'accelerometre (IMU simple)
    float pitch = atan2f(accel.y, sqrtf(accel.x * accel.x + accel.z * accel.z)) * 180.0f / M_PI;
    float roll = atan2f(-accel.x, accel.z) * 180.0f / M_PI;

    // Yaw a partir du gyroscope (integration)
    static unsigned long last_time = 0;
    unsigned long current_time = xTaskGetTickCount() / portTICK_PERIOD_MS;
    float dt = (current_time - last_time) / 1000.0f;
    last_time = current_time;

    if (dt > 0 && dt < 0.1f) {  // Valide si dt raisonnable
        euler_angles.yaw += gyro.z * dt;
    }

    euler_angles.pitch = pitch;
    euler_angles.roll = roll;

    // Normaliser yaw entre -180 et 180
    while (euler_angles.yaw > 180.0f) euler_angles.yaw -= 360.0f;
    while (euler_angles.yaw < -180.0f) euler_angles.yaw += 360.0f;

    return euler_angles;
}

bool is_ready()
{
    uint8_t whoami = 0;
    return read_reg(REG_WHOAMI, &whoami) && whoami == WHOAMI_ID;
}

} // namespace mpu9250
