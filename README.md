# GPS Chain Oiler (ESP32-based)

An advanced, GPS-controlled chain oiler for motorcycles based on the ESP32. The system dynamically adjusts oiling intervals based on the driven speed and offers extensive configuration options via a web interface.

## 🚀 Features

*   **Speed-Dependent Oiling:** 5 configurable speed ranges with individual intervals (km) and pump pulses.
*   **Smart Smoothing:** Uses a lookup table with linear interpolation and a low-pass filter to avoid harsh jumps in lubrication intervals.
*   **Smart GPS Filter:** Detects and ignores "ghost speeds" (multipath reflections) indoors or in tunnels (HDOP > 5.0 or < 5 satellites).
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

### MOSFET Wiring example (check actual GPIO settings)

To prevent the pump from triggering briefly during boot (floating pin), desolder the J3Y transistor that triggers the MOSFET gate on powerup. Add pull-down resistors instead to directly connect the GPIO pin to the MOSFET gate:

```ascii
ESP32 GPIO 27  ----[ 220R ]----+-----> MOSFET Gate
                               |
                              [10k]
                               |
                              GND
```

*   **220R (Series):** Protects the ESP32 pin from current spikes (Gate capacitance).
*   **10k (Pull-Down):** Keeps the Gate at GND (0V) while the ESP32 is booting.

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
*   🟣 **Magenta:** No GPS Signal (Searching...)
*   🔵 **Cyan:** Emergency Mode (No GPS, Simulation active)
*   🔴 **Red (Bright):** Emergency Timeout (> 31 Min without GPS)
*   🟡 **Yellow:** Oiling in progress (lit for 3s)
*   ⚪ **White (pulsing):** WiFi Configuration Mode active
*   🔴 **Red (pulsing 2x):** Tank Warning (Reserve reached)
*   🔴 **Red (blinking):** Bleeding Mode active

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

## 🛒 BOM

1.  **ESP32 Board:** ESP32 WROOM 32e (4Mb) or better. I used this [board with integrated MOS switch and DCDC converter](https://de.aliexpress.com/item/1005009961593556.html) or similar.
2.  **GPS Module:** UBLOX M10 or compatible (UART).
3.  **Dosing Pump:** 12V Pulse Dosing Pump.
4.  **LED:** WS2812B (NeoPixel).
5.  **Button:** Momentary push button (normally open).
6.  **Resistors:**
    *   1x 220R - 470R (Series resistor).
    *   1x 10k (Pull-down resistor).
    *   *Note: Required to suppress the initial pump trigger during powerup with the above board.*
7.  **Wires & Connectors:** Various lengths and types.
8.  **Housing:** 3D printed housing. *[Link to MakerWorld or Thingiverse to be added]*
9.  **Screws:** 8x M3x8 screws for housing and board.
10. **Fuse:** 5A fuse and fuse holder.
11. **Misc:** Bits and pieces (heat shrink, cable ties, etc.).




