#include "SD_Card.h"
#include "Display_ST7789.h"
#include "DisplayConfig.h"
#include "SDCardConfig.h"
#include "SystemInfo.h"
#include "Wireless.h"
#include "WirelessConfig.h"
#include "LVGL_Driver.h"
#include "LVGL_Example.h"

// ============================================================================
// Global Objects - Using Object-Oriented API
// ============================================================================
ST7789Display display;    // Display object
SDCardManager sdcard;     // SD card management object
WirelessScanner wireless; // Wireless scanning object

void setup()
{
  Serial.begin(115200);
  delay(100);
  
  printf("\n========================================\n");
  printf("  ESP32-C6 LVGL Demo (OOP Version)\n");
  printf("========================================\n\n");
  
  // 1. System Information (using the new object-oriented API)
  printf("=== System Info ===\n");
  SystemInfo::printFlashInfo();
  
  // Update global variables (for LVGL UI display)
  extern uint16_t Flash_Size;
  Flash_Size = SystemInfo::getFlashSizeMB();
  
  // 2. Initialize Display (using the new object-oriented API)
  printf("=== Display Initialization ===\n");
  if (!display.begin()) {
    printf("ERROR: Display initialization failed!\n");
    return;
  }
  printf("✓ Display initialized: %dx%d\n", display.width(), display.height());
  // Set backlight brightness
  display.setBacklight(60);  // 60% brightness (using percentage directly here)
  
  // 3. Initialize LVGL
  printf("=== LVGL Initialization ===\n");
  Lvgl_Init();
  printf("✓ LVGL initialized\n\n");
  
  // 4. Initialize SD Card (using the new object-oriented API)
  printf("=== SD Card Initialization ===\n");
  
  // Update global variables (for LVGL UI display)
  extern uint16_t SDCard_Size;
  
  if (sdcard.begin()) {
    printf("✓ SD Card initialized\n");
    printf("  - Type: %s\n", sdcard.getCardTypeName());
    printf("  - Size: %u MB\n", sdcard.getSizeMB());
    
    // Update global variables for LVGL display
    SDCard_Size = sdcard.getSizeMB();
    
    // Example: Search for files (optional)
    // std::vector<String> pngFiles = sdcard.searchFiles("/", ".png", 10);
    // printf("  - Found %d PNG files\n", pngFiles.size());
  } else {
    printf("✗ SD Card initialization failed (optional)\n");
    SDCard_Size = 0;
  }
  printf("\n");

  // 5. Start LVGL Example
  Lvgl_Example1();     
  // lv_demo_widgets();               
  // lv_demo_benchmark();          
  // lv_demo_keypad_encoder();     
  // lv_demo_music();  
  // lv_demo_stress();   

  // 6. Start Wireless Scan (using the new object-oriented API)
  printf("=== Wireless Scan ===\n");
  printf("Starting wireless scan (async)...\n");
  wireless.scanAsync();  // Asynchronous scan
  printf("Wireless scan task started\n");
  
  printf("\n========================================\n");
  printf("  Setup Complete!\n");
  printf("========================================\n\n");
}

void loop()
{
  // Update wireless scan global variables (for LVGL UI display)
  if (wireless.isFinished()) {
    WIFI_NUM = wireless.getWiFiCount();
    BLE_NUM = wireless.getBLECount();
    Scan_finish = true;
  }
  
  Timer_Loop();
  delay(5);
}
