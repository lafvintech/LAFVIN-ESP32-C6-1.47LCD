/**
 * @file ST7789.c
 * @brief ST7789 LCD Driver Implementation - Object-Oriented Design
 * @author Refactored for better modularity
 * @date 2025
 */

 #include "ST7789.h"
 #include <stdlib.h>
 #include <string.h>
 
 /******************************************************************************
  * Private Constants and Macros
  ******************************************************************************/
 
 static const char *TAG = "ST7789_OOP";
 
 // Brightness calculation constants
 #define BRIGHTNESS_SCALE_FACTOR     81
 #define BRIGHTNESS_MIN              0
 #define BRIGHTNESS_MAX              100
 
 /******************************************************************************
  * Private Function Prototypes
  ******************************************************************************/
 
 static esp_err_t backlight_init(st7789_backlight_t *backlight, gpio_num_t gpio_num);
 static esp_err_t backlight_deinit(st7789_backlight_t *backlight);
 static esp_err_t backlight_set_duty(st7789_backlight_t *backlight, uint8_t brightness);
 static uint16_t brightness_to_duty(uint8_t brightness);
 
 /******************************************************************************
  * Public API Implementation
  ******************************************************************************/
 
 /**
  * @brief Get default configuration
  */
 st7789_config_t st7789_get_default_config(void)
 {
     st7789_config_t config = {
         // SPI Configuration
         .spi_host = ST7789_SPI_HOST,
         .pin_sclk = ST7789_PIN_SCLK,
         .pin_mosi = ST7789_PIN_MOSI,
         .pin_cs = ST7789_PIN_CS,
         .pin_dc = ST7789_PIN_DC,
         .pin_rst = ST7789_PIN_RST,
         .pixel_clock_hz = ST7789_PIXEL_CLOCK_HZ,
         
         // Display Properties
         .h_res = ST7789_H_RES,
         .v_res = ST7789_V_RES,
         .offset_x = ST7789_OFFSET_X,
         .offset_y = ST7789_OFFSET_Y,
         .rgb_endian = LCD_RGB_ENDIAN_BGR,
         .bits_per_pixel = 16,
         
         // Backlight Configuration
         .pin_backlight = ST7789_PIN_BACKLIGHT,
         .initial_brightness = 75,
     };
     
     return config;
 }
 
 /**
  * @brief Create LCD device object
  */
 st7789_device_t* st7789_create(const st7789_config_t *config)
 {
     if (config == NULL) {
         ESP_LOGE(TAG, "Configuration cannot be NULL");
         return NULL;
     }
     
     // Allocate device object
     st7789_device_t *device = (st7789_device_t*)calloc(1, sizeof(st7789_device_t));
     if (device == NULL) {
         ESP_LOGE(TAG, "Failed to allocate memory for device object");
         return NULL;
     }
     
     // Copy configuration
     memcpy(&device->config, config, sizeof(st7789_config_t));
     device->is_initialized = false;
     
     ESP_LOGI(TAG, "LCD device object created successfully");
     return device;
 }
 
 /**
  * @brief Initialize LCD device
  */
 esp_err_t st7789_init(st7789_device_t *device)
 {
     if (device == NULL) {
         ESP_LOGE(TAG, "Device object cannot be NULL");
         return ESP_ERR_INVALID_ARG;
     }
     
     if (device->is_initialized) {
         ESP_LOGW(TAG, "Device already initialized");
         return ESP_OK;
     }
     
     esp_err_t ret;
     
     // Step 1: Install panel IO
     ESP_LOGI(TAG, "Installing panel IO interface");
     
     esp_lcd_panel_io_spi_config_t io_config = {
         .dc_gpio_num = device->config.pin_dc,
         .cs_gpio_num = device->config.pin_cs,
         .pclk_hz = device->config.pixel_clock_hz,
         .lcd_cmd_bits = ST7789_CMD_BITS,
         .lcd_param_bits = ST7789_PARAM_BITS,
         .spi_mode = 0,
         .trans_queue_depth = 10,
         .on_color_trans_done = NULL,  // No callback (LVGL will handle flushing)
         .user_ctx = NULL,
     };
     
     ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)device->config.spi_host, 
                                     &io_config, 
                                     &device->io_handle);
     if (ret != ESP_OK) {
         ESP_LOGE(TAG, "Failed to create panel IO: %s", esp_err_to_name(ret));
         return ret;
     }
     
     // Step 2: Install ST7789T panel driver
     ESP_LOGI(TAG, "Installing ST7789T panel driver");
     
     esp_lcd_panel_dev_st7789t_config_t panel_config = {
         .reset_gpio_num = device->config.pin_rst,
         .rgb_endian = device->config.rgb_endian,
         .bits_per_pixel = device->config.bits_per_pixel,
     };
     
     ret = esp_lcd_new_panel_st7789t(device->io_handle, &panel_config, &device->panel_handle);
     if (ret != ESP_OK) {
         ESP_LOGE(TAG, "Failed to create ST7789T panel: %s", esp_err_to_name(ret));
         return ret;
     }
     
     // Step 3: Reset and initialize panel
     ESP_LOGI(TAG, "Resetting and initializing panel");
     
     ret = esp_lcd_panel_reset(device->panel_handle);
     if (ret != ESP_OK) {
         ESP_LOGE(TAG, "Panel reset failed: %s", esp_err_to_name(ret));
         return ret;
     }
     
     ret = esp_lcd_panel_init(device->panel_handle);
     if (ret != ESP_OK) {
         ESP_LOGE(TAG, "Panel initialization failed: %s", esp_err_to_name(ret));
         return ret;
     }
     
     // Step 4: Configure display settings
     ret = esp_lcd_panel_mirror(device->panel_handle, true, false);
     if (ret != ESP_OK) {
         ESP_LOGE(TAG, "Setting mirror mode failed: %s", esp_err_to_name(ret));
         return ret;
     }
     
     // Step 5: Turn on display
     ret = esp_lcd_panel_disp_on_off(device->panel_handle, true);
     if (ret != ESP_OK) {
         ESP_LOGE(TAG, "Failed to turn on display: %s", esp_err_to_name(ret));
         return ret;
     }
     
     // Step 6: Initialize backlight
     ESP_LOGI(TAG, "Initializing backlight controller");
     ret = backlight_init(&device->backlight, device->config.pin_backlight);
     if (ret != ESP_OK) {
         ESP_LOGE(TAG, "Backlight initialization failed: %s", esp_err_to_name(ret));
         return ret;
     }
     
     // Set initial brightness
     ret = st7789_backlight_set(device, device->config.initial_brightness);
     if (ret != ESP_OK) {
         ESP_LOGW(TAG, "Failed to set initial brightness: %s", esp_err_to_name(ret));
     }
     
     device->is_initialized = true;
     ESP_LOGI(TAG, "LCD device initialized successfully");
     
     return ESP_OK;
 }
 
 /**
  * @brief Destroy LCD device object
  */
 esp_err_t st7789_destroy(st7789_device_t *device)
 {
     if (device == NULL) {
         return ESP_ERR_INVALID_ARG;
     }
     
     // Deinitialize backlight
     backlight_deinit(&device->backlight);
     
     // Note: Panel handles are managed by ESP-IDF driver framework
     // They will be automatically cleaned up
     
     // Free device object
     free(device);
     
     ESP_LOGI(TAG, "LCD device destroyed");
     return ESP_OK;
 }
 
 /**
  * @brief Turn display on/off
  */
 esp_err_t st7789_display_on_off(st7789_device_t *device, bool on)
 {
     if (device == NULL || !device->is_initialized) {
         return ESP_ERR_INVALID_STATE;
     }
     
     return esp_lcd_panel_disp_on_off(device->panel_handle, on);
 }
 
 /**
  * @brief Set display mirror mode
  */
 esp_err_t st7789_set_mirror(st7789_device_t *device, bool mirror_x, bool mirror_y)
 {
     if (device == NULL || !device->is_initialized) {
         return ESP_ERR_INVALID_STATE;
     }
     
     return esp_lcd_panel_mirror(device->panel_handle, mirror_x, mirror_y);
 }
 
 /**
  * @brief Set display rotation
  */
 esp_err_t st7789_set_rotation(st7789_device_t *device, bool swap_xy)
 {
     if (device == NULL || !device->is_initialized) {
         return ESP_ERR_INVALID_STATE;
     }
     
     return esp_lcd_panel_swap_xy(device->panel_handle, swap_xy);
 }
 
 /**
  * @brief Reset LCD panel
  */
 esp_err_t st7789_reset(st7789_device_t *device)
 {
     if (device == NULL || !device->is_initialized) {
         return ESP_ERR_INVALID_STATE;
     }
     
     return esp_lcd_panel_reset(device->panel_handle);
 }
 
 /**
  * @brief Get panel handle
  */
 esp_lcd_panel_handle_t st7789_get_panel_handle(st7789_device_t *device)
 {
     if (device == NULL) {
         return NULL;
     }
     
     return device->panel_handle;
 }
 
 /******************************************************************************
  * Backlight Control API Implementation
  ******************************************************************************/
 
 /**
  * @brief Set backlight brightness
  */
 esp_err_t st7789_backlight_set(st7789_device_t *device, uint8_t brightness)
 {
     if (device == NULL || !device->is_initialized) {
         return ESP_ERR_INVALID_STATE;
     }
     
     if (!device->backlight.is_initialized) {
         return ESP_ERR_INVALID_STATE;
     }
     
     return backlight_set_duty(&device->backlight, brightness);
 }
 
 /**
  * @brief Get current backlight brightness
  */
 uint8_t st7789_backlight_get(st7789_device_t *device)
 {
     if (device == NULL || !device->backlight.is_initialized) {
         return 0;
     }
     
     return device->backlight.current_brightness;
 }
 
 /**
  * @brief Enable/disable backlight
  */
 esp_err_t st7789_backlight_enable(st7789_device_t *device, bool enable)
 {
     if (device == NULL || !device->is_initialized) {
         return ESP_ERR_INVALID_STATE;
     }
     
     if (enable) {
         // Restore previous brightness
         return backlight_set_duty(&device->backlight, device->backlight.current_brightness);
     } else {
         // Turn off backlight but remember current brightness
         uint16_t duty = 0;
         ledc_set_duty(device->backlight.speed_mode, device->backlight.channel, duty);
         return ledc_update_duty(device->backlight.speed_mode, device->backlight.channel);
     }
 }
 
 /**
  * @brief Fade backlight to target brightness
  */
 esp_err_t st7789_backlight_fade(st7789_device_t *device, uint8_t target_brightness, uint32_t duration_ms)
 {
     if (device == NULL || !device->is_initialized) {
         return ESP_ERR_INVALID_STATE;
     }
     
     if (!device->backlight.is_initialized) {
         return ESP_ERR_INVALID_STATE;
     }
     
     // Clamp brightness
     if (target_brightness > BRIGHTNESS_MAX) {
         target_brightness = BRIGHTNESS_MAX;
     }
     
     uint16_t target_duty = brightness_to_duty(target_brightness);
     
     // Set fade parameters
     esp_err_t ret = ledc_set_fade_with_time(device->backlight.speed_mode, 
                                              device->backlight.channel,
                                              target_duty, 
                                              duration_ms);
     if (ret != ESP_OK) {
         return ret;
     }
     
     // Start fade
     ret = ledc_fade_start(device->backlight.speed_mode, 
                           device->backlight.channel, 
                           LEDC_FADE_NO_WAIT);
     if (ret == ESP_OK) {
         device->backlight.current_brightness = target_brightness;
     }
     
     return ret;
 }
 
 /******************************************************************************
  * Private Functions - Backlight Implementation
  ******************************************************************************/
 
 /**
  * @brief Initialize backlight controller
  */
 static esp_err_t backlight_init(st7789_backlight_t *backlight, gpio_num_t gpio_num)
 {
     if (backlight == NULL) {
         return ESP_ERR_INVALID_ARG;
     }
     
     esp_err_t ret;
     
     // Configure GPIO
     gpio_config_t gpio_conf = {
         .mode = GPIO_MODE_OUTPUT,
         .pin_bit_mask = 1ULL << gpio_num
     };
     ret = gpio_config(&gpio_conf);
     if (ret != ESP_OK) {
         return ret;
     }
     
     // Configure LEDC timer
     ledc_timer_config_t ledc_timer = {
         .duty_resolution = ST7789_BL_RESOLUTION,
         .freq_hz = ST7789_BL_FREQUENCY,
         .speed_mode = ST7789_BL_MODE,
         .timer_num = ST7789_BL_TIMER,
         .clk_cfg = LEDC_AUTO_CLK
     };
     ret = ledc_timer_config(&ledc_timer);
     if (ret != ESP_OK) {
         return ret;
     }
     
     // Configure LEDC channel
     ledc_channel_config_t ledc_channel = {
         .channel = ST7789_BL_CHANNEL,
         .duty = 0,
         .gpio_num = gpio_num,
         .speed_mode = ST7789_BL_MODE,
         .timer_sel = ST7789_BL_TIMER,
         .hpoint = 0
     };
     ret = ledc_channel_config(&ledc_channel);
     if (ret != ESP_OK) {
         return ret;
     }
     
     // Install fade function
     ret = ledc_fade_func_install(0);
     if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
         // ESP_ERR_INVALID_STATE means already installed, which is OK
         return ret;
     }
     
     // Initialize backlight object
     backlight->gpio_num = gpio_num;
     backlight->channel = ST7789_BL_CHANNEL;
     backlight->speed_mode = ST7789_BL_MODE;
     backlight->timer = ST7789_BL_TIMER;
     backlight->current_brightness = 0;
     backlight->is_initialized = true;
     
     ESP_LOGI(TAG, "Backlight controller initialized on GPIO %d", gpio_num);
     return ESP_OK;
 }
 
 /**
  * @brief Deinitialize backlight controller
  */
 static esp_err_t backlight_deinit(st7789_backlight_t *backlight)
 {
     if (backlight == NULL || !backlight->is_initialized) {
         return ESP_OK;
     }
     
     // Turn off backlight
     ledc_set_duty(backlight->speed_mode, backlight->channel, 0);
     ledc_update_duty(backlight->speed_mode, backlight->channel);
     
     // Reset GPIO
     gpio_reset_pin(backlight->gpio_num);
     
     backlight->is_initialized = false;
     
     return ESP_OK;
 }
 
 /**
  * @brief Set backlight PWM duty cycle
  */
 static esp_err_t backlight_set_duty(st7789_backlight_t *backlight, uint8_t brightness)
 {
     if (backlight == NULL || !backlight->is_initialized) {
         return ESP_ERR_INVALID_STATE;
     }
     
     // Clamp brightness
     if (brightness > BRIGHTNESS_MAX) {
         brightness = BRIGHTNESS_MAX;
     }
     
     uint16_t duty = brightness_to_duty(brightness);
     
     esp_err_t ret = ledc_set_duty(backlight->speed_mode, backlight->channel, duty);
     if (ret != ESP_OK) {
         return ret;
     }
     
     ret = ledc_update_duty(backlight->speed_mode, backlight->channel);
     if (ret == ESP_OK) {
         backlight->current_brightness = brightness;
     }
     
     return ret;
 }
 
 /**
  * @brief Convert brightness percentage to PWM duty value
  */
 static uint16_t brightness_to_duty(uint8_t brightness)
 {
     if (brightness == 0) {
         return 0;
     }
     
     if (brightness > BRIGHTNESS_MAX) {
         brightness = BRIGHTNESS_MAX;
     }
     
     // Non-linear brightness curve
     // Formula: duty = MAX_DUTY - (SCALE_FACTOR * (100 - brightness))
     uint16_t duty = ST7789_BL_MAX_DUTY - (BRIGHTNESS_SCALE_FACTOR * (BRIGHTNESS_MAX - brightness));
     
     return duty;
 }
 
