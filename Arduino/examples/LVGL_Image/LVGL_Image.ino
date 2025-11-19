#include "SD_Card.h"
#include "Display_ST7789.h"
#include "LCD_Image.h"

// ============================================================================
// Global Objects - Using Object-Oriented API
// ============================================================================
ST7789Display display;    // Display object
SDCardManager sdcard;     // SD card management object

void setup()
{
  // Initialize USB CDC (USB Serial/JTAG on ESP32-C6)
  Serial.begin(115200);
  
  // Wait for serial connection (up to 3 seconds to avoid getting stuck without a serial monitor)
  // This can be commented out if debug output is not needed
  uint32_t startTime = millis();
  while (!Serial && (millis() - startTime < 3000)) {
    delay(10);
  }
  
  printf("\r\n========================================\r\n");
  printf("ESP32-C6 Image Display Demo (OOP Version)\r\n");
  printf("========================================\r\n\r\n");
  
  // 1. Print Flash Information
  printf("=== Flash Info ===\r\n");
  uint32_t flashSize = ESP.getFlashChipSize();
  uint16_t flashSizeMB = flashSize / (1024 * 1024);
  printf("Flash size: %d MB (%u bytes)\r\n", flashSizeMB, flashSize);
  printf("\r\n");
  
  // 2. Initialize Display (using the new object-oriented API)
  printf("=== Display Initialization ===\r\n");
  if (!display.begin()) {
    printf("ERROR: Display initialization failed!\r\n");
    return;
  }
  printf("✓ Display initialized: %dx%d\r\n", display.width(), display.height());
  display.setBacklight(100);  // 100% brightness
  printf("✓ Backlight set to 100%%\r\n");
  printf("\r\n");
  
  // 3. Initialize SD Card (using the new object-oriented API)
  printf("=== SD Card Initialization ===\r\n");
  if (sdcard.begin()) {
    printf("✓ SD Card initialized\r\n");
    printf("  - Type: %s\r\n", sdcard.getCardTypeName());
    printf("  - Size: %u MB\r\n", sdcard.getSizeMB());
  } else {
    printf("✗ SD Card initialization failed\r\n");
  }
  printf("\r\n");
  
  // 4. Display the first image
  printf("=== Starting Image Display ===\r\n");
  displayImage("/", ".png", 0);
  
  printf("\r\n========================================\r\n");
  printf("  System Ready!\r\n");
  printf("========================================\r\n");
  printf("- Auto-play: Images will change every ~1.5 seconds\r\n\r\n");
}

void loop()
{
  // Auto-play images (switch every 300 loops)
  autoPlayImages("/", ".png", 300);
  
  delay(5);
}
