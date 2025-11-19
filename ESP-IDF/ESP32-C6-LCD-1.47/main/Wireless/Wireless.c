/**
 * @file Wireless.c
 * @brief Wireless (WiFi & BLE) Scanner Module Implementation - OOP Style
 */

#include "Wireless.h"
#include "esp_netif.h"
#include "esp_event.h"
#include <stdlib.h>

/*******************************************************************************
 * Module Constants
 ******************************************************************************/
#define TAG_WIFI "WIFI_SCANNER"
#define TAG_BLE  "BLE_SCANNER"
#define TAG_WIRELESS "WIRELESS_MGR"

/*******************************************************************************
 * Static Variables for BLE GAP Callback
 ******************************************************************************/
static ble_scanner_t *s_current_ble_scanner = NULL;

/*******************************************************************************
 * Helper Functions
 ******************************************************************************/

/**
 * @brief Check if a BLE device is already discovered
 */
static bool is_device_discovered(const ble_scanner_t *scanner, const uint8_t *addr) {
    if (!scanner || !addr) return false;
    
    for (uint16_t i = 0; i < scanner->device_count; i++) {
        if (memcmp(scanner->devices[i].address, addr, 6) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Add a BLE device to the scanner's device list
 */
static bool add_device_to_list(ble_scanner_t *scanner, const uint8_t *addr, int8_t rssi) {
    if (!scanner || !addr) return false;
    if (scanner->device_count >= WIRELESS_MAX_BLE_DEVICES) return false;
    
    ble_device_info_t *device = &scanner->devices[scanner->device_count];
    memcpy(device->address, addr, 6);
    device->rssi = rssi;
    device->has_name = false;
    device->is_valid = true;
    device->name[0] = '\0';
    
    scanner->device_count++;
    return true;
}

/**
 * @brief Extract device name from BLE advertisement data
 */
static bool extract_device_name(const uint8_t *adv_data, uint8_t adv_data_len, 
                                 char *device_name, size_t max_name_len) {
    if (!adv_data || !device_name) return false;
    
    size_t offset = 0;
    while (offset < adv_data_len) {
        if (adv_data[offset] == 0) break;
        
        uint8_t length = adv_data[offset];
        if (length == 0 || offset + length > adv_data_len) break;
        
        uint8_t type = adv_data[offset + 1];
        if (type == ESP_BLE_AD_TYPE_NAME_CMPL || type == ESP_BLE_AD_TYPE_NAME_SHORT) {
            if (length > 1 && length - 1 < max_name_len) {
                memcpy(device_name, &adv_data[offset + 2], length - 1);
                device_name[length - 1] = '\0';
                return true;
            } else {
                return false;
            }
        }
        offset += length + 1;
    }
    return false;
}

/**
 * @brief Update device name in scanner's device list
 */
static void update_device_name(ble_scanner_t *scanner, const uint8_t *addr, const char *name) {
    if (!scanner || !addr || !name) return;
    
    for (uint16_t i = 0; i < scanner->device_count; i++) {
        if (memcmp(scanner->devices[i].address, addr, 6) == 0) {
            strncpy(scanner->devices[i].name, name, WIRELESS_DEVICE_NAME_MAX_LEN - 1);
            scanner->devices[i].name[WIRELESS_DEVICE_NAME_MAX_LEN - 1] = '\0';
            scanner->devices[i].has_name = true;
            scanner->named_device_count++;
            break;
        }
    }
}

/**
 * @brief BLE GAP event callback (for OOP API)
 */
static void ble_gap_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    if (!s_current_ble_scanner) return;
    
    char device_name[WIRELESS_DEVICE_NAME_MAX_LEN] = {0};
    
    switch (event) {
        case ESP_GAP_BLE_SCAN_RESULT_EVT:
            if (param->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT) {
                if (!is_device_discovered(s_current_ble_scanner, param->scan_rst.bda)) {
                    // Add device to list
                    if (add_device_to_list(s_current_ble_scanner, param->scan_rst.bda, param->scan_rst.rssi)) {
                        // Try to extract device name
                        if (extract_device_name(param->scan_rst.ble_adv, param->scan_rst.adv_data_len, 
                                                device_name, sizeof(device_name))) {
                            update_device_name(s_current_ble_scanner, param->scan_rst.bda, device_name);
                            ESP_LOGI(TAG_BLE, "Device %02X:%02X:%02X:%02X:%02X:%02X, Name: %s, RSSI: %d",
                                     param->scan_rst.bda[0], param->scan_rst.bda[1],
                                     param->scan_rst.bda[2], param->scan_rst.bda[3],
                                     param->scan_rst.bda[4], param->scan_rst.bda[5],
                                     device_name, param->scan_rst.rssi);
                        } else {
                            ESP_LOGI(TAG_BLE, "Device %02X:%02X:%02X:%02X:%02X:%02X, Name: Unknown, RSSI: %d",
                                     param->scan_rst.bda[0], param->scan_rst.bda[1],
                                     param->scan_rst.bda[2], param->scan_rst.bda[3],
                                     param->scan_rst.bda[4], param->scan_rst.bda[5],
                                     param->scan_rst.rssi);
                        }
                    }
                }
            }
            break;
            
        case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
            ESP_LOGI(TAG_BLE, "Scan complete. Total: %d, Named: %d", 
                     s_current_ble_scanner->device_count, s_current_ble_scanner->named_device_count);
            s_current_ble_scanner->scan_finished = true;
            break;
            
        default:
            break;
    }
}

/*******************************************************************************
 * WiFi Scanner OOP Implementation
 ******************************************************************************/

wifi_scanner_t* wifi_scanner_create(void) {
    wifi_scanner_t *scanner = (wifi_scanner_t*)calloc(1, sizeof(wifi_scanner_t));
    if (!scanner) {
        ESP_LOGE(TAG_WIFI, "Failed to allocate WiFi scanner");
        return NULL;
    }
    
    scanner->ap_count = 0;
    scanner->scan_finished = false;
    scanner->is_initialized = false;
    
    ESP_LOGI(TAG_WIFI, "WiFi scanner created");
    return scanner;
}

void wifi_scanner_init(wifi_scanner_t *scanner, void *arg) {
    if (!scanner) {
        ESP_LOGE(TAG_WIFI, "Invalid scanner object");
        return;
    }
    
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    
    scanner->is_initialized = true;
    scanner->ap_count = wifi_scanner_scan(scanner);
    
    ESP_LOGI(TAG_WIFI, "WiFi scanner initialized, found %d APs", scanner->ap_count);
    
    // Delete task after scanning (for task-based usage)
    vTaskDelete(NULL);
}

uint16_t wifi_scanner_scan(wifi_scanner_t *scanner) {
    if (!scanner) {
        ESP_LOGE(TAG_WIFI, "Invalid scanner object");
        return 0;
    }
    
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&scanner->ap_count));
    esp_wifi_scan_stop();
    
    scanner->scan_finished = true;
    
    return scanner->ap_count;
}

