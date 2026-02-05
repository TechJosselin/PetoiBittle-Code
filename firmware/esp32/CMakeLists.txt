cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(PetoiBittle)

# Créer l'image littleFS à partir du dossier data/
littlefs_create_partition_image(littlefs ${CMAKE_SOURCE_DIR}/data FLASH_IN_PROJECT)
