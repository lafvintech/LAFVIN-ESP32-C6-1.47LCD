/**
 * @file SD_SPI.c
 * @brief SD Card and Flash Manager - OOP Implementation
 * @date 2025
 */

#include "SD_SPI.h"
#include "esp_log.h"
#include <stdlib.h>

static const char *TAG = "Storage";

/******************************************************************************
 * Global Variables (for backward compatibility)
 ******************************************************************************/

uint32_t Flash_Size = 0;
uint32_t SDCard_Size = 0;

/******************************************************************************
 * SD Card OOP Implementation
 ******************************************************************************/

sd_card_device_t* sd_card_create(const char *mount_point)
{
    if (mount_point == NULL) {
        ESP_LOGE(TAG, "Mount point cannot be NULL");
        return NULL;
    }
    
    sd_card_device_t *device = (sd_card_device_t*)calloc(1, sizeof(sd_card_device_t));
    if (device == NULL) {
        ESP_LOGE(TAG, "Failed to allocate SD card device");
        return NULL;
    }
    
    strncpy(device->mount_point, mount_point, sizeof(device->mount_point) - 1);
    device->is_mounted = false;
    device->card = NULL;
    device->card_size_mb = 0;
    
    ESP_LOGI(TAG, "SD card device created with mount point: %s", mount_point);
    return device;
}

esp_err_t sd_card_init(sd_card_device_t *device, const sd_mount_config_t *config)
{
    if (device == NULL) {
        ESP_LOGE(TAG, "Device object is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (device->is_mounted) {
        ESP_LOGW(TAG, "SD card already mounted");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Initializing SD card on SPI bus");
    
    // Mount configuration
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = config ? config->format_if_mount_failed : false,
        .max_files = config ? config->max_files : SD_MAX_FILES,
        .allocation_unit_size = config ? config->allocation_unit_size : 16 * 1024
    };
    
    // Host configuration (use existing SPI bus from LCD)
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    device->host = host;
    
    // Slot configuration
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_PIN_CS;
    slot_config.host_id = device->host.slot;
    
    ESP_LOGI(TAG, "Mounting filesystem at %s", device->mount_point);
    
    // Mount the SD card
    esp_err_t ret = esp_vfs_fat_sdspi_mount(
        device->mount_point,
        &device->host,
        &slot_config,
        &mount_config,
        &device->card
    );
    
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem");
        } else {
            ESP_LOGE(TAG, "Failed to initialize card: %s", esp_err_to_name(ret));
        }
        return ret;
    }
    
    device->is_mounted = true;
    
    // Print card info
    sdmmc_card_print_info(stdout, device->card);
    
    // Calculate and store card size
    device->card_size_mb = ((uint64_t)device->card->csd.capacity * 
                            device->card->csd.sector_size) / (1024 * 1024);
    
    // Update global variable for backward compatibility
    SDCard_Size = device->card_size_mb;
    
    ESP_LOGI(TAG, "SD card initialized: %lu MB", (unsigned long)device->card_size_mb);
    return ESP_OK;
}