uint16_t wifi_scanner_get_ap_count(const wifi_scanner_t *scanner) {
    return scanner ? scanner->ap_count : 0;
}

bool wifi_scanner_is_finished(const wifi_scanner_t *scanner) {
    return scanner ? scanner->scan_finished : false;
}

void wifi_scanner_destroy(wifi_scanner_t *scanner) {
    if (scanner) {
        ESP_LOGI(TAG_WIFI, "WiFi scanner destroyed");
        free(scanner);
    }
}

/*******************************************************************************
 * BLE Scanner OOP Implementation
 ******************************************************************************/

ble_scanner_t* ble_scanner_create(void) {
    ble_scanner_t *scanner = (ble_scanner_t*)calloc(1, sizeof(ble_scanner_t));
    if (!scanner) {
        ESP_LOGE(TAG_BLE, "Failed to allocate BLE scanner");
        return NULL;
    }
    
    scanner->device_count = 0;
    scanner->named_device_count = 0;
    scanner->scan_finished = false;
    scanner->is_initialized = false;
    
    // Initialize device array
    for (int i = 0; i < WIRELESS_MAX_BLE_DEVICES; i++) {
        scanner->devices[i].is_valid = false;
    }
    
    ESP_LOGI(TAG_BLE, "BLE scanner created");
    return scanner;
}

