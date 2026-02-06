#include "pca9685.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace pca9685 {

static const char* TAG = "PCA9685";

// Registres PCA9685
enum Register {
    MODE1 = 0x00,
    MODE2 = 0x01,
    SUBADR1 = 0x02,
    SUBADR2 = 0x03,
    SUBADR3 = 0x04,
    PRESCALE = 0xFE,
    LED0_ON_L = 0x06,
    LED0_ON_H = 0x07,
    LED0_OFF_L = 0x08,
    LED0_OFF_H = 0x09,
    ALL_LED_ON_L = 0xFA,
    ALL_LED_ON_H = 0xFB,
    ALL_LED_OFF_L = 0xFC,
    ALL_LED_OFF_H = 0xFD,
};

// Bits MODE1
enum Mode1Bits {
    MODE1_RESTART = 0x80,
    MODE1_SLEEP = 0x10,
    MODE1_ALLCALL = 0x01,
    MODE1_AI = 0x20,  // Auto-increment
};

static uint8_t s_address = 0x40;
static i2c_port_t s_i2c_port = I2C_NUM_0;
static uint16_t s_pwm_freq = 50;
static bool s_initialized = false;

/**
 * @brief Écrit une valeur dans un registre du PCA9685
 * @param reg Adresse du registre
 * @param value Valeur à écrire
 * @return ESP_OK si succès, erreur sinon
 */
static esp_err_t write_register(uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    return i2c_master_write_to_device(s_i2c_port, s_address, data, 2, pdMS_TO_TICKS(100));
}

/**
 * @brief Lit une valeur depuis un registre du PCA9685
 * @param reg Adresse du registre
 * @param value Pointeur pour stocker la valeur lue
 * @return ESP_OK si succès, erreur sinon
 */
static esp_err_t read_register(uint8_t reg, uint8_t* value) {
    return i2c_master_write_read_device(s_i2c_port, s_address, &reg, 1, value, 1, pdMS_TO_TICKS(100));
}

void reset() {
    if (!s_initialized) return;
    write_register(MODE1, MODE1_RESTART);
    vTaskDelay(pdMS_TO_TICKS(10));
}

bool init(int i2c_port, uint16_t freq, uint8_t address) {
    ESP_LOGI(TAG, "Initialisation PCA9685 (0x%02X, %dHz)...", address, freq);

    s_address = address;
    s_i2c_port = (i2c_port_t)i2c_port;
    s_pwm_freq = freq;

    // Test de communication
    uint8_t test_val;
    esp_err_t ret = read_register(MODE1, &test_val);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Échec communication I2C: %s", esp_err_to_name(ret));
        return false;
    }

    // Reset
    write_register(MODE1, MODE1_RESTART);
    vTaskDelay(pdMS_TO_TICKS(10));

    // Configuration MODE1: wake up + auto-increment
    write_register(MODE1, MODE1_AI | MODE1_ALLCALL);
    vTaskDelay(pdMS_TO_TICKS(1));

    // Configuration MODE2: totem pole (push-pull)
    write_register(MODE2, 0x04);

    // Calcul du prescaler
    uint8_t prescale = (uint8_t)((25000000.0f / (4096.0f * freq)) - 1.0f + 0.5f);

    // Séquence configuration fréquence
    uint8_t oldmode;
    read_register(MODE1, &oldmode);
    write_register(MODE1, (oldmode & 0x7F) | MODE1_SLEEP);
    write_register(PRESCALE, prescale);
    write_register(MODE1, oldmode);
    vTaskDelay(pdMS_TO_TICKS(1));
    write_register(MODE1, oldmode | MODE1_RESTART | MODE1_AI);

    s_initialized = true;
    ESP_LOGI(TAG, "✓ PCA9685 initialisé");
    return true;
}

void set_pwm(uint8_t channel, uint16_t on, uint16_t off) {
    if (!s_initialized || channel > 15) return;

    uint8_t reg = LED0_ON_L + 4 * channel;
    uint8_t buf[5] = {
        reg,
        (uint8_t)(on & 0xFF),
        (uint8_t)(on >> 8),
        (uint8_t)(off & 0xFF),
        (uint8_t)(off >> 8)
    };
    esp_err_t ret = i2c_master_write_to_device(s_i2c_port, s_address, buf, 5, pdMS_TO_TICKS(100));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "set_pwm ch=%d FAIL: %s", channel, esp_err_to_name(ret));
    }
}

void set_servo_pulse_us(uint8_t channel, uint16_t pulse_us) {
    if (!s_initialized) return;

    float period_us = 1000000.0f / s_pwm_freq;
    uint16_t pwm_value = (uint16_t)((pulse_us * 4096.0f) / period_us);
    set_pwm(channel, 0, pwm_value);
}

void set_servo_angle(uint8_t channel, uint8_t angle) {
    if (!s_initialized || angle > 180) return;

    uint16_t pulse_us = 500 + (angle * 2000) / 180;
    ESP_LOGI(TAG, "servo ch=%d angle=%d pulse=%uus", channel, angle, pulse_us);
    set_servo_pulse_us(channel, pulse_us);

    // Read-back verification: lire le registre OFF pour vérifier l'écriture
    uint8_t reg = LED0_OFF_L + 4 * channel;
    uint8_t readback[2] = {};
    esp_err_t ret = i2c_master_write_read_device(s_i2c_port, s_address, &reg, 1, readback, 2, pdMS_TO_TICKS(100));
    if (ret == ESP_OK) {
        uint16_t off_val = readback[0] | (readback[1] << 8);
        ESP_LOGI(TAG, "  -> readback OFF=%u (expected ~%u)", off_val,
                 (uint16_t)((pulse_us * 4096.0f) / (1000000.0f / s_pwm_freq)));
    } else {
        ESP_LOGE(TAG, "  -> readback FAIL: %s", esp_err_to_name(ret));
    }
}

void all_off() {
    if (!s_initialized) return;

    write_register(ALL_LED_OFF_L, 0x00);
    write_register(ALL_LED_OFF_H, 0x10);  // Bit 4 = full OFF
}

void test_servo_loop(uint16_t delay_ms) {
    if (!s_initialized) return;

    static uint8_t position = 0;
    const uint8_t angles[] = {0, 45, 90, 135, 180, 135, 90, 45};
    set_servo_angle(0, angles[position]);
    position = (position + 1) % 8;
    (void)delay_ms;
}

} // namespace pca9685