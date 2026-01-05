#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <Arduino.h>

/**
 * Abstract Interface for Data Persistence.
 * Allows the Core Logic to save/load data without knowing the underlying hardware implementation
 * (e.g. Preferences on ESP32 vs. LittleFS/InternalFS on nRF52).
 */
class IPersistence {
public:
    virtual ~IPersistence() {}

    // Lifecycle
    virtual void begin(const char* namespaceName, bool readOnly) = 0;
    virtual void end() = 0;
    
    // Clear
    virtual void clear() = 0;
    
    // Integer
    virtual void putInt(const char* key, int32_t value) = 0;
    virtual int32_t getInt(const char* key, int32_t defaultValue) = 0;
    
    // Unsigned Integer (often mapped to Int internally if platform lacks native support)
    virtual void putUInt(const char* key, uint32_t value) = 0;
    virtual uint32_t getUInt(const char* key, uint32_t defaultValue) = 0;

    // Float
    virtual void putFloat(const char* key, float value) = 0;
    virtual float getFloat(const char* key, float defaultValue) = 0;
    
    // Double (Note: Some platforms might truncate to float)
    virtual void putDouble(const char* key, double value) = 0;
    virtual double getDouble(const char* key, double defaultValue) = 0;

    // Boolean
    virtual void putBool(const char* key, bool value) = 0;
    virtual bool getBool(const char* key, bool defaultValue) = 0;
    
    // Char / Byte
    virtual void putUChar(const char* key, uint8_t value) = 0;
    virtual uint8_t getUChar(const char* key, uint8_t defaultValue) = 0;

    // Binary Blob / Structs
    virtual void putBytes(const char* key, const void* value, size_t len) = 0;
    virtual size_t getBytes(const char* key, void* buf, size_t maxLen) = 0;
    virtual size_t getBytesLength(const char* key) = 0;
};

#endif
