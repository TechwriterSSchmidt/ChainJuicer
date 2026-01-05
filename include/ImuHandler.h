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
    void calibrateZero(); // "Tare" - Set current orientation as flat
    void startCalibration(); // Non-blocking calibration with countdown
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
    bool isStationary(); // Garage Guard: Returns true if bike is stable (not moving) for 3 seconds
    bool isCrashed(); // Lean > 70
    bool isMotionDetected(); // Smart Stop helper (Vibration/Accel)
    bool isLeaningTowardsTire(float thresholdDeg); // Returns true if leaning towards the tire (Unsafe to oil)

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
    
    // Chain Configuration
    bool _chainOnRight = true; // Default: Right side

    // Motion Detection
    float _linAccelX = 0.0;
    float _linAccelY = 0.0;
    float _linAccelZ = 0.0;
    unsigned long _lastMotionTime = 0;

    // Stability Check (Garage Guard)
    static const int HISTORY_SIZE = 100; // 5 seconds at ~20Hz (50ms update)
    float _rollHistory[HISTORY_SIZE];
    float _pitchHistory[HISTORY_SIZE];
    int _historyIndex = 0;
    bool _historyFilled = false;

    Preferences _prefs;
    
    void processOrientation();
    void updateHistory(float roll, float pitch);
    float calculateVariance(float* data, int size);
    
    unsigned long _lastUpdate = 0;

    // Calibration State Machine
    enum CalibrationState {
        CAL_IDLE,
        CAL_WAIT,
        CAL_MEASURE
    };
    CalibrationState _calState = CAL_IDLE;
    unsigned long _calTimer = 0;
    double _calSumRoll = 0;
    double _calSumPitch = 0;
    int _calSamples = 0;
    int _calLastSec = 0;
};

#endif
