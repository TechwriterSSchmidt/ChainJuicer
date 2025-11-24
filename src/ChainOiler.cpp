/**
 * @file ChainOiler.cpp
 * @brief Implementation of ChainOiler class
 */

#include "ChainOiler.h"

ChainOiler::ChainOiler() 
    : gpsSerial(2), // Use HardwareSerial2
      gpsFixValid(false),
      currentSpeed(0.0f),
      satellites(0),
      lastGPSUpdate(0),
      lastValidFix(0),
      totalDistance(0.0f),
      lastLatitude(0.0f),
      lastLongitude(0.0f),
      firstFix(true),
      currentPumpDuty(0),
      lastOilShot(0),
      nextOilInterval(OIL_INTERVAL_MAX),
      oilShotCount(0),
      pumpActive(false),
      pumpStartTime(0)
{
}

bool ChainOiler::begin() {
    // Initialize GPS serial
    gpsSerial.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    
    // Initialize pump PWM
    ledcSetup(PUMP_PWM_CHANNEL, PUMP_PWM_FREQ, PUMP_PWM_RESOLUTION);
    ledcAttachPin(PUMP_PIN, PUMP_PWM_CHANNEL);
    ledcWrite(PUMP_PWM_CHANNEL, 0);
    
    // Initialize status LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);
    
    #if SERIAL_DEBUG
    Serial.println(F("ChainOiler: Initialized"));
    Serial.println(F("Waiting for GPS fix..."));
    #endif
    
    return true;
}

void ChainOiler::update() {
    updateGPS();
    updatePump();
}

void ChainOiler::updateGPS() {
    unsigned long currentMillis = millis();
    
    // Read GPS data
    while (gpsSerial.available() > 0) {
        char c = gpsSerial.read();
        gps.encode(c);
    }
    
    // Update GPS data at specified interval
    if (currentMillis - lastGPSUpdate >= GPS_UPDATE_INTERVAL) {
        lastGPSUpdate = currentMillis;
        
        // Get satellite count first
        if (gps.satellites.isValid()) {
            satellites = gps.satellites.value();
        }
        
        // Check for valid fix with minimum satellite count
        if (gps.location.isValid() && gps.location.age() < 2000 && satellites >= MIN_SATELLITES) {
            gpsFixValid = true;
            lastValidFix = currentMillis;
            
            // Get speed in km/h
            if (gps.speed.isValid()) {
                // Constrain speed to valid range
                currentSpeed = constrain(gps.speed.kmph(), 0.0f, SPEED_THRESHOLD_MAX);
            }
            
            // Update distance calculation
            updateDistance();
            
            // Blink LED to show GPS fix
            digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
            
        } else {
            // Check for GPS timeout
            if (currentMillis - lastValidFix > GPS_FIX_TIMEOUT) {
                gpsFixValid = false;
                currentSpeed = 0.0f;
                digitalWrite(STATUS_LED_PIN, LOW);
            }
        }
        
        #if SERIAL_DEBUG
        if (gpsFixValid) {
            Serial.printf("GPS: Speed=%.1f km/h, Sats=%d, Dist=%.2f km\n", 
                         currentSpeed, satellites, totalDistance);
        }
        #endif
    }
}

void ChainOiler::updateDistance() {
    if (!gpsFixValid || !gps.location.isValid()) {
        return;
    }
    
    float currentLat = gps.location.lat();
    float currentLng = gps.location.lng();
    
    if (!firstFix) {
        // Calculate distance from last position
        float distanceMeters = TinyGPSPlus::distanceBetween(
            lastLatitude, lastLongitude,
            currentLat, currentLng
        );
        
        // Convert to kilometers and add to total
        totalDistance += distanceMeters / 1000.0f;
    } else {
        firstFix = false;
    }
    
    // Update last position
    lastLatitude = currentLat;
    lastLongitude = currentLng;
}

