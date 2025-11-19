#pragma once
#include <Arduino.h>
#include <SPI.h>
#include "DisplayConfig.h"

// ============================================================================
// Object-Oriented Interface
// ============================================================================

/**
 * ST7789 LCD Driver Class
 * Provides modern C++ interface, supports configuration injection and object management
 */
class ST7789Display {
public:
    /**
     * Constructor - Uses default config
     */
    ST7789Display();
    /**
     * Constructor - Uses custom config
     * @param config Config structure
     */
    ST7789Display(const ST7789Config& config);
    /**
     * Destructor
     */
    ~ST7789Display();

    // ========== Initialization Methods ==========

    /**
     * Initialize display
     * @return true=success, false=failure
     */
    bool begin();

    // ========== Display Operation Methods ==========

    /**
     * Set drawing window
     * @param x1 Start X coordinate
     * @param y1 Start Y coordinate
     * @param x2 End X coordinate
     * @param y2 End Y coordinate
     */
    void setWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    /**
     * Draw pixel buffer to the specified area
     * @param x1 Start X coordinate
     * @param y1 Start Y coordinate
     * @param x2 End X coordinate
     * @param y2 End Y coordinate
     * @param buffer Pixel data buffer (RGB565 format)
     */
    void drawPixelBuffer(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t* buffer);

    /**
     * Clear screen (fill with single color)
     * @param color RGB565 color value
     */
    void clearScreen(uint16_t color);

    // ========== Backlight Control Methods ==========

    /**
     * Set backlight brightness
     * @param brightness value 0-100
     */
    void setBacklight(uint8_t brightness);

    // ========== Property Accessors ==========

    /**
     * Get screen width
     * @return Width (pixels)
     */
    uint16_t width() const { return _width; }
    /**
     * Get screen height
     * @return Height (pixels)
     */
    uint16_t height() const { return _height; }
    /**
     * Check if initialized
     * @return true=initialized
     */
    bool isInitialized() const { return _initialized; }

private:
    // ========== Configuration Parameters ==========
    uint8_t _pin_cs;
    uint8_t _pin_dc;
    uint8_t _pin_rst;
    uint8_t _pin_backlight;
    uint16_t _width;
    uint16_t _height;
    uint8_t _offset_x;
    uint8_t _offset_y;
    bool _horizontal;
    uint32_t _spi_freq;
    uint16_t _backlight_freq;
    uint8_t _backlight_resolution;

    bool _initialized;

    // ========== Private Hardware Operation Methods ==========
    void spiInit();
    void writeCommand(uint8_t cmd);
    void writeData(uint8_t data);
    void writeData16(uint16_t data);
    void writeDataBytes(uint8_t* data, uint32_t len);
    void hardwareReset();
    void initRegisters();
    void backlightInit();
};

// ============================================================================
// Constants Definition
// ============================================================================

// Display size
#define LCD_WIDTH   172
#define LCD_HEIGHT  320

// Hardware pin definitions
#define EXAMPLE_PIN_NUM_MISO           5
#define EXAMPLE_PIN_NUM_MOSI           6
#define EXAMPLE_PIN_NUM_SCLK           7
#define EXAMPLE_PIN_NUM_LCD_CS         14
#define EXAMPLE_PIN_NUM_LCD_DC         15
#define EXAMPLE_PIN_NUM_LCD_RST        21
#define EXAMPLE_PIN_NUM_BK_LIGHT       22

// SPI and backlight configuration
#define SPIFreq      80000000
#define Frequency    1000
#define Resolution   10

// Screen orientation
#define VERTICAL     0
#define HORIZONTAL   1

// Screen offset
#define Offset_X     34
#define Offset_Y     0
