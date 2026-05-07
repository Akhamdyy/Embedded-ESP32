/******************************************************************************
 *
 * Module: Platform
 *
 * File Name: platform.h
 *
 * Description: Platform-specific attribute macros for ESP32.
 *              Provides portable aliases so HAL modules never include
 *              ESP-IDF headers directly for compiler attributes.
 *
 *******************************************************************************/

#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "esp_attr.h"

/* Place ISR callbacks in IRAM so they execute safely if flash cache is busy */
#define ISR_ATTR    IRAM_ATTR

#endif /* PLATFORM_H_ */
