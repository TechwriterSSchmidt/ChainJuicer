#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Hardware Pins
#define PUMP_PIN 16        // Pin for MOSFET (Pump) - RIGHT SIDE
#define LED_PIN 32         // WS2812B Data Pin - LEFT SIDE (Bottom)
#define GPS_RX_PIN 27      // GPS RX to ESP TX - LEFT SIDE (Middle)
#define GPS_TX_PIN 26      // GPS TX to ESP RX - LEFT SIDE (Middle)
#define BUTTON_PIN 4      // Handlebar button - LEFT SIDE (Top of block)

// Pump Logic Configuration
// Normal Logic: HIGH = Pump ON, LOW = Pump OFF
// Use a Pull-DOWN Resistor (10k to GND) to prevent boot glitches.
#define PUMP_ON HIGH
#define PUMP_OFF LOW

// PWM Soft-Start / Soft-Stop (Silent Mode)
// Instead of hard 12V pulses, we ramp the voltage up and down.
// This reduces mechanical noise ("clack") and wear.
#define PUMP_USE_PWM true
#define PUMP_PWM_FREQ 5000      // 5 kHz is safe for most solenoids
#define PUMP_PWM_CHANNEL 0      // ESP32 LEDC Channel
#define PUMP_PWM_RESOLUTION 8   // 8-bit resolution (0-255)
#define PUMP_RAMP_UP_MS 12      // Soft-Start duration (Generic: 12ms)
#define PUMP_RAMP_DOWN_MS 12    // Soft-Stop duration (Generic: 12ms)

#define BOOT_BUTTON_PIN 0  // Onboard Boot Button (GPIO 0)

// Temperature Sensor (DS18B20)
#define TEMP_SENSOR_PIN 15 // GPIO 15 for DS18B20 Data
#define TEMP_UPDATE_INTERVAL_MS 60000 // Measure every 60 seconds

// Temperature Compensation Settings (Lookup Table)
// Hysteresis: Temp must cross the threshold by this amount to switch states.
// Prevents rapid toggling (e.g., at 15.0°C).
#define TEMP_HYSTERESIS_C 2.0 

// Range 1: Very Cold (< 5°C)
#define TEMP_R1_MAX 5.0
#define TEMP_R1_PULSE 90
#define TEMP_R1_PAUSE 2120

// Range 2: Cold (5 - 15°C)
#define TEMP_R2_MAX 15.0
#define TEMP_R2_PULSE 70
#define TEMP_R2_PAUSE 1590

// Range 3: Normal (15 - 25°C)
#define TEMP_R3_MAX 25.0
#define TEMP_R3_PULSE 50
#define TEMP_R3_PAUSE 1060

// Range 4: Warm (25 - 35°C)
#define TEMP_R4_MAX 35.0
#define TEMP_R4_PULSE 45
#define TEMP_R4_PAUSE 850

// Range 5: Hot (> 35°C)
#define TEMP_R5_PULSE 40
#define TEMP_R5_PAUSE 635

// PWM Safety Check:
// Pulse Width must be > PUMP_RAMP_UP_MS (12ms) to ensure the pump actually opens.
// The configured Pulse values above include the Ramp-Up time.

#define GPS_BAUD 9600  // GPS Baud Rate

// Debug Configuration
#define GPS_DEBUG          // Uncomment to enable GPS debug output on Serial

// LED Configuration
#define NUM_LEDS 2
#define LED_BRIGHTNESS_DIM 20   // Brightness for status LED during normal operation (0-255)
#define LED_BRIGHTNESS_HIGH 150 // Brightness for events (0-255)

// SD Logging Configuration
// Uncomment to enable SD Card Logging
#define SD_LOGGING_ACTIVE 

#ifdef SD_LOGGING_ACTIVE
    #define SD_CS_PIN 5
    #define SD_MOSI_PIN 23
    #define SD_MISO_PIN 19
    #define SD_CLK_PIN 18
    #define LOG_FILE_PREFIX "/log_"
    #define LOG_INTERVAL_MS 1000 // Log every 1 second
#endif

// LED Timings
#define LED_PERIOD_OILING 1500      // Breathing duration for Oiling
#define LED_PERIOD_EMERGENCY 1500   // Pulse cycle for Emergency Mode
#define LED_PERIOD_WIFI 2000        // Pulse cycle for WiFi
#define LED_PERIOD_GPS 1000         // Pulse cycle for GPS Search
#define LED_BLINK_FAST 100          // Fast blink (Bleeding/Reset)
#define LED_BLINK_TANK 2000         // Tank warning cycle
#define LED_WIFI_SHOW_DURATION 10000 // How long to show WiFi LED after activation

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
#define FACTORY_RESET_PRESS_MS 10000 // > 10s at Boot: Factory Reset

// Safety
// STARTUP_DELAY_MS removed as hardware pull-down handles boot glitches
#define PUMP_SAFETY_CUTOFF_MS 30000 // HARD LIMIT: Max continuous pump run (10s)

// Timeouts & Intervals
#define WIFI_TIMEOUT_MS 300000    // 5 Minutes (5 * 60 * 1000)
#define RAIN_MODE_AUTO_OFF_MS 1800000 // 30 Minutes (30 * 60 * 1000)
#define SAVE_INTERVAL_MS 300000   // 5 Minutes (Regular Save)
#define STANDSTILL_SAVE_MS 120000 // 2 Minutes (Min interval for standstill save)
#define EMERGENCY_TIMEOUT_MS 180000 // 3 Minutes (Timeout for Auto-Emergency)

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

