#include "ImuHandler.h"

ImuHandler::ImuHandler() {
    _lastMotionTime = 0;
}

bool ImuHandler::begin(int sda, int scl) {
    // Initialize I2C
    // Wire.begin(sda, scl); // Assumed Wire is already started or we start it here
    // Better to let main handle Wire.begin if shared, but here we can do it safely.
    // Check if Wire is already active? 
    // For ESP32, we can call Wire.begin(sda, scl) multiple times, it just reconfigures.
    
    if (!_bno.begin_I2C()) {
        Serial.println("IMU: BNO08x not detected. Disabling IMU features.");
        _available = false;
        return false;
    }

    Serial.println("IMU: BNO08x Found!");
    
    // Enable Reports
    // Rotation Vector for Orientation (50ms interval)
    if (!_bno.enableReport(SH2_ARVR_STABILIZED_RV, 50000)) {
        Serial.println("IMU: Could not enable Rotation Vector");
    }
    
    // Linear Acceleration for Motion Detection
    // 800 RPM (Idle) = ~13Hz. We need >26Hz sampling to detect it reliably.
    // Setting to 20ms (50Hz) to capture engine vibrations.
    if (!_bno.enableReport(SH2_LINEAR_ACCELERATION, 20000)) {
        Serial.println("IMU: Could not enable Linear Accel");
    }

    loadCalibration();
    _available = true;
    return true;
}

void ImuHandler::update() {
    if (!_available) return;

    if (_bno.wasReset()) {
        Serial.println("IMU: Sensor was reset");
        // Re-enable reports?
        _bno.enableReport(SH2_ARVR_STABILIZED_RV, 50000);
        _bno.enableReport(SH2_LINEAR_ACCELERATION, 100000);
    }

    if (_bno.getSensorEvent(&_sensorValue)) {
        switch (_sensorValue.sensorId) {
            case SH2_ARVR_STABILIZED_RV:
                processOrientation();
                break;
            case SH2_LINEAR_ACCELERATION:
                _linAccelX = _sensorValue.un.linearAcceleration.x;
                _linAccelY = _sensorValue.un.linearAcceleration.y;
                _linAccelZ = _sensorValue.un.linearAcceleration.z;
                
                // Simple motion check: Magnitude > threshold
                // Gravity is removed in Linear Acceleration
                // 0.5 m/s^2 is a reasonable threshold for engine vibration or movement
                if ((_linAccelX*_linAccelX + _linAccelY*_linAccelY + _linAccelZ*_linAccelZ) > (0.5 * 0.5)) { 
                    _lastMotionTime = millis();
                }
                break;
        }
    }
}

void ImuHandler::loop() {
    update();
}

void ImuHandler::processOrientation() {
    // Convert Quaternion to Euler
    float qw = _sensorValue.un.arvrStabilizedRV.real;
    float qx = _sensorValue.un.arvrStabilizedRV.i;
    float qy = _sensorValue.un.arvrStabilizedRV.j;
    float qz = _sensorValue.un.arvrStabilizedRV.k;

    // Roll (x-axis rotation)
    float sinr_cosp = 2 * (qw * qx + qy * qz);
    float cosr_cosp = 1 - 2 * (qx * qx + qy * qy);
    float rawRoll = atan2(sinr_cosp, cosr_cosp);

    // Pitch (y-axis rotation)
    float sinp = 2 * (qw * qy - qz * qx);
    float rawPitch;
    if (abs(sinp) >= 1)
        rawPitch = copysign(M_PI / 2, sinp); // use 90 degrees if out of range
    else
        rawPitch = asin(sinp);

    // Yaw (z-axis rotation)
    float siny_cosp = 2 * (qw * qz + qx * qy);
    float cosy_cosp = 1 - 2 * (qy * qy + qz * qz);
    float rawYaw = atan2(siny_cosp, cosy_cosp);

    // Convert to Degrees
    rawRoll = rawRoll * 180.0 / M_PI;
    rawPitch = rawPitch * 180.0 / M_PI;
    rawYaw = rawYaw * 180.0 / M_PI;

    // Apply Calibration Offsets
    _roll = rawRoll - _offsetRoll;
    _pitch = rawPitch - _offsetPitch;
    _yaw = rawYaw; // Yaw is relative to magnetic north usually, or start. We don't offset it for now.
}

