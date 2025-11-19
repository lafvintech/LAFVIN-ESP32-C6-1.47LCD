/**
 * @file ST7789.h
 * @brief ST7789 LCD Driver - Object-Oriented Interface
 * @author Refactored for better modularity
 * @date 2025
 * 
 * This module provides an object-oriented interface for controlling
 * ST7789T LCD panels with integrated backlight management.
 */

 #pragma once

 #ifdef __cplusplus
 extern "C" {
 #endif
 
 #include <stdio.h>
 #include <stdbool.h>
 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 #include "esp_timer.h"
 #include "esp_lcd_panel_io.h"
 #include "esp_lcd_panel_vendor.h"
 #include "esp_lcd_panel_ops.h"
 #include "driver/gpio.h"
 #include "driver/spi_master.h"
 #include "esp_err.h"
 #include "esp_log.h"
 #include "driver/ledc.h"

 #include "Vernon_ST7789T.h"

 /******************************************************************************
  * Hardware Configuration Constants
  ******************************************************************************/
 
 // SPI Configuration
 #define ST7789_SPI_HOST             SPI2_HOST
 #define ST7789_PIXEL_CLOCK_HZ       (12 * 1000 * 1000)  // 12MHz
 
 // GPIO Pin Definitions
 #define ST7789_PIN_SCLK             7
 #define ST7789_PIN_MOSI             6
 #define ST7789_PIN_CS               14
 #define ST7789_PIN_DC               15
 #define ST7789_PIN_RST              21
 #define ST7789_PIN_BACKLIGHT        22
 
 // Display Resolution
 #define ST7789_H_RES                172
 #define ST7789_V_RES                320
 #define ST7789_OFFSET_X             34
 #define ST7789_OFFSET_Y             0
 
 // SPI Command/Parameter Bits
 #define ST7789_CMD_BITS             8
 #define ST7789_PARAM_BITS           8
 
 // Backlight PWM Configuration
 #define ST7789_BL_TIMER             LEDC_TIMER_0
 #define ST7789_BL_MODE              LEDC_LOW_SPEED_MODE
 #define ST7789_BL_CHANNEL           LEDC_CHANNEL_0
 #define ST7789_BL_RESOLUTION        LEDC_TIMER_13_BIT
 #define ST7789_BL_FREQUENCY         5000  // 5kHz
 #define ST7789_BL_MAX_DUTY          ((1 << ST7789_BL_RESOLUTION) - 1)
 
 /******************************************************************************
  * Type Definitions - Object-Oriented Structures
  ******************************************************************************/
 
 /**
  * @brief Backlight Controller Object
  * 
  * Encapsulates backlight PWM control functionality
  */
 typedef struct {
     gpio_num_t gpio_num;                    // GPIO pin number
     ledc_channel_t channel;                 // LEDC channel
     ledc_mode_t speed_mode;                 // LEDC speed mode
     ledc_timer_t timer;                     // LEDC timer
     uint8_t current_brightness;             // Current brightness (0-100)
     bool is_initialized;                    // Initialization status
 } st7789_backlight_t;
 
 /**
  * @brief LCD Panel Configuration
  * 
  * Configuration parameters for LCD initialization
  */
 typedef struct {
     // SPI Configuration
     spi_host_device_t spi_host;
     int pin_sclk;
     int pin_mosi;
     int pin_cs;
     int pin_dc;
     int pin_rst;
     int pixel_clock_hz;
     
     // Display Properties
     uint16_t h_res;
     uint16_t v_res;
     uint16_t offset_x;
     uint16_t offset_y;
     lcd_color_rgb_endian_t rgb_endian;
     uint8_t bits_per_pixel;
     
     // Backlight Configuration
     int pin_backlight;
     uint8_t initial_brightness;
 } st7789_config_t;
 
 /**
  * @brief LCD Device Object
  * 
  * Main LCD controller object encapsulating all LCD operations
  */
 typedef struct {
     esp_lcd_panel_handle_t panel_handle;    // Low-level panel handle
     esp_lcd_panel_io_handle_t io_handle;    // Panel IO handle
     st7789_backlight_t backlight;           // Backlight controller
     st7789_config_t config;                 // Configuration
     bool is_initialized;                    // Initialization status
 } st7789_device_t;
 
 /******************************************************************************
  * Public API - Object Methods
  ******************************************************************************/
 
 /**
  * @brief Create default LCD configuration
  * 
  * @return Default configuration structure
  */
 st7789_config_t st7789_get_default_config(void);
 
 /**
  * @brief Create and initialize LCD device object
  * 
  * @param config Pointer to configuration structure
  * @return Pointer to created device object, NULL on failure
  */
 st7789_device_t* st7789_create(const st7789_config_t *config);
 
 /**
  * @brief Initialize LCD device
  * 
  * @param device Pointer to device object
  * @return ESP_OK on success, error code otherwise
  */
 esp_err_t st7789_init(st7789_device_t *device);
 
 /**
  * @brief Destroy LCD device object and free resources
  * 
  * @param device Pointer to device object
  * @return ESP_OK on success, error code otherwise
  */
 esp_err_t st7789_destroy(st7789_device_t *device);
 
 /**
  * @brief Turn display on/off
  * 
  * @param device Pointer to device object
  * @param on true to turn on, false to turn off
  * @return ESP_OK on success, error code otherwise
  */
 esp_err_t st7789_display_on_off(st7789_device_t *device, bool on);
 
 /**
  * @brief Set display mirror mode
  * 
  * @param device Pointer to device object
  * @param mirror_x Mirror horizontally
  * @param mirror_y Mirror vertically
  * @return ESP_OK on success, error code otherwise
  */
 esp_err_t st7789_set_mirror(st7789_device_t *device, bool mirror_x, bool mirror_y);
 
 /**
  * @brief Set display rotation
  * 
  * @param device Pointer to device object
  * @param swap_xy Swap X and Y axes
  * @return ESP_OK on success, error code otherwise
  */
 esp_err_t st7789_set_rotation(st7789_device_t *device, bool swap_xy);
 
 /**
  * @brief Reset LCD panel
  * 
  * @param device Pointer to device object
  * @return ESP_OK on success, error code otherwise
  */
 esp_err_t st7789_reset(st7789_device_t *device);
 
 /**
  * @brief Get panel handle (for LVGL integration)
  * 
  * @param device Pointer to device object
  * @return Panel handle
  */
 esp_lcd_panel_handle_t st7789_get_panel_handle(st7789_device_t *device);
 
 /******************************************************************************
  * Backlight Control API
  ******************************************************************************/
 
 /**
  * @brief Set backlight brightness
  * 
  * @param device Pointer to device object
  * @param brightness Brightness level (0-100)
  * @return ESP_OK on success, error code otherwise
  */
 esp_err_t st7789_backlight_set(st7789_device_t *device, uint8_t brightness);
 
 /**
  * @brief Get current backlight brightness
  * 
  * @param device Pointer to device object
  * @return Current brightness level (0-100)
  */
 uint8_t st7789_backlight_get(st7789_device_t *device);
 
 /**
  * @brief Enable/disable backlight
  * 
  * @param device Pointer to device object
  * @param enable true to enable, false to disable
  * @return ESP_OK on success, error code otherwise
  */
 esp_err_t st7789_backlight_enable(st7789_device_t *device, bool enable);
 
 /**
  * @brief Fade backlight to target brightness
  * 
  * @param device Pointer to device object
  * @param target_brightness Target brightness (0-100)
  * @param duration_ms Fade duration in milliseconds
  * @return ESP_OK on success, error code otherwise
  */
 esp_err_t st7789_backlight_fade(st7789_device_t *device, uint8_t target_brightness, uint32_t duration_ms);
 
 #ifdef __cplusplus
 }
 #endif