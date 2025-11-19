/**
 * @file SD_SPI.h
 * @brief SD Card and Flash Manager - OOP Interface
 * @date 2025
 */

#pragma once

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "esp_flash.h"
#include "esp_err.h"
#include <stdbool.h>

#include "ST7789.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Configuration
 ******************************************************************************/

#define SD_PIN_MOSI        EXAMPLE_PIN_NUM_MOSI    
#define SD_PIN_MISO        5    
#define SD_PIN_SCLK        EXAMPLE_PIN_NUM_SCLK    
#define SD_PIN_CS          4

#define SD_MOUNT_POINT     "/sdcard"
#define SD_MAX_FILES       5

/******************************************************************************
 * Type Definitions
 ******************************************************************************/

/**
 * @brief SD Card mount configuration
 */
typedef struct {
    bool format_if_mount_failed;
    uint8_t max_files;
    size_t allocation_unit_size;
} sd_mount_config_t;

/**
 * @brief SD Card device object
 */
typedef struct {
    sdmmc_card_t *card;
    sdmmc_host_t host;
    bool is_mounted;
    char mount_point[32];
    uint32_t card_size_mb;
} sd_card_device_t;

/**
 * @brief Storage manager object (SD Card + Flash)
 */
typedef struct {
    sd_card_device_t *sd_card;
    uint32_t flash_size_mb;
    bool sd_available;
    bool flash_available;
} storage_manager_t;

/******************************************************************************
 * Global Variables (for backward compatibility)
 ******************************************************************************/

extern uint32_t SDCard_Size;
extern uint32_t Flash_Size;

/******************************************************************************
 * SD Card OOP API
 ******************************************************************************/

/**
 * @brief Create SD card device object
 * @param mount_point Mount point path (e.g., "/sdcard")
 * @return SD card device object or NULL on failure
 */
sd_card_device_t* sd_card_create(const char *mount_point);

/**
 * @brief Initialize and mount SD card
 * @param device SD card device object
 * @param config Mount configuration
 * @return ESP_OK on success
 */
esp_err_t sd_card_init(sd_card_device_t *device, const sd_mount_config_t *config);

/**
 * @brief Unmount and destroy SD card device
 * @param device SD card device object
 * @return ESP_OK on success
 */
esp_err_t sd_card_destroy(sd_card_device_t *device);

/**
 * @brief Get SD card size in MB
 * @param device SD card device object
 * @return Size in MB, 0 if not available
 */
uint32_t sd_card_get_size(sd_card_device_t *device);

/**
 * @brief Check if SD card is mounted
 * @param device SD card device object
 * @return true if mounted
 */
bool sd_card_is_mounted(sd_card_device_t *device);

/**
 * @brief Write data to file on SD card
 * @param device SD card device object
 * @param path File path relative to mount point
 * @param data Data string to write
 * @return ESP_OK on success
 */
esp_err_t sd_card_write_file(sd_card_device_t *device, const char *path, const char *data);

/**
 * @brief Read data from file on SD card
 * @param device SD card device object
 * @param path File path relative to mount point
 * @param buffer Buffer to store read data
 * @param buffer_size Size of buffer
 * @return ESP_OK on success
 */
esp_err_t sd_card_read_file(sd_card_device_t *device, const char *path, char *buffer, size_t buffer_size);

/******************************************************************************
 * Storage Manager OOP API
 ******************************************************************************/

/**
 * @brief Create storage manager
 * @return Storage manager object or NULL on failure
 */
storage_manager_t* storage_manager_create(void);

/**
 * @brief Initialize storage manager (SD + Flash)
 * @param manager Storage manager object
 * @return ESP_OK on success
 */
esp_err_t storage_manager_init(storage_manager_t *manager);

/**
 * @brief Get Flash size in MB
 * @param manager Storage manager object
 * @return Flash size in MB
 */
uint32_t storage_manager_get_flash_size(storage_manager_t *manager);

/**
 * @brief Get SD card size in MB
 * @param manager Storage manager object
 * @return SD card size in MB
 */
uint32_t storage_manager_get_sd_size(storage_manager_t *manager);

/**
 * @brief Destroy storage manager
 * @param manager Storage manager object
 */
void storage_manager_destroy(storage_manager_t *manager);

/******************************************************************************
 * Legacy API (for backward compatibility)
 ******************************************************************************/

void SD_Init(void);
void Flash_Searching(void);
esp_err_t s_example_write_file(const char *path, char *data);
esp_err_t s_example_read_file(const char *path);

#ifdef __cplusplus
}
#endif