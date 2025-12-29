#ifndef IMU_HANDLER_H
#define IMU_HANDLER_H

#include <Arduino.h>
#include <Adafruit_BNO08x.h>
#include <Preferences.h>

class ImuHandler {
public:
    ImuHandler();
    bool begin(int sda, int scl);
    void update();
    void loop(); // Call frequently

    // Calibration
    void calibrateZero();
    void calibrateSideStand();
    void saveCalibration();
    void loadCalibration();

    // Status
    bool isAvailable() const { return _available; }
    String getModel() const { return "BNO085"; }
    String getStatus() const { return _available ? "OK" : "Not Found"; }

    // Data
    float getRoll() const { return _roll; }
    float getPitch() const { return _pitch; }
    float getYaw() const { return _yaw; }
    
    // Features
    bool isParked(); // Garage Guard (Side stand or Center stand)
    bool isCrashed(); // Lean > 70
    bool isMotionDetected(); // Smart Stop helper
    bool isLeaningOnChainSide(float thresholdDeg); // Returns true if leaning towards the chain side

    // Configuration
    void setChainSide(bool isRight); // false = Left (Default), true = Right
    bool isChainOnRight() const { return _chainOnRight; }

private:
    Adafruit_BNO08x _bno;
    sh2_SensorValue_t _sensorValue;
    bool _available = false;
    
    // Orientation
    float _roll = 0.0;
    float _pitch = 0.0;
    float _yaw = 0.0;

    // Calibration Offsets (Zero position)
    float _offsetRoll = 0.0;
    float _offsetPitch = 0.0;
    
    // Side Stand Calibration (Target angles)
    float _sideStandRoll = 0.0; // If 0, use default threshold
    bool _sideStandCalibrated = false;
    
    // Chain Configuration
    bool _chainOnRight = false; // Default: Left side

    // Motion Detection
    float _linAccelX = 0.0;
    float _linAccelY = 0.0;
    float _linAccelZ = 0.0;
    unsigned long _lastMotionTime = 0;

    Preferences _prefs;
    
    void processOrientation();
    unsigned long _lastUpdate = 0;
};

#endif
