/**
 * @file ChainOiler.h
 * @brief ChainOiler class for GPS-controlled chain oiling system
 * 
 * Manages GPS data, pump control, and automatic chain oiling logic
 */

#ifndef CHAIN_OILER_H
#define CHAIN_OILER_H

#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include "config.h"

/**
 * @class ChainOiler
 * @brief Main controller class for the GPS-based chain oiler system
 */
class ChainOiler {
public:
    /**
     * @brief Constructor
     */
    ChainOiler();
    
    /**
     * @brief Initialize the chain oiler system
     * @return true if initialization successful, false otherwise
     */
    bool begin();
    
    /**
     * @brief Main update loop - call this frequently
     */
    void update();
    
    /**
     * @brief Get current GPS speed in km/h
     * @return Current speed, or 0.0 if no GPS fix
     */
    float getSpeed() const { return currentSpeed; }
    
    /**
     * @brief Get GPS fix status
     * @return true if GPS has valid fix
     */
    bool hasGPSFix() const { return gpsFixValid; }
    
    /**
     * @brief Get number of satellites
     * @return Satellite count
     */
    uint8_t getSatellites() const { return satellites; }
    
    /**
     * @brief Get current pump duty cycle
     * @return PWM duty cycle (0-255)
     */
    uint8_t getPumpDuty() const { return currentPumpDuty; }
    
    /**
     * @brief Get time until next oil shot (ms)
     * @return Milliseconds until next oiling
     */
    uint32_t getTimeUntilNextOil() const;
    
    /**
     * @brief Manually trigger oil shot (for testing)
     */
    void manualOilShot();
    
    /**
     * @brief Get total distance traveled (km)
     * @return Total distance
     */
    float getTotalDistance() const { return totalDistance; }
    
    /**
     * @brief Get total oil shots delivered
     * @return Oil shot count
     */
    uint32_t getOilShotCount() const { return oilShotCount; }

private:
    // GPS
    TinyGPSPlus gps;
    HardwareSerial gpsSerial;
    bool gpsFixValid;
    float currentSpeed;
    uint8_t satellites;
    unsigned long lastGPSUpdate;
    unsigned long lastValidFix;
    float totalDistance;
    float lastLatitude;
    float lastLongitude;
    bool firstFix;
    
    // Pump control
    uint8_t currentPumpDuty;
    unsigned long lastOilShot;
    unsigned long nextOilInterval;
    uint32_t oilShotCount;
    bool pumpActive;
    unsigned long pumpStartTime;
    
    /**
     * @brief Update GPS data from serial
     */
    void updateGPS();
    
    /**
     * @brief Calculate next oil interval based on speed
     * @param speed Current speed in km/h
     * @return Interval in milliseconds
     */
    uint32_t calculateOilInterval(float speed);
    
    /**
     * @brief Calculate pump duty cycle based on speed
     * @param speed Current speed in km/h
     * @return PWM duty cycle (0-255)
     */
    uint8_t calculatePumpDuty(float speed);
    
    /**
     * @brief Update pump control logic
     */
    void updatePump();
    
    /**
     * @brief Trigger an oil shot
     */
    void triggerOilShot();
    
    /**
     * @brief Stop the pump
     */
    void stopPump();
    
    /**
     * @brief Update distance calculation
     */
    void updateDistance();
};

#endif // CHAIN_OILER_H
