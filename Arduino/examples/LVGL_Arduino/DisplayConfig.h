#pragma once
#include <Arduino.h>

/**
 * ST7789 LCD configuration structure
 * Used for parameter configuration of the object-oriented interface
 */
struct ST7789Config {
    // SPI pin configuration
    uint8_t pin_miso;
    uint8_t pin_mosi;
    uint8_t pin_sclk;
    uint8_t pin_cs;
    uint8_t pin_dc;
    uint8_t pin_rst;
    uint8_t pin_backlight;
    
    // SPI communication parameters
    uint32_t spi_freq;          // SPI frequency (Hz)
    
    // Display parameters
    uint16_t width;             // Screen width (pixels)
    uint16_t height;            // Screen height (pixels)
    uint8_t offset_x;           // X-axis offset
    uint8_t offset_y;           // Y-axis offset
    bool horizontal;            // true=horizontal, false=vertical
    
    // Backlight parameters
    uint16_t backlight_freq;    // PWM frequency
    uint8_t backlight_resolution; // PWM resolution (bits)
    
    /**
     * Get default configuration
     * Based on the standard configuration of the current hardware
     */
    static ST7789Config getDefault() {
        ST7789Config cfg;
        cfg.pin_miso = 5;
        cfg.pin_mosi = 6;
        cfg.pin_sclk = 7;
        cfg.pin_cs = 14;
        cfg.pin_dc = 15;
        cfg.pin_rst = 21;
        cfg.pin_backlight = 22;
        cfg.spi_freq = 80000000;  // 80MHz
        cfg.width = 172;
        cfg.height = 320;
        cfg.offset_x = 34;
        cfg.offset_y = 0;
        cfg.horizontal = true;    // HORIZONTAL = 1
        cfg.backlight_freq = 1000;
        cfg.backlight_resolution = 10;
        return cfg;
    }
};

