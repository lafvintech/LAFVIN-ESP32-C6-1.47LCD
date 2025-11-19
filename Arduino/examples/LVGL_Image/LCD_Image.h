#pragma once

#include <PNGdec.h>
#include <vector>
#include "SD_Card.h"
#include "Display_ST7789.h"

// ============================================================================
// Configuration Constants
// ============================================================================
#define MAX_IMAGE_WIDTH  172  // Maximum image width (pixels)

// ============================================================================
// Image Management Functions
// ============================================================================

/**
 * Search for image files in the specified directory
 * @param directory Directory path
 * @param fileExtension File extension (e.g., ".png")
 * @return Number of images found
 */
uint16_t searchImages(const char* directory, const char* fileExtension);

/**
 * Display the image at the specified path
 * @param filePath Full path to the image
 * @return true=success, false=failure
 */
bool showImage(const char* filePath);

/**
 * Display the image at the specified index
 * @param directory Directory path
 * @param fileExtension File extension
 * @param imageIndex Image index (starting from 0)
 * @return true=success, false=failure
 */
bool displayImage(const char* directory, const char* fileExtension, uint16_t imageIndex);

/**
 * Auto-play images in a loop
 * @param directory Directory path
 * @param fileExtension File extension
 * @param intervalCount Switch interval (loop count)
 */
void autoPlayImages(const char* directory, const char* fileExtension, uint32_t intervalCount);

/**
 * Get the total number of current images
 * @return Number of images
 */
uint16_t getImageCount();

/**
 * Get the index of the currently displayed image
 * @return Image index
 */
uint16_t getCurrentImageIndex();