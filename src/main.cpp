#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <TinyGPS++.h>
#include <esp_task_wdt.h>
#include <Update.h>
#include <Preferences.h>
#include "config.h"
#include "Oiler.h"
#include "html_pages.h"

#ifdef SD_LOGGING_ACTIVE
    #include "FS.h"
    #include "SD.h"
    #include "SPI.h"
#endif

// Watchdog Timeout in seconds
#define WDT_TIMEOUT 8

// Global Objects
TinyGPSPlus gps;
HardwareSerial gpsSerial(2); // UART2
WebServer server(80);
DNSServer dnsServer;
Oiler oiler;

#ifdef SD_LOGGING_ACTIVE
    File logFile;
    String currentLogFileName = "";
    unsigned long lastLogTime = 0;
    bool sdInitialized = false;
#endif

// WiFi Timer Variables
unsigned long wifiStartTime = 0;
bool wifiActive = false; // Default OFF
const unsigned long WIFI_TIMEOUT = WIFI_TIMEOUT_MS;

String getZurichTime() {
    if (!gps.time.isValid() || !gps.date.isValid()) return "--:--";
    
    int year = gps.date.year();
    int month = gps.date.month();
    int day = gps.date.day();
    int hour = gps.time.hour();
    int minute = gps.time.minute();
    
    // Use Oiler's centralized logic
    int localHour = oiler.calculateLocalHour(hour, day, month, year);
    
    // Calculate offset for display string (UTC+1 or +2)
    int diff = localHour - hour;
    if (diff < 0) diff += 24;
    
    char buf[32];
    sprintf(buf, "%02d:%02d (UTC+%d)", localHour, minute, diff);
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

void handleUpdate() {
    resetWifiTimer();
    server.send(200, "text/html", htmlUpdate);
}

void handleUpdateResult() {
    resetWifiTimer();
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
}

void handleUpdateProcess() {
    resetWifiTimer();
    oiler.setUpdateMode(true); // Enable LED indication
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        /* flashing firmware to ESP*/
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
            Update.printError(Serial);
        }
    }
}

#ifdef SD_LOGGING_ACTIVE
void initSD() {
    SPI.begin(SD_CLK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("SD Card Mount Failed");
        return;
    }
    
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return;
    }

    // Find next available log file
    int logIndex = 0;
    do {
        logIndex++;
        currentLogFileName = String(LOG_FILE_PREFIX) + String(logIndex) + ".csv";
    } while (SD.exists(currentLogFileName));

    Serial.print("Logging to: ");
    Serial.println(currentLogFileName);

    logFile = SD.open(currentLogFileName, FILE_WRITE);
    if (logFile) {
        // Write Header
        logFile.println("Type,Time_ms,Speed_GPS,Speed_Smooth,Odo_Total,Dist_Accum,Target_Int,Pump_State,Rain_Mode,Temp_C,Sats,HDOP,Message,Turbo_Mode");
        
        // Dump Config
        logFile.println("EVENT,0,,,,,,,,,,CONFIG DUMP START");
        logFile.printf("EVENT,0,,,,,,,,,,Rain Multiplier: %d\n", (oiler.isRainMode() ? 2 : 1));
        for(int i=0; i<5; i++) {
            SpeedRange* r = oiler.getRangeConfig(i);
            if(r) {
                logFile.printf("EVENT,0,,,,,,,,,,Range %d: >%.1f km/h -> %.1f km\n", 
                    i, r->minSpeed, r->intervalKm);
            }
        }
        
        // Log Boot Reason
        esp_reset_reason_t reason = esp_reset_reason();
        logFile.printf("EVENT,%lu,,,,,,,,,,Boot Reason: %d\n", millis(), reason);
        
        logFile.close();
        sdInitialized = true;
    } else {
        Serial.println("Failed to open log file for writing");
    }
}

void writeLogLine(String type, String message = "") {
    if (!sdInitialized) return;

    File f = SD.open(currentLogFileName, FILE_APPEND);
    if (f) {
        f.printf("%s,%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%.1f,%d,%.2f,%s,%d\n",
            type.c_str(),
            millis(),
            gps.speed.kmph(),
            oiler.getSmoothedSpeed(),
            oiler.getOdometer(),
            oiler.getCurrentDistAccumulator(),
            oiler.getCurrentTargetDistance(),
            oiler.isPumpRunning(),
            oiler.isRainMode(),
            oiler.getCurrentTempC(),
            gps.satellites.value(),
            gps.hdop.hdop(),
            message.c_str(),
            oiler.isTurboMode()
        );
        f.close();
    }
}
#endif

