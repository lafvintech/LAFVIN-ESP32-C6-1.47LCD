#include "SD_Card.h"



// ============================================================================
// SDCardManager Class Implementation
// ============================================================================

/**
 * Default constructor
 */
SDCardManager::SDCardManager()
    : config_(SDCardConfig::getDefault()),
      initialized_(false)
{
}

/**
 * Configuration constructor
 */
SDCardManager::SDCardManager(const SDCardConfig& config)
    : config_(config),
      initialized_(false)
{
}

/**
 * Destructor
 */
SDCardManager::~SDCardManager() {
    // Clean up resources (if needed)
}

/**
 * Initialize SD card
 */
bool SDCardManager::begin() {
    // Configure CS pin
    pinMode(config_.cs_pin, OUTPUT);
    digitalWrite(config_.cs_pin, HIGH);
    
    // Initialize SD card
    if (!SD.begin(config_.cs_pin, SPI)) {
        printf("SD card initialization failed!\r\n");
        cardInfo_.available = false;
        initialized_ = true;
        return false;
    }
    
    printf("SD card initialization successful!\r\n");
    
    // Update card information
    updateCardInfo();
    
    initialized_ = true;
    return cardInfo_.available;
}

/**
 * Update card information
 */
void SDCardManager::updateCardInfo() {
    cardInfo_.type = SD.cardType();
    
    if (cardInfo_.type == CARD_NONE) {
        printf("No SD card attached\r\n");
        cardInfo_.available = false;
        return;
    }
    
    // Get capacity information
    cardInfo_.totalBytes = SD.totalBytes();
    cardInfo_.usedBytes = SD.usedBytes();
    cardInfo_.freeBytes = cardInfo_.totalBytes - cardInfo_.usedBytes;
    cardInfo_.sizeMB = cardInfo_.totalBytes / (1024 * 1024);
    cardInfo_.available = true;
    
    // Print card information
    printf("SD Card Type: %s\r\n", getCardTypeName());
    printf("Total space: %llu bytes (%u MB)\r\n", cardInfo_.totalBytes, cardInfo_.sizeMB);
    printf("Used space: %llu bytes\r\n", cardInfo_.usedBytes);
    printf("Free space: %llu bytes\r\n", cardInfo_.freeBytes);
}

/**
 * Get card type name
 */
const char* SDCardManager::getCardTypeName() const {
    switch (cardInfo_.type) {
        case CARD_MMC:  return "MMC";
        case CARD_SD:   return "SDSC";
        case CARD_SDHC: return "SDHC";
        default:        return "UNKNOWN";
    }
}

/**
 * Search if file exists
 */
bool SDCardManager::fileExists(const char* directory, const char* fileName) {
    if (!cardInfo_.available) {
        printf("SD card not available\r\n");
        return false;
    }
    
    File dir = SD.open(directory);
    if (!dir) {
        printf("Path: <%s> does not exist\r\n", directory);
        return false;
    }
    
    File file = dir.openNextFile();
    while (file) {
        if (strcmp(file.name(), fileName) == 0) {
            String fullPath = joinPath(directory, fileName);
            printf("File '%s' found\r\n", fullPath.c_str());
            dir.close();
            return true;
        }
        file = dir.openNextFile();
    }
    
    String fullPath = joinPath(directory, fileName);
    printf("File '%s' not found\r\n", fullPath.c_str());
    dir.close();
    return false;
}

/**
 * Search files with specified extension
 */
std::vector<String> SDCardManager::searchFiles(const char* directory, 
                                                const char* extension,
                                                uint16_t maxFiles) {
    std::vector<String> fileList;
    
    if (!cardInfo_.available) {
        printf("SD card not available\r\n");
        return fileList;
    }
    
    File dir = SD.open(directory);
    if (!dir) {
        printf("Path: <%s> does not exist\r\n", directory);
        return fileList;
    }
    
    File file = dir.openNextFile();
    while (file && fileList.size() < maxFiles) {
        if (!file.isDirectory() && strstr(file.name(), extension)) {
            fileList.push_back(String(file.name()));
            String fullPath = joinPath(directory, file.name());
            printf("File found: %s\r\n", fullPath.c_str());
        }
        file = dir.openNextFile();
    }
    
    dir.close();
    
    if (fileList.size() > 0) {
        printf("%d <%s> files were retrieved\r\n", fileList.size(), extension);
    } else {
        printf("No files with extension '%s' found in directory: %s\r\n", 
               extension, directory);
    }
    
    return fileList;
}

/**
 * Get file count
 */
uint16_t SDCardManager::getFileCount(const char* directory, const char* extension) {
    return searchFiles(directory, extension).size();
}

/**
 * Remove file extension (static utility method)
 */
String SDCardManager::removeExtension(const String& filename) {
    int dotPos = filename.lastIndexOf('.');
    if (dotPos > 0) {
        return filename.substring(0, dotPos);
    }
    return filename;
}

/**
 * Join path (static utility method)
 */
String SDCardManager::joinPath(const char* directory, const char* filename) {
    String path;
    if (strcmp(directory, "/") == 0) {
        path = String(directory) + filename;
    } else {
        path = String(directory) + "/" + filename;
    }
    return path;
}

