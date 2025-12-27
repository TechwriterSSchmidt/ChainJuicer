#include "Oiler.h"
#include <Preferences.h>
#include <OneWire.h>
#include <DallasTemperature.h>

Preferences preferences;

// Setup OneWire and DallasTemperature
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);

Oiler::Oiler() : strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800) {
    pumpPin = PUMP_PIN;
    // Pin initialization moved to begin() to avoid issues during global constructor execution
    
    // Initialize default configuration
    ranges[0] = {10, 35, 15.0, 2};
    ranges[1] = {35, 55, 15.0, 2};
    ranges[2] = {55, 75, 15.0, 2};
    ranges[3] = {75, 95, 15.0, 2};
    ranges[4] = {95, MAX_SPEED_KMH, 15.0, 2};
    
    // Initialize Temperature Configuration (Defaults)
    tempConfig.basePulse25 = 60.0;
    tempConfig.basePause25 = 230.0;
    tempConfig.oilType = OIL_NORMAL;
    lastTemp = 25.0; // Init hysteresis memory

    currentProgress = 0.0;
    lastLat = 0.0;
    lastLon = 0.0;
    hasFix = false;
    lastSaveTime = 0;
    progressChanged = false;

    // Stats & Smoothing Init
    totalDistance = 0.0;
    pumpCycles = 0;
    for(int i=0; i<SPEED_BUFFER_SIZE; i++) speedBuffer[i] = 0.0;
    speedBufferIndex = 0;
    
    // Time Stats Init
    for(int i=0; i<NUM_RANGES; i++) {
        currentIntervalTime[i] = 0.0;
    }
    // Init History
    history.head = 0;
    history.count = 0;
    for(int i=0; i<20; i++) {
        history.oilingRange[i] = -1;
        for(int j=0; j<NUM_RANGES; j++) {
            history.timeInRanges[i][j] = 0.0;
        }
    }
    lastTimeUpdate = 0;

    // Button & Modes Init
    rainMode = false;
    rainModeStartTime = 0;
    emergencyMode = false;
    wifiActive = false;
    updateMode = false;
    bleedingMode = false;
    bleedingStartTime = 0;
    buttonPressStartTime = 0;
    buttonState = false;
    lastButtonState = false;
    lastDebounceTime = 0; // Init
    lastLedUpdate = 0;
    currentSpeed = 0.0;
    smoothedInterval = 0.0; // Init

    // Oiling State Init
    isOiling = false;
    oilingStartTime = 0;
    pumpActivityStartTime = 0;
    oilingPulsesRemaining = 0;
    lastPulseTime = 0;
    pulseState = false;

    // LED Defaults
    ledBrightnessDim = LED_BRIGHTNESS_DIM;
    ledBrightnessHigh = LED_BRIGHTNESS_HIGH;
    
    // Night Mode Defaults
    nightModeEnabled = false;
    nightStartHour = 20; // 20:00
    nightEndHour = 6;    // 06:00
    nightBrightness = 15; // Very dim
    nightBrightnessHigh = 100; // Default for night events
    currentHour = 12;    // Default noon

    // Tank Monitor Defaults
    tankMonitorEnabled = false;
    tankCapacityMl = 100.0;
    currentTankLevelMl = 100.0;
    dropsPerMl = 20;
    dropsPerPulse = 5;
    tankWarningThresholdPercent = 10;
    
    lastEmergUpdate = 0;
    lastStandstillSaveTime = 0;
    
    // Init LUT
    rebuildLUT();

    // Emergency Mode Init
    emergencyModeForced = false;
    emergencyModeStartTime = 0;
    lastEmergencyOilTime = 0;
    emergencyOilCount = 0;

    // Temperature Init
    currentTempC = 20.0; // Default start temp
    currentTempRangeIndex = 2; // Default Normal
    dynamicPulseMs = TEMP_R3_PULSE; // Default Normal
    dynamicPauseMs = TEMP_R3_PAUSE; // Default Normal
    lastTempUpdate = 0;
}

void Oiler::begin() {
    // Hardware Init
    // Ensure Pump is OFF immediately
    digitalWrite(pumpPin, PUMP_OFF);
    pinMode(pumpPin, OUTPUT);

    // Initialize PWM for Pump
    if (PUMP_USE_PWM) {
        ledcSetup(PUMP_PWM_CHANNEL, PUMP_PWM_FREQ, PUMP_PWM_RESOLUTION);
        ledcAttachPin(pumpPin, PUMP_PWM_CHANNEL);
    }

    // Initialize Temp Sensor
    sensors.begin();

    ledOilingEndTimestamp = 0;

    preferences.begin("oiler", false);
    loadConfig();

    // Hardware Init
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP); // Init onboard button
    strip.begin();
    strip.setBrightness(ledBrightnessDim);
    strip.show(); // All pixels off
}

