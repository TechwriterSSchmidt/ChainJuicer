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
const unsigned long WIFI_TIMEOUT = 5 * 60 * 1000; // 5 Minutes

// HTML Page for Configuration
const char* htmlHeader = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'><title>Chain Oiler</title><style>body{font-family:sans-serif;margin:0;padding:10px;background:#f4f4f9} h2{text-align:center;color:#333} h3{color:#555;margin-top:20px} form{background:#fff;padding:15px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1)} table{width:100%;border-collapse:collapse} th{text-align:left;color:#666;font-size:0.9em} td{padding:10px 5px;border-bottom:1px solid #eee} input{width:100%;padding:8px;border:1px solid #ddd;border-radius:4px;font-size:16px;box-sizing:border-box} input[type=checkbox]{width:20px;height:20px} .btn{background:#007bff;color:white;padding:12px;border:none;width:100%;font-size:16px;border-radius:4px;margin-top:15px;cursor:pointer} .progress{text-align:center;margin-top:15px;color:#555} .time{text-align:center;color:#888;font-size:0.9em;margin-bottom:10px} .help-link{text-align:center;margin-bottom:15px} .help-link a{color:#007bff;text-decoration:none}</style></head><body><h2>Chain Oiler Config</h2><div class='help-link'><a href='/help'>Help & Manual</a></div><div class='time'>Time (GPS): %TIME%</div><form action='/save' method='POST'><table><tr><th>Range</th><th>Km</th><th>Pulses</th></tr>";
const char* htmlFooter = "</table><h3>General</h3><table><tr><td>Rain Mode (x2)</td><td><input type='checkbox' name='rain_mode' %RAIN_CHECKED%></td></tr><tr><td>Emergency Mode (No GPS=50kmh)</td><td><input type='checkbox' name='emerg_mode' %EMERG_CHECKED%></td></tr></table><h3>LED Settings</h3><table><tr><td>Normal (Dim)</td><td><input type='number' name='led_dim' value='%LED_DIM%'></td></tr><tr><td>Event (Bright)</td><td><input type='number' name='led_high' value='%LED_HIGH%'></td></tr></table><h3>Night Mode</h3><table><tr><td>Enable</td><td><input type='checkbox' name='night_en' %NIGHT_CHECKED%></td></tr><tr><td>Start (Hour)</td><td><input type='number' name='night_start' value='%NIGHT_START%'></td></tr><tr><td>End (Hour)</td><td><input type='number' name='night_end' value='%NIGHT_END%'></td></tr><tr><td>Brightness</td><td><input type='number' name='night_bri' value='%NIGHT_BRI%'></td></tr></table><h3>Statistics</h3><table><tr><td>Total Distance</td><td>%TOTAL_DIST% km</td></tr><tr><td>Oiling Cycles</td><td>%PUMP_COUNT%</td></tr></table><div style='margin-top:10px'><a href='/reset_stats' onclick=\"return confirm('Really reset?')\" style='color:red;text-decoration:none;font-size:0.9em'>[Reset Stats]</a></div><div class='progress'>Current Progress: %PROGRESS%%</div><input type='submit' value='Save' class='btn'></form></body></html>";

const char* htmlHelp = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'><title>Help</title><style>body{font-family:sans-serif;margin:0;padding:15px;background:#fff;line-height:1.6} h2{color:#333} h3{color:#007bff;border-bottom:1px solid #eee;padding-bottom:5px} ul{padding-left:20px} .color-box{display:inline-block;width:12px;height:12px;margin-right:5px;border-radius:50%} .btn{display:block;background:#6c757d;color:white;text-align:center;padding:10px;text-decoration:none;border-radius:4px;margin-top:20px}</style></head><body><h2>Manual</h2><h3>Features</h3><ul><li><b>GPS Control:</b> Speed-dependent intervals.</li><li><b>Rain Mode:</b> Double oil amount, Auto-Off after 30 min.</li><li><b>Emergency Mode:</b> Oiling without GPS (simulates 50 km/h).</li><li><b>Night Mode:</b> Automatic LED dimming.</li><li><b>Web Config:</b> Settings via Smartphone.</li><li><b>Memory:</b> All settings are retained.</li><li><b>Statistics:</b> Odometer and oiling counter.</li><li><b>Watchdog:</b> Automatic restart on system error.</li></ul><h3>WiFi & Web Interface</h3><p>WiFi is <b>OFF</b> by default.</p><ul><li><b>Activate:</b> Hold button (> 3s) while standing still (< 5 km/h).</li><li><b>Deactivate:</b> Automatically when driving (> 10 km/h) or after 5 min inactivity.</li><li><b>Signal:</b> LED blinks white when WiFi is active.</li></ul><h3>Modes & Button</h3><ul><li><b>Short Press (< 1.5s):</b> Toggle Rain Mode (Half interval).</li><li><b>Long Press (> 10s):</b> Bleeding Mode (Pump runs 5s for bleeding).</li></ul><h3>LED Color Scheme</h3><ul><li><span class='color-box' style='background:green'></span> <b>Green:</b> Normal Operation (GPS OK).</li><li><span class='color-box' style='background:blue'></span> <b>Blue:</b> Rain Mode Active.</li><li><span class='color-box' style='background:magenta'></span> <b>Magenta:</b> No GPS Signal.</li><li><span class='color-box' style='background:cyan'></span> <b>Cyan:</b> Emergency Mode (No GPS).</li><li><span class='color-box' style='background:yellow'></span> <b>Yellow:</b> Oiling in progress.</li><li><span class='color-box' style='border:1px solid #ccc'></span> <b>White blinking:</b> WiFi Config Active.</li><li><span class='color-box' style='background:red'></span> <b>Red blinking:</b> Bleeding Mode Active.</li></ul><h3>LED Brightness</h3><p>The brightness of the status LED can be adjusted. Range: 0 (Off) to 255 (Max).</p><ul><li><b>Normal (Dim):</b> Standard brightness during operation.</li><li><b>Event (Bright):</b> Brightness during events (e.g. oiling).</li></ul><h3>Night Mode</h3><p>Night mode automatically reduces LED brightness during a defined time window.</p><ul><li><b>Start/End:</b> Hour (0-23) for night mode.</li><li><b>Brightness:</b> Brightness value during night (recommended: 5-10).</li></ul><a href='/' class='btn'>Back</a></body></html>";

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

