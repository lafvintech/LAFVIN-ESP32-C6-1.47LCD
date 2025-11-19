#include "Display_ST7789.h"
   
#define SPI_WRITE(_dat)         SPI.transfer(_dat)
#define SPI_WRITE_Word(_dat)    SPI.transfer16(_dat)



// ============================================================================
// ST7789Display Class Implementation
// ============================================================================

/**
 * Default constructor - uses hardware default configuration
 */
ST7789Display::ST7789Display() 
    : _pin_cs(EXAMPLE_PIN_NUM_LCD_CS),
      _pin_dc(EXAMPLE_PIN_NUM_LCD_DC),
      _pin_rst(EXAMPLE_PIN_NUM_LCD_RST),
      _pin_backlight(EXAMPLE_PIN_NUM_BK_LIGHT),
      _width(LCD_WIDTH),
      _height(LCD_HEIGHT),
      _offset_x(Offset_X),
      _offset_y(Offset_Y),
      _horizontal(HORIZONTAL),
      _spi_freq(SPIFreq),
      _backlight_freq(Frequency),
      _backlight_resolution(Resolution),
      _initialized(false)
{
}

/**
 * Configuration constructor - uses custom configuration
 */
ST7789Display::ST7789Display(const ST7789Config& config)
    : _pin_cs(config.pin_cs),
      _pin_dc(config.pin_dc),
      _pin_rst(config.pin_rst),
      _pin_backlight(config.pin_backlight),
      _width(config.width),
      _height(config.height),
      _offset_x(config.offset_x),
      _offset_y(config.offset_y),
      _horizontal(config.horizontal),
      _spi_freq(config.spi_freq),
      _backlight_freq(config.backlight_freq),
      _backlight_resolution(config.backlight_resolution),
      _initialized(false)
{
}

/**
 * Destructor
 */
ST7789Display::~ST7789Display() {
    // Clean up resources (if needed)
}

/**
 * Initialize SPI bus
 */
void ST7789Display::spiInit() {
    SPI.begin(EXAMPLE_PIN_NUM_SCLK, EXAMPLE_PIN_NUM_MISO, EXAMPLE_PIN_NUM_MOSI);
}

/**
 * Write command to LCD
 */
void ST7789Display::writeCommand(uint8_t cmd) {
    SPI.beginTransaction(SPISettings(_spi_freq, MSBFIRST, SPI_MODE0));
    digitalWrite(_pin_cs, LOW);
    digitalWrite(_pin_dc, LOW);
    SPI_WRITE(cmd);
    digitalWrite(_pin_cs, HIGH);
    SPI.endTransaction();
}

/**
 * Write 8-bit data to LCD
 */
void ST7789Display::writeData(uint8_t data) {
    SPI.beginTransaction(SPISettings(_spi_freq, MSBFIRST, SPI_MODE0));
    digitalWrite(_pin_cs, LOW);
    digitalWrite(_pin_dc, HIGH);
    SPI_WRITE(data);
    digitalWrite(_pin_cs, HIGH);
    SPI.endTransaction();
}

/**
 * Write 16-bit data to LCD
 */
void ST7789Display::writeData16(uint16_t data) {
    SPI.beginTransaction(SPISettings(_spi_freq, MSBFIRST, SPI_MODE0));
    digitalWrite(_pin_cs, LOW);
    digitalWrite(_pin_dc, HIGH);
    SPI_WRITE_Word(data);
    digitalWrite(_pin_cs, HIGH);
    SPI.endTransaction();
}

/**
 * Write data bytes to LCD
 */
void ST7789Display::writeDataBytes(uint8_t* data, uint32_t len) {
    SPI.beginTransaction(SPISettings(_spi_freq, MSBFIRST, SPI_MODE0));
    digitalWrite(_pin_cs, LOW);
    digitalWrite(_pin_dc, HIGH);
    SPI.transferBytes(data, nullptr, len);
    digitalWrite(_pin_cs, HIGH);
    SPI.endTransaction();
}

/**
 * Hardware reset
 */
void ST7789Display::hardwareReset() {
    digitalWrite(_pin_cs, LOW);
    delay(50);
    digitalWrite(_pin_rst, LOW);
    delay(50);
    digitalWrite(_pin_rst, HIGH);
    delay(50);
}

/**
 * Initialize registers
 */
