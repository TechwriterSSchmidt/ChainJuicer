#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Hardware Pins
#define PUMP_PIN 16        // Pin for MOSFET (Pump) - RIGHT SIDE
#define LED_PIN 32
         // WS2812B Data Pin - LEFT SIDE (Bottom)
#define GPS_RX_PIN 26      // GPS RX to ESP TX - LEFT SIDE (Middle)
#define GPS_TX_PIN 27      // GPS TX to ESP RX - LEFT SIDE (Middle)
#define BUTTON_PIN 25      // Handlebar button - LEFT SIDE (Middle)

// Pump Logic Configuration
// Set to true if using an NPN transistor (J3Y) to drive the MOSFET (Inverted Logic)
// IMPORTANT: If PUMP_INVERTED is false (Normal), use a Pull-DOWN Resistor (10k to GND) to prevent boot glitches.
// If PUMP_INVERTED is true, use a Pull-UP Resistor.
#define PUMP_INVERTED false

#if PUMP_INVERTED
  #define PUMP_ON LOW
  #define PUMP_OFF HIGH
#else
  #define PUMP_ON HIGH
  #define PUMP_OFF LOW
#endif

#define BOOT_BUTTON_PIN 0  // Onboard Boot Button (GPIO 0)

#define GPS_BAUD 115200

// Debug Configuration
#define GPS_DEBUG          // Uncomment to enable GPS debug output on Serial

// LED Configuration
#define NUM_LEDS 2
#define LED_BRIGHTNESS_DIM 20   // Brightness for status LED during normal operation (0-255)
#define LED_BRIGHTNESS_HIGH 150 // Brightness for events (0-255)

// Default Values
#define PULSE_DURATION_MS 150      // Duration in ms of the pump impulse (HIGH)
#define PAUSE_DURATION_MS 850     // Pause in ms between impulses (LOW)
#define MIN_SPEED_KMH 7.0         // Minimum speed for oiling (Standstill threshold)
#define MIN_ODOMETER_SPEED_KMH 2.0 // Minimum speed to count distance for odometer (less restrictive than MIN_SPEED_KMH for more accurate reading)
#define MAX_SPEED_KMH 250.0        // Maximum speed of the motorcycle (Plausibility Check)
#define BLEEDING_DURATION_MS 10000 // Pumping time in ms for bleeding

// Button Timings
#define RAIN_TOGGLE_MS 1500       // < 1.5s: Toggle Rain Mode
#define WIFI_PRESS_MS 3000        // > 3s: Activate WiFi (if standing still)
#define BLEEDING_PRESS_MS 10000   // > 10s: Start Bleeding Mode

// Safety
// STARTUP_DELAY_MS removed as hardware pull-down handles boot glitches

// Timeouts & Intervals
#define WIFI_TIMEOUT_MS 300000    // 5 Minutes (5 * 60 * 1000)
#define RAIN_MODE_AUTO_OFF_MS 1800000 // 30 Minutes (30 * 60 * 1000)
#define SAVE_INTERVAL_MS 300000   // 5 Minutes (Regular Save)
#define STANDSTILL_SAVE_MS 120000 // 2 Minutes (Min interval for standstill save)
#define EMERGENCY_WAIT_MS 300000  // 5 Minutes (Wait before Auto-Emergency)
#define EMERGENCY_OIL_1_MS 900000 // 15 Minutes (1st Oil)
#define EMERGENCY_OIL_2_MS 1800000 // 30 Minutes (2nd Oil)
#define EMERGENCY_TIMEOUT_MS 1860000 // 31 Minutes (Timeout)

// AP Configuration
#define AP_SSID "ChainJuicer"
// No password required


struct SpeedRange {
    float minSpeed;
    float maxSpeed;
    float intervalKm;
    int pulses;
};

// 5 Speed Ranges
// Default: 15km interval, 2 pulses
const int NUM_RANGES = 5;

#endif

