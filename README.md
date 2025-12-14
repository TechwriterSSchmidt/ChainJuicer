# GPS Chain Oiler (ESP32-based)

An advanced, GPS-controlled chain oiler for motorcycles based on the ESP32. The system dynamically adjusts oiling intervals based on the driven speed and offers extensive configuration options via a web interface.

## 🚀 Features

*   **Speed-Dependent Oiling:** 5 configurable speed ranges with individual intervals (km) and pump pulses.
*   **Smart Smoothing:** Uses a lookup table with linear interpolation and a low-pass filter to avoid harsh jumps in lubrication intervals.
*   **Drift Filter:** Detects and ignores satellite signal reflections (multipath) to prevent "ghost mileage" indoors or in tunnels (HDOP > 5.0 or < 5 satellites).
*   **GPS Precision:** Exact distance measurement via GPS module (TinyGPS++).
*   **Rain Mode:** Doubles the oil amount in wet conditions. Activatable via button. Automatic shut-off after 30 minutes or upon restart (ignition off).
    *   *Note:* Automatically disabled if Emergency Mode is active (forced or auto).
*   **Emergency Mode:**
    *   **Automatic:** Activates if no GPS signal is received for more than 3 minutes.
        *   Simulates driving at 50 km/h.
        *   Automatically disables Rain Mode.
    *   **Manual (Forced):** Continuous simulation of 50 km/h. Forces Rain Mode OFF.
        *   *Safety Feature:* Automatically resets to OFF when the ignition is turned off (reboot) to prevent accidental oiling in the garage.
*   **WiFi & Web Interface:**
    *   Configure all parameters conveniently via smartphone.
    *   LED brightness adjustable in percent (0-100%).
    *   Activation only at standstill (< 7 km/h) via long button press (> 3s).
    *   Automatic shut-off when driving (> 10 km/h) or after inactivity (5 min).
    *   **New:** Displays current Satellite count in the header.
*   **Night Mode:** Automatic dimming of the status LED based on GPS time. Separate brightness adjustable for events (oiling, WiFi).
*   **Bleeding Mode:** Continuous pumping to fill the oil line after maintenance.
*   **Tank Monitor:** Calculates oil consumption and warns (Red 2x blink) when the supply runs low.
*   **Advanced Statistics:**
    *   **Usage %:** Shows the percentage of driving time spent in each speed range.
    *   **Cycles:** Counts the number of triggered oilings per speed range.
    *   **Odometer:** Total distance counter (includes simulated distance in Emergency Mode).
*   **Auto-Save:** Odometer and settings are permanently saved in flash memory (NVS) at standstill (< 7 km/h).

## 🛠 Hardware

*   **MCU:** ESP32-S3-TINY (or compatible ESP32 board)
*   **GPS:** ATGM336H or NEO-6M (UART, 9600 Baud)
*   **Pump:** Dosing pump (controlled via Logic Level MOSFET, e.g., NCE6020AK)
*   **LED:** WS2812B (NeoPixel) for status indication
*   **Button:** Normally Open against GND (Input Pullup)

### Pin Assignment (Current Config)

| Component | ESP32 Pin | Description |
| :--- | :--- | :--- |
| **Pump** | GPIO 16 | MOSFET Gate (Direct Drive) |
| **GPS RX** | GPIO 27 | Connected to GPS TX |
| **GPS TX** | GPIO 26 | Connected to GPS RX |
| **Button** | GPIO 4 | Switched against GND |
| **Boot Button** | GPIO 0 | Onboard Button (Parallel function) |
| **LED** | GPIO 32 | WS2812B Data In |

*(Configurable in `include/config.h`)*

### MOSFET Wiring (Direct Drive)

To prevent the pump from triggering briefly during boot (floating pin), use a Logic Level MOSFET (e.g., NCE6020AK) and a Pull-Down resistor. Do **not** use an NPN driver stage (Emitter Follower) as it may cause threshold voltage issues with 3.3V logic.

```ascii
ESP32 GPIO (the one that switches the MOSFET
)  ----[ 200R ]----+-----> MOSFET Gate
                               |
                              [10k]
                               |
                              GND
```

