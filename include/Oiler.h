#ifndef OILER_H
#define OILER_H

#include "config.h"
#include <TinyGPS++.h>
#include <Adafruit_NeoPixel.h>

#define SPEED_BUFFER_SIZE 5
#define LUT_STEP 5
#define LUT_MAX_SPEED ((int)MAX_SPEED_KMH)
#define LUT_SIZE ((LUT_MAX_SPEED / LUT_STEP) + 1)

class Oiler {
public:
    Oiler();
    void begin();
    void update(float speedKmh, double lat, double lon, bool gpsValid);
    void loop(); // Main loop for button and LED
    void saveConfig();
    void saveProgress(); // Public for manual saving
    SpeedRange* getRangeConfig(int index);
    float getCurrentProgress() { return currentProgress; }
    bool isRainMode() { return rainMode; }
    void setRainMode(bool mode);

    // Emergency Mode
    bool isEmergencyMode() { return emergencyMode; }
    void setEmergencyMode(bool mode) { emergencyMode = mode; }

    // WiFi Status
    void setWifiActive(bool active) { wifiActive = active; }

    // Stats
    double getTotalDistance() { return totalDistance; }
    unsigned long getPumpCycles() { return pumpCycles; }
    void resetStats();
    
    // Time Stats
    double rangeTimeSeconds[NUM_RANGES];
    unsigned long rangeOilingCounts[NUM_RANGES]; // New: Count oilings per range
    double totalTimeSeconds;
    void resetTimeStats();

    // LED Settings
    uint8_t ledBrightnessDim;
    uint8_t ledBrightnessHigh;
    
    // Night Mode Settings
    bool nightModeEnabled;
    int nightStartHour;
    int nightEndHour;
    uint8_t nightBrightness;
    uint8_t nightBrightnessHigh;

    void setCurrentHour(int hour);

    void setOilDistance(unsigned long distance);
    unsigned long getOilDistance();
    
    bool isButtonPressed(); // Expose button state for main.cpp

    // Tank Monitor
    bool tankMonitorEnabled;
    float tankCapacityMl;
    float currentTankLevelMl;
    int dropsPerMl;
    int dropsPerPulse;
    int tankWarningThresholdPercent;

    void setTankFill(float levelMl); // Manually set level (e.g. refill)
    void resetTankToFull();

private:
    int pumpPin;
    int currentHour;
    SpeedRange ranges[NUM_RANGES];
    float intervalLUT[LUT_SIZE]; // Lookup Table for smoothed intervals
    void rebuildLUT(); // Helper to fill LUT

    float currentProgress; // 0.0 bis 1.0 (1.0 = Ölen fällig)
    
    double lastLat;
    double lastLon;
    bool hasFix;
    unsigned long lastSaveTime;
    bool progressChanged;

    // Stats
    double totalDistance;
    unsigned long pumpCycles;

    // GPS Smoothing
    float speedBuffer[SPEED_BUFFER_SIZE];
    int speedBufferIndex;

    // Taster & Modi
    bool rainMode;
    unsigned long rainModeStartTime;
    bool emergencyMode;
    bool wifiActive; // WiFi Status
    bool bleedingMode;
    unsigned long bleedingStartTime;
    unsigned long buttonPressStartTime;
    bool buttonState;
    bool lastButtonState;
    float currentSpeed; // Added for logic suppression
    float smoothedInterval; // Low-Pass Filter für Intervall
    
    // LED
    Adafruit_NeoPixel strip;
    unsigned long lastLedUpdate;
    void updateLED();
    void handleButton();
    void processPump(); // Unified pump logic

    void loadConfig();
    void validateConfig();
    // saveProgress ist jetzt public
    void processDistance(double distKm, float speedKmh);
    void triggerOil(int pulses);

    // Emergency update and standstill save time
    unsigned long lastEmergUpdate;
    unsigned long lastStandstillSaveTime;
    unsigned long lastTimeUpdate; // For time stats calculation

    // Non-blocking oiling state
    bool isOiling;
    unsigned long oilingStartTime;
    int oilingPulsesRemaining;
    unsigned long lastPulseTime;
    bool pulseState; // true = HIGH, false = LOW

    // Safety & UX
    unsigned long startupTime;
    unsigned long ledOilingEndTimestamp;
};

#endif
