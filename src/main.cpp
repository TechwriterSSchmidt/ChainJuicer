/**
 * @file main.cpp
 * @brief Main application for ChainJuicer-light GPS-controlled chain oiler
 * 
 * ESP32-based automatic chain oiler system with GPS speed control and OLED display
 * 
 * Hardware Requirements:
 * - ESP32 Development Board
 * - GPS Module (UART) connected to GPIO16 (RX) and GPIO17 (TX)
 * - OLED Display (SSD1306, I2C) connected to GPIO21 (SDA) and GPIO22 (SCL)
 * - Pump/Servo connected to GPIO25 (PWM)
 * - Status LED on GPIO2
 * 
 * Features:
 * - GPS speed-based automatic chain oiling
 * - Adjustable oiling intervals based on speed
 * - OLED display showing speed, GPS status, and oiling info
 * - Distance tracking
 * - Oil shot counting
 * - Serial debug output
 * 
 * @author ChainJuicer Development Team
 * @version 1.0
 */

#include <Arduino.h>
#include "config.h"
#include "ChainOiler.h"

#if DISPLAY_ENABLED
#include "Display.h"
#endif

// Global objects
ChainOiler chainOiler;

#if DISPLAY_ENABLED
Display display;
#endif

/**
 * @brief Arduino setup function - runs once at startup
 */
void setup() {
    // Initialize serial for debugging
    #if SERIAL_DEBUG
    Serial.begin(SERIAL_BAUDRATE);
    delay(1000);
    Serial.println(F("\n\n================================="));
    Serial.println(F("ChainJuicer-light v1.0"));
    Serial.println(F("GPS-Controlled Chain Oiler"));
    Serial.println(F("ESP32 + Arduino Framework"));
    Serial.println(F("=================================\n"));
    #endif
    
    // Initialize display
    #if DISPLAY_ENABLED
    if (display.begin()) {
        display.showSplash();
        delay(2000);
    } else {
        #if SERIAL_DEBUG
        Serial.println(F("Warning: Display initialization failed"));
        #endif
    }
    #endif
    
    // Initialize chain oiler
    if (!chainOiler.begin()) {
        #if SERIAL_DEBUG
        Serial.println(F("ERROR: ChainOiler initialization failed!"));
        #endif
        while (1) {
            delay(1000);
        }
    }
    
    #if SERIAL_DEBUG
    Serial.println(F("\nSystem ready!"));
    Serial.println(F("Waiting for GPS fix..."));
    Serial.println(F("\nSpeed thresholds:"));
    Serial.printf("  Min: %.1f km/h\n", SPEED_THRESHOLD_MIN);
    Serial.printf("  Max: %.1f km/h\n", SPEED_THRESHOLD_MAX);
    Serial.println(F("\nOil intervals:"));
    Serial.printf("  Min: %d ms (high speed)\n", OIL_INTERVAL_MIN);
    Serial.printf("  Max: %d ms (low speed)\n", OIL_INTERVAL_MAX);
    Serial.printf("  Pulse duration: %d ms\n", OIL_PULSE_DURATION);
    Serial.println(F("\n=================================\n"));
    #endif
}

/**
 * @brief Arduino main loop - runs continuously
 */
void loop() {
    // Update chain oiler (GPS and pump control)
    chainOiler.update();
    
    // Update display
    #if DISPLAY_ENABLED
    display.update(chainOiler);
    #endif
    
    // Small delay to prevent overwhelming the system
    delay(10);
}
