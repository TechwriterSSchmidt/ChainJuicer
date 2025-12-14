#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Hardware Pins
#define PUMP_PIN 16        // Pin for MOSFET (Pump)
#define GPS_RX_PIN 26      // GPS RX to ESP TX
#define GPS_TX_PIN 27      // GPS TX to ESP RX
#define BUTTON_PIN 4       // Handlebar button (connected to GND, INPUT_PULLUP)
#define BOOT_BUTTON_PIN 0  // Onboard Boot Button (GPIO 0)
#define LED_PIN 32          // WS2812B Data Pin

#define GPS_BAUD 9600

// Debug Configuration
#define GPS_DEBUG          // Uncomment to enable GPS debug output on Serial

// LED Configuration
#define NUM_LEDS 2
#define LED_BRIGHTNESS_DIM 20   // Brightness for status LED during normal operation (0-255)
#define LED_BRIGHTNESS_HIGH 150 // Brightness for events

// LED Timings
#define LED_PERIOD_OILING 1500      // Breathing duration for Oiling
#define LED_PERIOD_EMERGENCY 1500   // Pulse cycle for Emergency Mode
#define LED_PERIOD_WIFI 2000        // Pulse cycle for WiFi
#define LED_PERIOD_GPS 1000         // Pulse cycle for GPS Search
#define LED_BLINK_FAST 100          // Fast blink (Bleeding/Reset)
#define LED_BLINK_TANK 2000         // Tank warning cycle
#define LED_WIFI_SHOW_DURATION 10000 // How long to show WiFi LED after activation

// Default Values
#define PULSE_DURATION_MS 250      // Duration of the pump impulse (HIGH)
#define PAUSE_DURATION_MS 750     // Pause between impulses (LOW)
#define MIN_SPEED_KMH 7.0         // Minimum speed for oiling (Standstill threshold)
#define MIN_ODOMETER_SPEED_KMH 5.0 // Minimum speed to count distance for odometer
#define MAX_SPEED_KMH 250.0        // Maximum speed of the motorcycle (Plausibility Check)
#define BLEEDING_DURATION_MS 10000 // 10 seconds pumping for bleeding

// Button Timings
#define RAIN_TOGGLE_MS 1500       // < 1.5s: Toggle Rain Mode
#define WIFI_PRESS_MS 3000        // > 3s: Activate WiFi (if standing still)
#define BLEEDING_PRESS_MS 10000   // > 10s: Start Bleeding Mode
#define FACTORY_RESET_PRESS_MS 10000 // > 10s at Boot: Factory Reset

// Safety
#define STARTUP_DELAY_MS 10000    // 10s delay after boot before pump is allowed to run
#define PUMP_SAFETY_CUTOFF_MS 10000 // HARD LIMIT: Max continuous pump run (10s)

// Timeouts & Intervals
#define WIFI_TIMEOUT_MS 300000    // 5 Minutes (5 * 60 * 1000)
#define RAIN_MODE_AUTO_OFF_MS 1800000 // 30 Minutes (30 * 60 * 1000)
#define SAVE_INTERVAL_MS 300000   // 5 Minutes (Regular Save)
#define STANDSTILL_SAVE_MS 120000 // 2 Minutes (Min interval for standstill save)
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