esp_err_t sd_card_destroy(sd_card_device_t *device)
{
    if (device == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (device->is_mounted) {
        esp_vfs_fat_sdcard_unmount(device->mount_point, device->card);
        device->is_mounted = false;
        ESP_LOGI(TAG, "SD card unmounted");
    }
    
    free(device);
    return ESP_OK;
}

uint32_t sd_card_get_size(sd_card_device_t *device)
{
    if (device == NULL || !device->is_mounted) {
        return 0;
    }
    return device->card_size_mb;
}

bool sd_card_is_mounted(sd_card_device_t *device)
{
    if (device == NULL) {
        return false;
    }
    return device->is_mounted;
}

esp_err_t sd_card_write_file(sd_card_device_t *device, const char *path, const char *data)
{
    if (device == NULL || !device->is_mounted) {
        ESP_LOGE(TAG, "SD card not mounted");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (path == NULL || data == NULL) {
        ESP_LOGE(TAG, "Invalid path or data");
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Writing to file: %s", path);
    
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    
    fprintf(f, "%s", data);
    fclose(f);
    
    ESP_LOGI(TAG, "File written successfully");
    return ESP_OK;
}

esp_err_t sd_card_read_file(sd_card_device_t *device, const char *path, char *buffer, size_t buffer_size)
{
    if (device == NULL || !device->is_mounted) {
        ESP_LOGE(TAG, "SD card not mounted");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (path == NULL || buffer == NULL || buffer_size == 0) {
        ESP_LOGE(TAG, "Invalid parameters");
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Reading from file: %s", path);
    
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    
    if (fgets(buffer, buffer_size, f) == NULL) {
        fclose(f);
        ESP_LOGE(TAG, "Failed to read file");
        return ESP_FAIL;
    }
    
    fclose(f);
    
    // Strip newline
    char *pos = strchr(buffer, '\n');
    if (pos) {
        *pos = '\0';
    }
    
    ESP_LOGI(TAG, "Read from file: '%s'", buffer);
    return ESP_OK;
}

/******************************************************************************
 * Storage Manager OOP Implementation
 ******************************************************************************/

storage_manager_t* storage_manager_create(void)
{
    storage_manager_t *manager = (storage_manager_t*)calloc(1, sizeof(storage_manager_t));
    if (manager == NULL) {
        ESP_LOGE(TAG, "Failed to allocate storage manager");
        return NULL;
    }
    
    manager->sd_card = NULL;
    manager->flash_size_mb = 0;
    manager->sd_available = false;
    manager->flash_available = false;
    
    ESP_LOGI(TAG, "Storage manager created");
    return manager;
}

esp_err_t storage_manager_init(storage_manager_t *manager)
{
    if (manager == NULL) {
        ESP_LOGE(TAG, "Manager object is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Initializing storage manager");
    
    // Initialize Flash
    uint32_t flash_size_bytes = 0;
    if (esp_flash_get_physical_size(NULL, &flash_size_bytes) == ESP_OK) {
        manager->flash_size_mb = flash_size_bytes / (1024 * 1024);
        manager->flash_available = true;
        Flash_Size = manager->flash_size_mb;  // Update global
        ESP_LOGI(TAG, "Flash detected: %lu MB", (unsigned long)manager->flash_size_mb);
    } else {
        ESP_LOGW(TAG, "Failed to detect Flash size");
        manager->flash_available = false;
    }
    
    // Initialize SD Card
    manager->sd_card = sd_card_create(SD_MOUNT_POINT);
    if (manager->sd_card != NULL) {
        sd_mount_config_t sd_config = {
            .format_if_mount_failed = false,
            .max_files = SD_MAX_FILES,
            .allocation_unit_size = 16 * 1024
        };
        
        esp_err_t ret = sd_card_init(manager->sd_card, &sd_config);
        if (ret == ESP_OK) {
            manager->sd_available = true;
            ESP_LOGI(TAG, "SD card available: %lu MB", 
                     (unsigned long)sd_card_get_size(manager->sd_card));
        } else {
            ESP_LOGW(TAG, "SD card not available");
            sd_card_destroy(manager->sd_card);
            manager->sd_card = NULL;
            manager->sd_available = false;
        }
    }
    
    ESP_LOGI(TAG, "Storage manager initialized (Flash: %s, SD: %s)",
             manager->flash_available ? "YES" : "NO",
             manager->sd_available ? "YES" : "NO");
    
    return ESP_OK;
}

uint32_t storage_manager_get_flash_size(storage_manager_t *manager)
{
    if (manager == NULL || !manager->flash_available) {
        return 0;
    }
    return manager->flash_size_mb;
}

uint32_t storage_manager_get_sd_size(storage_manager_t *manager)
{
    if (manager == NULL || !manager->sd_available || manager->sd_card == NULL) {
        return 0;
    }
    return sd_card_get_size(manager->sd_card);
}

void storage_manager_destroy(storage_manager_t *manager)
{
    if (manager == NULL) {
        return;
    }
    
    if (manager->sd_card != NULL) {
        sd_card_destroy(manager->sd_card);
    }
    
    free(manager);
    ESP_LOGI(TAG, "Storage manager destroyed");
}

/******************************************************************************
 * Legacy API Implementation (for backward compatibility)
 ******************************************************************************/

// Global storage manager for legacy API
static storage_manager_t *g_legacy_manager = NULL;

void Flash_Searching(void)
{
    uint32_t flash_size_bytes = 0;
    if (esp_flash_get_physical_size(NULL, &flash_size_bytes) == ESP_OK) {
        Flash_Size = flash_size_bytes / (1024 * 1024);
        ESP_LOGI(TAG, "Flash size: %lu MB", (unsigned long)Flash_Size);
    } else {
        ESP_LOGE(TAG, "Get flash size failed");
    }
}

void SD_Init(void)
{
    ESP_LOGI(TAG, "Legacy SD_Init() called");
    
    // Create SD card device
    sd_card_device_t *device = sd_card_create(SD_MOUNT_POINT);
    if (device == NULL) {
        ESP_LOGE(TAG, "Failed to create SD card device");
        return;
    }
    
    // Mount configuration
    sd_mount_config_t config = {
        .format_if_mount_failed = false,
        .max_files = SD_MAX_FILES,
        .allocation_unit_size = 16 * 1024
    };
    
    // Initialize SD card
    esp_err_t ret = sd_card_init(device, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SD card");
        sd_card_destroy(device);
        return;
    }
    
    // Store for legacy access
    if (g_legacy_manager == NULL) {
        g_legacy_manager = storage_manager_create();
    }
    if (g_legacy_manager != NULL) {
        g_legacy_manager->sd_card = device;
        g_legacy_manager->sd_available = true;
    }
}

esp_err_t s_example_write_file(const char *path, char *data)
{
    if (g_legacy_manager != NULL && g_legacy_manager->sd_card != NULL) {
        return sd_card_write_file(g_legacy_manager->sd_card, path, data);
    }
    
    // Fallback to direct file access
    ESP_LOGI(TAG, "Writing to file: %s", path);
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, "%s", data);
    fclose(f);
    ESP_LOGI(TAG, "File written");
    return ESP_OK;
}

esp_err_t s_example_read_file(const char *path)
{
    char buffer[64];
    
    if (g_legacy_manager != NULL && g_legacy_manager->sd_card != NULL) {
        return sd_card_read_file(g_legacy_manager->sd_card, path, buffer, sizeof(buffer));
    }
    
    // Fallback to direct file access
    ESP_LOGI(TAG, "Reading from file: %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    
    if (fgets(buffer, sizeof(buffer), f) != NULL) {
        char *pos = strchr(buffer, '\n');
        if (pos) *pos = '\0';
        ESP_LOGI(TAG, "Read from file: '%s'", buffer);
    }
    
    fclose(f);
    return ESP_OK;
}
