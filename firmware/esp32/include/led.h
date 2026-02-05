#pragma once

#include "driver/gpio.h"

/**
 * @file led.h
 * @brief Module de contrôle de la LED intégrée du ESP32-C6
 * 
 * Ce module permet de contrôler la LED intégrée en mode Active LOW.
 * La LED est connectée au GPIO15 sur le Seeed Xiao ESP32-C6.
 */

namespace led {

/**
 * @brief Initialise le module LED
 * @param gpio_num Numéro GPIO de la LED
 */
void init(gpio_num_t gpio_num);

/**
 * @brief Allume la LED
 */
void on();

/**
 * @brief Éteint la LED
 */
void off();

/**
 * @brief Change l'état de la LED (toggle)
 */
void toggle();

/**
 * @brief Obtient l'état actuel de la LED
 * @return true si allumée, false sinon
 */
bool get_state();

/**
 * @brief Définit l'état de la LED
 * @param state true pour allumer, false pour éteindre
 */
void set_state(bool state);

} // namespace led
