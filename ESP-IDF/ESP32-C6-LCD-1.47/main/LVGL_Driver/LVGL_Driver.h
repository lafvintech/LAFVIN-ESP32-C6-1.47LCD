/**
 * @file LVGL_Driver.h
 * @brief LVGL Display Driver - Object-Oriented Interface
 * @author Refactored for better modularity and configurability
 * @date 2025
 *
 * This module provides an object-oriented interface for integrating LVGL
 * graphics library with ST7789 LCD driver on ESP32-C6.
 *
 * Features:
 * - Configurable buffer allocation (internal RAM / SPIRAM)
 * - Double/single buffer support
 * - Display rotation and mirroring
 * - Automatic tick timer management
 * - Clean object lifecycle (create/init/destroy)
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "lvgl.h"
#include "demos/lv_demos.h"
#include "ST7789.h"  // Include ST7789 driver for st7789_device_t type

/******************************************************************************
 * Configuration Constants
 ******************************************************************************/

#define LVGL_DEFAULT_BUF_LINES      20      // Default buffer size (lines)
#define LVGL_TICK_PERIOD_MS         2       // LVGL tick period in milliseconds

/******************************************************************************
 * Type Definitions - Object-Oriented Structures
 ******************************************************************************/

/**
 * @brief Buffer allocation strategy
 */
typedef enum {
    LVGL_BUF_ALLOC_INTERNAL,    // Allocate from internal RAM
    LVGL_BUF_ALLOC_SPIRAM,      // Allocate from SPIRAM (if available)
    LVGL_BUF_ALLOC_DMA          // Allocate from DMA-capable memory
} lvgl_buffer_alloc_t;

/**
 * @brief LVGL Driver Configuration
 *
 * Configuration parameters for LVGL driver initialization
 */
typedef struct {
    // Display resolution (auto-filled from LCD driver)
    uint16_t hor_res;
    uint16_t ver_res;

    // Buffer configuration
    uint16_t buf_lines;                 // Buffer size in lines (e.g., 20)
    bool use_double_buffer;             // Use double buffering
    lvgl_buffer_alloc_t buf_alloc;      // Buffer allocation strategy

    // Display settings
    bool full_refresh;                  // Always redraw entire screen
    uint8_t rotation;                   // Initial rotation (0/90/180/270)

    // Associated LCD device
    st7789_device_t *lcd_device;        // ST7789 LCD driver instance

    // Tick timer settings
    uint16_t tick_period_ms;            // Tick period in milliseconds
} lvgl_config_t;

/**
 * @brief LVGL Driver Device Object
 *
 * Main LVGL driver object encapsulating all LVGL operations
 */
typedef struct {
    // Configuration
    lvgl_config_t config;

    // LVGL display objects
    lv_disp_t *display;
    lv_disp_draw_buf_t draw_buf;
    lv_disp_drv_t disp_drv;

    // Display buffers
    lv_color_t *buf1;
    lv_color_t *buf2;
    size_t buf_size;                    // Buffer size in pixels

    // Tick timer
    esp_timer_handle_t tick_timer;

    // State
    bool is_initialized;
} lvgl_driver_t;

/******************************************************************************
 * Public API - Object Methods
 ******************************************************************************/

/**
 * @brief Get default LVGL driver configuration
 *
 * @param lcd_device Associated ST7789 LCD device (required)
 * @return Default configuration structure
 */
lvgl_config_t lvgl_get_default_config(st7789_device_t *lcd_device);

/**
 * @brief Create LVGL driver object
 *
 * @param config Pointer to configuration structure
 * @return Pointer to created driver object, NULL on failure
 */
lvgl_driver_t* lvgl_driver_create(const lvgl_config_t *config);

/**
 * @brief Initialize LVGL driver
 *
 * This function:
 * - Initializes LVGL library
 * - Allocates display buffers
 * - Registers display driver
 * - Starts tick timer
 *
 * @param driver Pointer to driver object
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t lvgl_driver_init(lvgl_driver_t *driver);

/**
 * @brief Destroy LVGL driver and free all resources
 *
 * @param driver Pointer to driver object
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t lvgl_driver_destroy(lvgl_driver_t *driver);

/**
 * @brief Set display rotation
 *
 * @param driver Pointer to driver object
 * @param rotation Rotation angle (0, 90, 180, 270 degrees)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t lvgl_driver_set_rotation(lvgl_driver_t *driver, uint16_t rotation);

/**
 * @brief Get current LVGL display object
 *
 * @param driver Pointer to driver object
 * @return LVGL display object pointer
 */
lv_disp_t* lvgl_driver_get_display(lvgl_driver_t *driver);

/**
 * @brief Task handler - must be called periodically
 *
 * Call this in your main loop or dedicated task.
 * Recommended interval: 5-10ms
 *
 * @param driver Pointer to driver object (can be NULL if using legacy API)
 */
void lvgl_driver_task_handler(lvgl_driver_t *driver);

/******************************************************************************
 * Callback Functions (Internal)
 ******************************************************************************/

/**
 * @brief Tick timer callback
 *
 * @param arg Timer argument (driver object)
 */
void lvgl_tick_callback(void *arg);

/**
 * @brief Display flush callback
 *
 * @param drv LVGL display driver
 * @param area Display area to flush
 * @param color_map Color data
 */
void lvgl_flush_callback(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);

/**
 * @brief Display rotation update callback
 *
 * @param drv LVGL display driver
 */
void lvgl_rotation_callback(lv_disp_drv_t *drv);

/******************************************************************************
 * Usage Example (OOP API)
 ******************************************************************************/

/*
// Example 1: Basic initialization with default config
st7789_config_t lcd_config = st7789_get_default_config();
st7789_device_t *lcd = st7789_create(&lcd_config);
st7789_init(lcd);
st7789_backlight_set(lcd, 100);  // Set brightness to 100%

lvgl_config_t lvgl_config = lvgl_get_default_config(lcd);
lvgl_driver_t *lvgl = lvgl_driver_create(&lvgl_config);
lvgl_driver_init(lvgl);

// In main loop:
while (1) {
    lvgl_driver_task_handler(lvgl);
    vTaskDelay(pdMS_TO_TICKS(10));
}

// Example 2: Custom configuration with SPIRAM buffers
st7789_config_t lcd_config = st7789_get_default_config();
lcd_config.initial_brightness = 80;         // Set initial brightness
st7789_device_t *lcd = st7789_create(&lcd_config);
st7789_init(lcd);

lvgl_config_t lvgl_config = lvgl_get_default_config(lcd);
lvgl_config.buf_lines = 40;                      // Larger buffer
lvgl_config.use_double_buffer = true;
lvgl_config.buf_alloc = LVGL_BUF_ALLOC_SPIRAM;   // Use external RAM
lvgl_config.rotation = 90;                       // Landscape mode

lvgl_driver_t *lvgl = lvgl_driver_create(&lvgl_config);
lvgl_driver_init(lvgl);

// Example 3: Runtime rotation and backlight control
lvgl_driver_set_rotation(lvgl, 180);        // Flip display
st7789_backlight_fade(lcd, 50, 1000);       // Fade to 50% in 1 second
*/

#ifdef __cplusplus
}
#endif
