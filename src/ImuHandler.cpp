#include "ImuHandler.h"
#include "WebConsole.h"

ImuHandler::ImuHandler() {
    _lastMotionTime = 0;
    // Initialize history
    for(int i=0; i<HISTORY_SIZE; i++) {
        _rollHistory[i] = 0;
        _pitchHistory[i] = 0;
    }
}

bool ImuHandler::begin(int sda, int scl) {
    // 1. Standard Startversuch mit Timeout
    Wire.begin(sda, scl);
    Wire.setTimeOut(10); // 10ms Timeout verhindert Hängenbleiben
    
    if (!_bno.begin_I2C()) {
        Serial.println("IMU: Not found. Attempting Bus Recovery...");
        
        // 2. Bus Recovery Sequenz
        // Wir beenden I2C und wackeln manuell am Clock-Pin
        Wire.end();
        pinMode(sda, INPUT_PULLUP);
        pinMode(scl, OUTPUT);
        
        // 16 Clock-Pulse senden, um verwirrte Slaves zu resetten
        for(int i=0; i<16; i++) {
            digitalWrite(scl, LOW);
            delayMicroseconds(10);
            digitalWrite(scl, HIGH);
            delayMicroseconds(10);
        }
        
        pinMode(sda, INPUT);
        pinMode(scl, INPUT);
        
        // 3. Neustart I2C
        Wire.begin(sda, scl);
        Wire.setTimeOut(10);
        delay(50);

        // 4. Zweiter Versuch
        if (!_bno.begin_I2C()) {
            Serial.println("IMU: Recovery failed. Disabling IMU features.");
            _available = false;
            return false;
        }
        Serial.println("IMU: Recovered successfully!");
    }

    Serial.println("IMU: BNO08x Found!");
    
    // Enable Reports
    // Rotation Vector for Orientation (50ms interval = 20Hz)
    if (!_bno.enableReport(SH2_ARVR_STABILIZED_RV, 50000)) {
        Serial.println("IMU: Could not enable Rotation Vector");
    }
    
    // Linear Acceleration for Motion Detection
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
        _bno.enableReport(SH2_ARVR_STABILIZED_RV, 50000);
        _bno.enableReport(SH2_LINEAR_ACCELERATION, 20000);
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
                if ((_linAccelX*_linAccelX + _linAccelY*_linAccelY + _linAccelZ*_linAccelZ) > (0.5 * 0.5)) { 
                    _lastMotionTime = millis();
                }
                break;
        }
    }
}

void ImuHandler::loop() {
    update();

    if (_calState == CAL_WAIT) {
        unsigned long elapsed = millis() - _calTimer;
        int sec = elapsed / 1000;
        
        if (sec > _calLastSec) {
            int remaining = 5 - sec;
            if (remaining > 0) {
                webConsole.log("IMU: " + String(remaining) + "...");
            }
            _calLastSec = sec;
        }
        
        if (elapsed >= 5000) {
            _calState = CAL_MEASURE;
            _calTimer = millis();
            _calSumRoll = 0;
            _calSumPitch = 0;
            _calSamples = 0;
            webConsole.log("IMU: Measuring... Hold still!");
        }
    } else if (_calState == CAL_MEASURE) {
        // Accumulate
        float currentRawRoll = _roll + _offsetRoll;
        float currentRawPitch = _pitch + _offsetPitch;
        
        _calSumRoll += currentRawRoll;
        _calSumPitch += currentRawPitch;
        _calSamples++;
        
        if (millis() - _calTimer >= 3000) {
            if (_calSamples > 0) {
                _offsetRoll = _calSumRoll / _calSamples;
                _offsetPitch = _calSumPitch / _calSamples;
                saveCalibration();
                webConsole.log("IMU: Calibration DONE.");
                webConsole.logf("IMU: Offsets: R=%.2f P=%.2f", _offsetRoll, _offsetPitch);
            } else {
                webConsole.log("IMU: Calibration FAILED (No samples).");
            }
            _calState = CAL_IDLE;
        }
    }
}

void ImuHandler::startCalibration() {
    if (!_available) {
        webConsole.log("IMU: Sensor not available!");
        return;
    }
    _calState = CAL_WAIT;
    _calTimer = millis();
    _calLastSec = 0;
    webConsole.log("IMU: Calibration requested.");
    webConsole.log("IMU: Get ready! 5 seconds...");
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
    _yaw = rawYaw; 
    
    updateHistory(_roll, _pitch);
}

