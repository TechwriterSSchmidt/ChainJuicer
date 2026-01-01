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
#include "AuxManager.h"
#include "html_pages.h"
#include "WebConsole.h"

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
AuxManager auxManager;

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

// Restart / Reset Flags
bool shouldRestart = false;
bool shouldFactoryReset = false;
unsigned long restartTimer = 0;

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

void handleCss() {
    server.sendHeader("Cache-Control", "public, max-age=86400");
    server.send(200, "text/css", htmlCss);
}

void handleHelp() {
    resetWifiTimer();
    String html = htmlHelp;
    html.replace("%VERSION%", FIRMWARE_VERSION);
    server.send(200, "text/html", html);
}

void handleResetStats() {
#ifdef GPS_DEBUG
    Serial.println("CMD: Reset Stats");
#endif
    webConsole.log("CMD: Reset Stats");
    oiler.resetStats();
    server.sendHeader("Location", "/settings");
    server.send(303);
}

void handleResetTimeStats() {
#ifdef GPS_DEBUG
    Serial.println("CMD: Reset Time Stats");
#endif
    webConsole.log("CMD: Reset Time Stats");
    oiler.resetTimeStats();
    server.sendHeader("Location", "/settings");
    server.send(303);
}

void handleRefill() {
#ifdef GPS_DEBUG
    Serial.println("CMD: Refill Tank");
#endif
    webConsole.log("CMD: Refill Tank");
    oiler.resetTankToFull();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleToggleEmerg() {
    resetWifiTimer();
    bool current = oiler.isEmergencyModeForced();
    oiler.setEmergencyModeForced(!current);
    webConsole.log("CMD: Toggle Emergency Mode " + String(!current ? "ON" : "OFF"));
    oiler.saveConfig();
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
        webConsole.logf("Update Start: %s", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
            Update.printError(Serial);
            webConsole.log("Update Begin Error");
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        /* flashing firmware to ESP*/
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            webConsole.logf("Update Success: %u bytes", upload.totalSize);
        } else {
            Update.printError(Serial);
            webConsole.log("Update End Error");
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
        logFile.println("Type,Time_ms,Speed_GPS,Speed_Smooth,Odo_Total,Dist_Accum,Target_Int,Pump_State,Rain_Mode,Temp_C,Sats,HDOP,Message,Flush_Mode");
        
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
            oiler.isFlushMode()
        );
        f.close();
    }
}
#endif

void handleSettings() {
    resetWifiTimer();
#ifdef GPS_DEBUG
    Serial.println("Serving Settings Page"); 
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
        html += "</td><td style='text-align:center;color:#fff'>" + String(pct, 1) + "%";
        html += "</td><td style='text-align:center;color:#fff'>" + String(oiler.getRecentOilingCount(i));
        html += "</td><td><input type='number' name='p" + String(i) + "' value='" + String(r->pulses) + "' class='pulse-input'></td></tr>";
    }
    
    // Add Reset Link below the table
    html += "</table><div style='text-align:left;margin-top:10px;margin-bottom:10px'><a href='/reset_time_stats' style='color:red;text-decoration:none;font-size:1.1em'>[Reset Stats]</a></div>";
    
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
    
    footer.replace("%EMERG_CHECKED%", oiler.isEmergencyModeForced() ? "checked" : "");
    footer.replace("%START_DLY%", String(oiler.startupDelayMeters, 0));
    footer.replace("%OFFROAD_INT%", String(oiler.offroadIntervalMin));
    
    footer.replace("%FLUSH_EV%", String(oiler.flushConfigEvents));
    footer.replace("%FLUSH_PLS%", String(oiler.flushConfigPulses));
    footer.replace("%FLUSH_INT%", String(oiler.flushConfigIntervalSec));
    
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

void handleLEDSettings() {
    resetWifiTimer();
    String html = htmlLEDSettings;
    
    html.replace("%LED_DIM%", String(map(oiler.ledBrightnessDim, 2, 202, 0, 100)));
    html.replace("%LED_HIGH%", String(map(oiler.ledBrightnessHigh, 2, 202, 0, 100)));
    
    html.replace("%NIGHT_CHECKED%", oiler.nightModeEnabled ? "checked" : "");
    html.replace("%NIGHT_START%", String(oiler.nightStartHour));
    html.replace("%NIGHT_END%", String(oiler.nightEndHour));
    html.replace("%NIGHT_BRI%", String(map(oiler.nightBrightness, 2, 202, 0, 100)));
    html.replace("%NIGHT_BRI_H%", String(map(oiler.nightBrightnessHigh, 2, 202, 0, 100)));
    
    server.send(200, "text/html", html);
}

