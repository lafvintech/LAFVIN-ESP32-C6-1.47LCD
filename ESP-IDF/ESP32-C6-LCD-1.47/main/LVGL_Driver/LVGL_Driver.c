/**
 * @file LVGL_Driver.c
 * @brief LVGL Display Driver - Object-Oriented Implementation
 * @author Refactored for better modularity
 * @date 2025
 */

#include "LVGL_Driver.h"
#include "ST7789.h"  // Include full ST7789 definitions

static const char *TAG = "LVGL_Driver";

/******************************************************************************
 * Default Configuration
 ******************************************************************************/

lvgl_config_t lvgl_get_default_config(st7789_device_t *lcd_device)
{
    if (lcd_device == NULL) {
        ESP_LOGE(TAG, "LCD device cannot be NULL");
        lvgl_config_t empty = {0};
        return empty;
    }

    lvgl_config_t config = {
        .hor_res = lcd_device->config.h_res,
        .ver_res = lcd_device->config.v_res,
        .buf_lines = LVGL_DEFAULT_BUF_LINES,
        .use_double_buffer = true,
        .buf_alloc = LVGL_BUF_ALLOC_INTERNAL,
        .full_refresh = false,
        .rotation = 0,
        .lcd_device = lcd_device,
        .tick_period_ms = LVGL_TICK_PERIOD_MS,
    };

    return config;
}

/******************************************************************************
 * Object Lifecycle Management
 ******************************************************************************/

lvgl_driver_t* lvgl_driver_create(const lvgl_config_t *config)
{
    if (config == NULL || config->lcd_device == NULL) {
        ESP_LOGE(TAG, "Invalid configuration");
        return NULL;
    }

    // Allocate driver object
    lvgl_driver_t *driver = (lvgl_driver_t *)heap_caps_calloc(1, sizeof(lvgl_driver_t), MALLOC_CAP_8BIT);
    if (driver == NULL) {
        ESP_LOGE(TAG, "Failed to allocate driver object");
        return NULL;
    }

    // Copy configuration
    memcpy(&driver->config, config, sizeof(lvgl_config_t));

    // Calculate buffer size
    driver->buf_size = driver->config.hor_res * driver->config.buf_lines;

    ESP_LOGI(TAG, "LVGL driver created: %dx%d, buf_lines=%d, double_buf=%d",
             driver->config.hor_res, driver->config.ver_res,
             driver->config.buf_lines, driver->config.use_double_buffer);

    return driver;
}

