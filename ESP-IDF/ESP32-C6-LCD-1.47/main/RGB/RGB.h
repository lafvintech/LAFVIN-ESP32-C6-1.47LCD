/**
 * @file RGB.h
 * @brief Enhanced RGB LED Controller with Multiple Effects
 * @author ESP32-C6 Development Team
 * @date 2025
 */

#pragma once

#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Configuration
 ******************************************************************************/

#define RGB_LED_GPIO           8
#define RGB_DEFAULT_BRIGHTNESS 100  // 0-100%
#define RGB_DEFAULT_SPEED      20   // milliseconds per step

/******************************************************************************
 * Type Definitions
 ******************************************************************************/

/**
 * @brief RGB LED effect modes
 */
typedef enum {
    RGB_EFFECT_RAINBOW,     ///< Smooth rainbow color transition
    RGB_EFFECT_BREATHE,     ///< Breathing effect (fade in/out)
    RGB_EFFECT_BLINK,       ///< Simple blink on/off
    RGB_EFFECT_SOLID,       ///< Solid color
    RGB_EFFECT_WAVE,        ///< Wave effect with multiple colors
    RGB_EFFECT_CUSTOM       ///< Custom user-defined effect
} rgb_effect_t;

/**
 * @brief HSV color model (Hue, Saturation, Value)
 */
typedef struct {
    uint16_t hue;           ///< Hue: 0-360 degrees
    uint8_t saturation;     ///< Saturation: 0-100%
    uint8_t value;          ///< Value (brightness): 0-100%
} hsv_color_t;

/**
 * @brief RGB color model
 */
typedef struct {
    uint8_t red;            ///< Red: 0-255
    uint8_t green;          ///< Green: 0-255
    uint8_t blue;           ///< Blue: 0-255
} rgb_color_t;

/******************************************************************************
 * Public API
 ******************************************************************************/

/**
 * @brief Initialize RGB LED controller
 * @return ESP_OK on success
 */
esp_err_t RGB_Init(void);

/**
 * @brief Set RGB LED to specific color
 * @param red Red value (0-255)
 * @param green Green value (0-255)
 * @param blue Blue value (0-255)
 * @return ESP_OK on success
 */
esp_err_t RGB_SetColor(uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief Set RGB LED using HSV color model
 * @param hue Hue (0-360)
 * @param saturation Saturation (0-100)
 * @param value Brightness (0-100)
 * @return ESP_OK on success
 */
esp_err_t RGB_SetColorHSV(uint16_t hue, uint8_t saturation, uint8_t value);

/**
 * @brief Start RGB LED effect with default settings
 */
void RGB_Example(void);

/**
 * @brief Set LED effect mode
 * @param effect Effect type
 * @return ESP_OK on success
 */
esp_err_t RGB_SetEffect(rgb_effect_t effect);

/**
 * @brief Set effect speed
 * @param speed_ms Milliseconds per step (10-1000)
 * @return ESP_OK on success
 */
esp_err_t RGB_SetSpeed(uint16_t speed_ms);

/**
 * @brief Set LED brightness
 * @param brightness Brightness percentage (0-100)
 * @return ESP_OK on success
 */
esp_err_t RGB_SetBrightness(uint8_t brightness);

/**
 * @brief Pause LED effect animation
 */
void RGB_Pause(void);

/**
 * @brief Resume LED effect animation
 */
void RGB_Resume(void);

/**
 * @brief Stop LED effect and turn off
 */
void RGB_Stop(void);

/**
 * @brief Turn off LED immediately
 */
void RGB_Off(void);

/******************************************************************************
 * Legacy API (for backward compatibility)
 ******************************************************************************/

/**
 * @brief Legacy function - set RGB color (deprecated, use RGB_SetColor)
 */
void Set_RGB(uint8_t red_val, uint8_t green_val, uint8_t blue_val);

#ifdef __cplusplus
}
#endif