void handleRoot() {
    resetWifiTimer();
#ifdef GPS_DEBUG
    Serial.println("Serving Root Page"); 
#endif
    String html = htmlHeader;
    html.replace("%TIME%", getZurichTime());
    html.replace("%SATS%", String(gps.satellites.value()));
    
    String tempHeader = "--";
    if (oiler.isTempSensorConnected()) {
        tempHeader = String(oiler.getCurrentTempC(), 1);
    }
    html.replace("%TEMP%", tempHeader);

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
        html += "</td><td><input type='number' step='0.1' name='km" + String(i) + "' value='" + String(r->intervalKm) + "' class='km-input'>";
        html += "</td><td style='text-align:center;color:#555'>" + String(pct, 1) + "%";
        html += "</td><td style='text-align:center;color:#555'>" + String(oiler.getRecentOilingCount(i));
        html += "</td><td><input type='number' name='p" + String(i) + "' value='" + String(r->pulses) + "' class='pulse-input'></td></tr>";
    }
    
    // Add Reset Link below the table
    html += "</table><div style='text-align:left;margin-top:10px;margin-bottom:10px'><a href='/reset_time_stats' style='color:red;text-decoration:none;font-size:0.9em'>[Reset Stats]</a></div>";
    
    String footer = htmlFooter;
    
    // Remove "</table>" from the start of htmlFooter since we already closed the table above
    if (footer.startsWith("</table>")) {
        footer = footer.substring(8); // Remove </table>
    }
    
    // Temperature Compensation Injection
    bool sensorConnected = oiler.isTempSensorConnected();
    
    footer.replace("%TC_PULSE%", String((int)oiler.tempConfig.basePulse25));
    footer.replace("%TC_PAUSE%", String((int)oiler.tempConfig.basePause25));
    
    footer.replace("%OIL_THIN%", (oiler.tempConfig.oilType == Oiler::OIL_THIN) ? "checked" : "");
    footer.replace("%OIL_NORMAL%", (oiler.tempConfig.oilType == Oiler::OIL_NORMAL) ? "checked" : "");
    footer.replace("%OIL_THICK%", (oiler.tempConfig.oilType == Oiler::OIL_THICK) ? "checked" : "");
    
    footer.replace("%TEMP_C%", String(oiler.getCurrentTempC(), 1));

    footer.replace("%PROGRESS%", String(oiler.getCurrentProgress() * 100.0, 1));
    
    // Convert 0-255 to 0-100% for Display
    footer.replace("%LED_DIM%", String(map(oiler.ledBrightnessDim, 2, 202, 0, 100)));
    footer.replace("%LED_HIGH%", String(map(oiler.ledBrightnessHigh, 2, 202, 0, 100)));
    
    footer.replace("%RAIN_CHECKED%", oiler.isRainMode() ? "checked" : "");
    footer.replace("%RAIN_FLUSH_CHECKED%", oiler.rainFlushEnabled ? "checked" : "");
    footer.replace("%EMERG_CHECKED%", oiler.isEmergencyModeForced() ? "checked" : "");
    footer.replace("%START_DLY%", String(oiler.startupDelayKm, 1));
    footer.replace("%CC_INT%", String(oiler.crossCountryIntervalMin));
    
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
    
    // Save Temperature Compensation (New Simplified Model)
    if(server.hasArg("tc_pulse")) {
        float val = server.arg("tc_pulse").toFloat();
        if (val < 50.0) val = 50.0;
        oiler.tempConfig.basePulse25 = val;
    }
    if(server.hasArg("tc_pause")) oiler.tempConfig.basePause25 = server.arg("tc_pause").toFloat();
    if(server.hasArg("oil_type")) oiler.tempConfig.oilType = (Oiler::OilType)server.arg("oil_type").toInt();

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
    oiler.rainFlushEnabled = server.hasArg("rain_flush");
    oiler.setEmergencyModeForced(server.hasArg("emerg_mode"));
    
    if(server.hasArg("start_dly")) oiler.startupDelayKm = server.arg("start_dly").toFloat();
    if(server.hasArg("cc_int")) oiler.crossCountryIntervalMin = server.arg("cc_int").toInt();
    
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

