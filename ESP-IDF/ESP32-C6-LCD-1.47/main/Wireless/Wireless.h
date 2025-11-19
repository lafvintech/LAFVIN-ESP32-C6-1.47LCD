/**
 * @file Wireless.h
 * @brief Wireless (WiFi & BLE) Scanner Module - OOP Style
 * 
 * This module provides object-oriented WiFi and BLE scanning functionality
 * for the ESP32-C6 platform.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "esp_system.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_bt_main.h"

/*******************************************************************************
 * Configuration Constants
 ******************************************************************************/
#define WIRELESS_MAX_BLE_DEVICES        100
#define WIRELESS_BLE_SCAN_DURATION_S    5
#define WIRELESS_WIFI_TASK_STACK_SIZE   8192
#define WIRELESS_BLE_TASK_STACK_SIZE    4096
#define WIRELESS_WIFI_TASK_PRIORITY     1
#define WIRELESS_BLE_TASK_PRIORITY      2
#define WIRELESS_DEVICE_NAME_MAX_LEN    100

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

/**
 * @brief BLE device information structure
 */
typedef struct {
    uint8_t address[6];     ///< BLE device MAC address
    char name[WIRELESS_DEVICE_NAME_MAX_LEN];  ///< Device name (if available)
    int8_t rssi;            ///< Signal strength
    bool has_name;          ///< Whether device name is available
    bool is_valid;          ///< Whether this entry is valid
} ble_device_info_t;

/**
 * @brief WiFi scanner object
 */
typedef struct wifi_scanner_s {
    uint16_t ap_count;      ///< Number of APs found
    bool scan_finished;     ///< Scan completion flag
    bool is_initialized;    ///< Initialization status
} wifi_scanner_t;

/**
 * @brief BLE scanner object
 */
typedef struct ble_scanner_s {
    ble_device_info_t devices[WIRELESS_MAX_BLE_DEVICES];  ///< Discovered devices
    uint16_t device_count;          ///< Total devices found
    uint16_t named_device_count;    ///< Devices with names
    bool scan_finished;             ///< Scan completion flag
    bool is_initialized;            ///< Initialization status
} ble_scanner_t;

/**
 * @brief Wireless manager object (combines WiFi and BLE)
 */
typedef struct wireless_manager_s {
    wifi_scanner_t *wifi;   ///< WiFi scanner instance
    ble_scanner_t *ble;     ///< BLE scanner instance
    bool nvs_initialized;   ///< NVS initialization status
} wireless_manager_t;

/*******************************************************************************
 * OOP API - WiFi Scanner
 ******************************************************************************/

/**
 * @brief Create a WiFi scanner object
 * @return Pointer to created object, NULL on failure
 */
wifi_scanner_t* wifi_scanner_create(void);

/**
 * @brief Initialize WiFi scanner
 * @param scanner WiFi scanner object
 * @param arg Task argument (can be NULL)
 */
void wifi_scanner_init(wifi_scanner_t *scanner, void *arg);

/**
 * @brief Perform WiFi scan
 * @param scanner WiFi scanner object
 * @return Number of APs found
 */
uint16_t wifi_scanner_scan(wifi_scanner_t *scanner);

/**
 * @brief Get WiFi scan results
 * @param scanner WiFi scanner object
 * @return Number of APs found
 */
uint16_t wifi_scanner_get_ap_count(const wifi_scanner_t *scanner);

/**
 * @brief Check if WiFi scan is finished
 * @param scanner WiFi scanner object
 * @return true if finished, false otherwise
 */
bool wifi_scanner_is_finished(const wifi_scanner_t *scanner);

/**
 * @brief Destroy WiFi scanner object
 * @param scanner WiFi scanner object
 */
void wifi_scanner_destroy(wifi_scanner_t *scanner);

/*******************************************************************************
 * OOP API - BLE Scanner
 ******************************************************************************/

/**
 * @brief Create a BLE scanner object
 * @return Pointer to created object, NULL on failure
 */
ble_scanner_t* ble_scanner_create(void);

/**
 * @brief Initialize BLE scanner
 * @param scanner BLE scanner object
 * @param arg Task argument (can be NULL)
 */
void ble_scanner_init(ble_scanner_t *scanner, void *arg);

/**
 * @brief Perform BLE scan
 * @param scanner BLE scanner object
 * @return Number of BLE devices found
 */
uint16_t ble_scanner_scan(ble_scanner_t *scanner);

/**
 * @brief Get BLE scan results
 * @param scanner BLE scanner object
 * @return Number of BLE devices found
 */
uint16_t ble_scanner_get_device_count(const ble_scanner_t *scanner);

/**
 * @brief Get number of BLE devices with names
 * @param scanner BLE scanner object
 * @return Number of named devices
 */
uint16_t ble_scanner_get_named_count(const ble_scanner_t *scanner);

/**
 * @brief Check if BLE scan is finished
 * @param scanner BLE scanner object
 * @return true if finished, false otherwise
 */
bool ble_scanner_is_finished(const ble_scanner_t *scanner);

/**
 * @brief Get specific BLE device info
 * @param scanner BLE scanner object
 * @param index Device index (0 to device_count-1)
 * @return Pointer to device info, NULL if invalid index
 */
const ble_device_info_t* ble_scanner_get_device(const ble_scanner_t *scanner, uint16_t index);

/**
 * @brief Destroy BLE scanner object
 * @param scanner BLE scanner object
 */
void ble_scanner_destroy(ble_scanner_t *scanner);

/*******************************************************************************
 * OOP API - Wireless Manager
 ******************************************************************************/

/**
 * @brief Create a wireless manager object
 * @return Pointer to created object, NULL on failure
 */
wireless_manager_t* wireless_manager_create(void);

/**
 * @brief Initialize wireless manager (NVS, WiFi and BLE tasks)
 * @param manager Wireless manager object
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t wireless_manager_init(wireless_manager_t *manager);

/**
 * @brief Check if all scans are finished
 * @param manager Wireless manager object
 * @return true if both WiFi and BLE scans are finished
 */
bool wireless_manager_all_scans_finished(const wireless_manager_t *manager);

/**
 * @brief Destroy wireless manager object
 * @param manager Wireless manager object
 */
void wireless_manager_destroy(wireless_manager_t *manager);

/*******************************************************************************
 * Legacy API (for backward compatibility)
 ******************************************************************************/

/// Legacy global variables
extern uint16_t BLE_NUM;
extern uint16_t WIFI_NUM;
extern bool Scan_finish;

/// Legacy functions
void Wireless_Init(void);
void WIFI_Init(void *arg);
uint16_t WIFI_Scan(void);
void BLE_Init(void *arg);
uint16_t BLE_Scan(void);

#ifdef __cplusplus
}
#endif