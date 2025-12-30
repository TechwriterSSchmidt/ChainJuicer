#ifndef AUX_MANAGER_H
#define AUX_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"
#include "ImuHandler.h"

enum AuxMode {
    AUX_MODE_OFF = 0,
    AUX_MODE_SMART_POWER = 1,
    AUX_MODE_HEATED_GRIPS = 2
};

class AuxManager {
public:
    AuxManager();
    void begin(ImuHandler* imu);
    void loop(float currentSpeedKmh, float currentTempC, bool isRainMode);
    
    // Configuration
    void setMode(AuxMode mode);
    AuxMode getMode() const { return _mode; }
    
    // Heated Grips Settings
    void setGripSettings(int baseLevel, float speedFactor, float tempFactor, float tempOffset, float startTemp, int rainBoost, int startupBoostLevel, int startupBoostSec);
    void getGripSettings(int &baseLevel, float &speedFactor, float &tempFactor, float &tempOffset, float &startTemp, int &rainBoost, int &startupBoostLevel, int &startupBoostSec);
    
    // Status
    int getCurrentPwm() const { return _currentPwm; }
    bool isPowered() const { return _isPowered; }
    
    // Manual Control
    void toggleManualOverride();
    bool isManualOverrideActive() const { return _manualOverride; }

private:
    ImuHandler* _imu;
    Preferences _prefs;
    AuxMode _mode = AUX_MODE_OFF;
    
    // Pin State
    int _currentPwm = 0;
    float _smoothedPwm = 0.0;
    bool _isPowered = false;
    bool _manualOverride = true; // Default ON (Auto)
    
    // Smart Power Logic
    unsigned long _lastMotionTime = 0;
    bool _engineRunning = false;
    
    // Heated Grips Logic
    int _baseLevel = 30;        // 0-100%
    float _speedFactor = 0.5;   // % per km/h
    float _tempFactor = 2.0;    // % per degree below 20C
    float _tempOffset = 0.0;    // Correction for sensor placement
    float _startTemp = 20.0;    // Temperature below which compensation starts
    int _rainBoost = 10;        // % boost in rain mode
    int _startupBoostLevel = 80;// % for startup
    int _startupBoostSec = 60;  // Seconds for startup boost
    
    unsigned long _startTime = 0;
    
    void handleSmartPower();
    void handleHeatedGrips(float speed, float temp, bool rain);
    void setPwm(int percent);
};

#endif
