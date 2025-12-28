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
    
    // --- Configuration Getters ---
    SpeedRange* getRangeConfig(int index);

    // Temperature Configuration
    enum OilType {
        OIL_THIN = 0,
        OIL_NORMAL = 1,
        OIL_THICK = 2
    };

    struct TempConfig {
        float basePulse25; // Pulse width at 25°C
        float basePause25; // Pause duration at 25°C
        OilType oilType;
    };
    
    TempConfig tempConfig;
    float lastTemp; // For hysteresis
    
    bool isTempSensorConnected();
    
    // --- Logging & Stats Getters ---
    float getSmoothedSpeed() { return currentSpeed; }
    double getOdometer() { return totalDistance; }
    float getCurrentDistAccumulator() { return currentProgress * smoothedInterval; }
    float getCurrentTargetDistance() { return smoothedInterval; }
    bool isPumpRunning() { return isOiling; }
    float getCurrentProgress() { return currentProgress; }
    float getCurrentTempC() { return currentTempC; }
    
    // Mode Getters & Setters
    bool isRainMode() { return rainMode; }
    void setRainMode(bool mode);
    bool rainFlushEnabled; // Configurable setting

    bool isTurboMode() { return turboMode; }
    void setTurboMode(bool mode);

    bool isCrossCountryMode() { return crossCountryMode; }
    void setCrossCountryMode(bool mode);

    bool isEmergencyMode() { return emergencyMode; }
    void setEmergencyMode(bool mode) { emergencyMode = mode; }

    bool isEmergencyModeForced() { return emergencyModeForced; }
    void setEmergencyModeForced(bool forced);

    // Factory Reset
    void checkFactoryReset();

    // WiFi Status
    void setWifiActive(bool active);

    // Update Status
    void setUpdateMode(bool mode);

    // Stats
    double getTotalDistance() { return totalDistance; }
    unsigned long getPumpCycles() { return pumpCycles; }
    void resetStats();
    
    // Time Stats (History for last 20 oilings)
    struct StatsHistory {
        uint8_t head;
        uint8_t count;
        int8_t oilingRange[20];
        double timeInRanges[20][NUM_RANGES];
    };
    
    StatsHistory history;
    double currentIntervalTime[NUM_RANGES]; // Time accumulated in current interval (not yet oiled)
    
    // Helper to get summed stats for UI
    double getRecentTimeSeconds(int rangeIndex);
    int getRecentOilingCount(int rangeIndex);
    double getRecentTotalTime();

    void resetTimeStats();

    // Time Helper
    int calculateLocalHour(int utcHour, int day, int month, int year);

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

    // Cross Country Settings
    bool crossCountryMode;
    int crossCountryIntervalMin;
    unsigned long lastCrossCountryOilTime;

    // Startup Delay
    float startupDelayKm;
    float currentStartupDistance;

private:
    void processDistance(double distKm, float speedKmh);
    
    int pumpPin;
    int currentHour;
    bool updateMode;
    SpeedRange ranges[NUM_RANGES];
    float intervalLUT[LUT_SIZE]; // Lookup Table for smoothed intervals
    void rebuildLUT(); // Helper to fill LUT

    float currentProgress; // 0.0 to 1.0 (1.0 = Oiling due)
    
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

    // Button & Modes
    bool rainMode;
    unsigned long rainModeStartTime;
    bool turboMode;
    unsigned long turboModeStartTime;
    int buttonClickCount;
    unsigned long lastClickTime;
    bool emergencyMode;
    bool wifiActive; // WiFi Status
    bool bleedingMode;
    unsigned long bleedingStartTime;
    unsigned long wifiActivationTime;
    unsigned long buttonPressStartTime;
    bool buttonState;
    bool lastButtonState;
    unsigned long lastDebounceTime; // Added for Debouncing
    float currentSpeed; // Added for logic suppression
    float smoothedInterval; // Low-Pass Filter for Interval
    
    // LED
    Adafruit_NeoPixel strip;
    unsigned long lastLedUpdate;
    void updateLED();
    void handleButton();
    void processPump(); // Unified pump logic

    void loadConfig();
    void validateConfig();
    // saveProgress is public
    void triggerOil(int pulses);

    // Emergency update and standstill save time
    unsigned long lastEmergUpdate;
    unsigned long lastStandstillSaveTime;
    unsigned long lastTimeUpdate; // For time stats calculation

    // Non-blocking oiling state
    bool isOiling;
    bool isRainFlush; // State for Rain Flush sequence
    unsigned long oilingStartTime;
    unsigned long pumpActivityStartTime; // Safety Cutoff
    int oilingPulsesRemaining;
    unsigned long lastPulseTime;
    bool pulseState; // true = HIGH, false = LOW

    // Helper for PWM Pump Control
    void pumpPulse(unsigned long durationMs);

    // Temperature Compensation
    float currentTempC;
    unsigned long dynamicPulseMs;
    unsigned long dynamicPauseMs;
    unsigned long lastTempUpdate;
    void updateTemperature();

    // Safety & UX
    unsigned long ledOilingEndTimestamp;

    // Emergency Mode Settings
    bool emergencyModeForced; // Manually enabled via WebUI
    unsigned long emergencyModeStartTime;
    unsigned long lastEmergencyOilTime;
    int emergencyOilCount;
};

#endif
