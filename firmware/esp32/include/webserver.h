#pragma once

namespace webserver {
//ZIZI
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
