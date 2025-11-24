/**
 * @file Display.h
 * @brief Display manager for ChainJuicer-light
 * 
 * Manages OLED display updates and UI
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "ChainOiler.h"

/**
 * @class Display
 * @brief Handles OLED display for system status
 */
class Display {
public:
    /**
     * @brief Constructor
     */
    Display();
    
    /**
     * @brief Initialize display
     * @return true if successful
     */
    bool begin();
    
    /**
     * @brief Update display with current system status
     * @param oiler Reference to ChainOiler instance
     */
    void update(const ChainOiler& oiler);
    
    /**
     * @brief Show splash screen
     */
    void showSplash();
    
    /**
     * @brief Clear display
     */
    void clear();

private:
    Adafruit_SSD1306 display;
    unsigned long lastUpdate;
    
    /**
     * @brief Draw GPS status
     */
    void drawGPSStatus(bool hasFixParam, uint8_t sats);
    
    /**
     * @brief Draw speed
     */
    void drawSpeed(float speed);
    
    /**
     * @brief Draw oil status
     */
    void drawOilStatus(uint8_t duty, uint32_t timeUntilNext);
    
    /**
     * @brief Draw statistics
     */
    void drawStats(float distance, uint32_t oilCount);
};

#endif // DISPLAY_H
