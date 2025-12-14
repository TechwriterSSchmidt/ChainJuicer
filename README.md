# GPS Chain Oiler (ESP32)

An advanced, GPS-controlled chain oiler for motorcycles based on the ESP32. The system dynamically adjusts oiling intervals based on the driven speed and offers extensive configuration options via a web interface.

## 🚀 Features

*   **Speed-Dependent Oiling:** 5 configurable speed ranges with individual intervals (km) and pump pulses.
*   **Smart Smoothing:** Uses a lookup table with linear interpolation and a low-pass filter to avoid harsh jumps in lubrication intervals.
*   **Smart GPS Filter:** Detects and ignores "ghost speeds" (multipath reflections) indoors or in tunnels (HDOP > 5.0 or < 5 satellites).
*   **Startup Delay:** 10-second safety delay after boot to prevent pump malfunctions during initialization.
*   **GPS Precision:** Exact distance measurement via GPS module (TinyGPS++).
*   **Rain Mode:** Doubles the oil amount in wet conditions. Activatable via button. Automatic shut-off after 30 minutes or upon restart (ignition off).
*   **Emergency Mode:**
    *   **Automatic:** Activates if no GPS signal is received for more than 5 minutes.
        *   After 15 min: 1st oiling.
        *   After 30 min: 2nd oiling.
        *   After 31 min: Timeout (Red LED, no further oiling).
    *   **Manual (Forced):** Continuous simulation of 50 km/h (ignores timeout).
*   **Smart WiFi & Web Interface:**
    *   Configure all parameters conveniently via smartphone.
    *   **OTA Update:** Upload new firmware directly via the web interface.
    *   LED brightness adjustable in percent (0-100%).
    *   Activation only at standstill (< 7 km/h) via long button press (> 3s).
    *   Automatic shut-off when driving (> 10 km/h) or after inactivity (5 min).
*   **Night Mode:** Automatic dimming of the status LED based on GPS time. Separate brightness adjustable for events (oiling, WiFi).
*   **Bleeding Mode:** Continuous pumping to fill the oil line after maintenance.
*   **Tank Monitor:** Calculates oil consumption and warns (pulsing LED) when the supply runs low.
*   **Advanced Statistics:**
    *   **Driving Profile (Time %):** Shows the percentage of driving time spent in each speed range.
    *   **Oiling Counter:** Counts the number of triggered oilings per speed range.
    *   **Odometer:** Total distance counter.
*   **Data Security:** Odometer and settings are permanently saved in flash memory (NVS).

## 🛠 Hardware

*   **MCU:** ESP32 Development Board
*   **GPS:** NEO-6M or compatible module (UART)
*   **Pump:** Dosing pump (controlled via MOSFET/Transistor)
*   **LED:** WS2812B (NeoPixel) for status indication (Supports multiple LEDs, e.g., status + case lighting)
*   **Button:** Normally Open against GND (Input Pullup)

### Pin Assignment (Default)

| Component | ESP32 Pin | Description |
| :--- | :--- | :--- |
| **Pump** | GPIO 27 | MOSFET Gate |
| **GPS RX** | GPIO 32 | Connected to GPS TX |
| **GPS TX** | GPIO 33 | Connected to GPS RX |
| **Button (Handlebar)** | GPIO 4 | Switched against GND |
| **Boot Button** | GPIO 0 | Onboard Button (Parallel function) |
| **LED** | GPIO 5 | WS2812B Data In |

*(Configurable in `include/config.h`)*

## 📖 Operation

### Button Functions

| Action | Duration | Condition | Function |
| :--- | :--- | :--- | :--- |
| **Short Press** | < 1.5s | Always | **Rain Mode** On/Off (LED: Blue) |
| **Hold** | > 3s | At Standstill (< 7 km/h) | Activate **WiFi & Web Interface** (LED: White pulsing) |
| **Long Hold** | > 10s | At Standstill (< 7 km/h) | Start **Bleeding Mode** (LED: Red blinking, pump runs 10s) |

### LED Status Codes

*   🟢 **Green:** Normal Operation (GPS Fix available)
*   🔵 **Blue:** Rain Mode Active
*   🟣 **Magenta (pulsing):** No GPS Signal (Searching...)
*   🟠/🟢 **Orange (2x short) / Green:** Emergency Mode (No GPS > 3 min, Simulation active)
*   🟡 **Yellow (breathing):** Oiling in progress
*   ⚪ **White (pulsing):** WiFi Configuration Mode active (only when Idle)
*   🟠 **Orange (2x blink):** Tank Warning (Reserve reached)
*   🔴 **Red (fast blink):** Bleeding Mode active
*   🔵 **Cyan (fast blink):** Firmware Update in progress

## 📱 Web Interface

Connect to the WiFi network (Default SSID: `ChainJuicer`, no password) after activating it. Open `192.168.4.1` in your browser.

**Adjustable Parameters:**
*   **Intervals:** Distance and pump pulses for 5 speed ranges.
*   **Modes:** Rain Mode, Emergency Mode, Night Mode (Times & Brightness).
*   **LED:** Brightness for Day and Night (in %).
*   **Statistics:**
    *   **Time %:** Driving profile analysis to optimize intervals.
    *   **Oilings:** Counter for oilings per range.
    *   Total odometer and pump cycles (Reset possible).

## ⚙️ Technical Details

*   **Non-Blocking:** Pump control is asynchronous.
*   **Adaptive Smoothing:** Combination of Lookup Table and Low-Pass Filter.
*   **Smart Oiling (Hysteresis):** Oiling is triggered at **95% of the calculated distance**.
*   **Memory Protection:** The odometer is saved intelligently (at standstill < 7 km/h, but max. every 2 minutes).
*   **Timezone:** Automatic calculation of Central European Time (CET/CEST).

## 💻 Installation (PlatformIO)

1.  Clone or download the repository.
2.  Open in VS Code with installed **PlatformIO** Extension.
3.  Adjust Upload Port in `platformio.ini` if necessary.
4.  Compile and upload project (`Upload`).
5.  File system is not necessary (Data is saved in NVS/Preferences).