void ImuHandler::calibrateZero() {
    if (!_available) return;
    // Assume current position is "Zero" (Upright)
    // We need the RAW values, so we temporarily revert the offset
    float currentRawRoll = _roll + _offsetRoll;
    float currentRawPitch = _pitch + _offsetPitch;

    _offsetRoll = currentRawRoll;
    _offsetPitch = currentRawPitch;
    
    saveCalibration();
    Serial.println("IMU: Zero Position Calibrated");
}

void ImuHandler::calibrateSideStand() {
    if (!_available) return;
    // Store the current angle as the "Side Stand" reference
    _sideStandRoll = _roll; 
    _sideStandCalibrated = true;
    saveCalibration();
    Serial.println("IMU: Side Stand Position Calibrated");
}

void ImuHandler::saveCalibration() {
    _prefs.begin("imu", false);
    _prefs.putFloat("off_r", _offsetRoll);
    _prefs.putFloat("off_p", _offsetPitch);
    _prefs.putFloat("side_r", _sideStandRoll);
    _prefs.putBool("side_cal", _sideStandCalibrated);
    _prefs.putBool("chain_r", _chainOnRight);
    _prefs.end();
}

void ImuHandler::loadCalibration() {
    _prefs.begin("imu", true);
    _offsetRoll = _prefs.getFloat("off_r", 0.0);
    _offsetPitch = _prefs.getFloat("off_p", 0.0);
    _sideStandRoll = _prefs.getFloat("side_r", 0.0);
    _sideStandCalibrated = _prefs.getBool("side_cal", false);
    _chainOnRight = _prefs.getBool("chain_r", true);
    _prefs.end();
}

void ImuHandler::setChainSide(bool isRight) {
    if (_chainOnRight != isRight) {
        _chainOnRight = isRight;
        saveCalibration();
    }
}

bool ImuHandler::isParked() {
    if (!_available) return false;
    
    // Garage Guard Logic
    // 1. Side Stand: Lean > 10 degrees (absolute)
    if (abs(_roll) > 10.0) return true;

    // 2. Side Stand (Calibrated): If we are close to the calibrated side stand position
    if (_sideStandCalibrated) {
        if (abs(_roll - _sideStandRoll) < 5.0) return true;
    }

    return false;
}

bool ImuHandler::isCrashed() {
    if (!_available) return false;
    // Crash Logic: Lean > 70 degrees
    if (abs(_roll) > 70.0) return true;
    if (abs(_pitch) > 70.0) return true; // Wheelie fail or nose dive
    return false;
}

bool ImuHandler::isMotionDetected() {
    if (!_available) return true; // Default to true if no IMU, so we rely on GPS
    
    // Check if we had motion in the last 5 seconds
    // This covers engine vibration (continuous updates) or stop-and-go
    if (millis() - _lastMotionTime < 5000) return true;
    
    return false; 
}

bool ImuHandler::isLeaningTowardsTire(float thresholdDeg) {
    if (!_available) return false;
    
    // Determine Left direction based on Side Stand Calibration
    // Side Stand is almost always on the Left.
    // If Side Stand Roll is Negative (e.g. -12), then Left is Negative.
    // If Side Stand Roll is Positive (e.g. +12), then Left is Positive.
    // Default (if not calibrated or 0): Assume Standard (Left = Negative).
    
    float leftSign = -1.0; // Default Left = Negative
    if (abs(_sideStandRoll) > 1.0) {
        leftSign = (_sideStandRoll > 0) ? 1.0 : -1.0;
    }
    
    bool isLeaningLeft = false;
    if (leftSign < 0) {
        // Left is Negative
        isLeaningLeft = (_roll < -thresholdDeg);
    } else {
        // Left is Positive
        isLeaningLeft = (_roll > thresholdDeg);
    }

    // Logic: We want to avoid oiling if we lean towards the TIRE.
    // Tire is always opposite to the Chain.
    
    if (_chainOnRight) {
        // Chain Right -> Tire Left
        // Unsafe if leaning LEFT
        return isLeaningLeft;
    } else {
        // Chain Left -> Tire Right
        // Unsafe if leaning RIGHT (which is !Left)
        
        if (leftSign < 0) {
            // Left is Negative, so Right is Positive
            return (_roll > thresholdDeg);
        } else {
            // Left is Positive, so Right is Negative
            return (_roll < -thresholdDeg);
        }
    }
}
