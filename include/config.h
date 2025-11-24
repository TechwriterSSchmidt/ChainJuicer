/**
 * @file config.h
 * @brief Configuration settings for ChainJuicer-light GPS-controlled chain oiler
 * 
 * ESP32-based automatic chain oiler system with GPS speed control
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// GPIO Pin Definitions
// ============================================================================

// GPS Module (Serial2)
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17

// Chain Oiler Pump Control
#define PUMP_PIN 25          // PWM pin for pump/servo control
#define PUMP_PWM_CHANNEL 0   // PWM channel
#define PUMP_PWM_FREQ 1000   // PWM frequency in Hz
#define PUMP_PWM_RESOLUTION 8 // 8-bit resolution (0-255)

// OLED Display (I2C)
#define OLED_SDA_PIN 21
#define OLED_SCL_PIN 22
#define OLED_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Status LED
#define STATUS_LED_PIN 2

// ============================================================================
// GPS Settings
// ============================================================================

#define GPS_BAUDRATE 9600
#define GPS_UPDATE_INTERVAL 1000  // Update GPS data every 1 second (ms)

// ============================================================================
// Chain Oiler Logic Settings
// ============================================================================

// Speed thresholds (km/h)
#define SPEED_THRESHOLD_MIN 5.0f    // Minimum speed to activate oiler
#define SPEED_THRESHOLD_MAX 120.0f  // Maximum speed (cap for calculations)

// Pump control settings
#define PUMP_OFF_DUTY 0            // Pump off (PWM duty cycle)
#define PUMP_MIN_DUTY 50           // Minimum pump duty cycle (0-255)
#define PUMP_MAX_DUTY 200          // Maximum pump duty cycle (0-255)

// Oil delivery intervals (milliseconds)
#define OIL_INTERVAL_MIN 5000      // Minimum interval between oil shots at high speed
#define OIL_INTERVAL_MAX 30000     // Maximum interval between oil shots at low speed
#define OIL_PULSE_DURATION 200     // Duration of oil pulse in ms

// Temperature compensation (optional, for future use)
#define TEMP_COMPENSATION_ENABLED false

// ============================================================================
// Display Settings
// ============================================================================

#define DISPLAY_UPDATE_INTERVAL 500  // Update display every 500ms
#define DISPLAY_ENABLED true

// ============================================================================
// System Settings
// ============================================================================

#define SERIAL_DEBUG true
#define SERIAL_BAUDRATE 115200

// GPS fix timeout
#define GPS_FIX_TIMEOUT 10000  // Consider GPS lost if no fix for 10 seconds

// Satellite count minimum for valid fix
#define MIN_SATELLITES 4

#endif // CONFIG_H
