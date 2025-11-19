#include "Wireless.h"

// ============================================================================
// Global Variables (for LVGL UI display)
// ============================================================================
bool WIFI_Connection = 0;
uint8_t WIFI_NUM = 0;
uint8_t BLE_NUM = 0;
bool Scan_finish = 0;

// ============================================================================
// WirelessScanner Class Implementation
// ============================================================================

/**
 * Default constructor
 */
WirelessScanner::WirelessScanner()
    : config_(WirelessConfig::getDefault())
{
}

/**
 * Configuration constructor
 */
WirelessScanner::WirelessScanner(const WirelessConfig& config)
    : config_(config)
{
}

/**
 * Destructor
 */
WirelessScanner::~WirelessScanner() {
    // Clean up resources
}

/**
 * Scan for WiFi devices
 */
uint8_t WirelessScanner::scanWiFi() {
    if (!config_.wifi_enable) {
        printf("WiFi scan disabled\r\n");
        result_.wifi_success = false;
        return 0;
    }
    
    printf("/**********WiFi Scan**********/\r\n");
    
    // Set WiFi to Station mode
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(true);
    
    // Scan for networks
    int count = WiFi.scanNetworks(config_.wifi_passive, 
                                   config_.wifi_show_hidden);
    
    if (count == 0) {
        printf("No WiFi device was scanned\r\n");
        result_.wifi_success = true;
        result_.wifi_count = 0;
    } else {
        printf("Scanned %d Wi-Fi devices\r\n", count);
        result_.wifi_success = true;
        result_.wifi_count = (count > 255) ? 255 : count;
    }
    
    // Clean up and turn off WiFi
    WiFi.disconnect(true);
    WiFi.scanDelete();
    WiFi.mode(WIFI_OFF);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    printf("/*******WiFi Scan Over********/\r\n\r\n");
    return result_.wifi_count;
}

/**
 * Scan for BLE devices
 */
uint8_t WirelessScanner::scanBLE() {
    if (!config_.ble_enable) {
        printf("BLE scan disabled\r\n");
        result_.ble_success = false;
        return 0;
    }
    
    printf("/**********BLE Scan**********/\r\n");
    
    // Initialize BLE
    BLEDevice::init("ESP32");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(config_.ble_active_scan);
    
    // Start scanning
    BLEScanResults* foundDevices = pBLEScan->start(config_.ble_scan_time);
    int count = foundDevices->getCount();
    
    if (count == 0) {
        printf("No Bluetooth device was scanned\r\n");
        result_.ble_success = true;
        result_.ble_count = 0;
    } else {
        printf("Scanned %d Bluetooth devices\r\n", count);
        result_.ble_success = true;
        result_.ble_count = (count > 255) ? 255 : count;
    }
    
    // Clean up BLE resources
    pBLEScan->stop();
    pBLEScan->clearResults();
    BLEDevice::deinit(true);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    printf("/**********BLE Scan Over**********/\r\n\r\n");
    return result_.ble_count;
}

/**
 * Synchronous scan (blocking)
 */
WirelessScanner::ScanResult WirelessScanner::scanSync() {
    reset();
    
    // Scan BLE first, then WiFi
    if (config_.ble_enable) {
        scanBLE();
    }
    
    if (config_.wifi_enable) {
        scanWiFi();
    }
    
    result_.scan_finished = true;
    return result_;
}

/**
 * Asynchronous scan task
 */
void WirelessScanner::scanTask() {
    // Perform synchronous scan
    scanSync();
}

/**
 * FreeRTOS task wrapper function
 */
void WirelessScanner::scanTaskWrapper(void* parameter) {
    WirelessScanner* scanner = static_cast<WirelessScanner*>(parameter);
    if (scanner) {
        scanner->scanTask();
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    vTaskDelete(NULL);
}

/**
 * Asynchronous scan (using FreeRTOS task)
 */
void WirelessScanner::scanAsync() {
    reset();
    
    if (config_.use_async) {
        xTaskCreatePinnedToCore(
            scanTaskWrapper,
            "WirelessScan",
            config_.task_stack_size,
            this,  // Pass this pointer
            config_.task_priority,
            NULL,
            config_.task_core
        );
    } else {
        // If not using async, perform a synchronous scan directly
        scanSync();
    }
}

/**
 * Reset scan results
 */
void WirelessScanner::reset() {
    result_ = ScanResult();
}
