#include "LVGL_Example.h"
#include "esp_timer.h"

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    DISP_SMALL,
    DISP_MEDIUM,
    DISP_LARGE,
} disp_size_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void Onboard_create(lv_obj_t * parent);
static void example1_increase_lvgl_tick(lv_timer_t * t);

/**********************
 *  STATIC VARIABLES
 **********************/
static disp_size_t disp_size;

static lv_obj_t * tv;
lv_style_t style_text_muted;
lv_style_t style_title;
static lv_style_t style_icon;
static lv_style_t style_bullet;
static lv_style_t style_value_bold;  // New: Bold style for values

static const lv_font_t * font_large;
static const lv_font_t * font_normal;

static lv_timer_t * auto_step_timer;
static lv_timer_t * meter2_timer;

lv_obj_t * SD_Size;
lv_obj_t * FlashSize;
lv_obj_t * Runtime_Display;
lv_obj_t * Board_angle;
lv_obj_t * RTC_Time;
lv_obj_t * Wireless_Scan;



void Lvgl_Example1(void){

  disp_size = DISP_SMALL;                            

  font_large = LV_FONT_DEFAULT;                             
  font_normal = LV_FONT_DEFAULT;                         
  
  lv_coord_t tab_h;
  tab_h = 50;  // Increase tab height to accommodate larger font
  #if LV_FONT_MONTSERRAT_18
    font_large     = &lv_font_montserrat_18;
  #else
    LV_LOG_WARN("LV_FONT_MONTSERRAT_18 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
  #endif
  #if LV_FONT_MONTSERRAT_12
    font_normal    = &lv_font_montserrat_12;
  #else
    LV_LOG_WARN("LV_FONT_MONTSERRAT_12 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
  #endif
  
  // Removed unused color variables, now using black and white color scheme
  
  // Initialize LVGL theme with custom colors
  lv_theme_default_init(NULL, lv_color_hex(0xffffff), lv_color_hex(0xffffff), false, font_normal);
  
  lv_style_init(&style_text_muted);
  lv_style_set_text_opa(&style_text_muted, LV_OPA_90);

  lv_style_init(&style_title);
  lv_style_set_text_font(&style_title, font_large);

  lv_style_init(&style_icon);
  lv_style_set_text_color(&style_icon, lv_color_hex(0x000000));  // Black
  lv_style_set_text_font(&style_icon, font_large);

  lv_style_init(&style_bullet);
  lv_style_set_border_width(&style_bullet, 0);
  lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);
  
  // Initialize bold style for values
  lv_style_init(&style_value_bold);
  // lv_style_set_text_color(&style_value_bold, lv_color_hex(0x000000));  // Black
  lv_style_set_text_font(&style_value_bold, font_large);               // Use large font
  lv_style_set_text_opa(&style_value_bold, LV_OPA_COVER);              // Fully opaque

  tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_h);
  
  // Set tabview overall background to white
  lv_obj_set_style_bg_color(tv, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_bg_opa(tv, LV_OPA_COVER, 0);

  // Customize tabview indicator line style (bold)
  lv_obj_t * tab_btns = lv_tabview_get_tab_btns(tv);
  lv_obj_set_style_border_width(tab_btns, 3, LV_PART_ITEMS | LV_STATE_CHECKED);  // Set indicator line width to 3 pixels
  lv_obj_set_style_border_color(tab_btns, lv_color_hex(0x000000), LV_PART_ITEMS | LV_STATE_CHECKED);  // Set indicator color
  lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_BOTTOM, LV_PART_ITEMS | LV_STATE_CHECKED);  // Indicator at bottom
  
  // Set "LAFVIN" label text color to #007bba and make it bold and larger
  lv_color_t tab_text_color = lv_color_hex(0x007bba);  // New blue color
  lv_obj_set_style_text_color(tab_btns, tab_text_color, LV_PART_ITEMS | LV_STATE_DEFAULT); 
  lv_obj_set_style_text_color(tab_btns, tab_text_color, LV_PART_ITEMS | LV_STATE_CHECKED);

  // Completely remove the right scrollbar
  lv_obj_set_scrollbar_mode(tv, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_width(tv, 0, LV_PART_SCROLLBAR);
  lv_obj_set_style_bg_opa(tv, LV_OPA_TRANSP, LV_PART_SCROLLBAR);
  
  // Get tabview content area and hide scrollbar
  lv_obj_t * tab_content = lv_tabview_get_content(tv);
  lv_obj_set_scrollbar_mode(tab_content, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_width(tab_content, 0, LV_PART_SCROLLBAR);
  
  // Apply bold style to LAFVIN tab
  lv_obj_add_style(tab_btns, &style_value_bold, LV_PART_ITEMS | LV_STATE_DEFAULT);
  lv_obj_add_style(tab_btns, &style_value_bold, LV_PART_ITEMS | LV_STATE_CHECKED);
  
  // Try to use larger font (if available)
  #if LV_FONT_MONTSERRAT_22
    lv_obj_set_style_text_font(tab_btns, &lv_font_montserrat_22, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(tab_btns, &lv_font_montserrat_22, LV_PART_ITEMS | LV_STATE_CHECKED);
  #elif LV_FONT_MONTSERRAT_20
    lv_obj_set_style_text_font(tab_btns, &lv_font_montserrat_20, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(tab_btns, &lv_font_montserrat_20, LV_PART_ITEMS | LV_STATE_CHECKED);
  #else
    lv_obj_set_style_text_font(tab_btns, font_large, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(tab_btns, font_large, LV_PART_ITEMS | LV_STATE_CHECKED);
  #endif
  
  // Increase letter spacing for bolder visual effect
  lv_obj_set_style_text_letter_space(tab_btns, 3, LV_PART_ITEMS | LV_STATE_DEFAULT);
  lv_obj_set_style_text_letter_space(tab_btns, 3, LV_PART_ITEMS | LV_STATE_CHECKED);
  
  // Use shadow/outline effect to simulate bold (key optimization)
  lv_obj_set_style_text_decor(tab_btns, LV_TEXT_DECOR_NONE, LV_PART_ITEMS | LV_STATE_DEFAULT);
  lv_obj_set_style_text_decor(tab_btns, LV_TEXT_DECOR_NONE, LV_PART_ITEMS | LV_STATE_CHECKED);
  
  // Add text shadow to make font appear bolder (multi-layer overlay effect)
  lv_obj_set_style_text_opa(tab_btns, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(tab_btns, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_CHECKED);   

  lv_obj_set_style_text_font(lv_scr_act(), font_normal, 0);


  lv_obj_t * t1 = lv_tabview_add_tab(tv, "LAFVIN");
  
  // Ensure tab page also has no scrollbar
  lv_obj_set_scrollbar_mode(t1, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_width(t1, 0, LV_PART_SCROLLBAR);
  lv_obj_set_style_bg_opa(t1, LV_OPA_TRANSP, LV_PART_SCROLLBAR);
  
  Onboard_create(t1);
  
  
}

void Lvgl_Example1_close(void)
{
  /*Delete all animation*/
  lv_anim_del(NULL, NULL);

  if(auto_step_timer != NULL) {
    lv_timer_del(auto_step_timer);
    auto_step_timer = NULL;
  }

  if(meter2_timer != NULL) {
    lv_timer_del(meter2_timer);
    meter2_timer = NULL;
  }

  lv_obj_clean(lv_scr_act());

  lv_style_reset(&style_text_muted);
  lv_style_reset(&style_title);
  lv_style_reset(&style_icon);
  lv_style_reset(&style_bullet);
  lv_style_reset(&style_value_bold);
}

// New: Hide LVGL interface (without destroying objects)
void Lvgl_Example1_hide(void) {
  if (tv != NULL) {
    lv_obj_add_flag(tv, LV_OBJ_FLAG_HIDDEN);
    printf("LVGL interface hidden (objects retained)\n");
  }
}

// New: Show LVGL interface (restore display)
void Lvgl_Example1_show(void) {
  if (tv != NULL) {
    lv_obj_clear_flag(tv, LV_OBJ_FLAG_HIDDEN);
    lv_obj_invalidate(lv_scr_act());  // Refresh screen
    printf("LVGL interface shown\n");
  }
}

// New: Set screen background to black (avoid white screen flicker)
void Lvgl_Set_Screen_Black(void) {
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);  // Black
  lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);
  printf("[Optimization] Screen background set to black\n");
}


/**********************
*   STATIC FUNCTIONS
**********************/

static void Onboard_create(lv_obj_t * parent)
{

  /*Create a panel*/
  lv_obj_t * panel1 = lv_obj_create(parent);
  lv_obj_set_height(panel1, LV_SIZE_CONTENT);
  
  // Remove panel border and set white background
  lv_obj_set_style_border_width(panel1, 0, 0);                    // Remove border
  lv_obj_set_style_bg_opa(panel1, LV_OPA_COVER, 0);               // Background fully opaque
  lv_obj_set_style_radius(panel1, 0, 0);                          // Remove rounded corners

  // Completely remove right scrollbar
  lv_obj_set_scrollbar_mode(panel1, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_width(panel1, 0, LV_PART_SCROLLBAR);
  lv_obj_set_style_bg_opa(panel1, LV_OPA_TRANSP, LV_PART_SCROLLBAR);
  lv_obj_set_style_opa(panel1, LV_OPA_TRANSP, LV_PART_SCROLLBAR);

  lv_obj_t * panel1_title = lv_label_create(panel1);
  lv_label_set_text(panel1_title, "Onboard INFO");
  lv_obj_add_style(panel1_title, &style_title, 0);

  lv_obj_t * SD_label = lv_label_create(panel1);
  lv_label_set_text(SD_label, "SD");
  lv_obj_add_style(SD_label, &style_text_muted, 0);

  SD_Size = lv_textarea_create(panel1);
  lv_textarea_set_one_line(SD_Size, true);
  lv_textarea_set_placeholder_text(SD_Size, "SD Size");
  lv_obj_set_style_text_color(SD_Size, lv_color_hex(0x006D70), LV_PART_TEXTAREA_PLACEHOLDER);
  lv_obj_add_style(SD_Size, &style_value_bold, LV_PART_TEXTAREA_PLACEHOLDER);  // Bold style

  lv_obj_t * Flash_label = lv_label_create(panel1);
  lv_label_set_text(Flash_label, "Flash");
  lv_obj_add_style(Flash_label, &style_text_muted, 0);

  FlashSize = lv_textarea_create(panel1);
  lv_textarea_set_one_line(FlashSize, true);
  lv_textarea_set_placeholder_text(FlashSize, "Flash Size");
  lv_obj_set_style_text_color(FlashSize, lv_color_hex(0x006D70), LV_PART_TEXTAREA_PLACEHOLDER);
  lv_obj_add_style(FlashSize, &style_value_bold, LV_PART_TEXTAREA_PLACEHOLDER);  // Bold style

  lv_obj_t * Runtime_label = lv_label_create(panel1);
  lv_label_set_text(Runtime_label, "Time");
  lv_obj_add_style(Runtime_label, &style_text_muted, 0);

  Runtime_Display = lv_textarea_create(panel1);
  lv_textarea_set_one_line(Runtime_Display, true);
  lv_textarea_set_placeholder_text(Runtime_Display, "00:00:00");
  lv_obj_set_style_text_color(Runtime_Display, lv_color_hex(0x000000), LV_PART_TEXTAREA_PLACEHOLDER);
  lv_obj_add_style(Runtime_Display, &style_value_bold, LV_PART_TEXTAREA_PLACEHOLDER);  // Bold style

  lv_obj_t * Wireless_label = lv_label_create(panel1);
  lv_label_set_text(Wireless_label, "Wireless scan");
  lv_obj_add_style(Wireless_label, &style_text_muted, 0);

  Wireless_Scan = lv_textarea_create(panel1);
  lv_textarea_set_one_line(Wireless_Scan, true);
  lv_textarea_set_placeholder_text(Wireless_Scan, "Wireless number");
  lv_obj_set_style_text_color(Wireless_Scan, lv_color_hex(0x000000), LV_PART_TEXTAREA_PLACEHOLDER);

  // Component layout
  static lv_coord_t grid_main_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t grid_main_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(parent, grid_main_col_dsc, grid_main_row_dsc);

  // Column width and spacing: fixed label column width, let text box get more space
  static lv_coord_t grid_2_col_dsc[] = {20, 3, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST}; // Label fixed 20px | 3px spacing | Text box takes remaining space
  // Row height and spacing
  static lv_coord_t grid_2_row_dsc[] = {
    LV_GRID_CONTENT,  /*Title*/
    5,                /*Separator*/
    35,               /*SD Card row*/
    35,               /*Flash Size row*/
    35,               /*Runtime row*/
    LV_GRID_CONTENT,  /*Wireless label row*/
    40,               /*Wireless box row*/
    LV_GRID_TEMPLATE_LAST
  };

  lv_obj_set_grid_cell(panel1, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 0, 1);
  lv_obj_set_grid_dsc_array(panel1, grid_2_col_dsc, grid_2_row_dsc);

  // Title spans entire row (across 3 columns: label + spacing + text box)
  lv_obj_set_grid_cell(panel1_title, LV_GRID_ALIGN_START, 0, 3, LV_GRID_ALIGN_CENTER, 0, 1);

  // SD Card row: label on left (column 0), text box on right (column 2)
  lv_obj_set_grid_cell(SD_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
  lv_obj_set_grid_cell(SD_Size, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_CENTER, 2, 1);

  // Flash Size row: label on left (column 0), text box on right (column 2)
  lv_obj_set_grid_cell(Flash_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);
  lv_obj_set_grid_cell(FlashSize, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_CENTER, 3, 1);

  // Runtime row: label on left (column 0), text box on right (column 2)
  lv_obj_set_grid_cell(Runtime_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 4, 1);
  lv_obj_set_grid_cell(Runtime_Display, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_CENTER, 4, 1);

  // Wireless spans entire row (across 3 columns)
  lv_obj_set_grid_cell(Wireless_label, LV_GRID_ALIGN_START, 0, 3, LV_GRID_ALIGN_START, 5, 1);
  lv_obj_set_grid_cell(Wireless_Scan, LV_GRID_ALIGN_STRETCH, 0, 3, LV_GRID_ALIGN_CENTER, 6, 1);

  // Component layout END
  
  auto_step_timer = lv_timer_create(example1_increase_lvgl_tick, 100, NULL);
}

static void example1_increase_lvgl_tick(lv_timer_t * t)
{
  char buf[100]={0};
  
  snprintf(buf, sizeof(buf), "%lu MB\r\n", (unsigned long)SDCard_Size);
  lv_textarea_set_placeholder_text(SD_Size, buf);
  snprintf(buf, sizeof(buf), "%lu MB\r\n", (unsigned long)Flash_Size);
  lv_textarea_set_placeholder_text(FlashSize, buf);
  
  // Calculate runtime (time since boot) - ESP-IDF version uses esp_timer_get_time()
  unsigned long runtime_ms = (unsigned long)(esp_timer_get_time() / 1000);  // Convert to milliseconds
  unsigned long seconds = runtime_ms / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  
  // Format runtime: HH:MM:SS
  snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu\r\n", 
           hours, minutes % 60, seconds % 60);
  lv_textarea_set_placeholder_text(Runtime_Display, buf);
  
  if(Scan_finish)
    snprintf(buf, sizeof(buf), "W: %d  B: %d    OK.\r\n",WIFI_NUM,BLE_NUM);
    // snprintf(buf, sizeof(buf), "WIFI: %d     ..OK.\r\n",WIFI_NUM);
  else
    snprintf(buf, sizeof(buf), "W: %d  B: %d\r\n",WIFI_NUM,BLE_NUM);
    // snprintf(buf, sizeof(buf), "WIFI: %d  \r\n",WIFI_NUM);
  lv_textarea_set_placeholder_text(Wireless_Scan, buf);
}
