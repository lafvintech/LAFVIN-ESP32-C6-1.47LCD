#pragma once
#include <Arduino.h>
#include <SD.h>
#include <FS.h>
#include <vector>
#include "SDCardConfig.h"

// ============================================================================
// Object-Oriented Interface
// ============================================================================

/**
 * SD Card Management Class
 * Responsible for SD card initialization, file searching, and other operations
 */
class SDCardManager {
public:
    /**
     * SD Card Information Structure
     */
    struct CardInfo {
        uint8_t type;          // Card type: CARD_NONE, CARD_MMC, CARD_SD, CARD_SDHC
        uint64_t totalBytes;   // Total capacity (bytes)
        uint64_t usedBytes;    // Used (bytes)
        uint64_t freeBytes;    // Free space (bytes)
        uint16_t sizeMB;       // Total capacity (MB)
        bool available;        // Is available
        
        CardInfo() : type(CARD_NONE), totalBytes(0), usedBytes(0), 
                     freeBytes(0), sizeMB(0), available(false) {}
    };
    
    /**
     * Constructor - Uses default configuration
     */
    SDCardManager();
    
    /**
     * Constructor - Uses custom configuration
     * @param config Configuration structure
     */
    SDCardManager(const SDCardConfig& config);
    
    /**
     * Destructor
     */
    ~SDCardManager();
    
    // ========== Initialization Methods ==========
    
    /**
     * Initializes the SD card
     * @return true=success, false=failure
     */
    bool begin();
    
    /**
     * Checks if the SD card is available
     * @return true=available
     */
    bool isAvailable() const { return cardInfo_.available; }
    
    // ========== Card Information Methods ==========
    
    /**
     * Gets detailed SD card information
     * @return Card information structure
     */
    CardInfo getCardInfo() const { return cardInfo_; }
    
    /**
     * Gets SD card size (MB)
     * @return Size (MB)
     */
    uint16_t getSizeMB() const { return cardInfo_.sizeMB; }
    
    /**
     * Gets card type name
     * @return Type string
     */
    const char* getCardTypeName() const;
    
    // ========== File Operation Methods ==========
    
    /**
     * Checks if a file exists
     * @param directory Directory path
     * @param fileName File name
     * @return true=exists
     */
    bool fileExists(const char* directory, const char* fileName);
    
    /**
     * Searches for files with a specified extension
     * @param directory Directory path
     * @param extension File extension (e.g., ".png")
     * @param maxFiles Maximum number of files
     * @return List of file names
     */
    std::vector<String> searchFiles(const char* directory, 
                                     const char* extension,
                                     uint16_t maxFiles = 100);
    
    /**
     * Gets the number of files
     * @param directory Directory path
     * @param extension File extension
     * @return Number of files
     */
    uint16_t getFileCount(const char* directory, const char* extension);
    
    // ========== Utility Methods ==========
    
    /**
     * Removes the file extension
     * @param filename File name
     * @return File name without extension
     */
    static String removeExtension(const String& filename);
    
    /**
     * Joins paths
     * @param directory Directory
     * @param filename File name
     * @return Full path
     */
    static String joinPath(const char* directory, const char* filename);

private:
    SDCardConfig config_;
    CardInfo cardInfo_;
    bool initialized_;
    
    void updateCardInfo();
};

// ============================================================================
// Global Variables (for LVGL UI display)
// ============================================================================
extern uint16_t SDCard_Size;
extern uint16_t Flash_Size;