esp_err_t lvgl_driver_init(lvgl_driver_t *driver)
{
    if (driver == NULL) {
        ESP_LOGE(TAG, "Driver object is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (driver->is_initialized) {
        ESP_LOGW(TAG, "Driver already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing LVGL driver...");

    // Step 1: Initialize LVGL library
    lv_init();
    ESP_LOGI(TAG, "✓ LVGL library initialized");

    // Step 2: Allocate display buffers
    uint32_t malloc_caps = MALLOC_CAP_8BIT;
    switch (driver->config.buf_alloc) {
        case LVGL_BUF_ALLOC_SPIRAM:
            malloc_caps = MALLOC_CAP_SPIRAM;
            ESP_LOGI(TAG, "Using SPIRAM for display buffers");
            break;
        case LVGL_BUF_ALLOC_DMA:
            malloc_caps = MALLOC_CAP_DMA;
            ESP_LOGI(TAG, "Using DMA-capable memory for buffers");
            break;
        case LVGL_BUF_ALLOC_INTERNAL:
        default:
            malloc_caps = MALLOC_CAP_INTERNAL;
            ESP_LOGI(TAG, "Using internal RAM for buffers");
            break;
    }

    size_t buf_bytes = driver->buf_size * sizeof(lv_color_t);
    driver->buf1 = (lv_color_t *)heap_caps_malloc(buf_bytes, malloc_caps);
    if (driver->buf1 == NULL) {
        ESP_LOGE(TAG, "Failed to allocate buffer1 (%d bytes)", buf_bytes);
        return ESP_ERR_NO_MEM;
    }
    ESP_LOGI(TAG, "✓ Buffer1 allocated: %d bytes", buf_bytes);

    if (driver->config.use_double_buffer) {
        driver->buf2 = (lv_color_t *)heap_caps_malloc(buf_bytes, malloc_caps);
        if (driver->buf2 == NULL) {
            ESP_LOGE(TAG, "Failed to allocate buffer2");
            free(driver->buf1);
            return ESP_ERR_NO_MEM;
        }
        ESP_LOGI(TAG, "✓ Buffer2 allocated: %d bytes (double buffering)", buf_bytes);
    } else {
        driver->buf2 = NULL;
        ESP_LOGI(TAG, "✓ Single buffer mode");
    }

    // Step 3: Initialize LVGL draw buffer
    lv_disp_draw_buf_init(&driver->draw_buf, driver->buf1, driver->buf2, driver->buf_size);
    ESP_LOGI(TAG, "✓ LVGL draw buffer initialized");

    // Step 4: Initialize and register display driver
    lv_disp_drv_init(&driver->disp_drv);
    driver->disp_drv.hor_res = driver->config.hor_res;
    driver->disp_drv.ver_res = driver->config.ver_res;
    driver->disp_drv.flush_cb = lvgl_flush_callback;
    driver->disp_drv.drv_update_cb = lvgl_rotation_callback;
    driver->disp_drv.draw_buf = &driver->draw_buf;
    driver->disp_drv.user_data = driver;  // Store driver object for callbacks
    driver->disp_drv.full_refresh = driver->config.full_refresh;

    driver->display = lv_disp_drv_register(&driver->disp_drv);
    if (driver->display == NULL) {
        ESP_LOGE(TAG, "Failed to register display driver");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "✓ Display driver registered");

    // Step 5: Apply initial rotation
    if (driver->config.rotation != 0) {
        lvgl_driver_set_rotation(driver, driver->config.rotation);
    }

    // Step 6: Create and start tick timer
    const esp_timer_create_args_t timer_args = {
        .callback = lvgl_tick_callback,
        .arg = driver,
        .name = "lvgl_tick",
        .dispatch_method = ESP_TIMER_TASK,
    };

    esp_err_t ret = esp_timer_create(&timer_args, &driver->tick_timer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create tick timer: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_timer_start_periodic(driver->tick_timer, driver->config.tick_period_ms * 1000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start tick timer: %s", esp_err_to_name(ret));
        esp_timer_delete(driver->tick_timer);
        return ret;
    }
    ESP_LOGI(TAG, "✓ Tick timer started (%d ms period)", driver->config.tick_period_ms);

    driver->is_initialized = true;
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "LVGL driver initialization complete!");
    ESP_LOGI(TAG, "Resolution: %dx%d", driver->config.hor_res, driver->config.ver_res);
    ESP_LOGI(TAG, "Buffer: %d lines (%s)",
             driver->config.buf_lines,
             driver->config.use_double_buffer ? "double" : "single");
    ESP_LOGI(TAG, "========================================");

    return ESP_OK;
}

esp_err_t lvgl_driver_destroy(lvgl_driver_t *driver)
{
    if (driver == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Destroying LVGL driver...");

    // Stop and delete tick timer
    if (driver->tick_timer != NULL) {
        esp_timer_stop(driver->tick_timer);
        esp_timer_delete(driver->tick_timer);
        driver->tick_timer = NULL;
    }

    // Free buffers
    if (driver->buf1 != NULL) {
        free(driver->buf1);
        driver->buf1 = NULL;
    }
    if (driver->buf2 != NULL) {
        free(driver->buf2);
        driver->buf2 = NULL;
    }

    // Free driver object
    free(driver);

    ESP_LOGI(TAG, "LVGL driver destroyed");
    return ESP_OK;
}

/******************************************************************************
 * Display Control Functions
 ******************************************************************************/

esp_err_t lvgl_driver_set_rotation(lvgl_driver_t *driver, uint16_t rotation)
{
    if (driver == NULL || !driver->is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    lv_disp_rot_t lv_rotation;
    switch (rotation) {
        case 0:
            lv_rotation = LV_DISP_ROT_NONE;
            break;
        case 90:
            lv_rotation = LV_DISP_ROT_90;
            break;
        case 180:
            lv_rotation = LV_DISP_ROT_180;
            break;
        case 270:
            lv_rotation = LV_DISP_ROT_270;
            break;
        default:
            ESP_LOGE(TAG, "Invalid rotation: %d (must be 0/90/180/270)", rotation);
            return ESP_ERR_INVALID_ARG;
    }

    lv_disp_set_rotation(driver->display, lv_rotation);
    driver->config.rotation = rotation;

    ESP_LOGI(TAG, "Display rotation set to %d degrees", rotation);
    return ESP_OK;
}

lv_disp_t* lvgl_driver_get_display(lvgl_driver_t *driver)
{
    if (driver == NULL) {
        return NULL;
    }
    return driver->display;
}

void lvgl_driver_task_handler(lvgl_driver_t *driver)
{
    // Driver parameter is currently unused, but reserved for future use
    // LVGL task handler works globally
    lv_timer_handler();
}

/******************************************************************************
 * Callback Functions
 ******************************************************************************/

void lvgl_tick_callback(void *arg)
{
    lvgl_driver_t *driver = (lvgl_driver_t *)arg;
    if (driver != NULL) {
        lv_tick_inc(driver->config.tick_period_ms);
    }
}

void lvgl_flush_callback(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    if (drv == NULL || drv->user_data == NULL) {
        ESP_LOGE(TAG, "Invalid flush callback parameters");
        return;
    }

    lvgl_driver_t *driver = (lvgl_driver_t *)drv->user_data;
    st7789_device_t *lcd = driver->config.lcd_device;

    if (lcd == NULL || lcd->panel_handle == NULL) {
        ESP_LOGE(TAG, "LCD device not available");
        lv_disp_flush_ready(drv);
        return;
    }

    // Calculate display coordinates with offset
    int x1 = area->x1 + lcd->config.offset_x;
    int y1 = area->y1 + lcd->config.offset_y;
    int x2 = area->x2 + lcd->config.offset_x;
    int y2 = area->y2 + lcd->config.offset_y;

    // Draw bitmap to LCD panel
    esp_lcd_panel_draw_bitmap(lcd->panel_handle, x1, y1, x2 + 1, y2 + 1, color_map);

    // Notify LVGL that flushing is done
    lv_disp_flush_ready(drv);
}

void lvgl_rotation_callback(lv_disp_drv_t *drv)
{
    if (drv == NULL || drv->user_data == NULL) {
        return;
    }

    lvgl_driver_t *driver = (lvgl_driver_t *)drv->user_data;
    st7789_device_t *lcd = driver->config.lcd_device;

    if (lcd == NULL || lcd->panel_handle == NULL) {
        return;
    }

    // Apply rotation to LCD panel based on LVGL rotation state
    switch (drv->rotated) {
        case LV_DISP_ROT_NONE:
            esp_lcd_panel_swap_xy(lcd->panel_handle, false);
            esp_lcd_panel_mirror(lcd->panel_handle, true, false);
            ESP_LOGI(TAG, "Rotation: 0°");
            break;
        case LV_DISP_ROT_90:
            esp_lcd_panel_swap_xy(lcd->panel_handle, true);
            esp_lcd_panel_mirror(lcd->panel_handle, true, true);
            ESP_LOGI(TAG, "Rotation: 90°");
            break;
        case LV_DISP_ROT_180:
            esp_lcd_panel_swap_xy(lcd->panel_handle, false);
            esp_lcd_panel_mirror(lcd->panel_handle, false, true);
            ESP_LOGI(TAG, "Rotation: 180°");
            break;
        case LV_DISP_ROT_270:
            esp_lcd_panel_swap_xy(lcd->panel_handle, true);
            esp_lcd_panel_mirror(lcd->panel_handle, false, false);
            ESP_LOGI(TAG, "Rotation: 270°");
            break;
    }
}


