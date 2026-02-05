#pragma once

#include <cstdint>
//ZIZI
namespace pca9685 {

/**
 * @brief Initialise le contrôleur PCA9685
 * @param i2c_port Port I2C à utiliser (I2C_NUM_0 ou I2C_NUM_1)
 * @param freq Fréquence PWM en Hz (50Hz pour servos standards)
 * @param address Adresse I2C du PCA9685 (0x40 par défaut)
 * @return true si initialisation réussie, false sinon
 */
bool init(int i2c_port = 0, uint16_t freq = 50, uint8_t address = 0x40);

/**
 * @brief Définit l'angle d'un servo (0-180°)
 * @param channel Canal du servo (0-15)
 * @param angle Angle en degrés (0-180)
 */
void set_servo_angle(uint8_t channel, uint8_t angle);

/**
 * @brief Définit la largeur d'impulsion PWM en microsecondes
 * @param channel Canal (0-15)
 * @param pulse_us Largeur d'impulsion en µs (500-2500 typique pour servos)
 */
void set_servo_pulse_us(uint8_t channel, uint16_t pulse_us);

/**
 * @brief Définit directement la valeur PWM (0-4095)
 * @param channel Canal (0-15)
 * @param on Valeur de début (0-4095)
 * @param off Valeur de fin (0-4095)
 */
void set_pwm(uint8_t channel, uint16_t on, uint16_t off);

/**
 * @brief Éteint tous les canaux PWM
 */
void all_off();

/**
 * @brief Redémarre le PCA9685
 */
void reset();

/**
 * @brief Test automatique du servo 0 (boucle)
 * @param delay_ms Délai entre les mouvements en ms
 */
void test_servo_loop(uint16_t delay_ms = 2000);

} // namespace pca9685