void ST7789Display::initRegisters() {
    //************* Start Initial Sequence **********// 
    writeCommand(0x11);
    delay(120);
    writeCommand(0x36);
    if (_horizontal)
        writeData(0x00);
    else
        writeData(0x70);

    writeCommand(0x3A);
    writeData(0x05);

    writeCommand(0xB0);
    writeData(0x00);
    writeData(0xE8);
    
    writeCommand(0xB2);
    writeData(0x0C);
    writeData(0x0C);
    writeData(0x00);
    writeData(0x33);
    writeData(0x33);

    writeCommand(0xB7);
    writeData(0x35);

    writeCommand(0xBB);
    writeData(0x35);

    writeCommand(0xC0);
    writeData(0x2C);

    writeCommand(0xC2);
    writeData(0x01);

    writeCommand(0xC3);
    writeData(0x13);

    writeCommand(0xC4);
    writeData(0x20);

    writeCommand(0xC6);
    writeData(0x0F);

    writeCommand(0xD0);
    writeData(0xA4);
    writeData(0xA1);

    writeCommand(0xD6);
    writeData(0xA1);

    writeCommand(0xE0);
    writeData(0xF0);
    writeData(0x00);
    writeData(0x04);
    writeData(0x04);
    writeData(0x04);
    writeData(0x05);
    writeData(0x29);
    writeData(0x33);
    writeData(0x3E);
    writeData(0x38);
    writeData(0x12);
    writeData(0x12);
    writeData(0x28);
    writeData(0x30);

    writeCommand(0xE1);
    writeData(0xF0);
    writeData(0x07);
    writeData(0x0A);
    writeData(0x0D);
    writeData(0x0B);
    writeData(0x07);
    writeData(0x28);
    writeData(0x33);
    writeData(0x3E);
    writeData(0x36);
    writeData(0x14);
    writeData(0x14);
    writeData(0x29);
    writeData(0x32);

    writeCommand(0x21);

    writeCommand(0x11);
    delay(120);
    writeCommand(0x29); 
}

/**
 * Initialize backlight PWM
 */
void ST7789Display::backlightInit() {
    ledcAttach(_pin_backlight, _backlight_freq, _backlight_resolution);
    // Default to 10% brightness (consistent with original code)
    // 10-bit PWM: 10% = 100/1000 (default value in original code)
    ledcWrite(_pin_backlight, 100); // 100 here is the 10% PWM value
}

/**
 * Complete LCD initialization
 */
bool ST7789Display::begin() {
    // Configure GPIO pins
    pinMode(_pin_cs, OUTPUT);
    pinMode(_pin_dc, OUTPUT);
    pinMode(_pin_rst, OUTPUT);
    
    // Initialize backlight
    backlightInit();
    
    // Initialize SPI
    spiInit();
    
    // Hardware reset
    hardwareReset();
    
    // Initialize registers
    initRegisters();
    
    _initialized = true;
    return true;
}

/**
 * Set drawing window
 */
void ST7789Display::setWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    if (_horizontal) {
        // Horizontal mode
        // Set X coordinate
        writeCommand(0x2A);
        writeData(x1 >> 8);
        writeData(x1 + _offset_x);
        writeData(x2 >> 8);
        writeData(x2 + _offset_x);
        
        // Set Y coordinate
        writeCommand(0x2B);
        writeData(y1 >> 8);
        writeData(y1 + _offset_y);
        writeData(y2 >> 8);
        writeData(y2 + _offset_y);
    } else {
        // Vertical mode
        // Set X coordinate
        writeCommand(0x2A);
        writeData(y1 >> 8);
        writeData(y1 + _offset_y);
        writeData(y2 >> 8);
        writeData(y2 + _offset_y);
        
        // Set Y coordinate
        writeCommand(0x2B);
        writeData(x1 >> 8);
        writeData(x1 + _offset_x);
        writeData(x2 >> 8);
        writeData(x2 + _offset_x);
    }
    writeCommand(0x2C);
}

/**
 * Draw pixel buffer
 */
void ST7789Display::drawPixelBuffer(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t* buffer) {
    uint16_t width = x2 - x1 + 1;
    uint16_t height = y2 - y1 + 1;
    uint32_t numBytes = width * height * sizeof(uint16_t);
    
    setWindow(x1, y1, x2, y2);
    writeDataBytes((uint8_t*)buffer, numBytes);
}

/**
 * Clear screen
 */
void ST7789Display::clearScreen(uint16_t color) {
    // Create line buffer
    uint16_t lineBuffer[_width];
    for (uint16_t i = 0; i < _width; i++) {
        lineBuffer[i] = color;
    }
    
    setWindow(0, 0, _width - 1, _height - 1);
    
    // Fill line by line
    for (uint16_t y = 0; y < _height; y++) {
        writeDataBytes((uint8_t*)lineBuffer, _width * sizeof(uint16_t));
    }
}

/**
 * Set backlight brightness
 * @param brightness Brightness percentage 0-100
 */
void ST7789Display::setBacklight(uint8_t brightness) {
    if (brightness > 100) brightness = 100;
    
    // Use simple formula: brightness * 10
    // Map percentage 0-100 to PWM value 0-1000 (10-bit PWM max 1024)
    // Consistent with original code
    uint32_t duty = brightness * 10;
    ledcWrite(_pin_backlight, duty);
}







