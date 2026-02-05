#pragma once

#include <esp_littlefs.h>
#include <esp_log.h>

namespace littlefs {

static const char* TAG = "littleFS";

/**
 * @brief Initialise le système de fichiers littleFS
 * @return true si succès, false sinon
 */
inline bool init() {
    esp_vfs_littlefs_conf_t conf = {};
    conf.base_path = "/littlefs";
    conf.partition_label = "littlefs";
    conf.format_if_mount_failed = true;
    conf.dont_mount = false;

    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount or format filesystem (%s)", esp_err_to_name(ret));
        return false;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get littleFS info (%s)", esp_err_to_name(ret));
        return false;
    }

    ESP_LOGI(TAG, "littleFS initialized - Total: %zu bytes, Used: %zu bytes", total, used);
    return true;
}

/**
 * @brief Lit un fichier depuis littleFS
 * @param path Chemin du fichier (ex: "/littlefs/index.html")
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

} // namespace littlefs
