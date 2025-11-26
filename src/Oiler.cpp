#include "Oiler.h"
#include <Preferences.h>

Preferences preferences;

Oiler::Oiler() : strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800) {
    pumpPin = PUMP_PIN;
    pinMode(pumpPin, OUTPUT);
    digitalWrite(pumpPin, LOW);
    
    // Initialize default configuration
    ranges[0] = {10, 35, 15.0, 2};
    ranges[1] = {35, 55, 15.0, 2};
    ranges[2] = {55, 75, 15.0, 2};
    ranges[3] = {75, 95, 15.0, 2};
    ranges[4] = {95, 999, 15.0, 2};
    
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
    currentHour = 12;    // Default noon

    lastEmergUpdate = 0;
    lastStandstillSaveTime = 0;
}

void Oiler::begin() {
    preferences.begin("oiler", false);
    loadConfig();

    // Hardware Init
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    strip.begin();
    strip.setBrightness(ledBrightnessDim);
    strip.show(); // All pixels off
}

void Oiler::loop() {
    handleButton();
    handleOiling(); // Non-blocking oiling logic
    
    // Rain Mode Auto-Off (30 Minutes)
    if (rainMode && (millis() - rainModeStartTime > 30 * 60 * 1000)) {
        rainMode = false;
        Serial.println("Rain Mode Auto-Off");
        saveConfig();
    }

    // Bleeding Logic
    if (bleedingMode) {
        if (millis() - bleedingStartTime < BLEEDING_DURATION_MS) {
            digitalWrite(pumpPin, HIGH);
        } else {
            digitalWrite(pumpPin, LOW);
            bleedingMode = false;
        }
    }

    updateLED();
}

void Oiler::handleButton() {
    // Read button (Active LOW due to INPUT_PULLUP)
    bool currentReading = !digitalRead(BUTTON_PIN);

    // Debounce (simple)
    if (currentReading != lastButtonState) {
        // State changed
        if (currentReading) {
            // Pressed
            buttonPressStartTime = millis();
        } else {
            // Released
            unsigned long pressDuration = millis() - buttonPressStartTime;
            
            // Short Press (< 1.5s): Rain Mode Toggle
            if (pressDuration < RAIN_TOGGLE_MS && pressDuration > 50) {
                rainMode = !rainMode;
                if (rainMode) rainModeStartTime = millis();
                Serial.print("Rain Mode: ");
                Serial.println(rainMode ? "ON" : "OFF");
                saveConfig(); // Save setting
            }
        }
    }

    // Check Long Press while holding
    if (currentReading && !bleedingMode) {
        unsigned long duration = millis() - buttonPressStartTime;
        
        // > 10s: Bleeding Mode
        if (duration > BLEEDING_PRESS_MS) {
            bleedingMode = true;
            bleedingStartTime = millis();
            Serial.println("Bleeding Mode STARTED");
            
            pumpCycles++; // Bleeding counts as 1 cycle
            saveConfig(); // Save immediately

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
        }
    }

    if (bleedingMode) {
        // Bleeding: Red blinking 2Hz (250ms on, 250ms off)
        strip.setBrightness(ledBrightnessHigh);
        if ((now / 250) % 2 == 0) {
            color = strip.Color(255, 0, 0);
        } else {
            color = 0; // Off
        }
    } else if (isOiling) {
        // Oiling: Yellow
        strip.setBrightness(ledBrightnessHigh);
        color = strip.Color(255, 200, 0);
    } else if (wifiActive) {
        // WiFi Active: White blinking (1Hz)
        strip.setBrightness(ledBrightnessHigh);
        if ((now / 500) % 2 == 0) {
            color = strip.Color(255, 255, 255); // White
        } else {
            color = 0; // Off
        }
    } else if (!hasFix) {
        // No GPS: Magenta Dim
        // If Emergency Mode active: Cyan
        if (emergencyMode) {
             strip.setBrightness(currentDimBrightness);
             color = strip.Color(0, 255, 255); // Cyan
        } else {
             strip.setBrightness(currentDimBrightness);
             color = strip.Color(255, 0, 255); // Magenta
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

    strip.setPixelColor(0, color);
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
    
    // Restore Rain Mode
    rainMode = preferences.getBool("rain_mode", false);
    if (rainMode) rainModeStartTime = millis();
    emergencyMode = preferences.getBool("emerg_mode", false);

    // Load Stats
    totalDistance = preferences.getDouble("totalDist", 0.0);
    pumpCycles = preferences.getUInt("pumpCount", 0);

    validateConfig();
}

void Oiler::validateConfig() {
    // Ensure no 0 or negative values exist
    for(int i=0; i<NUM_RANGES; i++) {
        if(ranges[i].intervalKm < 1.0) ranges[i].intervalKm = 1.0; // Minimum 1km
        if(ranges[i].pulses < 1) ranges[i].pulses = 1;             // Minimum 1 pulse
    }
    
    // Brightness limits
    if(ledBrightnessDim > 255) ledBrightnessDim = 255;
    if(ledBrightnessHigh > 255) ledBrightnessHigh = 255;
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
    
    // Save Rain Mode
    preferences.putBool("rain_mode", rainMode);
    preferences.putBool("emerg_mode", emergencyMode);

    // Save Stats
    preferences.putDouble("totalDist", totalDistance);
    preferences.putUInt("pumpCount", pumpCycles);
}

void Oiler::saveProgress() {
    if (progressChanged) {
        preferences.putFloat("progress", currentProgress);
        // Save stats here too
        preferences.putDouble("totalDist", totalDistance);
        preferences.putUInt("pumpCount", pumpCycles);
        
        progressChanged = false;
        Serial.println("Progress & Stats saved.");
    }
}

void Oiler::resetStats() {
    totalDistance = 0.0;
    pumpCycles = 0;
    saveConfig();
}

void Oiler::update(float rawSpeedKmh, double lat, double lon, bool gpsValid) {
    unsigned long now = millis();

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

    // Regular saving (every 5 minutes or at standstill)
    if (now - lastSaveTime > 300000) { // 5 Min
        saveProgress();
        lastSaveTime = now;
    }
    // Save immediately at standstill (if we were moving before), but limit frequency (min 2 min)
    if (speedKmh < 5.0 && progressChanged && (now - lastStandstillSaveTime > 120000)) {
        saveProgress();
        lastStandstillSaveTime = now;
    }

    if (!gpsValid) {
        hasFix = false;
        
        // Emergency Mode: If active and no GPS, simulate 50 km/h
        if (emergencyMode) {
            // 50 km/h
            float simSpeed = 50.0;
            // Distance in km = (Speed km/h * Time h)
            // We use the difference to the last execution.
            
            if (lastEmergUpdate == 0) lastEmergUpdate = now;
            
            unsigned long dt = now - lastEmergUpdate;
            lastEmergUpdate = now;
            
            // Avoid huge jumps if loop was blocked
            if (dt > 1000) dt = 1000; // Cap at 1 second max per iteration

            double distKm = (double)simSpeed * ((double)dt / 3600000.0);
            
            processDistance(distKm, simSpeed);
        }
        
        return;
    }

    // First Fix?
    if (!hasFix) {
        lastLat = lat;
        lastLon = lon;
        hasFix = true;
        lastEmergUpdate = 0; // Reset Emergency Timer when GPS is back
        return;
    }
    
    // Reset Emergency Timer if we have valid GPS
    lastEmergUpdate = 0;

    // Calculate distance (Haversine or TinyGPS function)
    double distKm = TinyGPSPlus::distanceBetween(lastLat, lastLon, lat, lon) / 1000.0;
    
    // Only if moving and GPS not jumping (small filter)
    // Plausibility check: < 300 km/h
    if (distKm > 0.005 && speedKmh > 2.0 && speedKmh < 300.0) { 
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
        // This solves the problem when switching speed ranges.
        float interval = ranges[activeRangeIndex].intervalKm;
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
                triggerOil(ranges[activeRangeIndex].pulses);
                currentProgress = 0.0; // Reset
                saveProgress(); // Save progress
            }
        }
    }
}

