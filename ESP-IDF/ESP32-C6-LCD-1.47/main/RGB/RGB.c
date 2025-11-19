/**
 * @file RGB.c
 * @brief Enhanced RGB LED Controller Implementation
 * @author ESP32-C6 Development Team
 * @date 2025
 * @note Uses HSV color model and multiple effect modes
 */

#include "RGB.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include <string.h>

static const char *TAG = "RGB_LED";

/******************************************************************************
 * Private Variables
 ******************************************************************************/

static led_strip_handle_t s_led_strip = NULL;
static TaskHandle_t s_effect_task_handle = NULL;

// Effect control state
static struct {
    rgb_effect_t current_effect;
    uint16_t speed_ms;
    uint8_t brightness;
    bool is_running;
    bool is_paused;
    uint16_t effect_step;
} s_rgb_state = {
    .current_effect = RGB_EFFECT_RAINBOW,
    .speed_ms = RGB_DEFAULT_SPEED,
    .brightness = RGB_DEFAULT_BRIGHTNESS,
    .is_running = false,
    .is_paused = false,
    .effect_step = 0
};

/******************************************************************************
 * Private Function Prototypes
 ******************************************************************************/

static rgb_color_t hsv_to_rgb(hsv_color_t hsv);
static void rgb_effect_task(void *arg);
static void effect_rainbow(uint16_t step);
static void effect_breathe(uint16_t step);
static void effect_blink(uint16_t step);
static void effect_wave(uint16_t step);

/******************************************************************************
 * Public API Implementation
 ******************************************************************************/

esp_err_t RGB_Init(void)
{
    ESP_LOGI(TAG, "Initializing RGB LED controller");
    
    // LED strip configuration
    led_strip_config_t strip_config = {
        .strip_gpio_num = RGB_LED_GPIO,
        .max_leds = 1,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };
    
    // RMT configuration
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    
    esp_err_t ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &s_led_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LED strip: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Clear LED
    led_strip_clear(s_led_strip);
    
    ESP_LOGI(TAG, "RGB LED initialized successfully");
    return ESP_OK;
}

esp_err_t RGB_SetColor(uint8_t red, uint8_t green, uint8_t blue)
{
    if (s_led_strip == NULL) {
        ESP_LOGE(TAG, "RGB LED not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Apply brightness scaling
    red = (red * s_rgb_state.brightness) / 100;
    green = (green * s_rgb_state.brightness) / 100;
    blue = (blue * s_rgb_state.brightness) / 100;
    
    led_strip_set_pixel(s_led_strip, 0, red, green, blue);
    return led_strip_refresh(s_led_strip);
}

esp_err_t RGB_SetColorHSV(uint16_t hue, uint8_t saturation, uint8_t value)
{
    hsv_color_t hsv = {
        .hue = hue % 360,
        .saturation = (saturation > 100) ? 100 : saturation,
        .value = (value > 100) ? 100 : value
    };
    
    rgb_color_t rgb = hsv_to_rgb(hsv);
    return RGB_SetColor(rgb.red, rgb.green, rgb.blue);
}

esp_err_t RGB_SetEffect(rgb_effect_t effect)
{
    s_rgb_state.current_effect = effect;
    s_rgb_state.effect_step = 0;
    ESP_LOGI(TAG, "Effect set to mode: %d", effect);
    return ESP_OK;
}

esp_err_t RGB_SetSpeed(uint16_t speed_ms)
{
    if (speed_ms < 10 || speed_ms > 1000) {
        ESP_LOGW(TAG, "Speed out of range (10-1000ms), clamping");
        speed_ms = (speed_ms < 10) ? 10 : 1000;
    }
    
    s_rgb_state.speed_ms = speed_ms;
    ESP_LOGI(TAG, "Speed set to %d ms", speed_ms);
    return ESP_OK;
}

esp_err_t RGB_SetBrightness(uint8_t brightness)
{
    if (brightness > 100) {
        brightness = 100;
    }
    
    s_rgb_state.brightness = brightness;
    ESP_LOGI(TAG, "Brightness set to %d%%", brightness);
    return ESP_OK;
}

void RGB_Pause(void)
{
    s_rgb_state.is_paused = true;
    ESP_LOGI(TAG, "Effect paused");
}

void RGB_Resume(void)
{
    s_rgb_state.is_paused = false;
    ESP_LOGI(TAG, "Effect resumed");
}

void RGB_Stop(void)
{
    s_rgb_state.is_running = false;
    
    if (s_effect_task_handle != NULL) {
        vTaskDelete(s_effect_task_handle);
        s_effect_task_handle = NULL;
    }
    
    RGB_Off();
    ESP_LOGI(TAG, "Effect stopped");
}

void RGB_Off(void)
{
    if (s_led_strip != NULL) {
        led_strip_clear(s_led_strip);
    }
}

void RGB_Example(void)
{
    if (s_rgb_state.is_running) {
        ESP_LOGW(TAG, "Effect already running");
        return;
    }
    
    s_rgb_state.is_running = true;
    s_rgb_state.is_paused = false;
    s_rgb_state.effect_step = 0;
    
    ESP_LOGI(TAG, "Starting RGB effect task");
    
    xTaskCreatePinnedToCore(
        rgb_effect_task,
        "RGB_Effect",
        4096,
        NULL,
        4,
        &s_effect_task_handle,
        0
    );
}

/******************************************************************************
 * Legacy API (Backward Compatibility)
 ******************************************************************************/

void Set_RGB(uint8_t red_val, uint8_t green_val, uint8_t blue_val)
{
    RGB_SetColor(red_val, green_val, blue_val);
}

/******************************************************************************
 * Private Functions - Color Conversion
 ******************************************************************************/

/**
 * @brief Convert HSV to RGB color
 * @note HSV provides more intuitive color control than RGB
 */
static rgb_color_t hsv_to_rgb(hsv_color_t hsv)
{
    rgb_color_t rgb = {0, 0, 0};
    
    if (hsv.saturation == 0) {
        // Grayscale
        uint8_t v = (hsv.value * 255) / 100;
        rgb.red = rgb.green = rgb.blue = v;
        return rgb;
    }
    
    float h = hsv.hue / 60.0f;
    float s = hsv.saturation / 100.0f;
    float v = hsv.value / 100.0f;
    
    int i = (int)h;
    float f = h - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));
    
    float r, g, b;
    
    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
        default: r = g = b = 0; break;
    }
    
    rgb.red = (uint8_t)(r * 255);
    rgb.green = (uint8_t)(g * 255);
    rgb.blue = (uint8_t)(b * 255);
    
    return rgb;
}

