#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <TinyGPS++.h>
#include <esp_task_wdt.h>
#include "config.h"
#include "Oiler.h"

// Watchdog Timeout in seconds
#define WDT_TIMEOUT 8

// Global Objects
TinyGPSPlus gps;
HardwareSerial gpsSerial(2); // UART2
WebServer server(80);
DNSServer dnsServer;
Oiler oiler;

// WiFi Timer Variables
unsigned long wifiStartTime = 0;
bool wifiActive = false; // Default OFF
const unsigned long WIFI_TIMEOUT = WIFI_TIMEOUT_MS;

// HTML Page for Configuration
#include "html_pages.h"

bool isSummerTime(int year, int month, int day, int hour) {
    if (month < 3 || month > 10) return false; 
    if (month > 3 && month < 10) return true; 

    if (month == 3) {
        int lastSunday = 31 - ((5 * year / 4 + 4) % 7);
        return day > lastSunday || (day == lastSunday && hour >= 1); // Switch at 1:00 UTC
    }
    if (month == 10) {
        int lastSunday = 31 - ((5 * year / 4 + 1) % 7);
        return day < lastSunday || (day == lastSunday && hour < 1); // Switch at 1:00 UTC
    }
    return false;
}

String getZurichTime() {
    if (!gps.time.isValid() || !gps.date.isValid()) return "--:--";
    
    int year = gps.date.year();
    int month = gps.date.month();
    int day = gps.date.day();
    int hour = gps.time.hour();
    int minute = gps.time.minute();
    
    // Timezone Logic for Zurich (CET/CEST)
    int offset = isSummerTime(year, month, day, hour) ? 2 : 1;
    
    hour += offset; 
    if (hour >= 24) hour -= 24;
    
    char buf[32];
    sprintf(buf, "%02d:%02d (UTC+%d)", hour, minute, offset);
    return String(buf);
}

void resetWifiTimer() {
    wifiStartTime = millis();
}

void handleHelp() {
    resetWifiTimer();
    server.send(200, "text/html", htmlHelp);
}

