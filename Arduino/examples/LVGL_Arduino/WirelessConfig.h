#pragma once
#include <Arduino.h>

/**
 * Wireless scan configuration structure
 * Used for WiFi and BLE scan parameter configuration
 */
struct WirelessConfig {
    // WiFi scan parameters
    bool wifi_enable;           // Whether to enable WiFi scan
    bool wifi_show_hidden;      // Whether to show hidden networks
    bool wifi_passive;          // Whether to use passive scan
    uint32_t wifi_max_ms;       // Maximum WiFi scan duration (ms)
    
    // BLE scan parameters
    bool ble_enable;            // Whether to enable BLE scan
    bool ble_active_scan;       // Whether to use active scan
    uint32_t ble_scan_time;     // BLE scan duration (seconds)
    
    // FreeRTOS task parameters
    bool use_async;             // Whether to use async task for scan
    uint16_t task_stack_size;   // Task stack size
    uint8_t task_priority;      // Task priority
    uint8_t task_core;          // CPU core to run the task on (0 or 1)
    
    /**
     * Get default configuration
     */
    static WirelessConfig getDefault() {
        WirelessConfig cfg;
        // WiFi default config
        cfg.wifi_enable = true;
        cfg.wifi_show_hidden = false;
        cfg.wifi_passive = false;
        cfg.wifi_max_ms = 0;  // 0 means use default
        // BLE default config
        cfg.ble_enable = true;
        cfg.ble_active_scan = true;
        cfg.ble_scan_time = 5;  // 5 seconds
        // Task default config
        cfg.use_async = true;
        cfg.task_stack_size = 4096;
        cfg.task_priority = 2;
        cfg.task_core = 0;
        return cfg;
    }
};