void ble_scanner_init(ble_scanner_t *scanner, void *arg) {
    if (!scanner) {
        ESP_LOGE(TAG_BLE, "Invalid scanner object");
        return;
    }
    
    // Release classic BT memory
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    
    // Initialize BT controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_err_t ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_BLE, "BT controller init failed: %s", esp_err_to_name(ret));
        return;
    }
    
    // Enable BLE mode
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_BLE, "BT controller enable failed: %s", esp_err_to_name(ret));
        return;
    }
    
    // Initialize Bluedroid
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_BLE, "Bluedroid init failed: %s", esp_err_to_name(ret));
        return;
    }
    
    // Enable Bluedroid
    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_BLE, "Bluedroid enable failed: %s", esp_err_to_name(ret));
        return;
    }
    
    // Set current scanner for callback
    s_current_ble_scanner = scanner;
    
    // Register GAP callback
    ret = esp_ble_gap_register_callback(ble_gap_callback);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_BLE, "GAP register callback failed: %s", esp_err_to_name(ret));
        return;
    }
    
    scanner->is_initialized = true;
    
    // Perform scan
    ble_scanner_scan(scanner);
    
    ESP_LOGI(TAG_BLE, "BLE scanner initialized");
    
    // Delete task after scanning (for task-based usage)
    vTaskDelete(NULL);
}

uint16_t ble_scanner_scan(ble_scanner_t *scanner) {
    if (!scanner) {
        ESP_LOGE(TAG_BLE, "Invalid scanner object");
        return 0;
    }
    
    // Configure scan parameters
    esp_ble_scan_params_t scan_params = {
        .scan_type = BLE_SCAN_TYPE_ACTIVE,
        .own_addr_type = BLE_ADDR_TYPE_RPA_PUBLIC,
        .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval = 0x50,
        .scan_window = 0x30,
        .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
    };
    
    ESP_ERROR_CHECK(esp_ble_gap_set_scan_params(&scan_params));
    
    ESP_LOGI(TAG_BLE, "Starting BLE scan for %d seconds...", WIRELESS_BLE_SCAN_DURATION_S);
    ESP_ERROR_CHECK(esp_ble_gap_start_scanning(WIRELESS_BLE_SCAN_DURATION_S));
    
    // Wait for scan duration
    vTaskDelay(WIRELESS_BLE_SCAN_DURATION_S * 1000 / portTICK_PERIOD_MS);
    
    ESP_LOGI(TAG_BLE, "Stopping BLE scan...");
    ESP_ERROR_CHECK(esp_ble_gap_stop_scanning());
    
    scanner->scan_finished = true;
    
    return scanner->device_count;
}

uint16_t ble_scanner_get_device_count(const ble_scanner_t *scanner) {
    return scanner ? scanner->device_count : 0;
}

uint16_t ble_scanner_get_named_count(const ble_scanner_t *scanner) {
    return scanner ? scanner->named_device_count : 0;
}

bool ble_scanner_is_finished(const ble_scanner_t *scanner) {
    return scanner ? scanner->scan_finished : false;
}

const ble_device_info_t* ble_scanner_get_device(const ble_scanner_t *scanner, uint16_t index) {
    if (!scanner || index >= scanner->device_count) {
        return NULL;
    }
    return &scanner->devices[index];
}

void ble_scanner_destroy(ble_scanner_t *scanner) {
    if (scanner) {
        s_current_ble_scanner = NULL;
        ESP_LOGI(TAG_BLE, "BLE scanner destroyed");
        free(scanner);
    }
}

/*******************************************************************************
 * Wireless Manager OOP Implementation
 ******************************************************************************/