void Oiler::checkFactoryReset() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    // Check if button is pressed during boot
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println("Button pressed at boot. Checking for Factory Reset...");
        
        // Initialize LED for feedback
        strip.begin(); // Ensure strip is initialized
        strip.setBrightness(50);
        
        unsigned long startPress = millis();
        bool resetTriggered = false;

        while (digitalRead(BUTTON_PIN) == LOW) {
            unsigned long duration = millis() - startPress;
            
            // Visual Feedback: Yellow while holding
            for(int i=0; i<NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(255, 255, 0)); // Yellow
            strip.show();

            if (duration > FACTORY_RESET_PRESS_MS) {
                resetTriggered = true;
                // Visual Feedback: Red blinking fast
                for(int k=0; k<10; k++) {
                    for(int i=0; i<NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(255, 0, 0)); // Red
                    strip.show();
                    delay(100);
                    for(int i=0; i<NUM_LEDS; i++) strip.setPixelColor(i, 0); // Off
                    strip.show();
                    delay(100);
                }
                break; // Exit loop to perform reset
            }
            delay(10);
        }

        if (resetTriggered) {
            Serial.println("PERFORMING FACTORY RESET...");
            preferences.begin("oiler", false);
            preferences.clear(); // Nuke everything
            preferences.end();
            Serial.println("Done. Restarting...");
            ESP.restart();
        }
    }
}

void Oiler::loop() {
    handleButton();
    processPump(); // Unified pump logic
    
    // Temperature Update (Periodic)
    if (millis() - lastTempUpdate > TEMP_UPDATE_INTERVAL_MS) {
        updateTemperature();
        lastTempUpdate = millis();
    }

    // Rain Mode Auto-Off
    if (rainMode && (millis() - rainModeStartTime > RAIN_MODE_AUTO_OFF_MS)) {
        rainMode = false;
#ifdef GPS_DEBUG
        Serial.println("Rain Mode Auto-Off");
#endif
        saveConfig();
    }

    updateLED();
}

void Oiler::handleButton() {
    // Read button (Active LOW due to INPUT_PULLUP)
    // Check both external button AND onboard boot button
    bool currentReading = !digitalRead(BUTTON_PIN) || !digitalRead(BOOT_BUTTON_PIN);

    // Debounce Logic
    if (currentReading != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > 50) { // 50ms Debounce Delay
        // If the state has been stable for > 50ms, we accept it
        if (currentReading != buttonState) {
            buttonState = currentReading;

            // State changed (Stable)
            if (buttonState) {
                // Pressed
                buttonPressStartTime = millis();
            } else {
                // Released
                unsigned long pressDuration = millis() - buttonPressStartTime;
                
                // Short Press: Rain Mode Toggle
                if (pressDuration < RAIN_TOGGLE_MS && pressDuration > 50) {
                    // Only toggle if NOT in Emergency Mode (Forced or Auto)
                    if (!emergencyMode && !emergencyModeForced) {
                        rainMode = !rainMode;
                        if (rainMode) rainModeStartTime = millis();
#ifdef GPS_DEBUG
                        Serial.print("Rain Mode: ");
                        Serial.println(rainMode ? "ON" : "OFF");
#endif
                        saveConfig(); // Save setting
                    }
                }
            }
        }
    }

    // Check Long Press while holding (using stable buttonState)
    if (buttonState && !bleedingMode) {
        unsigned long duration = millis() - buttonPressStartTime;
        
        // Long Press: Bleeding Mode
        if (duration > BLEEDING_PRESS_MS) {
            // SAFETY: Only allow in standstill
            if (currentSpeed < MIN_SPEED_KMH) {
                bleedingMode = true;
                bleedingStartTime = millis();
                pumpActivityStartTime = millis(); // Safety Cutoff Start
#ifdef GPS_DEBUG
                Serial.println("Bleeding Mode STARTED");
#endif
                
                // Init Pump State for immediate start
                pulseState = false; 
                lastPulseTime = millis() - 1000; // Force start

                saveConfig(); // Save immediately
            } else {
#ifdef GPS_DEBUG
                Serial.println("Bleeding blocked: Speed > MIN_SPEED_KMH");
#endif
            }

            // Reset button start time to avoid re-triggering immediately
            buttonPressStartTime = millis(); 
        }
    }

    lastButtonState = currentReading;
}

void Oiler::setCurrentHour(int hour) {
    currentHour = hour;
}

