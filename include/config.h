#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

#define FIRMWARE_VERSION "2.0.4"

// ==========================================
// 1. HARDWARE PINS (ESP32)
// ==========================================
#define PUMP_PIN            16   // MOSFET Gate - RIGHT SIDE
#define LED_PIN             32   // WS2812B Data - LEFT SIDE (Bottom)
#define GPS_RX_PIN          27   // GPS RX -> ESP TX - LEFT SIDE (Middle)
#define GPS_TX_PIN          26   // GPS TX -> ESP RX - LEFT SIDE (Middle)
#define BUTTON_PIN          4    // Handlebar Button - LEFT SIDE (Top)
#define BOOT_BUTTON_PIN     0    // Onboard Boot Button (GPIO 0)
#define RAIN_SENSOR_PIN     34   // Analog Input
#define AUX_PIN             17   // Aux Power Relay / Heated Grips
#define TEMP_SENSOR_PIN     33   // DS18B20 Data Line
#define IMU_SDA             21   // BNO085 I2C SDA
#define IMU_SCL             22   // BNO085 I2C SCL

#ifdef SD_LOGGING_ACTIVE
    #define SD_CS_PIN       5
    #define SD_MOSI_PIN     23
    #define SD_MISO_PIN     19
    #define SD_CLK_PIN      18
#endif

// ==========================================
// 2. PUMP CONTROL & PWM
// ==========================================
#define PUMP_ON             HIGH
#define PUMP_OFF            LOW
#define PUMP_USE_PWM        true    // Enable Soft-Start/Stop
#define PUMP_PWM_FREQ       5000    // 5 kHz (Safe for most solenoids)
#define PUMP_PWM_CHANNEL    0       // LEDC Channel
#define PUMP_PWM_RESOLUTION 8       // 8-bit (0-255)
#define PUMP_RAMP_UP_MS     12      // Soft-Start Duration
#define PUMP_RAMP_DOWN_MS   12      // Soft-Stop Duration
#define PUMP_SAFETY_CUTOFF_MS 30000 // HARD LIMIT: Max continuous run

// ==========================================
// 3. CORE OILING LOGIC & SPEEDS
// ==========================================
// Standard Oiling
#define PULSE_DURATION_MS   55      // Time the pump stays OPEN
#define PAUSE_DURATION_MS   2000    // Time between multiple pulses
#define MIN_SPEED_KMH       7.0     // Minimum speed to trigger oiling
#define MIN_ODOMETER_SPEED_KMH 2.0  // Count distance above this speed

// Limits
#define MAX_SPEED_KMH       250.0   // Plausibility Ceiling

// ==========================================
// 4. SPECIAL MODES CONFIGURATION
// ==========================================
// Shared Settings
#define FLUSH_OFFROAD_MIN_SPEED_KMH 4.0 // Minimum speed for Flush & Offroad

// Chain Flush Mode
#define FLUSH_DEFAULT_EVENTS    15    // Total cycles
#define FLUSH_DEFAULT_PULSES    2     // Pulses per cycle
#define FLUSH_DEFAULT_INTERVAL_SEC 5 // Seconds between cycles
#define FLUSH_PRESS_COUNT       4     // Button clicks to activate? (See code logic)

// Offroad Mode
#define OFFROAD_INTERVAL_MIN_DEFAULT 5 // Minutes between cycles
#define OFFROAD_PULSES_DEFAULT       3
#define OFFROAD_PRESS_COUNT          3 // Button clicks (See code logic)

// Bleeding (Maintenance)
#define BLEEDING_DURATION_MS    25000 // Total run time (25s)
#define BLEEDING_PULSE_MS       60    // Pulse length (Needs to match/exceed ramp)
#define BLEEDING_PAUSE_MS       320   // Cycle pause

// Startup Delay
#define STARTUP_DELAY_METERS_DEFAULT 250.0 // Distance before first oiling allowed

// ==========================================
// 5. TIMINGS & TIMEOUTS
// ==========================================
#define WIFI_TIMEOUT_MS         300000   // 5 Min: Disable AP
#define RAIN_MODE_AUTO_OFF_MS   1800000  // 30 Min: Auto-reset Rain Mode
#define SAVE_INTERVAL_MS        300000   // 5 Min: Save progress to NVS
#define STANDSTILL_SAVE_MS      120000   // 2 Min: Save if stopped
#define EMERGENCY_TIMEOUT_MS    180000   // 3 Min: Switch to Emergency Mode if NO GPS
#define TEMP_UPDATE_INTERVAL_MS 60000    // 1 Min: Read Temperature

// Button Interactions
#define BUTTON_SEQUENCE_TIMEOUT_MS 600   // Time to wait after last click before processing
#define WIFI_PRESS_COUNT        5        // 5 Clicks: WiFi Config
#define AUX_HOLD_MS             2000     // Hold >2s: Toggle Aux
#define FACTORY_RESET_PRESS_MS  10000    // Hold >10s at boot

// ==========================================
// 6. LED / UI CONFIGURATION
// ==========================================
#define NUM_LEDS                2
#define LED_BRIGHTNESS_DIM      64    // Standard status
#define LED_BRIGHTNESS_HIGH     153   // Active event

// Animation Timings (ms)
#define LED_PERIOD_OILING       1000
#define LED_PERIOD_EMERGENCY    1500
#define LED_PERIOD_WIFI         1500
#define LED_PERIOD_GPS          1000
#define LED_BLINK_FAST          100
#define LED_BLINK_TANK          2000
#define LED_PERIOD_FLUSH        500
#define LED_WIFI_SHOW_DURATION  10000

// ==========================================
// 7. SYSTEM & DEBUG
// ==========================================
#define GPS_BAUD                9600
#define AP_SSID                 "ChainJuicer"
// #define SD_LOGGING_ACTIVE      // Uncomment for SD Support
#define GPS_DEBUG                 // Uncomment for Serial Debug

#ifdef SD_LOGGING_ACTIVE
    #define LOG_FILE_PREFIX     "/log_"
    #define LOG_INTERVAL_MS     1000
#endif

// ==========================================
// 8. DATA STRUCTURES
// ==========================================
struct SpeedRange {
    float minSpeed;
    float maxSpeed;
    float intervalKm;
    int pulses;
};

const int NUM_RANGES = 5;

#endif

