#pragma once
#include <Arduino.h>

/**
 * SD card configuration structure
 * Used for parameter configuration of the object-oriented interface
 */
struct SDCardConfig {
    // SPI pin configuration
    uint8_t cs_pin;           // CS pin
    
    // SPI communication parameters
    uint32_t spi_freq;        // SPI frequency (Hz)
    
    // Mount parameters
    const char* mount_point;  // Mount path
    
    /**
     * Get default configuration
     * Based on the standard configuration of the current hardware
     */
    static SDCardConfig getDefault() {
        SDCardConfig cfg;
        cfg.cs_pin = 4;                  // Default CS pin
        cfg.spi_freq = 80000000;         // 80MHz
        cfg.mount_point = "/sd";         // Default mount path
        return cfg;
    }
};