void Oiler::updateLED() {
    // LED Update
    uint32_t color = 0;
    unsigned long now = millis();

    // Determine Brightness
    uint8_t currentDimBrightness = ledBrightnessDim;
    uint8_t currentHighBrightness = ledBrightnessHigh;
    
    if (nightModeEnabled) {
        bool isNight = false;
        if (nightStartHour > nightEndHour) {
            // Crossing midnight (e.g. 20 to 6)
            if (currentHour >= nightStartHour || currentHour < nightEndHour) isNight = true;
        } else {
            // Same day (e.g. 0 to 4)
            if (currentHour >= nightStartHour && currentHour < nightEndHour) isNight = true;
        }
        
        if (isNight) {
            currentDimBrightness = nightBrightness;
            currentHighBrightness = nightBrightnessHigh;
        }
    }

    // Helper for sine wave pulse (0.0 to 1.0)
    auto getPulse = [&](int periodMs) -> float {
        float angle = (now % periodMs) * 2.0 * PI / periodMs;
        return (sin(angle) + 1.0) / 2.0;
    };

    // 0. Update Mode (Critical) -> CYAN Fast Blink
    if (updateMode) {
        strip.setBrightness(currentHighBrightness);
        if ((now / LED_BLINK_FAST) % 2 == 0) {
            color = strip.Color(0, 255, 255); // Cyan
        } else {
            color = 0; // Off
        }
    }
    // 1. Bleeding Mode (Highest Priority) -> RED Blinking fast
    else if (bleedingMode) {
        strip.setBrightness(currentHighBrightness);
        if ((now / LED_BLINK_FAST) % 2 == 0) {
            color = strip.Color(255, 0, 0);
        } else {
            color = 0; // Off
        }
    } 
    // 2. WiFi Active (High Priority Indication) -> WHITE Pulsing
    else if (wifiActive && (now - wifiActivationTime < LED_WIFI_SHOW_DURATION)) {
        float pulse = getPulse(LED_PERIOD_WIFI) * 0.8 + 0.2;
        uint8_t bri = (uint8_t)(pulse * currentHighBrightness);
        if (bri < 5) bri = 5;
        strip.setBrightness(bri);
        color = strip.Color(255, 255, 255);
    }
    // 3. Oiling Event -> YELLOW Breathing
    else if (isOiling || millis() < ledOilingEndTimestamp) {
        float breath = getPulse(LED_PERIOD_OILING); 
        uint8_t bri = (uint8_t)(breath * currentHighBrightness);
        if (bri < 5) bri = 5;
        strip.setBrightness(bri);
        color = strip.Color(255, 200, 0);
    } else if (wifiActive) {
        // WiFi Active: White pulsing
        // Use sine wave for smooth pulsing
        float pulse = (sin(now / 500.0) + 1.0) / 2.0; // 0.0 to 1.0
        uint8_t brightness = (uint8_t)(pulse * currentHighBrightness);
        if (brightness < 10) brightness = 10; // Minimum brightness
        
        strip.setBrightness(brightness);
        color = strip.Color(255, 255, 255); // White
    } 
    // 4. Tank Warning -> ORANGE Blinking (2x fast)
    else if (tankMonitorEnabled && (currentTankLevelMl / tankCapacityMl * 100.0) < tankWarningThresholdPercent) {
        strip.setBrightness(currentHighBrightness);
        int phase = now % LED_BLINK_TANK; // 2s cycle
        // Blink 1: 0-200, Blink 2: 400-600
        if ((phase >= 0 && phase < 200) || (phase >= 400 && phase < 600)) {
            color = strip.Color(255, 69, 0); // OrangeRed
        } else {
            color = 0; // Off
        }
    }
    else if (!hasFix) {
        // No GPS
        unsigned long timeSinceLoss = (lastEmergUpdate > 0) ? (now - lastEmergUpdate) : 0;
        
        if (emergencyModeForced) {
             // Forced Emergency: Cyan
             strip.setBrightness(currentDimBrightness);
             color = strip.Color(0, 255, 255); 
        } else if (emergencyMode) {
             // Auto Emergency Active: Cyan Dim
             strip.setBrightness(currentDimBrightness);
             color = strip.Color(0, 255, 255);
        } else {
            color = 0; // Off
        }
    }
    // 5. Emergency Mode (Forced or Auto) -> ORANGE Double Pulse over GREEN
    else if (emergencyModeForced || emergencyMode || (!hasFix && (lastEmergUpdate > 0 && (now - lastEmergUpdate) > EMERGENCY_TIMEOUT_MS))) {
         int phase = now % LED_PERIOD_EMERGENCY; // 1.5s Cycle
         // Pulse 1: 0-100, Pulse 2: 200-300
         if ((phase >= 0 && phase < 100) || (phase >= 200 && phase < 300)) {
             strip.setBrightness(currentHighBrightness);
             color = strip.Color(255, 140, 0); // Orange
         } else {
             strip.setBrightness(currentDimBrightness);
             color = strip.Color(0, 255, 0); // Green
         }
    }
    // 6. Rain Mode -> BLUE Static
    else if (rainMode) {
        strip.setBrightness(currentDimBrightness);
        color = strip.Color(0, 0, 255);
    }
    // 7. No GPS (Searching) -> MAGENTA Pulsing
    else if (!hasFix) {
        float pulse = getPulse(LED_PERIOD_GPS);
        uint8_t bri = (uint8_t)(pulse * currentDimBrightness);
        if (bri < 5) bri = 5;
        strip.setBrightness(bri);
        color = strip.Color(255, 0, 255);
    }
    // 8. Idle / Ready -> GREEN Static
    else {
        strip.setBrightness(currentDimBrightness);
        color = strip.Color(0, 255, 0);
    }

    for(int i=0; i<NUM_LEDS; i++) {
        strip.setPixelColor(i, color);
    }
    strip.show();
}

