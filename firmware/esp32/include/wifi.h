#pragma once

#include <cstdint>
#include <cstddef>

namespace wifi {
//ZIZIs
/**
 * @brief Configuration WiFi Access Point
 */
struct ApConfig {
    const char* ssid;              // Nom du réseau WiFi créé
    const char* password;          // Mot de passe (min 8 caractères, vide = réseau ouvert)
    uint8_t channel;               // Canal WiFi (1-13)
    uint8_t max_connections;       // Nombre max de clients (1-4)
    const char* ip_addr;           // IP du serveur AP (ex: "192.168.4.1")
    const char* gateway;           // Passerelle (généralement = ip_addr)
    const char* netmask;           // Masque réseau (ex: "255.255.255.0")
};

/**
 * @brief Initialise et démarre le WiFi en mode Access Point
 * @param config Configuration de l'Access Point
 * @return true si le démarrage réussit, false sinon
 */
bool start_ap(const ApConfig& config);

/**
 * @brief Arrête l'Access Point WiFi
 */
void stop_ap();

/**
 * @brief Vérifie si l'AP est actif
 * @return true si l'AP est démarré, false sinon
 */
bool is_ap_running();

/**
 * @brief Obtient l'adresse IP de l'AP
 * @param ip_str Buffer pour stocker l'IP (min 16 bytes)
 * @return true si IP obtenue, false sinon
 */
bool get_ap_ip(char* ip_str, size_t len);

/**
 * @brief Obtient le nombre de clients connectés
 * @return Nombre de clients actuellement connectés
 */
uint8_t get_client_count();

/**
 * @brief Obtient la liste des clients connectés
 * @param clients_info Buffer pour stocker les infos clients
 * @param max_clients Taille max du buffer
 * @return Nombre de clients récupérés
 */
uint8_t get_clients_info(void* clients_info, uint8_t max_clients);

} // namespace wifi