void handleResetStats() {
#ifdef GPS_DEBUG
    Serial.println("CMD: Reset Stats");
#endif
    oiler.resetStats();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleResetTimeStats() {
#ifdef GPS_DEBUG
    Serial.println("CMD: Reset Time Stats");
#endif
    oiler.resetTimeStats();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleRefill() {
#ifdef GPS_DEBUG
    Serial.println("CMD: Refill Tank");
#endif
    oiler.resetTankToFull();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleRoot() {
    resetWifiTimer();
#ifdef GPS_DEBUG
    Serial.println("Serving Root Page"); 
#endif
    String html = htmlHeader;
    html.replace("%TIME%", getZurichTime());
    html.replace("%SATS%", String(gps.satellites.value()));

    double totalRecentTime = oiler.getRecentTotalTime();

    for(int i=0; i<NUM_RANGES; i++) {
        SpeedRange* r = oiler.getRangeConfig(i);
        
        // Calculate percentage
        float pct = 0.0;
        if (totalRecentTime > 0) {
             pct = (oiler.getRecentTimeSeconds(i) / totalRecentTime) * 100.0;
        }

        html += "<tr><td>";
        html += String((int)r->minSpeed) + "-" + String((int)r->maxSpeed) + " km/h";
        html += "</td><td><input type='number' step='0.1' name='km" + String(i) + "' value='" + String(r->intervalKm) + "'>";
        html += "</td><td style='text-align:center;color:#555'>" + String(pct, 1) + "%";
        html += "</td><td style='text-align:center;color:#555'>" + String(oiler.getRecentOilingCount(i));
        html += "</td><td><input type='number' name='p" + String(i) + "' value='" + String(r->pulses) + "'></td></tr>";
    }
    
    // Add Reset Link below the table
    html += "</table><div style='text-align:left;margin-top:10px;margin-bottom:10px'><a href='/reset_time_stats' style='color:red;text-decoration:none;font-size:0.9em'>[Reset Stats]</a></div>";
    
    String footer = htmlFooter;
    
    // Remove "</table>" from the start of htmlFooter since we already closed the table above
    if (footer.startsWith("</table>")) {
        footer = footer.substring(8); // Remove </table>
    }
    
    footer.replace("%PROGRESS%", String(oiler.getCurrentProgress() * 100.0, 1));
    
    // Convert 0-255 to 0-100% for Display
    footer.replace("%LED_DIM%", String(map(oiler.ledBrightnessDim, 2, 202, 0, 100)));
    footer.replace("%LED_HIGH%", String(map(oiler.ledBrightnessHigh, 2, 202, 0, 100)));
    
    footer.replace("%RAIN_CHECKED%", oiler.isRainMode() ? "checked" : "");
    footer.replace("%EMERG_CHECKED%", oiler.isEmergencyModeForced() ? "checked" : "");
    
    footer.replace("%NIGHT_CHECKED%", oiler.nightModeEnabled ? "checked" : "");
    footer.replace("%NIGHT_START%", String(oiler.nightStartHour));
    footer.replace("%NIGHT_END%", String(oiler.nightEndHour));
    footer.replace("%NIGHT_BRI%", String(map(oiler.nightBrightness, 2, 202, 0, 100)));
    footer.replace("%NIGHT_BRI_H%", String(map(oiler.nightBrightnessHigh, 2, 202, 0, 100)));
    
    footer.replace("%TANK_CHECKED%", oiler.tankMonitorEnabled ? "checked" : "");
    footer.replace("%TANK_CAP%", String(oiler.tankCapacityMl, 0));
    footer.replace("%DROP_ML%", String(oiler.dropsPerMl));
    footer.replace("%DROP_PLS%", String(oiler.dropsPerPulse));
    footer.replace("%TANK_WARN%", String(oiler.tankWarningThresholdPercent));
    footer.replace("%TANK_LEVEL%", String(oiler.currentTankLevelMl, 1));
    float pct = (oiler.tankCapacityMl > 0) ? (oiler.currentTankLevelMl / oiler.tankCapacityMl) * 100.0 : 0.0;
    footer.replace("%TANK_PCT%", String(pct, 0));

    footer.replace("%TOTAL_DIST%", String(oiler.getTotalDistance(), 1));
    footer.replace("%PUMP_COUNT%", String(oiler.getPumpCycles()));

    html += footer;
    
    server.send(200, "text/html", html);
}

void handleSave() {
    resetWifiTimer();
    for(int i=0; i<NUM_RANGES; i++) {
        SpeedRange* r = oiler.getRangeConfig(i);
        if(server.hasArg("km" + String(i))) r->intervalKm = server.arg("km" + String(i)).toFloat();
        if(server.hasArg("p" + String(i))) r->pulses = server.arg("p" + String(i)).toInt();
    }
    
    // Convert 0-100% back to 0-255
    if(server.hasArg("led_dim")) {
        int val = server.arg("led_dim").toInt();
        if (val < 0) val = 0; if (val > 100) val = 100;
        oiler.ledBrightnessDim = map(val, 0, 100, 2, 202);
    }
    if(server.hasArg("led_high")) {
        int val = server.arg("led_high").toInt();
        if (val < 0) val = 0; if (val > 100) val = 100;
        oiler.ledBrightnessHigh = map(val, 0, 100, 2, 202);
    }
    
    // Rain Mode Checkbox handling
    oiler.setRainMode(server.hasArg("rain_mode"));
    oiler.setEmergencyModeForced(server.hasArg("emerg_mode"));
    
    oiler.nightModeEnabled = server.hasArg("night_en");
    if(server.hasArg("night_start")) oiler.nightStartHour = server.arg("night_start").toInt();
    if(server.hasArg("night_end")) oiler.nightEndHour = server.arg("night_end").toInt();
    if(server.hasArg("night_bri")) {
        int val = server.arg("night_bri").toInt();
        if (val < 0) val = 0; if (val > 100) val = 100;
        oiler.nightBrightness = map(val, 0, 100, 2, 202);
    }
    if(server.hasArg("night_bri_h")) {
        int val = server.arg("night_bri_h").toInt();
        if (val < 0) val = 0; if (val > 100) val = 100;
        oiler.nightBrightnessHigh = map(val, 0, 100, 2, 202);
    }

    oiler.tankMonitorEnabled = server.hasArg("tank_en");
    if(server.hasArg("tank_cap")) oiler.tankCapacityMl = server.arg("tank_cap").toFloat();
    if(server.hasArg("drop_ml")) oiler.dropsPerMl = server.arg("drop_ml").toInt();
    if(server.hasArg("drop_pls")) oiler.dropsPerPulse = server.arg("drop_pls").toInt();
    if(server.hasArg("tank_warn")) oiler.tankWarningThresholdPercent = server.arg("tank_warn").toInt();

    oiler.saveConfig();
    server.sendHeader("Location", "/");
    server.send(303);
}

void setup() {
    // Safety: Ensure Pump is OFF immediately
    // Set level LOW before switching to OUTPUT to prevent glitches
    digitalWrite(PUMP_PIN, PUMP_OFF);
    pinMode(PUMP_PIN, OUTPUT);

    Serial.begin(115200);
    
    // Initialize Watchdog
    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL);

    // Permanently disable Bluetooth
    btStop();

    // GPS Start
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    
    // Oiler Start
    oiler.begin();

    // WiFi Start Logic: Default OFF
    // Start DNS Server only when needed

    // Webserver Routes
    server.on("/", handleRoot);
    server.on("/save", handleSave);
    server.on("/help", handleHelp);
    server.on("/reset_stats", handleResetStats);
    server.on("/reset_time_stats", handleResetTimeStats);
    server.on("/refill", handleRefill);
    
    // Captive Portal / Connectivity Checks
    server.on("/generate_204", handleRoot);  // Android
    server.on("/hotspot-detect.html", handleRoot); // Apple
    server.on("/library/test/success.html", handleRoot); // Apple
    server.on("/ncsi.txt", handleRoot); // Windows
    server.on("/connecttest.txt", handleRoot); // Microsoft
    server.on("/favicon.ico", [](){ server.send(404, "text/plain", ""); }); 
    
    server.onNotFound([](){
        // Filter out common noise to keep serial clean
        String uri = server.uri();
        if (uri.indexOf("googleapis") != -1 || uri.indexOf("gstatic") != -1) {
            server.send(404); // Silent 404
        } else {
#ifdef GPS_DEBUG
            Serial.printf("404 Not Found: %s\n", uri.c_str());
#endif
            handleRoot(); // Redirect others to root
        }
    });

    wifiStartTime = millis();
}

void loop() {
    // Reset Watchdog
    esp_task_wdt_reset();

    // Read GPS Data
    while (gpsSerial.available() > 0) {
        char c = gpsSerial.read();
        gps.encode(c);
        // Optional: Uncomment to see raw data if needed
        // Serial.write(c); 
    }

    float currentSpeed = gps.speed.isValid() ? gps.speed.kmph() : 0.0;

    // GPS Filter: Ignore data if signal is poor (Multipath/Indoor protection)
    // 1. Minimum 6 Satellites (Outdoors usually > 8)
    // 2. HDOP must be good (< 5.0).
    bool signalPoor = false;
    if (gps.location.isValid()) {
        if (gps.satellites.value() < 5 || gps.hdop.hdop() > 5.0) {
            signalPoor = true;
            currentSpeed = 0.0; // Force 0 speed
        }
    }

#ifdef GPS_DEBUG
    // GPS Debug Output
    static unsigned long lastGpsDebug = 0;
    if (millis() - lastGpsDebug > 2000) {
        lastGpsDebug = millis();
        Serial.printf("GPS Status: Fix=%s, Sats=%d, Speed=%.1f km/h, Lat=%.6f, Lon=%.6f, HDOP=%.1f %s\n", 
            gps.location.isValid() ? "OK" : "NO", 
            gps.satellites.value(), 
            currentSpeed, // Show filtered speed
            gps.location.isValid() ? gps.location.lat() : 0.0,
            gps.location.isValid() ? gps.location.lng() : 0.0,
            gps.hdop.hdop(),
            signalPoor ? "[FILTERED]" : ""
        );
    }
#endif

    // Update Oiler Logic
    // Pass current time to Oiler (for Night Mode)
    if (gps.time.isValid() && gps.date.isValid()) {
        int offset = isSummerTime(gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour()) ? 2 : 1;
        int h = gps.time.hour() + offset;
        if (h >= 24) h -= 24;
        oiler.setCurrentHour(h);
    }
    
    // Update Oiler with GPS data
    if (gps.location.isUpdated() || gps.speed.isUpdated()) {
        oiler.update(currentSpeed, gps.location.lat(), gps.location.lng(), gps.location.isValid());
    }
    
    // Run Oiler main loop (Button, LED, Bleeding)
    oiler.loop();

    // --- WiFi Management ---
    unsigned long currentMillis = millis();

    // 1. Activation: Standstill + Button pressed
    static unsigned long wifiButtonPressStart = 0;
    static bool wifiButtonHeld = false;

    if (oiler.isButtonPressed()) {
        if (currentSpeed < MIN_SPEED_KMH) { // threshold to handle GPS drift
            if (!wifiButtonHeld) {
                wifiButtonPressStart = currentMillis;
                wifiButtonHeld = true;
            } else {
                // Check duration
                if (currentMillis - wifiButtonPressStart > WIFI_PRESS_MS) {
                    // Button held
                    wifiStartTime = currentMillis; // Reset timeout timer
                    
                    if (!wifiActive) {
                        WiFi.softAP(AP_SSID);
                        IPAddress IP = WiFi.softAPIP();
#ifdef GPS_DEBUG
                        Serial.print("WiFi activated via Button. IP: ");
                        Serial.println(IP);
#endif
                        dnsServer.start(53, "*", IP);
                        server.begin();
                        wifiActive = true;
                    }
                }
            }
        }
    } else {
        wifiButtonHeld = false;
    }

    // 2. Deactivation: Driving or Timeout
    if (wifiActive) {
        bool shouldStop = false;
        
        // Timeout Check
        if (currentMillis - wifiStartTime > WIFI_TIMEOUT) {
#ifdef GPS_DEBUG
            Serial.println("WiFi Timeout.");
#endif
            shouldStop = true;
        }
        
        // Speed Check (Auto-Off when driving)
        if (currentSpeed > MIN_SPEED_KMH) {
#ifdef GPS_DEBUG
            Serial.println("Driving detected -> WiFi off.");
#endif
            shouldStop = true;
        }

        if (shouldStop) {
            WiFi.softAPdisconnect(true);
            wifiActive = false;
        } else {
            // Handle Network Services
            dnsServer.processNextRequest();
            server.handleClient();
        }
    }
    
    // Pass WiFi status to Oiler (for LED indication)
    oiler.setWifiActive(wifiActive);

    delay(10);
}
