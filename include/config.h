#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Hardware Pins
#define PUMP_PIN 27        // Pin for MOSFET (Pump)
#define GPS_RX_PIN 32      // GPS RX to ESP TX
#define GPS_TX_PIN 33      // GPS TX to ESP RX
#define BUTTON_PIN 4       // Handlebar button (connected to GND, INPUT_PULLUP)
#define BOOT_BUTTON_PIN 0  // Onboard Boot Button (GPIO 0)
#define LED_PIN 5          // WS2812B Data Pin

#define GPS_BAUD 38400

// LED Configuration
#define NUM_LEDS 1
#define LED_BRIGHTNESS_DIM 20   // Brightness for status (0-255) - not too bright
#define LED_BRIGHTNESS_HIGH 150 // Brightness for events

// Default Values
#define PULSE_DURATION_MS 50
#define MIN_SPEED_KMH 10.0
#define BLEEDING_DURATION_MS 5000 // 5 seconds pumping for bleeding

// Button Timings
#define RAIN_TOGGLE_MS 1500       // < 1.5s: Toggle Rain Mode
#define WIFI_PRESS_MS 3000        // > 3s: Activate WiFi (if standing still)
#define BLEEDING_PRESS_MS 10000   // > 10s: Start Bleeding Mode

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