void handleRoot() {
    resetWifiTimer();
    String html = htmlLanding;
    
    html.replace("%TIME%", getZurichTime());
    html.replace("%SATS%", String(gps.satellites.value()));
    
    String tempHeader = "--";
    if (oiler.isTempSensorConnected()) {
        tempHeader = String(oiler.getCurrentTempC(), 1);
    }
    html.replace("%TEMP%", tempHeader);
    
    html.replace("%TANK_LEVEL%", String(oiler.currentTankLevelMl, 0));
    html.replace("%TANK_CAP%", String(oiler.tankCapacityMl, 0));
    float pct = (oiler.tankCapacityMl > 0) ? (oiler.currentTankLevelMl / oiler.tankCapacityMl) * 100.0 : 0.0;
    html.replace("%TANK_PCT%", String(pct, 0));
    
    String tankColor = (pct <= oiler.tankWarningThresholdPercent) ? "#d32f2f" : "#ffc107";
    html.replace("%TANK_COLOR%", tankColor);
    
    html.replace("%TOTAL_DIST%", String(oiler.getTotalDistance(), 1));
    html.replace("%PUMP_COUNT%", String(oiler.getPumpCycles()));
    html.replace("%PROGRESS%", String(oiler.getCurrentProgress() * 100.0, 1));
    
    bool emerg = oiler.isEmergencyModeForced();
    html.replace("%EMERG_CLASS%", emerg ? "btn-danger" : "btn-sec");
    html.replace("%EMERG_STATUS%", emerg ? "ON" : "OFF");
    
    server.send(200, "text/html", html);
}

void handleSaveLED() {
    resetWifiTimer();
    webConsole.log("CMD: Save LED Settings");
    
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
    
    oiler.saveConfig();
    server.sendHeader("Location", "/led_settings");
    server.send(303);
}

void handleSave() {
    resetWifiTimer();
    webConsole.log("CMD: Save Settings");
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

    oiler.setEmergencyModeForced(server.hasArg("emerg_mode"));
    
    if(server.hasArg("start_dly")) oiler.startupDelayMeters = server.arg("start_dly").toFloat();
    if(server.hasArg("offroad_int")) oiler.offroadIntervalMin = server.arg("offroad_int").toInt();
    
    if(server.hasArg("flush_ev")) oiler.flushConfigEvents = server.arg("flush_ev").toInt();
    if(server.hasArg("flush_pls")) oiler.flushConfigPulses = server.arg("flush_pls").toInt();
    if(server.hasArg("flush_int")) oiler.flushConfigIntervalSec = server.arg("flush_int").toInt();
    
    oiler.tankMonitorEnabled = server.hasArg("tank_en");
    if(server.hasArg("tank_cap")) oiler.tankCapacityMl = server.arg("tank_cap").toFloat();
    if(server.hasArg("drop_ml")) oiler.dropsPerMl = server.arg("drop_ml").toInt();
    if(server.hasArg("drop_pls")) oiler.dropsPerPulse = server.arg("drop_pls").toInt();
    if(server.hasArg("tank_warn")) oiler.tankWarningThresholdPercent = server.arg("tank_warn").toInt();

    oiler.saveConfig();
    server.sendHeader("Location", "/settings");
    server.send(303);
}

void handleIMU() {
    resetWifiTimer();
    String html = htmlIMU;
    
    html.replace("%IMU_MODEL%", oiler.imu.getModel());
    html.replace("%IMU_STATUS%", oiler.imu.isAvailable() ? "<span style='color:green'>OK</span>" : "<span style='color:red'>Not Found</span>");
    html.replace("%PITCH%", String(oiler.imu.getPitch(), 1));
    html.replace("%ROLL%", String(oiler.imu.getRoll(), 1));
    
    // Chain Side Config
    if (oiler.imu.isChainOnRight()) {
        html.replace("%CHAIN_LEFT%", "");
        html.replace("%CHAIN_RIGHT%", "selected");
    } else {
        html.replace("%CHAIN_LEFT%", "selected");
        html.replace("%CHAIN_RIGHT%", "");
    }

    server.send(200, "text/html", html);
}

void handleIMUConfig() {
    resetWifiTimer();
    if (server.hasArg("chain_side")) {
        bool isRight = (server.arg("chain_side").toInt() == 1);
        webConsole.log("CMD: Set Chain Side " + String(isRight ? "RIGHT" : "LEFT"));
        oiler.imu.setChainSide(isRight);
    }
    server.sendHeader("Location", "/imu");
    server.send(303);
}

