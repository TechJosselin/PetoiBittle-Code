#pragma once

#include <cstdint>

namespace mpu9250 {

/**
 * @brief Données d'accélération (en g)
 */
struct AccelData {
    float x, y, z;
};

/**
 * @brief Données de gyroscope (en deg/s)
 */
struct GyroData {
    float x, y, z;
};

/**
 * @brief Données de magnétomètre (en Gauss)
 */
struct MagnetoData {
    float x, y, z;
};

/**
 * @brief Angles Euler (en degrés)
 */
struct EulerAngles {
    float pitch;  // Rotation autour axe X (deg)
    float roll;   // Rotation autour axe Y (deg)
    float yaw;    // Rotation autour axe Z (deg)
};

/**
 * @brief Initialise le MPU-9250 sur I2C (adresse 0x68 par défaut)
 * @return true si succès, false sinon
 */
bool init(int sda_gpio, int scl_gpio, uint8_t addr = 0x68);

/**
 * @brief Lit les données d'accélération brutes (16 bits)
 * @return Accélération en g (± 16g, pleine échelle)
 */
AccelData read_accel();

/**
 * @brief Lit les données de gyroscope brutes (16 bits)
 * @return Vitesse angulaire en deg/s (± 2000 deg/s, pleine échelle)
 */
GyroData read_gyro();

/**
 * @brief Lit les données de magnétomètre brutes
 * @return Magnétomètre en Gauss
 */
MagnetoData read_magneto();

/**
 * @brief Calcule les angles Euler à partir de accel + gyro
 * @return Angles en degrés (pitch, roll, yaw)
 */
EulerAngles calculate_angles();

/**
 * @brief Vérifie si le capteur est actif et répond
 * @return true si réponse ok, false sinon
 */
bool is_ready();

} // namespace mpu9250
