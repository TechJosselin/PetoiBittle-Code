#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <cstdint>
#include <cstddef>

namespace webserver {

/**
 * Structure pour la configuration du serveur web
 */
struct WebServerConfig {
    uint16_t port;              // Port du serveur (défaut: 80)
};

/**
 * Initialise le serveur web asynchrone
 * Démarre le serveur sur le port spécifié
 * Serve les fichiers depuis LittleFS
 * 
 * @param port Port du serveur (défaut: 80)
 * @return true si succès, false sinon
 */
bool init(uint16_t port = 80);

/**
 * Arrête le serveur web
 */
void stop();

/**
 * Démarre la capture des logs ESP dans un ring buffer
 * accessible via /api/logs
 */
void start_log_capture();

/**
 * Obtient l'adresse IP du serveur
 * 
 * @param buffer Buffer pour stocker l'adresse IP
 * @param size Taille du buffer
 * @return true si succès, false sinon
 */
bool get_ip(char* buffer, size_t size);

} // namespace webserver

#endif // WEBSERVER_H
