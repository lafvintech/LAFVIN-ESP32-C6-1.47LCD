#pragma once
#include <Arduino.h>

/**
 * System information class
 * Responsible for obtaining ESP32 system information (Flash, memory, etc.)
 */
class SystemInfo {
public:
    /**
     * Get Flash size (MB)
     * @return Flash size, in MB
     */
    static uint16_t getFlashSizeMB() {
        uint32_t flashSize = ESP.getFlashChipSize();
        return flashSize / (1024 * 1024);
    }
    /**
     * Get Flash size (bytes)
     * @return Flash size, in bytes
     */
    static uint32_t getFlashSizeBytes() {
        return ESP.getFlashChipSize();
    }
    /**
     * Get free heap memory (bytes)
     * @return Free heap size
     */
    static uint32_t getFreeHeap() {
        return ESP.getFreeHeap();
    }
    /**
     * Get total heap memory (bytes)
     * @return Total heap size
     */
    static uint32_t getTotalHeap() {
        return ESP.getHeapSize();
    }
    /**
     * Get used heap memory percentage
     * @return Used percent 0-100
     */
    static uint8_t getHeapUsagePercent() {
        uint32_t total = getTotalHeap();
        if (total == 0) return 0;
        uint32_t used = total - getFreeHeap();
        return (used * 100) / total;
    }
    /**
     * Print Flash information to the serial port
     */
    static void printFlashInfo() {
        printf("/********** Flash Info **********/\r\n");
        printf("Flash size: %d MB\r\n", getFlashSizeMB());
        printf("Flash size: %u bytes\r\n", getFlashSizeBytes());
        printf("/******** Flash Info End ********/\r\n\r\n");
    }
    /**
     * Print memory information to the serial port
     */
    static void printMemoryInfo() {
        printf("/********** Memory Info **********/\r\n");
        printf("Total heap: %u bytes\r\n", getTotalHeap());
        printf("Free heap: %u bytes\r\n", getFreeHeap());
        printf("Used heap: %u bytes (%d%%)\r\n",
               getTotalHeap() - getFreeHeap(),
               getHeapUsagePercent());
        printf("/******** Memory Info End ********/\r\n\r\n");
    }
    /**
     * Print all system information
     */
    static void printAllInfo() {
        printFlashInfo();
        printMemoryInfo();
    }
};

