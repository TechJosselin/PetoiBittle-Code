#pragma once

/**
 * @file webserver.h
 * @brief Module serveur web HTTP pour le contrôle du robot
 * 
 * Ce module fournit une interface web complète avec:
 * - Contrôle LED et servos
 * - Lecture des données IMU en temps réel
 * - Interface de cinématique inverse (IK) interactive
 * - API REST JSON pour le contrôle batch des servos
 */

namespace webserver {

/**
 * @brief Démarre le serveur web HTTP
 * @param port Port d'écoute (par défaut 80)
 * @return true si le serveur démarre, false sinon
 */
bool start(int port = 80);

/**
 * @brief Arrête le serveur web
 */
void stop();

/**
 * @brief Vérifie si le serveur web est actif
 * @return true si actif, false sinon
 */
bool is_running();

} // namespace webserver
