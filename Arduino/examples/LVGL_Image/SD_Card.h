#pragma once
#include <Arduino.h>
#include <SD.h>
#include <FS.h>
#include <vector>
#include "SDCardConfig.h"

// SD card manager: initialization and file operations.
class SDCardManager {
public:
    struct CardInfo {
        uint8_t type;
        uint64_t totalBytes;
        uint64_t usedBytes;
        uint64_t freeBytes;
        uint16_t sizeMB;
        bool available;
        CardInfo() : type(0), totalBytes(0), usedBytes(0), freeBytes(0), sizeMB(0), available(false) {}
    };
    SDCardManager();
    SDCardManager(const SDCardConfig& config);
    ~SDCardManager();

    bool begin();
    bool isAvailable() const { return cardInfo_.available; }
    CardInfo getCardInfo() const { return cardInfo_; }
    uint16_t getSizeMB() const { return cardInfo_.sizeMB; }
    const char* getCardTypeName() const;

    // File operations
    bool fileExists(const char* directory, const char* fileName);
    std::vector<String> searchFiles(const char* directory, const char* extension, uint16_t maxFiles = 100);
    uint16_t getFileCount(const char* directory, const char* extension);
    static String removeExtension(const String& filename);
    static String joinPath(const char* directory, const char* filename);
private:
    SDCardConfig config_;
    CardInfo cardInfo_;
    bool initialized_;
    void updateCardInfo();
};