*   **200R (Series):** Protects the ESP32 pin from current spikes (Gate capacitance).
*   **10k (Pull-Down):** Keeps the Gate at GND (0V) while the ESP32 is booting.
*   **Software:** Ensure `PUMP_INVERTED` is set to `false` in `config.h`.

## 📖 Operation

### Button Functions

| Action | Duration | Condition | Function |
| :--- | :--- | :--- | :--- |
| **Short Press** | < 1.5s | Always | **Rain Mode** On/Off (LED: Blue). *Disabled in Emergency Mode.* |
| **Hold** | > 3s | At Standstill (< 7 km/h) | Activate **WiFi & Web Interface** (LED: White pulsing) |
| **Long Hold** | > 10s | At Standstill (< 7 km/h) | Start **Bleeding Mode** (LED: Red blinking, pump runs 10s) |

### LED Status Codes

*   🟢 **Green:** Normal Operation (GPS Fix available)
*   🔵 **Blue:** Rain Mode Active
*   🟣 **Magenta:** No GPS Signal (Searching...)
*   🔵 **Cyan:** Emergency Mode (No GPS > 3 min, Simulation active)
*   🟡 **Yellow:** Oiling in progress (lit for 3s)
*   ⚪ **White (pulsing):** WiFi Configuration Mode active
*   🔴 **Red (2x blink):** Tank Warning (Reserve reached)
*   🔴 **Red (blinking):** Bleeding Mode active

## 📱 Web Interface

Connect to the WiFi network (Default SSID: `ChainJuicer`, no password) after activating it. Open `192.168.4.1` in your browser.

**Adjustable Parameters:**
*   **Intervals:** Distance and pump pulses for 5 speed ranges.
*   **Modes:** Rain Mode, Emergency Mode (Force), Night Mode (Times & Brightness).
*   **LED:** Brightness for Day and Night (in %).
*   **Statistics:**
    *   **Usage %:** Driving profile analysis to optimize intervals.
    *   **Cycles:** Counter for oilings per range.
    *   **Reset:** Reset stats or refill tank.

## 💡 Usage Scenarios

### Scenario 1: Tunnel / Signal Loss
You enter a long tunnel. The GPS signal is lost.
1.  **0-3 Min:** LED turns Magenta (Searching). No oiling occurs yet.
2.  **> 3 Min:** System enters **Auto Emergency Mode**. LED turns Cyan.
    *   The system assumes a speed of 50 km/h for the Odometer and Oiling.
    *   Rain Mode is automatically disabled if it was on.
3.  **Exit Tunnel:** GPS signal returns. System switches back to Green (Normal). Odometer continues with real data.

### Scenario 2: Rain Ride
It starts raining.
1.  **Action:** Short press the button.
2.  **Feedback:** LED turns Blue.
3.  **Effect:** Oiling amount is doubled (or interval halved) for the current speed.
4.  **End:** Press button again, or wait 30 minutes (Auto-Off), or turn off ignition (Reset).

### Scenario 3: Maintenance (Bleeding)
You installed a new oil line and need to fill it.
1.  **Condition:** Bike must be standing still (< 7 km/h).
2.  **Action:** Hold button for > 10 seconds.
3.  **Feedback:** LED blinks Red. Pump runs continuously for 10 seconds.
4.  **Repeat:** If line is not full, repeat the process.

### Scenario 4: Hardware Debugging (Pump runs at boot)
If your pump runs immediately when powering on the ESP32:
1.  Check wiring: Ensure the 10k Pull-Down resistor is present between Gate and GND.
2.  Check Config: Ensure `PUMP_INVERTED` is `false` in `config.h`.
3.  Check MOSFET: Ensure you are using a Logic Level MOSFET (Vgs(th) < 2.5V) and not a standard MOSFET or BJT Darlington without proper bias.

## ⚙️ Technical Details

*   **Non-Blocking:** Pump control is asynchronous.
*   **Adaptive Smoothing:** Combination of Lookup Table and Low-Pass Filter.
*   **Smart Oiling (Hysteresis):** Oiling is triggered at **95% of the calculated distance**.
*   **Auto-Save:** The odometer is saved intelligently (at standstill < 7 km/h, but max. every 2 minutes).
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




