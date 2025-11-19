#include "LCD_Image.h"

// ============================================================================
// External References
// ============================================================================
extern ST7789Display display;  // Defined in LVGL_Image.ino
extern SDCardManager sdcard;   // Defined in LVGL_Image.ino

// ============================================================================
// Module-internal State (Encapsulated Global Variables)
// ============================================================================
static PNG pngDecoder;                          // PNG decoder instance
static File currentImageFile;                   // Currently open image file
static std::vector<String> imageFileList;       // Image file list (using vector instead of a fixed array)
static String currentDirectory = "";            // Current search directory
static String currentExtension = "";            // Current file extension
static uint16_t currentImageIndex = 0;          // Index of the currently displayed image
static uint32_t autoPlayCounter = 0;            // Auto-play counter

// Image display position
static int16_t imageXPos = 0;
static int16_t imageYPos = 0;

// Line buffer (for PNG decoding)
static uint16_t lineBuffer[MAX_IMAGE_WIDTH];

// ============================================================================
// PNG Decoding Callback Functions
// ============================================================================

/**
 * PNG open callback
 */
void* pngOpen(const char* filePath, int32_t* size) {
    currentImageFile = SD.open(filePath);
    if (currentImageFile) {
        *size = currentImageFile.size();
    } else {
        *size = 0;
    }
    return &currentImageFile;
}

/**
 * PNG close callback
 */
void pngClose(void* handle) {
    File imageFile = *((File*)handle);
    if (imageFile) {
        imageFile.close();
    }
}

/**
 * PNG read callback
 */
int32_t pngRead(PNGFILE* page, uint8_t* buffer, int32_t length) {
    if (!currentImageFile) return 0;
    (void)page; // Avoid unused parameter warning
    return currentImageFile.read(buffer, length);
}

/**
 * PNG seek callback
 */
int32_t pngSeek(PNGFILE* page, int32_t position) {
    if (!currentImageFile) return 0;
    (void)page; // Avoid unused parameter warning
    return currentImageFile.seek(position);
}

/**
 * PNG draw callback
 * Called for each decoded line of PNG data, used to render the data to the LCD
 */
void pngDraw(PNGDRAW* pDraw) {
    // Convert PNG data to RGB565 format
    pngDecoder.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
    
    // Byte order conversion (big-endian to little-endian)
    uint32_t pixelCount = pDraw->iWidth;
    for (uint32_t i = 0; i < pixelCount; i++) {
        lineBuffer[i] = ((lineBuffer[i] >> 8) & 0xFF) | ((lineBuffer[i] << 8) & 0xFF00);
    }
    
    // Draw to LCD (using OOP API)
    display.drawPixelBuffer(imageXPos, 
                            pDraw->y, 
                            imageXPos + pDraw->iWidth - 1, 
                            imageYPos + pDraw->y, 
                            lineBuffer);
}

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * Build the full file path
 * @param directory Directory path
 * @param filename File name
 * @return Full path
 */
static String buildFilePath(const char* directory, const char* filename) {
    String path;
    if (strcmp(directory, "/") == 0) {
        path = String(directory) + filename;
    } else {
        path = String(directory) + "/" + filename;
    }
    return path;
}

// ============================================================================
// Public Interface Implementation
// ============================================================================

/**
 * Search for image files in the specified directory
 */
uint16_t searchImages(const char* directory, const char* fileExtension) {
    // Clear the previous list
    imageFileList.clear();
    
    // Save the current search parameters
    currentDirectory = directory;
    currentExtension = fileExtension;
    
    // Use the SD card manager to search for files (using OOP API)
    if (!sdcard.isAvailable()) {
        printf("SD card not available for image search\r\n");
        return 0;
    }
    
    // Search for files (up to 100)
    imageFileList = sdcard.searchFiles(directory, fileExtension, 100);
    
    printf("Found %d image files in '%s'\r\n", imageFileList.size(), directory);
    
    return imageFileList.size();
}

/**
 * Display the image at the specified path
 */
bool showImage(const char* filePath) {
    printf("Displaying image: %s\r\n", filePath);
    
    // Open the PNG file
    int16_t result = pngDecoder.open(filePath, pngOpen, pngClose, pngRead, pngSeek, pngDraw);
    
    if (result == PNG_SUCCESS) {
        printf("Image specs: (%d x %d), %d bpp, pixel type: %d\r\n", 
               pngDecoder.getWidth(), 
               pngDecoder.getHeight(), 
               pngDecoder.getBpp(), 
               pngDecoder.getPixelType());
        
        // Check image width
        if (pngDecoder.getWidth() > MAX_IMAGE_WIDTH) {
            printf("ERROR: Image width (%d) exceeds buffer size (%d)\r\n", 
                   pngDecoder.getWidth(), MAX_IMAGE_WIDTH);
            pngDecoder.close();
            return false;
        }
        
        // Decode and display
        uint32_t startTime = millis();
        result = pngDecoder.decode(NULL, 0);
        pngDecoder.close();
        
        uint32_t decodeTime = millis() - startTime;
        printf("Decode time: %u ms\r\n", decodeTime);
        
        return true;
    } else {
        printf("ERROR: Failed to open PNG file (error code: %d)\r\n", result);
        return false;
    }
}

/**
 * Display the image at the specified index
 */
bool displayImage(const char* directory, const char* fileExtension, uint16_t imageIndex) {
    // If the list is empty or the parameters have changed, search again
    if (imageFileList.empty() || 
        currentDirectory != directory || 
        currentExtension != fileExtension) {
        searchImages(directory, fileExtension);
    }
    
    // Check index validity
    if (imageFileList.empty()) {
        printf("No images found in directory: %s\r\n", directory);
        return false;
    }
    
    if (imageIndex >= imageFileList.size()) {
        printf("ERROR: Image index %d out of range (0-%d)\r\n", 
               imageIndex, imageFileList.size() - 1);
        return false;
    }
    
    // Build the file path
    String filePath = buildFilePath(directory, imageFileList[imageIndex].c_str());
    
    printf("Display image [%d/%d]: %s\r\n", 
           imageIndex + 1, 
           imageFileList.size(), 
           filePath.c_str());
    
    // Display the image
    return showImage(filePath.c_str());
}

/**
 * Auto-play images in a loop
 */
void autoPlayImages(const char* directory, const char* fileExtension, uint32_t intervalCount) {
    autoPlayCounter++;
    
    if (autoPlayCounter >= intervalCount) {
        autoPlayCounter = 0;
        
        // Switch to the next image
        currentImageIndex++;
        if (currentImageIndex >= imageFileList.size()) {
            currentImageIndex = 0;  // Loop back to the first image
        }
        
        displayImage(directory, fileExtension, currentImageIndex);
    }
}

/**
 * Get the total number of current images
 */
uint16_t getImageCount() {
    return imageFileList.size();
}

/**
 * Get the index of the currently displayed image
 */
uint16_t getCurrentImageIndex() {
    return currentImageIndex;
}