uint32_t ChainOiler::calculateOilInterval(float speed) {
    // No oiling below minimum speed
    if (speed < SPEED_THRESHOLD_MIN) {
        return OIL_INTERVAL_MAX;
    }
    
    // Linear interpolation between min and max intervals
    // Higher speed = shorter interval (more frequent oiling)
    float speedRatio = (speed - SPEED_THRESHOLD_MIN) / 
                       (SPEED_THRESHOLD_MAX - SPEED_THRESHOLD_MIN);
    
    // Clamp ratio to 0.0 - 1.0
    if (speedRatio > 1.0f) speedRatio = 1.0f;
    if (speedRatio < 0.0f) speedRatio = 0.0f;
    
    // Calculate interval (inverse relationship with speed)
    uint32_t interval = OIL_INTERVAL_MAX - 
                        (uint32_t)((OIL_INTERVAL_MAX - OIL_INTERVAL_MIN) * speedRatio);
    
    return interval;
}

uint8_t ChainOiler::calculatePumpDuty(float speed) {
    // No pump below minimum speed
    if (speed < SPEED_THRESHOLD_MIN) {
        return PUMP_OFF_DUTY;
    }
    
    // Linear interpolation between min and max duty
    float speedRatio = (speed - SPEED_THRESHOLD_MIN) / 
                       (SPEED_THRESHOLD_MAX - SPEED_THRESHOLD_MIN);
    
    // Clamp ratio to 0.0 - 1.0
    if (speedRatio > 1.0f) speedRatio = 1.0f;
    if (speedRatio < 0.0f) speedRatio = 0.0f;
    
    // Calculate duty cycle
    uint8_t duty = PUMP_MIN_DUTY + 
                   (uint8_t)((PUMP_MAX_DUTY - PUMP_MIN_DUTY) * speedRatio);
    
    return duty;
}

void ChainOiler::updatePump() {
    unsigned long currentMillis = millis();
    
    // Handle active pump pulse
    if (pumpActive) {
        if (currentMillis - pumpStartTime >= OIL_PULSE_DURATION) {
            stopPump();
        }
        return;
    }
    
    // Check if we should trigger oil shot
    if (gpsFixValid && currentSpeed >= SPEED_THRESHOLD_MIN) {
        nextOilInterval = calculateOilInterval(currentSpeed);
        
        if (currentMillis - lastOilShot >= nextOilInterval) {
            triggerOilShot();
        }
    } else {
        // No GPS fix or below minimum speed - keep pump off
        stopPump();
    }
}

void ChainOiler::triggerOilShot() {
    if (pumpActive) return;
    
    currentPumpDuty = calculatePumpDuty(currentSpeed);
    
    if (currentPumpDuty > 0) {
        ledcWrite(PUMP_PWM_CHANNEL, currentPumpDuty);
        pumpActive = true;
        pumpStartTime = millis();
        lastOilShot = millis();
        oilShotCount++;
        
        #if SERIAL_DEBUG
        Serial.printf("OIL SHOT: Duty=%d, Speed=%.1f km/h, Count=%lu\n", 
                     currentPumpDuty, currentSpeed, oilShotCount);
        #endif
    }
}

void ChainOiler::stopPump() {
    if (pumpActive || currentPumpDuty > 0) {
        ledcWrite(PUMP_PWM_CHANNEL, 0);
        pumpActive = false;
        currentPumpDuty = 0;
    }
}

void ChainOiler::manualOilShot() {
    if (!pumpActive) {
        currentPumpDuty = PUMP_MIN_DUTY;
        ledcWrite(PUMP_PWM_CHANNEL, currentPumpDuty);
        pumpActive = true;
        pumpStartTime = millis();
        oilShotCount++;
        
        #if SERIAL_DEBUG
        Serial.println(F("MANUAL OIL SHOT"));
        #endif
    }
}

uint32_t ChainOiler::getTimeUntilNextOil() const {
    if (!gpsFixValid || currentSpeed < SPEED_THRESHOLD_MIN) {
        return 0;
    }
    
    unsigned long currentMillis = millis();
    unsigned long elapsed = currentMillis - lastOilShot;
    
    if (elapsed >= nextOilInterval) {
        return 0;
    }
    
    return nextOilInterval - elapsed;
}
