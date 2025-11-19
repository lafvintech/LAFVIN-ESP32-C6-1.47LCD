#pragma once
#include <Arduino.h>

/**
 * SD Card Configuration Structure
 * Parameter configuration for object-oriented interface
 */
struct SDCardConfig {
    // SPI pin configuration
    uint8_t cs_pin;           // CS chip select pin
    
    // SPI communication parameters
    uint32_t spi_freq;        // SPI frequency (Hz)
    
    // Mount parameters
    const char* mount_point;  // Mount point path
    
    /**
     * Get default configuration
     * Standard configuration based on current hardware
     */
    static SDCardConfig getDefault() {
        SDCardConfig cfg;
        cfg.cs_pin = 4;                  // Default CS pin
        cfg.spi_freq = 80000000;         // 80MHz
        cfg.mount_point = "/sd";         // Default mount point
        return cfg;
    }
};

