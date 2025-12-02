#include "Oiler.h"
#include <Preferences.h>

Preferences preferences;

Oiler::Oiler() : strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800) {
    pumpPin = PUMP_PIN;
    // Pin initialization moved to begin() to avoid issues during global constructor execution
    
    // Initialize default configuration
    ranges[0] = {10, 35, 15.0, 2};
    ranges[1] = {35, 55, 15.0, 2};
    ranges[2] = {55, 75, 15.0, 2};
    ranges[3] = {75, 95, 15.0, 2};
    ranges[4] = {95, MAX_SPEED_KMH, 15.0, 2};
    
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
    bleedingMode = false;
    bleedingStartTime = 0;
    buttonPressStartTime = 0;
    buttonState = false;
    lastButtonState = false;
    lastLedUpdate = 0;
    currentSpeed = 0.0;
    smoothedInterval = 0.0; // Init

    // Oiling State Init
    isOiling = false;
    oilingStartTime = 0;
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
}

void Oiler::begin() {
    // Hardware Init
    // Ensure Pump is OFF immediately
    digitalWrite(pumpPin, LOW);
    pinMode(pumpPin, OUTPUT);

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

void Oiler::loop() {
    handleButton();
    processPump(); // Unified pump logic
    
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

    // Debounce (simple)
    if (currentReading != lastButtonState) {
        // State changed
        if (currentReading) {
            // Pressed
            buttonPressStartTime = millis();
        } else {
            // Released
            unsigned long pressDuration = millis() - buttonPressStartTime;
            
            // Short Press: Rain Mode Toggle
            if (pressDuration < RAIN_TOGGLE_MS && pressDuration > 50) {
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

    // Check Long Press while holding
    if (currentReading && !bleedingMode) {
        unsigned long duration = millis() - buttonPressStartTime;
        
        // Long Press: Bleeding Mode
        if (duration > BLEEDING_PRESS_MS) {
            // SAFETY: Only allow in standstill
            if (currentSpeed < MIN_SPEED_KMH) {
                bleedingMode = true;
                bleedingStartTime = millis();
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

    if (bleedingMode) {
        // Bleeding: Red blinking
        strip.setBrightness(currentHighBrightness);
        if ((now / 250) % 2 == 0) {
            color = strip.Color(255, 0, 0);
        } else {
            color = 0; // Off
        }
    } else if (isOiling || millis() < ledOilingEndTimestamp) {
        // Oiling: Yellow
        strip.setBrightness(currentHighBrightness);
        color = strip.Color(255, 200, 0);
    } else if (wifiActive) {
        // WiFi Active: White pulsing
        // Use sine wave for smooth pulsing
        float pulse = (sin(now / 500.0) + 1.0) / 2.0; // 0.0 to 1.0
        uint8_t brightness = (uint8_t)(pulse * currentHighBrightness);
        if (brightness < 10) brightness = 10; // Minimum brightness
        
        strip.setBrightness(brightness);
        color = strip.Color(255, 255, 255); // White
    } else if (!hasFix) {
        // No GPS
        unsigned long timeSinceLoss = (lastEmergUpdate > 0) ? (now - lastEmergUpdate) : 0;
        
        if (emergencyModeForced) {
             // Forced Emergency: Cyan
             strip.setBrightness(currentDimBrightness);
             color = strip.Color(0, 255, 255); 
        } else if (timeSinceLoss > EMERGENCY_TIMEOUT_MS) {
             // Timeout: Red Bright
             strip.setBrightness(currentHighBrightness);
             color = strip.Color(255, 0, 0); 
        } else if (emergencyMode || timeSinceLoss > EMERGENCY_WAIT_MS) {
             // Auto Emergency Active/Waiting: Cyan Dim
             strip.setBrightness(currentDimBrightness);
             color = strip.Color(0, 255, 255);
        } else {
             // No GPS (Short term): Magenta Dim
             strip.setBrightness(currentDimBrightness);
             color = strip.Color(255, 0, 255);
        }
    } else if (rainMode) {
        // Rain Mode: Blue Dim
        strip.setBrightness(currentDimBrightness);
        color = strip.Color(0, 0, 255);
    } else {
        // Normal: Green Dim (Status OK)
        strip.setBrightness(currentDimBrightness);
        color = strip.Color(0, 255, 0);
    }

    // Tank Warning Overlay (Red Pulse 2x then pause)
    if (tankMonitorEnabled && (currentTankLevelMl / tankCapacityMl * 100.0) < tankWarningThresholdPercent) {
        unsigned long cycle = now % 7500; // 7.5s cycle (2x 1s pulse + 0.5s gap + 5s pause)
        float val = 0.0;
        bool active = false;
        
        // Pulse 1: 0-1000ms
        if (cycle < 1000) { 
            val = sin((cycle / 1000.0) * PI);
            active = true;
        } 
        // Pulse 2: 1500-2500ms (500ms gap)
        else if (cycle > 1500 && cycle < 2500) { 
            val = sin(((cycle - 1500) / 1000.0) * PI);
            active = true;
        }
        
        if (active) {
            uint8_t bri = (uint8_t)(val * currentHighBrightness);
            if (bri < 5) bri = 5; // Minimum visibility
            strip.setBrightness(bri);
            color = strip.Color(255, 0, 0); // Red
        }
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
    emergencyModeForced = preferences.getBool("emerg_force", false);
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
    preferences.putUChar("led_dim", ledBrightnessDim);
    preferences.putUChar("led_high", ledBrightnessHigh);
    
    preferences.putBool("night_en", nightModeEnabled);
    preferences.putInt("night_start", nightStartHour);
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
    
    preferences.putBool("emerg_force", emergencyModeForced);

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

        // Static variable for simulation step (shared across calls)
        static unsigned long lastSimStep = 0;

        if (emergencyModeForced) {
            // Forced Mode: Simulate 50 km/h continuously
            emergencyMode = true;
            
            if (lastSimStep == 0) lastSimStep = now;
            unsigned long dt = now - lastSimStep;
            lastSimStep = now;
            if (dt > 1000) dt = 1000; 

            float simSpeed = 50.0;
            double distKm = (double)simSpeed * ((double)dt / 3600000.0);
            processDistance(distKm, simSpeed);
            
        } else {
            // Auto Mode: Time-based Logic
            lastSimStep = 0; // Reset sim timer

            // 1. Wait -> Oil once
            if (timeSinceLoss > EMERGENCY_OIL_1_MS && emergencyOilCount == 0) {
                int pulses = ranges[1].pulses;
                if (rainMode) pulses *= 2; // Double oil amount in Rain Mode
                triggerOil(pulses); 
                emergencyOilCount++;
                emergencyMode = true; 
#ifdef GPS_DEBUG
                Serial.println("Emergency Mode: 1st Oiling");
#endif
            }
            
            // 2. Wait -> Oil once
            if (timeSinceLoss > EMERGENCY_OIL_2_MS && emergencyOilCount == 1) {
                int pulses = ranges[1].pulses;
                if (rainMode) pulses *= 2; // Double oil amount in Rain Mode
                triggerOil(pulses);
                emergencyOilCount++;
#ifdef GPS_DEBUG
                Serial.println("Emergency Mode: 2nd Oiling");
#endif
            }
            
            // 3. Timeout
            if (timeSinceLoss > EMERGENCY_TIMEOUT_MS) {
                // Timeout State (Red LED handled in updateLED)
                emergencyMode = true; 
            } else if (timeSinceLoss > EMERGENCY_WAIT_MS) {
                // Active Waiting State
                emergencyMode = true;
            } else {
                // Just waiting, not yet Emergency Mode
                emergencyMode = false;
            }
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
        
        // Increase Odometer
        totalDistance += distKm;
        progressChanged = true; // So Odometer gets saved

        if (speedKmh >= MIN_SPEED_KMH) {
            processDistance(distKm, speedKmh);
        }
    }
}

void Oiler::processDistance(double distKm, float speedKmh) {
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
    oilingPulsesRemaining = pulses;
    pulseState = false; // Will start with HIGH in handleOiling
    lastPulseTime = millis() - 1000; // Force immediate start
    
    // LED Indication
    ledOilingEndTimestamp = millis() + 3000;
}

void Oiler::processPump() {
    unsigned long now = millis();

    // Check if we should stop bleeding
    if (bleedingMode) {
        if (now - bleedingStartTime > BLEEDING_DURATION_MS) {
            bleedingMode = false;
            digitalWrite(pumpPin, LOW);
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
    if (!pulseState) {
        // Currently LOW (Pause phase)
        // Wait for PAUSE_DURATION_MS
        if (now - lastPulseTime >= PAUSE_DURATION_MS) {
            digitalWrite(pumpPin, HIGH);
            pulseState = true;
            lastPulseTime = now;
        }
    } else {
        // Currently HIGH (Pulse phase)
        // Wait for PULSE_DURATION
        if (now - lastPulseTime >= PULSE_DURATION_MS) {
            digitalWrite(pumpPin, LOW);
            pulseState = false;
            lastPulseTime = now;
            
            // Only decrement in normal oiling mode
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
                    // Serial.printf("Bleeding Pulse: %.2f ml consumed\n", mlConsumed);
                }
            }
        }
    }
}

void Oiler::setRainMode(bool mode) {
    if (mode && !rainMode) {
        rainModeStartTime = millis();
    }
    rainMode = mode;
}

SpeedRange* Oiler::getRangeConfig(int index) {
    if(index >= 0 && index < NUM_RANGES) return &ranges[index];
    return nullptr;
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
