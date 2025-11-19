#include "ST7789.h"
#include "SD_SPI.h"
#include "RGB.h"
#include "Wireless.h"
#include "LVGL_Example.h"

static const char *TAG = "MAIN";

// Global driver instances (using new OOP API)
static st7789_device_t *lcd_device = NULL;
static lvgl_driver_t *lvgl_driver = NULL;

/**
 * @brief Initialize SPI bus for LCD and SD card
 */
static void spi_bus_init(void)
{
    ESP_LOGI(TAG, "Initialize SPI bus");

    spi_bus_config_t buscfg = {
        .sclk_io_num = ST7789_PIN_SCLK,
        .mosi_io_num = ST7789_PIN_MOSI,
        .miso_io_num = 5,  // SD card MISO pin
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = ST7789_H_RES * ST7789_V_RES * sizeof(uint16_t),
    };

    esp_err_t ret_spi = spi_bus_initialize(ST7789_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret_spi != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret_spi));
    } else {
        ESP_LOGI(TAG, "SPI bus initialized successfully");
    }
}

void app_main(void)
{
    esp_err_t ret;
    
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "ESP32-C6 LCD Demo - OOP Refactored");
    ESP_LOGI(TAG, "========================================");

    // ========== Step 1: Initialize Wireless (WiFi/BLE) ==========
    ESP_LOGI(TAG, "Step 1: Initializing wireless...");
    Wireless_Init();
    Flash_Searching();

    // ========== Step 2: Initialize RGB LED ==========
    ESP_LOGI(TAG, "Step 2: Initializing RGB LED...");
    ret = RGB_Init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize RGB LED: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "✓ RGB LED initialized");
        RGB_Example();  // Start default rainbow effect
    }

    // ========== Step 3: Initialize SPI Bus ==========
    ESP_LOGI(TAG, "Step 3: Initializing SPI bus...");
    spi_bus_init();

    // ========== Step 4: Initialize LCD (New OOP API) ==========
    ESP_LOGI(TAG, "Step 4: Initializing ST7789 LCD...");

    // Create LCD with default configuration
    st7789_config_t lcd_config = st7789_get_default_config();
    lcd_device = st7789_create(&lcd_config);

    if (lcd_device == NULL) {
        ESP_LOGE(TAG, "Failed to create LCD device!");
        return;
    }

    // Initialize LCD hardware
    ret = st7789_init(lcd_device);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LCD: %s", esp_err_to_name(ret));
        return;
    }

    // Set backlight
    st7789_backlight_set(lcd_device, 100);

    // ========== Step 5: Initialize SD Card ==========
    ESP_LOGI(TAG, "Step 5: Initializing SD card...");
    SD_Init();

    // ========== Step 6: Initialize LVGL (New OOP API) ==========
    ESP_LOGI(TAG, "Step 6: Initializing LVGL driver...");

    // Get default LVGL configuration
    lvgl_config_t lvgl_config = lvgl_get_default_config(lcd_device);

    // Optional: Customize configuration
    // lvgl_config.buf_lines = 40;                      // Larger buffer (default: 20)
    // lvgl_config.buf_alloc = LVGL_BUF_ALLOC_SPIRAM;   // Use SPIRAM if available
    // lvgl_config.rotation = 90;                       // Landscape mode

    // Create LVGL driver
    lvgl_driver = lvgl_driver_create(&lvgl_config);
    if (lvgl_driver == NULL) {
        ESP_LOGE(TAG, "Failed to create LVGL driver!");
        return;
    }

    // Initialize LVGL
    ret = lvgl_driver_init(lvgl_driver);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LVGL: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "✓ LVGL driver initialized");

    // ========== Step 7: Load UI Example ==========
    ESP_LOGI(TAG, "Step 7: Loading LVGL UI...");
    Lvgl_Example1();

    // Alternative demos (uncomment to try):
    // lv_demo_widgets();
    // lv_demo_keypad_encoder();
    // lv_demo_benchmark();
    // lv_demo_stress();
    // lv_demo_music();

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Initialization complete! Entering main loop...");
    ESP_LOGI(TAG, "========================================");

    // ========== Main Loop ==========
    while (1) {
        // Call LVGL task handler
        lvgl_driver_task_handler(lvgl_driver);

        // Lower priority task delay (10ms recommended)
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Cleanup (never reached in this example, but shown for completeness)
    // lvgl_driver_destroy(lvgl_driver);
    // st7789_destroy(lcd_device);
}