void Oiler::loadConfig() {
    // Load configuration from Flash (NVS)
    // If nothing is saved yet, default values remain
    for(int i=0; i<NUM_RANGES; i++) {
        String keyBase = "r" + String(i);
        ranges[i].intervalKm = preferences.getFloat((keyBase + "_km").c_str(), ranges[i].intervalKm);
        ranges[i].pulses = preferences.getInt((keyBase + "_p").c_str(), ranges[i].pulses);
    }

    // Load Temperature Compensation Settings (New Simplified Model)
    tempConfig.basePulse25 = preferences.getFloat("tc_pulse", 60.0);
    tempConfig.basePause25 = preferences.getFloat("tc_pause", 230.0);
    tempConfig.oilType = (OilType)preferences.getInt("tc_oil", (int)OIL_NORMAL);

    currentProgress = preferences.getFloat("progress", 0.0);
    ledBrightnessDim = preferences.getUChar("led_dim", LED_BRIGHTNESS_DIM);
    ledBrightnessHigh = preferences.getUChar("led_high", LED_BRIGHTNESS_HIGH);
    
    nightModeEnabled = preferences.getBool("night_en", false);
    nightStartHour = preferences.getInt("night_start", 20);
    nightEndHour = preferences.getInt("night_end", 6);
    nightBrightness = preferences.getUChar("night_bri", 5);
    nightBrightnessHigh = preferences.getUChar("night_bri_h", 100);
    
    // Restore Rain Mode
    rainMode = false; // Always start with Rain Mode OFF
    
    emergencyMode = preferences.getBool("emerg_mode", false);

    // Load Stats
    totalDistance = preferences.getDouble("totalDist", 0.0);
    pumpCycles = preferences.getUInt("pumpCount", 0);
    
    // Load Time Stats History
    size_t len = preferences.getBytesLength("statsHist");
    if (len == sizeof(StatsHistory)) {
        preferences.getBytes("statsHist", &history, sizeof(StatsHistory));
    }
    // Load current interval time
    for(int i=0; i<NUM_RANGES; i++) {
        currentIntervalTime[i] = preferences.getDouble(("cit" + String(i)).c_str(), 0.0);
    }

    // Load Tank Monitor
    tankMonitorEnabled = preferences.getBool("tank_en", false);
    tankCapacityMl = preferences.getFloat("tank_cap", 100.0);
    currentTankLevelMl = preferences.getFloat("tank_lvl", 100.0);
    dropsPerMl = preferences.getInt("drop_ml", 20);
    dropsPerPulse = preferences.getInt("drop_pls", 5);
    tankWarningThresholdPercent = preferences.getInt("tank_warn", 10);

    // Load Emergency Mode forced setting
    // SAFETY: Always start with Forced Emergency Mode OFF to prevent accidental oiling in garage
    emergencyModeForced = false; 

    // If forced, activate immediately
    if (emergencyModeForced) {
        emergencyMode = true;
        emergencyModeStartTime = millis();
    }

    validateConfig();
    rebuildLUT(); // Re-calculate LUT after loading config
}

void Oiler::validateConfig() {
    // Ensure no 0 or negative values exist
    for(int i=0; i<NUM_RANGES; i++) {
        if(ranges[i].intervalKm < 1.0) ranges[i].intervalKm = 1.0; // Minimum 1km
        if(ranges[i].pulses < 1) ranges[i].pulses = 1;             // Minimum 1 pulse
    }
    
    // Brightness limits (2-202)
    if(ledBrightnessDim < 2) ledBrightnessDim = 2;
    if(ledBrightnessDim > 202) ledBrightnessDim = 202;
    
    if(ledBrightnessHigh < 2) ledBrightnessHigh = 2;
    if(ledBrightnessHigh > 202) ledBrightnessHigh = 202;
    
    if(nightBrightness < 2) nightBrightness = 2;
    if(nightBrightness > 202) nightBrightness = 202;
    
    if(nightBrightnessHigh < 2) nightBrightnessHigh = 2;
    if(nightBrightnessHigh > 202) nightBrightnessHigh = 202;
}

