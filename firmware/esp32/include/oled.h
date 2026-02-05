#pragma once

#include <cstdint>
#include <cstddef>

/**
 * @file oled.h
 * @brief Module de contrôle de l'écran OLED SSD1306
 * 
 * Driver pour écran OLED 128x64 pixels communiquant via I2C.
 * Supporte l'affichage de texte avec une police 5x7 intégrée,
 * contrôle du contraste et de l'alimentation.
 * 
 * @note L'écran utilise l'adresse I2C standard 0x3C
 */

namespace oled {

/**
 * @brief Initialise l'écran OLED SSD1306
 * @param scl_gpio GPIO pour SCL (horloge I2C)
 * @param sda_gpio GPIO pour SDA (données I2C)
 * @param i2c_freq Fréquence I2C en Hz (défaut 400kHz)
 * @return true si initialisation réussie, false sinon
 */
bool init(int scl_gpio, int sda_gpio, uint32_t i2c_freq = 400000);

/**
 * @brief Efface l'écran
 */
void clear();

/**
 * @brief Affiche un texte à une position donnée
 * @param text Texte à afficher
 * @param x Position X (0-127)
 * @param y Ligne (0-7 pour écran 128x64)
 */
void print(const char* text, uint8_t x = 0, uint8_t y = 0);

/**
 * @brief Met à jour l'affichage (envoie le buffer vers l'écran)
 */
void update();

/**
 * @brief Définit le contraste de l'écran
 * @param contrast Valeur 0-255
 */
void set_contrast(uint8_t contrast);

/**
 * @brief Active/désactive l'écran
 * @param on true pour allumer, false pour éteindre
 */
void power(bool on);

} // namespace oled