void ImuHandler::updateHistory(float roll, float pitch) {
    _rollHistory[_historyIndex] = roll;
    _pitchHistory[_historyIndex] = pitch;
    
    _historyIndex++;
    if (_historyIndex >= HISTORY_SIZE) {
        _historyIndex = 0;
        _historyFilled = true;
    }
}

float ImuHandler::calculateVariance(float* data, int size) {
    if (size <= 0) return 0.0;
    
    float mean = 0.0;
    for(int i=0; i<size; i++) {
        mean += data[i];
    }
    mean /= size;
    
    float variance = 0.0;
    for(int i=0; i<size; i++) {
        variance += (data[i] - mean) * (data[i] - mean);
    }
    return variance / size;
}

void ImuHandler::calibrateZero() {
    if (!_available) return;

    Serial.println("IMU: Waiting 5s for rider to stabilize...");
    unsigned long waitStart = millis();
    while(millis() - waitStart < 5000) {
        update();
        delay(10);
    }
    
    Serial.println("IMU: Starting calibration (3s)...");
    
    double sumRawRoll = 0;
    double sumRawPitch = 0;
    int samples = 0;
    unsigned long start = millis();
    const unsigned long CALIBRATION_DURATION = 3000; // 3 seconds averaging

    while (millis() - start < CALIBRATION_DURATION) {
        update(); // Fetch latest data from sensor
        
        // Reconstruct RAW values (Sensor Raw = Current Displayed + Current Offset)
        // We need the raw sensor values to calculate the new absolute offset.
        float currentRawRoll = _roll + _offsetRoll;
        float currentRawPitch = _pitch + _offsetPitch;
        
        sumRawRoll += currentRawRoll;
        sumRawPitch += currentRawPitch;
        samples++;
        
        delay(10); // Sample at ~100Hz
    }

    if (samples > 0) {
        _offsetRoll = sumRawRoll / samples;
        _offsetPitch = sumRawPitch / samples;
        
        saveCalibration();
        Serial.printf("IMU: Zero Calibrated. Samples: %d. Offsets: R=%.2f P=%.2f\n", samples, _offsetRoll, _offsetPitch);
    } else {
        Serial.println("IMU: Calibration failed - no samples");
    }
}

void ImuHandler::saveCalibration() {
    _prefs.begin("imu", false);
    _prefs.putFloat("off_r", _offsetRoll);
    _prefs.putFloat("off_p", _offsetPitch);
    _prefs.putBool("chain_r", _chainOnRight);
    _prefs.end();
}

void ImuHandler::loadCalibration() {
    _prefs.begin("imu", true);
    _offsetRoll = _prefs.getFloat("off_r", 0.0);
    _offsetPitch = _prefs.getFloat("off_p", 0.0);
    _chainOnRight = _prefs.getBool("chain_r", true);
    _prefs.end();
}

void ImuHandler::setChainSide(bool isRight) {
    if (_chainOnRight != isRight) {
        _chainOnRight = isRight;
        saveCalibration();
    }
}

bool ImuHandler::isStationary() {
    if (!_available) return false;
    if (!_historyFilled) return false; // Not enough data yet
    
    // Calculate variance for Roll and Pitch
    float varRoll = calculateVariance(_rollHistory, HISTORY_SIZE);
    float varPitch = calculateVariance(_pitchHistory, HISTORY_SIZE);
    
    // Threshold: 0.5 degree variance implies very stable
    return (varRoll < 0.5 && varPitch < 0.5);
}

bool ImuHandler::isCrashed() {
    if (!_available) return false;
    // Crash Logic: Lean > 70 degrees
    if (abs(_roll) > 70.0) return true;
    if (abs(_pitch) > 70.0) return true; 
    return false;
}

bool ImuHandler::isMotionDetected() {
    if (!_available) return true; 
    if (millis() - _lastMotionTime < 5000) return true;
    return false; 
}

bool ImuHandler::isLeaningTowardsTire(float thresholdDeg) {
    if (!_available) return false;
    
    // Default Left = Negative Roll
    bool isLeaningLeft = (_roll < -thresholdDeg);

    if (_chainOnRight) {
        // Chain Right -> Tire Left
        // Unsafe if leaning LEFT
        return isLeaningLeft;
    } else {
        // Chain Left -> Tire Right
        // Unsafe if leaning RIGHT (which is !Left)
        return !isLeaningLeft;
    }
}
