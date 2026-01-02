#include "AuxManager.h"

#define AUX_PWM_CHANNEL 1 // Use channel 1 (Pump uses 0)
#define AUX_PWM_FREQ 1000 // 1 kHz for grips/relays
#define AUX_PWM_RES 8     // 8-bit (0-255)

AuxManager::AuxManager() {
}

void AuxManager::begin(ImuHandler* imu) {
    _imu = imu;
    _startTime = 0; // Wait for engine start
    
    // Initialize Pin
    pinMode(AUX_PIN, OUTPUT);
    digitalWrite(AUX_PIN, LOW);
    
    // Setup PWM
    ledcSetup(AUX_PWM_CHANNEL, AUX_PWM_FREQ, AUX_PWM_RES);
    ledcAttachPin(AUX_PIN, AUX_PWM_CHANNEL);
    ledcWrite(AUX_PWM_CHANNEL, 0);
    
    // Load Preferences
    _prefs.begin("aux", false);
    _mode = (AuxMode)_prefs.getInt("mode", AUX_MODE_OFF);
    
    _baseLevel = _prefs.getInt("base", 25);
    _speedFactor = _prefs.getFloat("speedF", 0.5);
    _tempFactor = _prefs.getFloat("tempF", 2.0);
    _tempOffset = _prefs.getFloat("tempO", 0.0);
    _startTemp = _prefs.getFloat("startT", 20.0);
    _rainBoost = _prefs.getInt("rainB", 10);
    _startupBoostLevel = _prefs.getInt("startL", 100);
    _startupBoostSec = _prefs.getInt("startS", 75);
    _startDelaySec = _prefs.getInt("startD", 15);
    _reactionSpeed = (ReactionSpeed)_prefs.getInt("react", REACTION_SLOW);
    
    // Load saved state for manual override (persistence)
    _manualOverride = _prefs.getBool("man_ovr", true);
    
    // If enabled at boot, calculate boost end time
    if (_manualOverride) {
        calcBoostEndTime();
    }

    _prefs.end();
}

void AuxManager::loop(float currentSpeedKmh, float currentTempC, bool isRainMode) {
    if (_mode == AUX_MODE_OFF || !_manualOverride) {
        setPwm(0);
        return;
    }
    
    if (_mode == AUX_MODE_AUX_POWER) {
        handleAuxPower();
    } else if (_mode == AUX_MODE_HEATED_GRIPS) {
        handleHeatedGrips(currentSpeedKmh, currentTempC, isRainMode);
    }
}

void AuxManager::toggleManualOverride() {
    _manualOverride = !_manualOverride;
    
    if (_manualOverride) {
        // Recalculate boost when manually enabling
        calcBoostEndTime();
    }

    _prefs.begin("aux", false);
    _prefs.putBool("man_ovr", _manualOverride);
    _prefs.end();
}

void AuxManager::handleAuxPower() {
    // Simple logic: ESP is powered by ignition.
    // Wait configured delay after boot to protect battery during cranking/startup.
    if (millis() < ((unsigned long)_startDelaySec * 1000)) {
        setPwm(0);
    } else {
        setPwm(100);
    }
}

