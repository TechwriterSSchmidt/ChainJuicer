/**
 * @file Display.cpp
 * @brief Implementation of Display class
 */

#include "Display.h"

Display::Display() 
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
      lastUpdate(0)
{
}

bool Display::begin() {
    // Initialize I2C
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
    
    // Initialize display
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        #if SERIAL_DEBUG
        Serial.println(F("Display: SSD1306 allocation failed"));
        #endif
        return false;
    }
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    
    #if SERIAL_DEBUG
    Serial.println(F("Display: Initialized"));
    #endif
    
    return true;
}

void Display::showSplash() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.println(F("ChainJuicer"));
    display.setTextSize(1);
    display.setCursor(0, 35);
    display.println(F("GPS Chain Oiler"));
    display.setCursor(0, 50);
    display.println(F("v1.0 - ESP32"));
    display.display();
}

void Display::clear() {
    display.clearDisplay();
    display.display();
}

void Display::update(const ChainOiler& oiler) {
    unsigned long currentMillis = millis();
    
    // Throttle display updates
    if (currentMillis - lastUpdate < DISPLAY_UPDATE_INTERVAL) {
        return;
    }
    lastUpdate = currentMillis;
    
    display.clearDisplay();
    
    // Draw GPS status (top left)
    drawGPSStatus(oiler.hasGPSFix(), oiler.getSatellites());
    
    // Draw speed (large, center)
    drawSpeed(oiler.getSpeed());
    
    // Draw oil status
    drawOilStatus(oiler.getPumpDuty(), oiler.getTimeUntilNextOil());
    
    // Draw statistics (bottom)
    drawStats(oiler.getTotalDistance(), oiler.getOilShotCount());
    
    display.display();
}

void Display::drawGPSStatus(bool hasFixParam, uint8_t sats) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    
    if (hasFixParam) {
        display.print(F("GPS:"));
        display.print(sats);
        display.print(F(" SAT"));
    } else {
        display.print(F("GPS: NO FIX"));
    }
}

void Display::drawSpeed(float speed) {
    display.setTextSize(3);
    display.setCursor(0, 15);
    
    if (speed < 100) {
        display.print(F(" "));
    }
    if (speed < 10) {
        display.print(F(" "));
    }
    
    display.print((int)speed);
    
    display.setTextSize(1);
    display.setCursor(90, 25);
    display.print(F("km/h"));
}

void Display::drawOilStatus(uint8_t duty, uint32_t timeUntilNext) {
    display.setTextSize(1);
    display.setCursor(0, 42);
    
    if (duty > 0) {
        display.print(F("OILING:"));
        display.print((duty * 100) / 255);
        display.print(F("%"));
    } else {
        display.print(F("Next: "));
        uint32_t seconds = timeUntilNext / 1000;
        if (seconds > 0) {
            display.print(seconds);
            display.print(F("s"));
        } else {
            display.print(F("---"));
        }
    }
}

void Display::drawStats(float distance, uint32_t oilCount) {
    display.setTextSize(1);
    
    // Distance
    display.setCursor(0, 54);
    display.print(F("Dist:"));
    display.print(distance, 1);
    display.print(F("km"));
    
    // Oil count
    display.setCursor(75, 54);
    display.print(F("Oil:"));
    display.print(oilCount);
}