void handleAuxConfig() {
    resetWifiTimer();
    String html = htmlAuxConfig;
    
    AuxMode mode = auxManager.getMode();
    html.replace("%MODE_OFF%", (mode == AUX_MODE_OFF) ? "selected" : "");
    html.replace("%MODE_AUX%", (mode == AUX_MODE_AUX_POWER) ? "selected" : "");
    html.replace("%MODE_GRIPS%", (mode == AUX_MODE_HEATED_GRIPS) ? "selected" : "");
    
    int base, rainB, startL, startS, startD;
    float speedF, tempF, tempO, startT;
    auxManager.getGripSettings(base, speedF, tempF, tempO, startT, rainB, startL, startS, startD);
    
    html.replace("%BASE%", String(base));
    
    // Speed Factor Selection
    html.replace("%SPEED_LOW%", (abs(speedF - 0.2) < 0.1) ? "selected" : "");
    html.replace("%SPEED_MED%", (abs(speedF - 0.5) < 0.1) ? "selected" : "");
    html.replace("%SPEED_HIGH%", (abs(speedF - 1.0) < 0.1) ? "selected" : "");
    
    // Temp Factor Selection
    html.replace("%TEMP_LOW%", (abs(tempF - 1.0) < 0.1) ? "selected" : "");
    html.replace("%TEMP_MED%", (abs(tempF - 2.0) < 0.1) ? "selected" : "");
    html.replace("%TEMP_HIGH%", (abs(tempF - 3.0) < 0.1) ? "selected" : "");

    html.replace("%TEMPO%", String(tempO, 1));
    
    if (oiler.isTempSensorConnected()) {
        html.replace("%CURRENT_TEMP%", String(oiler.getCurrentTempC(), 1));
    } else {
        html.replace("%CURRENT_TEMP%", "no sensor");
    }

    html.replace("%STARTT%", String(startT, 0));
    html.replace("%RAINB%", String(rainB));
    html.replace("%STARTL%", String(startL));
    html.replace("%STARTS%", String(startS));
    html.replace("%STARTD%", String(startD));
    
    server.send(200, "text/html", html);
}

void handleMaintenance() {
    resetWifiTimer();
    server.send(200, "text/html", htmlMaintenance);
}

void handleConsole() {
    resetWifiTimer();
    server.send(200, "text/html", htmlConsole);
}

void handleConsoleData() {
    resetWifiTimer();
    server.send(200, "text/plain", webConsole.getLogs());
}

void handleConsoleClear() {
    resetWifiTimer();
    webConsole.clear();
    server.sendHeader("Location", "/console");
    server.send(303);
}

void handleSaveAux() {
    resetWifiTimer();
    webConsole.log("CMD: Save Aux Settings");
    
    if (server.hasArg("mode")) {
        auxManager.setMode((AuxMode)server.arg("mode").toInt());
    }
    
    int base = server.arg("base").toInt();
    float speedF = server.arg("speedF").toFloat();
    float tempF = server.arg("tempF").toFloat();
    float tempO = server.arg("tempO").toFloat();
    float startT = server.arg("startT").toFloat();
    int rainB = server.arg("rainB").toInt();
    int startL = server.arg("startL").toInt();
    int startS = server.arg("startS").toInt();
    int startD = server.arg("startD").toInt();
    
    auxManager.setGripSettings(base, speedF, tempF, tempO, startT, rainB, startL, startS, startD);
    
    server.sendHeader("Location", "/aux");
    server.send(303);
}

void handleIMUZero() {
    resetWifiTimer();
    webConsole.log("CMD: IMU Zero Calibration");
    oiler.imu.calibrateZero(); 
    server.sendHeader("Location", "/imu");
    server.send(303);
}