void Oiler::triggerOil(int pulses) {
    Serial.println("OILING START (Non-Blocking)");
    
    pumpCycles++; // Stats
    progressChanged = true; // Mark for saving

    // Initialize Non-Blocking Oiling
    isOiling = true;
    oilingPulsesRemaining = pulses;
    pulseState = false; // Will start with HIGH in handleOiling
    lastPulseTime = millis() - 1000; // Force immediate start
}

void Oiler::handleOiling() {
    if (!isOiling) return;

    unsigned long now = millis();
    
    if (oilingPulsesRemaining > 0) {
        if (!pulseState) {
            // Currently LOW (Pause), waiting to go HIGH
            // Initial start or after pause
            // Pause duration is 200ms, but for first pulse it doesn't matter
            if (now - lastPulseTime >= 200) {
                digitalWrite(pumpPin, HIGH);
                pulseState = true;
                lastPulseTime = now;
            }
        } else {
            // Currently HIGH (Pumping), waiting to go LOW
            if (now - lastPulseTime >= PULSE_DURATION_MS) {
                digitalWrite(pumpPin, LOW);
                pulseState = false;
                lastPulseTime = now;
                oilingPulsesRemaining--;
                
                if (oilingPulsesRemaining == 0) {
                    isOiling = false;
                    Serial.println("OILING DONE");
                }
            }
        }
    } else {
        isOiling = false;
        digitalWrite(pumpPin, LOW);
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
    // handleButton() updates 'buttonState' (which tracks press start) and 'lastButtonState' (raw reading)
    // But for a stable "is pressed" check, we can use the raw reading if we trust the user holds it,
    // OR we can implement a dedicated debounced state variable.
    // Given handleButton logic:
    // if (currentReading != lastButtonState) -> debounce logic
    // So 'lastButtonState' is actually the raw reading from the PREVIOUS loop.
    
    // Better approach: Read pin, but require it to be stable?
    // Since this is called from main loop for WiFi activation (long hold), 
    // a simple raw read is usually fine. 
    // But to be cleaner and use the class logic:
    return !digitalRead(BUTTON_PIN); // Active LOW -> returns true if pressed
}