void handleIMU() {
    resetWifiTimer();
    String html = htmlIMU;
    
    // Placeholders for now (IMU not yet implemented)
    html.replace("%IMU_MODEL%", "Not Connected");
    html.replace("%IMU_STATUS%", "<span style='color:red'>OFF</span>");
    html.replace("%PITCH%", "0.0");
    html.replace("%ROLL%", "0.0");
    
    server.send(200, "text/html", html);
}

void handleIMUZero() {
    resetWifiTimer();
    // Placeholder: Save zero position
    // oiler.imu.calibrateZero(); 
    server.sendHeader("Location", "/imu");
    server.send(303);
}

void handleIMUSide() {
    resetWifiTimer();
    // Placeholder: Save side stand position
    // oiler.imu.calibrateSideStand();
    server.sendHeader("Location", "/imu");
    server.send(303);
}

void setup() {
    Serial.begin(115200);

    // ---------------------------------------------------------
    // FACTORY RESET CHECK (Must be first)
    // ---------------------------------------------------------
    oiler.checkFactoryReset();
    // ---------------------------------------------------------

    // Safety: Ensure Pump is OFF immediately
    // Set level LOW before switching to OUTPUT to prevent glitches
    digitalWrite(PUMP_PIN, PUMP_OFF);
    pinMode(PUMP_PIN, OUTPUT);

    if(!Serial) Serial.begin(115200);
    
    // Initialize Watchdog
    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL);

    // Permanently disable Bluetooth
    btStop();

    // GPS Start
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    
    // Oiler Start
    oiler.begin();

#ifdef SD_LOGGING_ACTIVE
    initSD();
#endif

    // WiFi Start Logic: Default OFF
    // Start DNS Server only when needed

    // Webserver Routes
    server.on("/", handleRoot);
    server.on("/save", handleSave);
    server.on("/imu", handleIMU);
    server.on("/imu_zero", handleIMUZero);
    server.on("/imu_side", handleIMUSide);
    server.on("/help", handleHelp);
    server.on("/reset_stats", handleResetStats);
    server.on("/reset_time_stats", handleResetTimeStats);
    server.on("/refill", handleRefill);
    
    // OTA Update
    server.on("/update", HTTP_GET, handleUpdate);
    server.on("/update", HTTP_POST, handleUpdateResult, handleUpdateProcess);
    
    // Captive Portal / Connectivity Checks
    server.on("/generate_204", handleRoot);  // Android
    server.on("/fwlink", handleRoot);  // Microsoft
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
        int h = oiler.calculateLocalHour(gps.time.hour(), gps.date.day(), gps.date.month(), gps.date.year());
        oiler.setCurrentHour(h);
    }
    
    // Update Oiler with GPS data
    // Ensure update is called at least every 1000ms to handle Emergency Mode (Forced or Auto)
    static unsigned long lastOilerUpdate = 0;
    bool gpsFresh = gps.location.isUpdated() || gps.speed.isUpdated();
    
    if (gpsFresh || (millis() - lastOilerUpdate > 1000)) {
        // If called due to timeout (gpsFresh=false), we pass false as validity
        // This allows the Oiler to detect signal loss and trigger Auto-Emergency Mode
        // Also treat poor signal as invalid to ensure we don't get stuck in "0 km/h" state while driving
        oiler.update(currentSpeed, gps.location.lat(), gps.location.lng(), gpsFresh && !signalPoor);
        lastOilerUpdate = millis();
    }
    
    // Run Oiler main loop (Button, LED, Bleeding)
    oiler.loop();

#ifdef SD_LOGGING_ACTIVE
    if (millis() - lastLogTime > LOG_INTERVAL_MS) {
        writeLogLine("DATA");
        lastLogTime = millis();
    }
#endif

    // --- WiFi Management ---
    unsigned long currentMillis = millis();

    // 1. Activation: Standstill + Button pressed
    static unsigned long wifiButtonPressStart = 0;
    static bool wifiButtonHeld = false;

    if (oiler.isButtonPressed()) {
        // Allow WiFi activation if speed is low OR if we have no GPS fix yet (assuming standstill/setup)
        if (currentSpeed < MIN_SPEED_KMH || !gps.location.isValid()) { 
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