void handleRoot() {
    resetWifiTimer();
    String html = htmlHeader;
    html.replace("%TIME%", getZurichTime());

    for(int i=0; i<NUM_RANGES; i++) {
        SpeedRange* r = oiler.getRangeConfig(i);
        html += "<tr><td>";
        html += String((int)r->minSpeed) + "-" + String((int)r->maxSpeed) + " km/h";
        html += "</td><td><input type='number' step='0.1' name='km" + String(i) + "' value='" + String(r->intervalKm) + "'>";
        html += "</td><td><input type='number' name='p" + String(i) + "' value='" + String(r->pulses) + "'></td></tr>";
    }
    
    String footer = htmlFooter;
    footer.replace("%PROGRESS%", String(oiler.getCurrentProgress() * 100.0, 1));
    footer.replace("%LED_DIM%", String(oiler.ledBrightnessDim));
    footer.replace("%LED_HIGH%", String(oiler.ledBrightnessHigh));
    
    footer.replace("%RAIN_CHECKED%", oiler.isRainMode() ? "checked" : "");
    footer.replace("%EMERG_CHECKED%", oiler.isEmergencyMode() ? "checked" : "");
    
    footer.replace("%NIGHT_CHECKED%", oiler.nightModeEnabled ? "checked" : "");
    footer.replace("%NIGHT_START%", String(oiler.nightStartHour));
    footer.replace("%NIGHT_END%", String(oiler.nightEndHour));
    footer.replace("%NIGHT_BRI%", String(oiler.nightBrightness));
    
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
    
    if(server.hasArg("led_dim")) oiler.ledBrightnessDim = server.arg("led_dim").toInt();
    if(server.hasArg("led_high")) oiler.ledBrightnessHigh = server.arg("led_high").toInt();
    
    // Rain Mode Checkbox handling
    oiler.setRainMode(server.hasArg("rain_mode"));
    oiler.setEmergencyMode(server.hasArg("emerg_mode"));
    
    oiler.nightModeEnabled = server.hasArg("night_en");
    if(server.hasArg("night_start")) oiler.nightStartHour = server.arg("night_start").toInt();
    if(server.hasArg("night_end")) oiler.nightEndHour = server.arg("night_end").toInt();
    if(server.hasArg("night_bri")) oiler.nightBrightness = server.arg("night_bri").toInt();

    oiler.saveConfig();
    server.sendHeader("Location", "/");
    server.send(303);
}

void setup() {
    // Safety: Ensure Pump is OFF immediately
    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, LOW);

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
    // WiFi.softAP(AP_SSID); <-- Removed
    // IPAddress IP = WiFi.softAPIP();
    // Serial.print("AP IP address: ");
    // Serial.println(IP);

    // Start DNS Server only when needed
    // dnsServer.start(53, "*", IP);

    // Webserver Routes
    server.on("/", handleRoot);
    server.on("/save", handleSave);
    server.on("/help", handleHelp);
    server.on("/reset_stats", [](){
        oiler.resetStats();
        server.sendHeader("Location", "/");
        server.send(303);
    });
    // Captive Portal Android Check
    server.on("/generate_204", handleRoot);
    server.onNotFound(handleRoot); // Redirect all other requests to Root
    server.begin();

    wifiStartTime = millis();
}

void loop() {
    // Reset Watchdog
    esp_task_wdt_reset();

    // Read GPS Data
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }

    float currentSpeed = gps.speed.isValid() ? gps.speed.kmph() : 0.0;

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

    // 1. Activation: Standstill (< 5kmh) + Button pressed > 3s
    static unsigned long wifiButtonPressStart = 0;
    static bool wifiButtonHeld = false;

    if (oiler.isButtonPressed()) {
        if (currentSpeed < 5.0) {
            if (!wifiButtonHeld) {
                wifiButtonPressStart = currentMillis;
                wifiButtonHeld = true;
            } else {
                // Check duration
                if (currentMillis - wifiButtonPressStart > WIFI_PRESS_MS) {
                    // > 3s held
                    wifiStartTime = currentMillis; // Reset timeout timer
                    
                    if (!wifiActive) {
                        WiFi.softAP(AP_SSID);
                        IPAddress IP = WiFi.softAPIP();
                        Serial.print("WiFi activated via Button (>3s). IP: ");
                        Serial.println(IP);
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

    // 2. Deactivation: Driving (> 10kmh) or Timeout
    if (wifiActive) {
        bool shouldStop = false;
        
        // Timeout Check
        if (currentMillis - wifiStartTime > WIFI_TIMEOUT) {
            Serial.println("WiFi Timeout.");
            shouldStop = true;
        }
        
        // Speed Check (Auto-Off when driving)
        if (currentSpeed > 10.0) {
            Serial.println("Driving detected -> WiFi off.");
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