void Oiler::saveConfig() {
    for(int i=0; i<NUM_RANGES; i++) {
        String keyBase = "r" + String(i);
        preferences.putFloat((keyBase + "_km").c_str(), ranges[i].intervalKm);
        preferences.putInt((keyBase + "_p").c_str(), ranges[i].pulses);
    }

    // Save Temperature Compensation Settings
    for(int i=0; i<5; i++) {
        String keyBase = "t" + String(i);
        preferences.putInt((keyBase + "_p").c_str(), tempRanges[i].pulseMs);
        preferences.putInt((keyBase + "_w").c_str(), tempRanges[i].pauseMs);
        if (i < 4) {
            preferences.putFloat((keyBase + "_m").c_str(), tempRanges[i].maxTemp);
    preferences.putFloat("tc_pulse", tempConfig.basePulse25);
    preferences.putFloat("tc_pause", tempConfig.basePause25);
    preferences.putInt("tc_oil", (int)tempConfig.oilType);references.putInt("night_start", nightStartHour);
    preferences.putInt("night_end", nightEndHour);
    preferences.putUChar("night_bri", nightBrightness);
    preferences.putUChar("night_bri_h", nightBrightnessHigh);
    
    // Save Rain Mode
    preferences.putBool("rain_mode", rainMode);
    preferences.putBool("emerg_mode", emergencyMode);

    // Save Tank Monitor
    preferences.putBool("tank_en", tankMonitorEnabled);
    preferences.putFloat("tank_cap", tankCapacityMl);
    preferences.putFloat("tank_lvl", currentTankLevelMl);
    preferences.putInt("drop_ml", dropsPerMl);
    preferences.putInt("drop_pls", dropsPerPulse);
    preferences.putInt("tank_warn", tankWarningThresholdPercent);

    // Save Stats
    preferences.putDouble("totalDist", totalDistance);
    preferences.putUInt("pumpCount", pumpCycles);
    
    // Save Time Stats History
    preferences.putBytes("statsHist", &history, sizeof(StatsHistory));
    // Save current interval time
    for(int i=0; i<NUM_RANGES; i++) {
        preferences.putDouble(("cit" + String(i)).c_str(), currentIntervalTime[i]);
    }
    
    rebuildLUT(); // Ensure LUT is up to date when saving (in case ranges changed)
}

void Oiler::saveProgress() {
    if (progressChanged) {
        preferences.putFloat("progress", currentProgress);
        // Save Stats
        preferences.putDouble("totalDist", totalDistance);
        preferences.putUInt("pumpCount", pumpCycles);
        
        // Save Time Stats History
        preferences.putBytes("statsHist", &history, sizeof(StatsHistory));
        for(int i=0; i<NUM_RANGES; i++) {
            preferences.putDouble(("cit" + String(i)).c_str(), currentIntervalTime[i]);
        }
        
        // Save Tank Level
        preferences.putFloat("tank_lvl", currentTankLevelMl);

        progressChanged = false;
#ifdef GPS_DEBUG
        Serial.println("Progress & Stats saved.");
#endif
    }
}

void Oiler::resetStats() {
    totalDistance = 0.0;
    pumpCycles = 0;
    resetTimeStats(); // Also reset time stats
    saveConfig();
}

void Oiler::resetTimeStats() {
    for(int i=0; i<NUM_RANGES; i++) {
        currentIntervalTime[i] = 0.0;
    }
    history.head = 0;
    history.count = 0;
    for(int i=0; i<20; i++) {
        history.oilingRange[i] = -1;
        for(int j=0; j<NUM_RANGES; j++) {
            history.timeInRanges[i][j] = 0.0;
        }
    }
    saveConfig();
}

int Oiler::calculateLocalHour(int utcHour, int day, int month, int year) {
    // Simple CET/CEST Rule:
    // CEST (UTC+2) starts last Sunday in March, ends last Sunday in October.
    
    bool isSummer = false;
    if (month > 3 && month < 10) isSummer = true; // April to September
    else if (month == 3) {
        int lastSunday = 31 - ((5 * year / 4 + 4) % 7);
        if (day > lastSunday || (day == lastSunday && utcHour >= 1)) isSummer = true;
    }
    else if (month == 10) {
        int lastSunday = 31 - ((5 * year / 4 + 1) % 7);
        if (day < lastSunday || (day == lastSunday && utcHour < 1)) isSummer = true;
    }
    
    int offset = isSummer ? 2 : 1;
    int localH = (utcHour + offset) % 24;
    return localH;
}

void Oiler::update(float rawSpeedKmh, double lat, double lon, bool gpsValid) {
    unsigned long now = millis();

    // Force GPS invalid if Emergency Mode is manually forced
    if (emergencyModeForced) {
        gpsValid = false;
    }

    // GPS Smoothing (Moving Average)
    speedBuffer[speedBufferIndex] = rawSpeedKmh;
    speedBufferIndex = (speedBufferIndex + 1) % SPEED_BUFFER_SIZE;

    float smoothedSpeed = 0.0;
    for(int i=0; i<SPEED_BUFFER_SIZE; i++) {
        smoothedSpeed += speedBuffer[i];
    }
    smoothedSpeed /= SPEED_BUFFER_SIZE;

    // Use smoothedSpeed for logic
    float speedKmh = smoothedSpeed;
    currentSpeed = speedKmh; // Update member variable for handleButton logic

    // Update Time Stats
    if (lastTimeUpdate == 0) lastTimeUpdate = now;
    unsigned long dt = now - lastTimeUpdate;
    lastTimeUpdate = now;

    // Only count if moving fast enough to be in a range (or at least > MIN_SPEED)
    // And avoid huge jumps (e.g. after sleep)
    if (speedKmh >= MIN_SPEED_KMH && dt < 2000) {
        double dtSeconds = (double)dt / 1000.0;

        // Find matching range
        int activeRangeIndex = -1;
        for(int i=0; i<NUM_RANGES; i++) {
            if (speedKmh >= ranges[i].minSpeed && speedKmh < ranges[i].maxSpeed) {
                activeRangeIndex = i;
                break;
            }
        }

        if (activeRangeIndex != -1) {
            currentIntervalTime[activeRangeIndex] += dtSeconds;
            progressChanged = true; // Mark for saving
        }
    }

    // Regular saving
    if (now - lastSaveTime > SAVE_INTERVAL_MS) {
        saveProgress();
        lastSaveTime = now;
    }
    // Save immediately at standstill (if we were moving before), but limit frequency
    if (speedKmh < MIN_SPEED_KMH && progressChanged && (now - lastStandstillSaveTime > STANDSTILL_SAVE_MS)) {
        saveProgress();
        lastStandstillSaveTime = now;
    }

    if (!gpsValid) {
        hasFix = false;

        if (lastEmergUpdate == 0) {
            lastEmergUpdate = now;
            emergencyOilCount = 0;
        }

        unsigned long timeSinceLoss = now - lastEmergUpdate;
        bool autoEmergencyActive = (timeSinceLoss > EMERGENCY_TIMEOUT_MS);

        // Static variable for simulation step (shared across calls)
        static unsigned long lastSimStep = 0;

        if (emergencyModeForced || autoEmergencyActive) {
            // We are in Emergency Mode (either Forced or Auto Timeout)
            
            if (!emergencyMode) {
                // Just entered Emergency Mode
                emergencyMode = true;
                lastSimStep = now; // Initialize timer
                
                // Auto-Disable Rain Mode
                if (rainMode) {
                    setRainMode(false);
                    saveConfig();
                }
#ifdef GPS_DEBUG
                Serial.println("Emergency Mode ACTIVATED (50km/h Sim)");
#endif
            } else {
                // Already in Emergency Mode
                // Ensure Rain Mode stays OFF
                if (rainMode) setRainMode(false);
            }
            
            // Simulation Logic (50 km/h)
            if (lastSimStep == 0) lastSimStep = now;
            unsigned long dt = now - lastSimStep;
            lastSimStep = now;
            if (dt > 1000) dt = 1000;

            float simSpeed = 50.0;
            double distKm = (double)simSpeed * ((double)dt / 3600000.0);
            
            totalDistance += distKm; // Update Odometer
            progressChanged = true;

            // Update Usage Stats for 50km/h
            double dtSeconds = (double)dt / 1000.0;
            for(int i=0; i<NUM_RANGES; i++) {
                if (simSpeed >= ranges[i].minSpeed && simSpeed < ranges[i].maxSpeed) {
                    currentIntervalTime[i] += dtSeconds;
                    break;
                }
            }

            processDistance(distKm, simSpeed);

        } else {
            // Waiting for timeout...
            emergencyMode = false;
            lastSimStep = 0; // Reset sim timer
        }
        return;
    }

    // First Fix?
    if (!hasFix) {
        lastLat = lat;
        lastLon = lon;
        hasFix = true;
        lastEmergUpdate = 0;
        emergencyOilCount = 0;
        emergencyMode = false;
        return;
    }

    // Reset Emergency Timer if we have valid GPS
    lastEmergUpdate = 0;
    emergencyMode = false; // Disable Emergency Mode automatically

    // Calculate distance (Haversine or TinyGPS function)
    double distKm = TinyGPSPlus::distanceBetween(lastLat, lastLon, lat, lon) / 1000.0;

    // Only if moving and GPS not jumping (small filter)
    // Plausibility check: < MAX_SPEED_KMH + Buffer
    if (distKm > 0.005 && speedKmh > MIN_ODOMETER_SPEED_KMH && speedKmh < (MAX_SPEED_KMH + 50.0)) {
        lastLat = lat;
        lastLon = lon;

        // Process Distance (Odometer + Oiling Logic)
        if (speedKmh >= MIN_SPEED_KMH) {
            processDistance(distKm, speedKmh);
        } else {
            // Just add to odometer if moving slowly but valid? 
            // Usually we only count odometer if speed > MIN_ODOMETER_SPEED_KMH which is checked above.
            // But processDistance handles Oiling logic which requires MIN_SPEED_KMH usually.
            // Let's add to odometer anyway via processDistance, but speed might be low.
            // processDistance handles ranges. If speed < range[0].min, it might not trigger oiling but adds to totalDistance.
            // Let's call it.
             processDistance(distKm, speedKmh);
        }
    }
}

void Oiler::processDistance(double distKm, float speedKmh) {
    // 1. Add to Total Odometer
    totalDistance += distKm;
    progressChanged = true; // So Odometer gets saved

    // Find matching range
    int activeRangeIndex = -1;
    for(int i=0; i<NUM_RANGES; i++) {
        if (speedKmh >= ranges[i].minSpeed && speedKmh < ranges[i].maxSpeed) {
            activeRangeIndex = i;
            break;
        }
    }

    if (activeRangeIndex != -1) {
        // Virtual distance calculation:
        // We add % progress to next oiling, not km.

        // 1. Get Target Interval from LUT (Linear Interpolation)
        int lutIndex = (int)(speedKmh / LUT_STEP);
        if (lutIndex < 0) lutIndex = 0;
        if (lutIndex >= LUT_SIZE) lutIndex = LUT_SIZE - 1;

        float targetInterval = intervalLUT[lutIndex];

        // 2. Low-Pass Filter (Additional Smoothing)
        if (smoothedInterval == 0.0) smoothedInterval = targetInterval; // Init
        smoothedInterval = (smoothedInterval * 0.95) + (targetInterval * 0.05);

        float interval = smoothedInterval;

        if (interval > 0) {
            float progressDelta = distKm / interval;

            // Rain Mode: Double wear -> Double progression
            if (rainMode) {
                progressDelta *= 2.0;
            }

            currentProgress += progressDelta;
            progressChanged = true;

            // Debug Output
            // Serial.printf("Speed: %.1f, Dist: %.4f, Prog: %.4f\n", speedKmh, distKm, currentProgress);

            // Early Oiling: 95% instead of 100% Virtual Distance
            // Prevents edge cases during mode switches
            // 5% safety margin eliminates double oilings
            if (currentProgress >= 0.95) {
                // Update History BEFORE resetting currentIntervalTime
                int head = history.head;
                history.oilingRange[head] = activeRangeIndex;
                for(int i=0; i<NUM_RANGES; i++) {
                    history.timeInRanges[head][i] = currentIntervalTime[i];
                    currentIntervalTime[i] = 0.0; // Reset for next interval
                }
                history.head = (head + 1) % 20;
                if (history.count < 20) history.count++;

                triggerOil(ranges[activeRangeIndex].pulses);
                currentProgress = 0.0; // Reset
                saveProgress(); // Save progress
            }
        }
    }
}

void Oiler::triggerOil(int pulses) {
#ifdef GPS_DEBUG
    Serial.println("OILING START (Non-Blocking)");
#endif
    
    pumpCycles++; // Stats
    progressChanged = true; // Mark for saving

    // Tank Monitor Logic
    if (tankMonitorEnabled) {
        float mlConsumed = (float)(pulses * dropsPerPulse) / (float)dropsPerMl;
        currentTankLevelMl -= mlConsumed;
        if (currentTankLevelMl < 0) currentTankLevelMl = 0;
        
#ifdef GPS_DEBUG
        Serial.printf("Oil consumed: %.2f ml, Remaining: %.2f ml\n", mlConsumed, currentTankLevelMl);
#endif
    }

    // Initialize Non-Blocking Oiling
    isOiling = true;
    pumpActivityStartTime = millis(); // Safety Cutoff Start
    oilingPulsesRemaining = pulses;
    pulseState = false; // Will start with HIGH in handleOiling
    lastPulseTime = millis() - 1000; // Force immediate start
    
    // LED Indication
    ledOilingEndTimestamp = millis() + 3000;
}

void Oiler::processPump() {
    unsigned long now = millis();

    // SAFETY CUTOFF: Prevent pump from running too long (e.g. software bug)
    if ((isOiling || bleedingMode) && (now - pumpActivityStartTime > PUMP_SAFETY_CUTOFF_MS)) {
        Serial.println("[CRITICAL] Safety Cutoff triggered! Pump ran too long.");
        digitalWrite(pumpPin, PUMP_OFF);
        isOiling = false;
        bleedingMode = false;
        pulseState = false;
        return;
    }
    // Check if we should stop bleeding
    if (bleedingMode) {
        if (now - bleedingStartTime > BLEEDING_DURATION_MS) {
            bleedingMode = false;
            digitalWrite(pumpPin, PUMP_OFF);
            pulseState = false; // Reset state
#ifdef GPS_DEBUG
            Serial.println("Bleeding Finished");
#endif
            return; // Done
        }
        // If bleeding, we treat it as "always have pulses remaining"
    } else if (!isOiling) {
        // Not bleeding and not oiling -> Idle
        return;
    }

      
    // Logic for Pulse Generation
    // REVISED for Blocking PWM Pulse
    // We only track the PAUSE time. When pause is over, we execute the blocking pulse.
    // UPDATE: Use dynamicPauseMs instead of static PAUSE_DURATION_MS
    if (now - lastPulseTime >= dynamicPauseMs) {
        // Execute Blocking Pulse (Soft-Start/Stop)
        // UPDATE: Use dynamicPulseMs instead of static PULSE_DURATION_MS
        pumpPulse(dynamicPulseMs);
        
        // Reset Timer
        lastPulseTime = millis();
        
        // Handle Counters
        if (!bleedingMode) {
            oilingPulsesRemaining--;
            if (oilingPulsesRemaining == 0) {
                isOiling = false;
#ifdef GPS_DEBUG
                Serial.println("OILING DONE");
#endif
            }
        } else {
            // Bleeding Mode: Count every pulse as stats & consumption
            pumpCycles++;
            progressChanged = true;
            
            if (tankMonitorEnabled) {
                float mlConsumed = (float)(1 * dropsPerPulse) / (float)dropsPerMl;
                currentTankLevelMl -= mlConsumed;
                if (currentTankLevelMl < 0) currentTankLevelMl = 0;
            }
        }
    }
}

// --- HELPER FUNCTION: SMART PUMP DRIVER (PWM) ---
void Oiler::pumpPulse(unsigned long durationMs) {
    if (!PUMP_USE_PWM) {
        // Fallback: Hard Switching
        digitalWrite(pumpPin, PUMP_ON);
        delay(durationMs);
        digitalWrite(pumpPin, PUMP_OFF);
        return;
    }

    // 1. RAMP UP (Soft Start)
    // Linearly increase duty cycle from 0 to 255
    unsigned long stepDelay = (PUMP_RAMP_UP_MS * 1000) / 255; // Microseconds per step
    for (int duty = 0; duty <= 255; duty += 15) { // Step size 15 for speed
        ledcWrite(PUMP_PWM_CHANNEL, duty);
        delayMicroseconds(stepDelay * 15);
    }
    ledcWrite(PUMP_PWM_CHANNEL, 255); // Ensure full power

    // 2. HOLD (Main Pulse)
    // We subtract the ramp time from the duration to keep timing roughly accurate,
    // but ensure at least 5ms of full power hold.
    unsigned long holdTime = 0;
    if (durationMs > PUMP_RAMP_UP_MS) {
        holdTime = durationMs - PUMP_RAMP_UP_MS;
    }
    delay(holdTime);

    // 3. RAMP DOWN (Soft Stop)
    // Linearly decrease duty cycle from 255 to 0
    stepDelay = (PUMP_RAMP_DOWN_MS * 1000) / 255;
    for (int duty = 255; duty >= 0; duty -= 15) {
        ledcWrite(PUMP_PWM_CHANNEL, duty);
        delayMicroseconds(stepDelay * 15);
    }
    ledcWrite(PUMP_PWM_CHANNEL, 0); // Ensure off
    digitalWrite(pumpPin, PUMP_OFF);    // Safety: Disable PWM pin output
}

void Oiler::setEmergencyModeForced(bool forced) {
    emergencyModeForced = forced;
    if (emergencyModeForced) {
        // Automatically disable Rain Mode if Emergency Mode is forced
        setRainMode(false);
        
        // Also activate standard emergency mode flag immediately
        emergencyMode = true;
        emergencyModeStartTime = millis();
    }
}

void Oiler::setRainMode(bool mode) {
    // If Emergency Mode is forced, Rain Mode cannot be activated
    if (emergencyModeForced && mode) {
        mode = false; 
    }

    if (mode && !rainMode) {
        rainModeStartTime = millis();
    }
    rainMode = mode;
    // If Rain Mode is activated, disable forced Emergency Mode
    if (rainMode) {
        emergencyModeForced = false;
    }
}

SpeedRange* Oiler::getRangeConfig(int index) {
    if(index >= 0 && index < NUM_RANGES) return &ranges[index];
    return nullptr;
}

bool Oiler::isTempSensorConnected() {
    return sensors.getDeviceCount() > 0;
}

bool Oiler::isButtonPressed() {
    // Return the debounced state of the button
    return !digitalRead(BUTTON_PIN) || !digitalRead(BOOT_BUTTON_PIN); // Active LOW -> returns true if pressed
}

void Oiler::rebuildLUT() {
    // 1. Define Anchors (Center points of ranges)
    struct Anchor { float speed; float interval; };
    Anchor anchors[NUM_RANGES];

    for(int i=0; i<NUM_RANGES; i++) {
        float center;
        if (i == NUM_RANGES - 1) {
            // Last range (e.g. 95-999). Use start + 10km/h as anchor to avoid stretching
            center = ranges[i].minSpeed + 10.0;
        } else {
            center = (ranges[i].minSpeed + ranges[i].maxSpeed) / 2.0;
        }
        anchors[i].speed = center;
        anchors[i].interval = ranges[i].intervalKm;
    }

    // 2. Fill LUT with linear interpolation
    for (int i=0; i<LUT_SIZE; i++) {
        float speed = i * LUT_STEP;
        
        if (speed <= anchors[0].speed) {
            intervalLUT[i] = anchors[0].interval;
        } else if (speed >= anchors[NUM_RANGES-1].speed) {
            intervalLUT[i] = anchors[NUM_RANGES-1].interval;
        } else {
            // Interpolate between anchors
            for (int j=0; j<NUM_RANGES-1; j++) {
                if (speed >= anchors[j].speed && speed < anchors[j+1].speed) {
                    float slope = (anchors[j+1].interval - anchors[j].interval) / (anchors[j+1].speed - anchors[j].speed);
                    intervalLUT[i] = anchors[j].interval + slope * (speed - anchors[j].speed);
                    break;
                }
            }
        }
    }
}

void Oiler::setTankFill(float levelMl) {
    currentTankLevelMl = levelMl;
    if (currentTankLevelMl > tankCapacityMl) currentTankLevelMl = tankCapacityMl;
    saveConfig();
}

void Oiler::resetTankToFull() {
    currentTankLevelMl = tankCapacityMl;
    saveConfig();
}

void Oiler::setWifiActive(bool active) {
    if (active && !wifiActive) {
        wifiActivationTime = millis();
    }
    wifiActive = active;
}

void Oiler::setUpdateMode(bool mode) {
    updateMode = mode;
}

// --- NEW: Temperature Compensation Logic ---
void Oiler::updateTemperature() {
    sensors.requestTemperatures(); 
    float tempC = sensors.getTempCByIndex(0);

    // Check for error (-127 is error)
    if (tempC == DEVICE_DISCONNECTED_C) {
        // Sensor Error: Fallback to 25°C defaults
        currentTempC = 25.0;
        dynamicPulseMs = (unsigned long)tempConfig.basePulse25;
        dynamicPauseMs = (unsigned long)tempConfig.basePause25;
#ifdef GPS_DEBUG
        Serial.println("Temp Sensor Error! Using defaults.");
#endif
        return;
    }

    // Hysteresis Logic (3.0°C)
    // Only update calculation if temp changes significantly to avoid jitter
    if (abs(tempC - lastTemp) < 3.0) {
        // Keep old values, just update display temp
        currentTempC = tempC; 
        return; 
    }
    
    lastTemp = tempC;
    currentTempC = tempC;

    // 1. Determine Viscosity Factor based on Oil Type
    // Factor k: How much to increase pulse per 10°C drop
    float k_pulse = 1.25; // Default Normal
    
    switch (tempConfig.oilType) {
        case OIL_THIN:   k_pulse = 1.10; break; // +10% per 10°C colder
        case OIL_NORMAL: k_pulse = 1.25; break; // +25% per 10°C colder
        case OIL_THICK:  k_pulse = 1.40; break; // +40% per 10°C colder
    }

    // 2. Calculate Temperature Difference to Reference (25°C)
    // Positive Diff = Colder than 25°C
    float tempDiff = 25.0 - currentTempC;
    
    // 3. Calculate Exponential Factor
    // factor = k ^ (diff / 10)
    float factor = pow(k_pulse, tempDiff / 10.0);

    // 4. Apply Factor
    unsigned long newPulse = (unsigned long)(tempConfig.basePulse25 * factor);
    unsigned long newPause = (unsigned long)(tempConfig.basePause25 * factor);

    // Safety Limits
    if (newPulse > 150) newPulse = 150;
    if (newPulse < 20) newPulse = 20;
    
    // Pause should not be too short (min 100ms)
    if (newPause < 100) newPause = 100;

    dynamicPulseMs = newPulse;
    dynamicPauseMs = newPause;

    // PWM Safety Check
    if (PUMP_USE_PWM && dynamicPulseMs <= PUMP_RAMP_UP_MS) {
        dynamicPulseMs = PUMP_RAMP_UP_MS + 5; // Ensure at least 5ms hold time
    }

#ifdef GPS_DEBUG
    Serial.printf("Temp: %.1f C (Factor %.2f) -> Pulse: %lu ms, Pause: %lu ms\n", currentTempC, factor, dynamicPulseMs, dynamicPauseMs);
#endif
}