void AuxManager::handleHeatedGrips(float speed, float temp, bool rain) {
    // Simple logic: ESP is powered by ignition.
    // Use millis() directly as elapsed time since start.
    unsigned long now = millis();
    unsigned long bootDelayEnd = (unsigned long)_startDelaySec * 1000;

    // Check Configured Delay (Global Safety)
    if (now < bootDelayEnd) {
        setPwm(0);
        return;
    }

    // 1. Base Level
    float target = _baseLevel;
    
    // 2. Speed Compensation
    if (speed > 0) {
        target += (speed * _speedFactor);
    }
    
    // 3. Temperature Compensation (only if below startTemp)
    // If temp is invalid (-127), ignore this factor or assume cold? 
    // Let's assume 10C if invalid to be safe, or just ignore.
    if (temp > -50) {
        float effectiveTemp = temp + _tempOffset;
        if (effectiveTemp < _startTemp) {
            target += ((_startTemp - effectiveTemp) * _tempFactor);
        }
    }
    
    // 4. Rain Boost
    if (rain) {
        target += _rainBoost;
    }
    
    // 5. Startup Boost
    _isBoosting = false;
    if (now < _boostEndTime) {
        if (target < _startupBoostLevel) {
            target = _startupBoostLevel;
            _isBoosting = true;
        }
    }
    
    // Clamp
    if (target > 100) target = 100;
    if (target < 0) target = 0;
    
    // Smoothing (Low Pass Filter) based on Reaction Speed
    float alpha = 0.001; // Default SLOW (~10s)
    
    if (_reactionSpeed == REACTION_MEDIUM) {
        alpha = 0.002; // Medium (~5s)
    } else if (_reactionSpeed == REACTION_FAST) {
        alpha = 0.01; // Fast (~1s)
    }
    
    _smoothedPwm = (_smoothedPwm * (1.0 - alpha)) + (target * alpha);
    
    setPwm((int)_smoothedPwm);
}

void AuxManager::setPwm(int percent) {
    if (percent != _currentPwm) {
        _currentPwm = percent;
        int duty = map(percent, 0, 100, 0, 255);
        ledcWrite(AUX_PWM_CHANNEL, duty);
        _isPowered = (percent > 0);
    }
}

void AuxManager::setMode(AuxMode mode) {
    _mode = mode;
    _prefs.begin("aux", false);
    _prefs.putInt("mode", (int)_mode);
    _prefs.end();
}

void AuxManager::setGripSettings(int baseLevel, float speedFactor, float tempFactor, float tempOffset, float startTemp, int rainBoost, int startupBoostLevel, int startupBoostSec, int startDelaySec, int reactionSpeed) {
    _baseLevel = baseLevel;
    _speedFactor = speedFactor;
    _tempFactor = tempFactor;
    _tempOffset = tempOffset;
    _startTemp = startTemp;
    _rainBoost = rainBoost;
    _startupBoostLevel = startupBoostLevel;
    _startupBoostSec = startupBoostSec;
    _startDelaySec = startDelaySec;
    _reactionSpeed = (ReactionSpeed)reactionSpeed;
    
    _prefs.begin("aux", false);
    _prefs.putInt("base", _baseLevel);
    _prefs.putFloat("speedF", _speedFactor);
    _prefs.putFloat("tempF", _tempFactor);
    _prefs.putFloat("tempO", _tempOffset);
    _prefs.putFloat("startT", _startTemp);
    _prefs.putInt("rainB", _rainBoost);
    _prefs.putInt("startL", _startupBoostLevel);
    _prefs.putInt("startS", _startupBoostSec);
    _prefs.putInt("startD", _startDelaySec);
    _prefs.putInt("react", (int)_reactionSpeed);
    _prefs.end();
}

void AuxManager::getGripSettings(int &baseLevel, float &speedFactor, float &tempFactor, float &tempOffset, float &startTemp, int &rainBoost, int &startupBoostLevel, int &startupBoostSec, int &startDelaySec, int &reactionSpeed) {
    baseLevel = _baseLevel;
    speedFactor = _speedFactor;
    tempFactor = _tempFactor;
    tempOffset = _tempOffset;
    startTemp = _startTemp;
    rainBoost = _rainBoost;
    startupBoostLevel = _startupBoostLevel;
    startupBoostSec = _startupBoostSec;
    startDelaySec = _startDelaySec;
    reactionSpeed = (int)_reactionSpeed;
}

void AuxManager::calcBoostEndTime() {
    unsigned long now = millis();
    unsigned long delayMs = (unsigned long)_startDelaySec * 1000UL;
    unsigned long boostDurMs = (unsigned long)_startupBoostSec * 1000UL;

    // If we are still in start delay, boost starts after delay.
    // If we are past delay, boost starts now.
    unsigned long boostStartTime = (now > delayMs) ? now : delayMs;
    
    _boostEndTime = boostStartTime + boostDurMs;
}