wireless_manager_t* wireless_manager_create(void) {
    wireless_manager_t *manager = (wireless_manager_t*)calloc(1, sizeof(wireless_manager_t));
    if (!manager) {
        ESP_LOGE(TAG_WIRELESS, "Failed to allocate wireless manager");
        return NULL;
    }
    
    manager->wifi = NULL;
    manager->ble = NULL;
    manager->nvs_initialized = false;
    
    ESP_LOGI(TAG_WIRELESS, "Wireless manager created");
    return manager;
}

esp_err_t wireless_manager_init(wireless_manager_t *manager) {
    if (!manager) {
        ESP_LOGE(TAG_WIRELESS, "Invalid manager object");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    manager->nvs_initialized = true;
    
    // Create WiFi scanner
    manager->wifi = wifi_scanner_create();
    if (!manager->wifi) {
        ESP_LOGE(TAG_WIRELESS, "Failed to create WiFi scanner");
        return ESP_FAIL;
    }
    
    // Create BLE scanner
    manager->ble = ble_scanner_create();
    if (!manager->ble) {
        ESP_LOGE(TAG_WIRELESS, "Failed to create BLE scanner");
        wifi_scanner_destroy(manager->wifi);
        return ESP_FAIL;
    }
    
    // Start WiFi scan task
    xTaskCreatePinnedToCore(
        (TaskFunction_t)wifi_scanner_init,
        "WiFi_Scanner",
        WIRELESS_WIFI_TASK_STACK_SIZE,
        manager->wifi,
        WIRELESS_WIFI_TASK_PRIORITY,
        NULL,
        0
    );
    
    // Start BLE scan task
    xTaskCreatePinnedToCore(
        (TaskFunction_t)ble_scanner_init,
        "BLE_Scanner",
        WIRELESS_BLE_TASK_STACK_SIZE,
        manager->ble,
        WIRELESS_BLE_TASK_PRIORITY,
        NULL,
        0
    );
    
    ESP_LOGI(TAG_WIRELESS, "Wireless manager initialized");
    return ESP_OK;
}

bool wireless_manager_all_scans_finished(const wireless_manager_t *manager) {
    if (!manager || !manager->wifi || !manager->ble) {
        return false;
    }
    return manager->wifi->scan_finished && manager->ble->scan_finished;
}

void wireless_manager_destroy(wireless_manager_t *manager) {
    if (manager) {
        if (manager->wifi) {
            wifi_scanner_destroy(manager->wifi);
        }
        if (manager->ble) {
            ble_scanner_destroy(manager->ble);
        }
        ESP_LOGI(TAG_WIRELESS, "Wireless manager destroyed");
        free(manager);
    }
}

/*******************************************************************************
 * Legacy API Implementation (for backward compatibility)
 ******************************************************************************/

uint16_t BLE_NUM = 0;
uint16_t WIFI_NUM = 0;
bool Scan_finish = false;

static bool WiFi_Scan_Finish = false;
static bool BLE_Scan_Finish = false;

// Legacy global variables for BLE scanning
static ble_device_info_t legacy_devices[WIRELESS_MAX_BLE_DEVICES];
static size_t legacy_num_discovered_devices = 0;
static size_t legacy_num_devices_with_name = 0;

/**
 * @brief Legacy helper: check if device is discovered
 */
static bool legacy_is_device_discovered(const uint8_t *addr) {
    for (size_t i = 0; i < legacy_num_discovered_devices; i++) {
        if (memcmp(legacy_devices[i].address, addr, 6) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Legacy helper: add device to list
 */
static void legacy_add_device_to_list(const uint8_t *addr) {
    if (legacy_num_discovered_devices < WIRELESS_MAX_BLE_DEVICES) {
        memcpy(legacy_devices[legacy_num_discovered_devices].address, addr, 6);
        legacy_devices[legacy_num_discovered_devices].is_valid = true;
        legacy_num_discovered_devices++;
    }
}

/**
 * @brief Legacy BLE GAP callback
 */
static void legacy_esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    static char device_name[WIRELESS_DEVICE_NAME_MAX_LEN];
    
    switch (event) {
        case ESP_GAP_BLE_SCAN_RESULT_EVT:
            if (param->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT) {
                if (!legacy_is_device_discovered(param->scan_rst.bda)) {
                    legacy_add_device_to_list(param->scan_rst.bda);
                    BLE_NUM++;
                    
                    if (extract_device_name(param->scan_rst.ble_adv, param->scan_rst.adv_data_len,
                                           device_name, sizeof(device_name))) {
                        legacy_num_devices_with_name++;
                        printf("Found device: %02X:%02X:%02X:%02X:%02X:%02X\n        Name: %s\n        RSSI: %d\r\n",
                               param->scan_rst.bda[0], param->scan_rst.bda[1],
                               param->scan_rst.bda[2], param->scan_rst.bda[3],
                               param->scan_rst.bda[4], param->scan_rst.bda[5],
                               device_name, param->scan_rst.rssi);
                        printf("\r\n");
                    } else {
                        printf("Found device: %02X:%02X:%02X:%02X:%02X:%02X\n        Name: Unknown\n        RSSI: %d\r\n",
                               param->scan_rst.bda[0], param->scan_rst.bda[1],
                               param->scan_rst.bda[2], param->scan_rst.bda[3],
                               param->scan_rst.bda[4], param->scan_rst.bda[5],
                               param->scan_rst.rssi);
                        printf("\r\n");
                    }
                }
            }
            break;
            
        case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
            ESP_LOGI("GATTC_TAG", "Scan complete. Total devices found: %d (with names: %zu)", BLE_NUM, legacy_num_devices_with_name);
            break;
            
        default:
            break;
    }
}

void Wireless_Init(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // WiFi task
    xTaskCreatePinnedToCore(
        WIFI_Init,
        "WIFI task",
        WIRELESS_WIFI_TASK_STACK_SIZE,
        NULL,
        WIRELESS_WIFI_TASK_PRIORITY,
        NULL,
        0
    );
    
    // BLE task
    xTaskCreatePinnedToCore(
        BLE_Init,
        "BLE task",
        WIRELESS_BLE_TASK_STACK_SIZE,
        NULL,
        WIRELESS_BLE_TASK_PRIORITY,
        NULL,
        0
    );
}

void WIFI_Init(void *arg) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    
    WIFI_NUM = WIFI_Scan();
    printf("WIFI:%d\r\n", WIFI_NUM);
    
    vTaskDelete(NULL);
}

uint16_t WIFI_Scan(void) {
    uint16_t ap_count = 0;
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    esp_wifi_scan_stop();
    WiFi_Scan_Finish = true;
    if (BLE_Scan_Finish) {
        Scan_finish = true;
    }
    return ap_count;
}

void BLE_Init(void *arg) {
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_err_t ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        printf("%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        printf("%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_init();
    if (ret) {
        printf("%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        printf("%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    
    ret = esp_ble_gap_register_callback(legacy_esp_gap_cb);
    if (ret) {
        printf("%s gap register error, error code = %x\n", __func__, ret);
        return;
    }
    
    BLE_Scan();
    
    vTaskDelete(NULL);
}

uint16_t BLE_Scan(void) {
    esp_ble_scan_params_t scan_params = {
        .scan_type = BLE_SCAN_TYPE_ACTIVE,
        .own_addr_type = BLE_ADDR_TYPE_RPA_PUBLIC,
        .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval = 0x50,
        .scan_window = 0x30,
        .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
    };
    ESP_ERROR_CHECK(esp_ble_gap_set_scan_params(&scan_params));
    
    printf("Starting BLE scan...\n");
    ESP_ERROR_CHECK(esp_ble_gap_start_scanning(WIRELESS_BLE_SCAN_DURATION_S));
    
    vTaskDelay(WIRELESS_BLE_SCAN_DURATION_S * 1000 / portTICK_PERIOD_MS);
    
    printf("Stopping BLE scan...\n");
    ESP_ERROR_CHECK(esp_ble_gap_stop_scanning());
    BLE_Scan_Finish = true;
    if (WiFi_Scan_Finish) {
        Scan_finish = true;
    }
    return BLE_NUM;
}
