#ifndef AUX_MANAGER_H
#define AUX_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "ImuHandler.h"
#include "Persistence.h"

enum AuxMode {
    AUX_MODE_OFF = 0,
    AUX_MODE_AUX_POWER = 1,
    AUX_MODE_HEATED_GRIPS = 2
};

enum ReactionSpeed {
    REACTION_SLOW = 0,
    REACTION_MEDIUM = 1,
    REACTION_FAST = 2
};

class AuxManager {
public:
    AuxManager(IPersistence* store);
    void begin(ImuHandler* imu);
    void loop(float currentSpeedKmh, float currentTempC, bool isRainMode);
    
    // Configuration
    void setMode(AuxMode mode);
    AuxMode getMode() const { return _mode; }
    
    // Heated Grips Settings
    void setGripSettings(int baseLevel, float speedFactor, float tempFactor, float tempOffset, float startTemp, int rainBoost, int startupBoostLevel, int startupBoostSec, int startDelaySec, int reactionSpeed);
    void getGripSettings(int &baseLevel, float &speedFactor, float &tempFactor, float &tempOffset, float &startTemp, int &rainBoost, int &startupBoostLevel, int &startupBoostSec, int &startDelaySec, int &reactionSpeed);
    
    // Status
    int getCurrentPwm() const { return _currentPwm; }
    bool isPowered() const { return _isPowered; }
    bool isBoostActive() const { return _isBoosting; }
    
    // Manual Control
    void toggleManualOverride();
    bool isManualOverrideActive() const { return _manualOverride; }

private:
    ImuHandler* _imu;
    IPersistence* _store;
    AuxMode _mode = AUX_MODE_OFF;
    
    // Pin State
    int _currentPwm = 0;
    float _smoothedPwm = 0.0;
    bool _isPowered = false;
    bool _isBoosting = false;
    bool _manualOverride = true; // Default ON (Auto)
    
    // Aux Power Logic
    unsigned long _lastMotionTime = 0;
    bool _engineRunning = false;
    
    // Heated Grips Logic
    unsigned long _boostEndTime = 0; // Timestamp when boost ends
    int _baseLevel = 30;        // 0-100%
    float _speedFactor = 0.5;   // % per km/h
    float _tempFactor = 2.0;    // % per degree below startTemp
    float _tempOffset = 0.0;    // Correction for sensor placement
    float _startTemp = 15.0;    // Temperature below which compensation starts
    int _rainBoost = 10;        // % boost in rain mode
    int _startupBoostLevel = 85;// % for startup
    int _startupBoostSec = 75;  // Seconds for startup boost
    int _startDelaySec = 20;    // Seconds delay after start
    ReactionSpeed _reactionSpeed = REACTION_SLOW;
    
    unsigned long _startTime = 0;
    
    void handleAuxPower();
    void handleHeatedGrips(float speed, float temp, bool rain);
    void setPwm(int percent);
    void calcBoostEndTime();
};

#endif
