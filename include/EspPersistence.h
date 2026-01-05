#ifndef ESP_PERSISTENCE_H
#define ESP_PERSISTENCE_H

#include "Persistence.h"
#include <Preferences.h>

class EspPersistence : public IPersistence {
public:
    EspPersistence() {}
    
    void begin(const char* namespaceName, bool readOnly) override {
        _prefs.begin(namespaceName, readOnly);
    }
    
    void end() override {
        _prefs.end();
    }

    void clear() override {
        _prefs.clear();
    }

    void putInt(const char* key, int32_t value) override {
        _prefs.putInt(key, value);
    }
    
    int32_t getInt(const char* key, int32_t defaultValue) override {
        return _prefs.getInt(key, defaultValue);
    }

    void putUInt(const char* key, uint32_t value) override {
        _prefs.putUInt(key, value);
    }
    
    uint32_t getUInt(const char* key, uint32_t defaultValue) override {
        return _prefs.getUInt(key, defaultValue);
    }

    void putFloat(const char* key, float value) override {
        _prefs.putFloat(key, value);
    }
    
    float getFloat(const char* key, float defaultValue) override {
        return _prefs.getFloat(key, defaultValue);
    }

    void putDouble(const char* key, double value) override {
        _prefs.putDouble(key, value);
    }
    
    double getDouble(const char* key, double defaultValue) override {
        return _prefs.getDouble(key, defaultValue);
    }

    void putBool(const char* key, bool value) override {
        _prefs.putBool(key, value);
    }
    
    bool getBool(const char* key, bool defaultValue) override {
        return _prefs.getBool(key, defaultValue);
    }

    void putUChar(const char* key, uint8_t value) override {
        _prefs.putUChar(key, value);
    }
    
    uint8_t getUChar(const char* key, uint8_t defaultValue) override {
        return _prefs.getUChar(key, defaultValue);
    }

    void putBytes(const char* key, const void* value, size_t len) override {
        _prefs.putBytes(key, value, len);
    }
    
    size_t getBytes(const char* key, void* buf, size_t maxLen) override {
        return _prefs.getBytes(key, buf, maxLen);
    }
    
    size_t getBytesLength(const char* key) override {
        return _prefs.getBytesLength(key);
    }

private:
    Preferences _prefs;
};

#endif