void handleIMUSide() {
    resetWifiTimer();
    webConsole.log("CMD: IMU Side Stand Calibration");
    oiler.imu.calibrateSideStand();
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
    auxManager.begin(&oiler.imu);

#ifdef SD_LOGGING_ACTIVE
    initSD();
#endif

    // WiFi Start Logic: Default OFF
    // Start DNS Server only when needed

    // Webserver Routes
    server.on("/style.css", handleCss);
    server.on("/", handleRoot);
    server.on("/settings", handleSettings);
    server.on("/led_settings", handleLEDSettings);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/save_led", HTTP_POST, handleSaveLED);
    server.on("/toggle_emerg", handleToggleEmerg);
    server.on("/help", handleHelp);
    
    // IMU Routes
    server.on("/imu", handleIMU);
    server.on("/imu_zero", HTTP_POST, handleIMUZero);
    server.on("/imu_side", HTTP_POST, handleIMUSide);
    server.on("/imu_config", HTTP_POST, handleIMUConfig);
    
    // Aux Routes
    server.on("/aux", handleAuxConfig);
    server.on("/save_aux", HTTP_POST, handleSaveAux);
    
    // Console Routes
    server.on("/console", handleConsole);
    server.on("/console/data", handleConsoleData);
    server.on("/console/clear", HTTP_POST, handleConsoleClear);
    
    server.on("/reset_stats", handleResetStats);
    server.on("/reset_time_stats", handleResetTimeStats);
    server.on("/refill", handleRefill);
    
    // Maintenance Routes
    server.on("/maintenance", handleMaintenance);
    server.on("/test_pump", HTTP_GET, []() {
        webConsole.log("CMD: Test Pump (1 Pulse)");
        oiler.triggerOil(1); // Fire 1 pulse
        server.sendHeader("Location", "/maintenance");
        server.send(303);
    });
    
    server.on("/bleeding", HTTP_GET, []() {
        webConsole.log("CMD: Start Bleeding");
        oiler.startBleeding();
        server.sendHeader("Location", "/maintenance");
        server.send(303);
    });
    
    server.on("/restart", HTTP_GET, []() {
        webConsole.log("CMD: Restart System");
        server.send(200, "text/plain", "Restarting in 3s...");
        shouldRestart = true;
        restartTimer = millis();
    });

    server.on("/factory_reset", HTTP_GET, []() {
        webConsole.log("CMD: Factory Reset");
        server.send(200, "text/plain", "Factory Resetting in 3s... Please reconnect to WiFi AP after reboot.");
        shouldFactoryReset = true;
        restartTimer = millis();
    });
    
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

    // Handle Delayed Restart / Reset
    static int lastCountdown = 4;
    if (shouldRestart || shouldFactoryReset) {
        int remaining = 3 - ((millis() - restartTimer) / 1000);
        
        if (remaining < lastCountdown && remaining > 0) {
            webConsole.log("... " + String(remaining));
            lastCountdown = remaining;
        }
        
        if (millis() - restartTimer > 3000) {
             if (shouldRestart) {
                 webConsole.log("RESTARTING NOW");
                 delay(100);
                 ESP.restart();
             }
             if (shouldFactoryReset) {
                 oiler.performFactoryReset();
             }
        }
    } else {
        lastCountdown = 4;
    }

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
        
        String logMsg = String("GPS: Fix=") + (gps.location.isValid() ? "OK" : "NO") + 
                        ", Sats=" + String(gps.satellites.value()) + 
                        ", HDOP=" + String(gps.hdop.hdop(), 1);
        webConsole.log(logMsg);

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
    
    // Run Aux Manager Loop
    auxManager.loop(oiler.getSmoothedSpeed(), oiler.lastTemp, oiler.isRainMode());
    
    // Pass Aux Status to Oiler for LED
    oiler.setAuxStatus(auxManager.getCurrentPwm(), (int)auxManager.getMode(), auxManager.isBoostActive());

#ifdef SD_LOGGING_ACTIVE
    if (millis() - lastLogTime > LOG_INTERVAL_MS) {
        writeLogLine("DATA");
        lastLogTime = millis();
    }
#endif

    // --- WiFi Management & Aux Toggle ---
    unsigned long currentMillis = millis();

    // 1. Button Requests (Handled by Oiler)
    if (oiler.checkWifiToggleRequest()) {
        if (!wifiActive) {
            // Activate WiFi
            WiFi.softAP(AP_SSID);
            IPAddress IP = WiFi.softAPIP();
#ifdef GPS_DEBUG
            Serial.print("WiFi activated via Button. IP: ");
            Serial.println(IP);
#endif
            dnsServer.start(53, "*", IP);
            server.begin();
            wifiActive = true;
            wifiStartTime = currentMillis;
        } else {
            // WiFi is already active.
            // Prevent accidental deactivation via button (User Request).
            // Instead, we extend the timer.
            wifiStartTime = currentMillis;
            webConsole.log("BTN: WiFi Timer Extended");
        }
    }

    if (oiler.checkAuxToggleRequest()) {
        auxManager.toggleManualOverride();
#ifdef GPS_DEBUG
        Serial.print("Aux Manual Override Toggled: ");
        Serial.println(auxManager.isManualOverrideActive() ? "ON" : "OFF");
#endif
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
