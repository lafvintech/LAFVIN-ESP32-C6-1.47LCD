#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include "WirelessConfig.h"

// ============================================================================
// Object-Oriented Interface
// ============================================================================

/**
 * Wireless Scanner Class
 * Responsible for scanning WiFi and BLE devices
 */
class WirelessScanner {
public:
    /**
     * Scan Result Structure
     */
    struct ScanResult {
        uint8_t wifi_count;      // Number of WiFi devices
        uint8_t ble_count;       // Number of BLE devices
        bool scan_finished;      // Is the scan complete
        bool wifi_success;       // Was the WiFi scan successful
        bool ble_success;        // Was the BLE scan successful
        
        ScanResult() : wifi_count(0), ble_count(0), 
                       scan_finished(false), wifi_success(false), 
                       ble_success(false) {}
    };
    
    /**
     * Constructor - Uses default configuration
     */
    WirelessScanner();
    
    /**
     * Constructor - Uses custom configuration
     * @param config Configuration structure
     */
    WirelessScanner(const WirelessConfig& config);
    
    /**
     * Destructor
     */
    ~WirelessScanner();
    
    // ========== Scanning Methods ==========
    
    /**
     * Synchronous scan (blocking)
     * @return Scan result
     */
    ScanResult scanSync();
    
    /**
     * Asynchronous scan (using FreeRTOS task)
     * Non-blocking, scanning is done in the background
     */
    void scanAsync();
    
    /**
     * Scan for WiFi only
     * @return Number of WiFi devices
     */
    uint8_t scanWiFi();
    
    /**
     * Scan for BLE only
     * @return Number of BLE devices
     */
    uint8_t scanBLE();
    
    // ========== Status Query Methods ==========
    
    /**
     * Get scan results
     * @return Current scan result
     */
    ScanResult getResult() const { return result_; }
    
    /**
     * Get the number of WiFi devices
     * @return Number of WiFi devices
     */
    uint8_t getWiFiCount() const { return result_.wifi_count; }
    
    /**
     * Get the number of BLE devices
     * @return Number of BLE devices
     */
    uint8_t getBLECount() const { return result_.ble_count; }
    
    /**
     * Check if the scan is complete
     * @return true=complete
     */
    bool isFinished() const { return result_.scan_finished; }
    
    /**
     * Reset scan results
     */
    void reset();

private:
    WirelessConfig config_;
    ScanResult result_;
    
    // Static task function (for FreeRTOS)
    static void scanTaskWrapper(void* parameter);
    void scanTask();
};

// ============================================================================
// Global Variables (for LVGL UI display)
// ============================================================================
extern bool WIFI_Connection;
extern uint8_t WIFI_NUM;
extern uint8_t BLE_NUM;
extern bool Scan_finish;