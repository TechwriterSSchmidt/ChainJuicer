#ifndef WEB_CONSOLE_H
#define WEB_CONSOLE_H

#include <Arduino.h>
#include <vector>

class WebConsole {
public:
    void begin() {
        logs.reserve(MAX_LINES);
    }

    void log(String msg) {
        if (logs.size() >= MAX_LINES) {
            logs.erase(logs.begin());
        }
        // Simple timestamp
        unsigned long now = millis();
        String timestamp = String(now / 1000) + "." + String((now % 1000) / 100); // 123.4s
        logs.push_back("[" + timestamp + "] " + msg);
    }
    
    // Helper for printf style
    void logf(const char* format, ...) {
        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        log(String(buffer));
    }

    String getLogs() {
        String out;
        for (const auto& line : logs) {
            out += line + "\n";
        }
        return out;
    }
    
    void clear() {
        logs.clear();
    }

private:
    const size_t MAX_LINES = 50;
    std::vector<String> logs;
};

extern WebConsole webConsole;

#endif
