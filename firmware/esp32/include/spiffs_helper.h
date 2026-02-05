#pragma once

#include <esp_spiffs.h>
#include <esp_log.h>

namespace spiffs {

static const char* TAG = "SPIFFS";

/**
 * @brief Initialise le système de fichiers SPIFFS
 * @return true si succès, false sinon
 */
inline bool init() {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount or format filesystem (%s)", esp_err_to_name(ret));
        return false;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS info (%s)", esp_err_to_name(ret));
        return false;
    }

    ESP_LOGI(TAG, "SPIFFS initialized - Total: %zu bytes, Used: %zu bytes", total, used);
    return true;
}

/**
 * @brief Lit un fichier depuis SPIFFS
 * @param path Chemin du fichier (ex: "/spiffs/index.html")
 * @return Contenu du fichier, ou nullptr si erreur
 */
inline char* read_file(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) {
        ESP_LOGW(TAG, "Failed to open file: %s", path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buf = (char*)malloc(size + 1);
    if (!buf) {
        ESP_LOGE(TAG, "Failed to allocate memory for file: %s", path);
        fclose(f);
        return NULL;
    }

    size_t read = fread(buf, 1, size, f);
    buf[read] = '\0';
    fclose(f);

    return buf;
}

} // namespace spiffs