/******************************************************************************
 * Private Functions - Effect Implementations
 ******************************************************************************/

/**
 * @brief Main effect task - runs selected effect in loop
 */
static void rgb_effect_task(void *arg)
{
    ESP_LOGI(TAG, "RGB effect task started");
    
    while (s_rgb_state.is_running) {
        if (!s_rgb_state.is_paused) {
            // Execute current effect
            switch (s_rgb_state.current_effect) {
                case RGB_EFFECT_RAINBOW:
                    effect_rainbow(s_rgb_state.effect_step);
                    break;
                
                case RGB_EFFECT_BREATHE:
                    effect_breathe(s_rgb_state.effect_step);
                    break;
                
                case RGB_EFFECT_BLINK:
                    effect_blink(s_rgb_state.effect_step);
                    break;
                
                case RGB_EFFECT_WAVE:
                    effect_wave(s_rgb_state.effect_step);
                    break;
                
                case RGB_EFFECT_SOLID:
                    // Solid color - just set once and wait
                    RGB_SetColorHSV(120, 100, 100); // Green by default
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    continue;
                
                default:
                    effect_rainbow(s_rgb_state.effect_step);
                    break;
            }
            
            s_rgb_state.effect_step++;
        }
        
        vTaskDelay(pdMS_TO_TICKS(s_rgb_state.speed_ms));
    }
    
    ESP_LOGI(TAG, "RGB effect task ended");
    s_effect_task_handle = NULL;
    vTaskDelete(NULL);
}

/**
 * @brief Rainbow effect - smooth color wheel transition
 */
static void effect_rainbow(uint16_t step)
{
    // Generate smooth rainbow using HSV
    // Full cycle = 360 steps (one complete hue rotation)
    uint16_t hue = (step * 2) % 360;  // Speed: 2 degrees per step
    RGB_SetColorHSV(hue, 100, 100);
}

/**
 * @brief Breathing effect - fade in and out
 */
static void effect_breathe(uint16_t step)
{
    // Sine wave breathing pattern
    // Period = 100 steps for smooth breathing
    float phase = (step % 100) / 100.0f * 2.0f * M_PI;
    uint8_t brightness = (uint8_t)(50.0f + 50.0f * sinf(phase));
    
    // Use cyan color for breathing
    RGB_SetColorHSV(180, 100, brightness);
}

/**
 * @brief Blink effect - simple on/off
 */
static void effect_blink(uint16_t step)
{
    if ((step % 2) == 0) {
        RGB_SetColorHSV(60, 100, 100);  // Yellow - ON
    } else {
        RGB_Off();  // OFF
    }
}

/**
 * @brief Wave effect - cycling through primary colors
 */
static void effect_wave(uint16_t step)
{
    // Cycle through primary colors with smooth transitions
    // Red -> Green -> Blue -> Red
    const uint16_t colors[] = {0, 120, 240};  // Red, Green, Blue in HSV
    const uint8_t num_colors = 3;
    
    uint16_t cycle_position = step % (num_colors * 60);
    uint8_t color_index = cycle_position / 60;
    uint8_t transition = cycle_position % 60;
    
    uint16_t current_hue = colors[color_index];
    uint16_t next_hue = colors[(color_index + 1) % num_colors];
    
    // Linear interpolation between colors
    uint16_t hue = current_hue + (next_hue - current_hue) * transition / 60;
    
    RGB_SetColorHSV(hue, 100, 100);
}
