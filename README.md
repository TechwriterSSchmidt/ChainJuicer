# 🍋 Chain Juicer v1.1.0 (ESP32)

**The Swiss Army Knife for your Motorcycle Chain.**

An ESP32-based multi-tool featuring GPS-controlled chain lubrication, a Aux Power Manager for accessories, and an automated Heated Grips Controller. It combines intelligent maintenance with modern convenience and safety features like IMU-based crash detection.

**Easy to Build:** Designed around a widely available standard ESP32 Relay Board (LCTECH), this project requires **no custom PCB** and minimal soldering skills. It's the perfect entry point for DIY motorcycle electronics.

**Easy to Install:** No coding skills required! Use the [Web Installer](https://TechwriterSSchmidt.github.io/ChainJuicer/) to flash your ESP32 directly from the browser. Future updates can be done wirelessly via OTA (Over-The-Air) using the [latest firmware file](https://TechwriterSSchmidt.github.io/ChainJuicer/firmware.bin).

## Support my projects

If you like this project, consider a tip. Your tip motivates me to continue developing cool stuff for the DIY community. Thank you very much for your support!

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/D1D01HVT9A)

## Table of Contents
* [Features](#-features)
* [Optional IMU Features](#-optional-imu-features)
* [Optional SD Card Logging](#-optional-sd-card-logging)
* [Hardware & Functionality Matrix](#-hardware--functionality-matrix)
* [Automatic Temperature Compensation](#-automatic-temperature-compensation)
* [Aux Port Manager](#-aux-port-manager-gpio-17)
* [Hardware](#-hardware)
* [Operation](#-operation)
* [Web Interface](#-web-interface)
* [Installation](#-installation)
* [BOM & Costs](#-bom--costs-approx-2025)

## 🚀 Features

| Feature | Description | Details |
| :--- | :--- | :--- |
| **Speed-Dependent Oiling** | 5 configurable speed ranges with individual intervals. | Intervals down to **0.1 km**. Pre-configured "Swiss Alpine Profile" (Base 5km, optimized for passes & highways). Default: 2 pulses/event. |
| **Smart Smoothing** | Linear interpolation & low-pass filter. | Avoids harsh jumps in lubrication intervals. |
| **Drift Filter** | Ignores GPS multipath reflections. | Prevents "ghost mileage" indoors/tunnels (HDOP > 5.0 or < 5 Sats). |
| **Safety Cutoff** | Hard limit for pump runtime. | Max 30s continuous run to prevent hardware damage. |
| **Start Delay** | Distance driven before first oiling. | Default **250 m**. Keeps garage floor clean. |
| **GPS Precision** | Exact distance measurement. | Uses TinyGPS++ library. |
| **Rain Mode** | Doubles oil amount in wet conditions. | **Button:** 1x Click. **Auto-Off:** 30 min or restart. |
| **Chain Flush Mode** | Intensive oiling for cleaning/re-lubing. | **Button:** 4x Click. **Action:** Time-based (Configurable). LED: Cyan Blink. |
| **Offroad Mode** | Time-based oiling for slow offroad riding. | **Button:** 3x Click. **Action:** Time-based (e.g. 5 min). LED: Magenta Blink. |
| **Emergency Mode** | Simulates speed if GPS fails. | **Auto:** After 3 min no signal (50 km/h sim). **Forced:** Manual activation (resets on reboot). |
| **WiFi & WebUI** | Configuration via Smartphone. | **Activation:** 5x Click. **Features:** OTA Update, LED config, Stats, Test functions. |
| **Night Mode** | Auto-dimming of LED. | Based on GPS time. Separate brightness for events. |
| **Bleeding Mode** | Continuous pumping for maintenance. | **WebUI:** "Start Bleeding Mode" Button. Fills oil line (15s). |
| **Tank Monitor** | Virtual oil level tracking. | Warns (Red 2x blink) when low. Configurable capacity & consumption. |
| **Aux Port Manager** | Smart control for accessories. | **Aux Power:** Auto-ON after boot (Delay). **Heated Grips:** Auto-PWM based on Speed/Temp/Rain. **Toggle:** Hold > 2s. |
| **Web Console** | Debugging without USB. | View live logs (GPS, Oiler, System) via WiFi on `/console`. |
| **Advanced Stats** | Usage analysis. | Usage % per speed range, total juice counts, odometer. |
| **Auto-Save** | Persistent storage. | Saves settings & odometer to NVS at standstill (< 7 km/h). |
| **Factory Reset** | Reset to defaults. | **WebUI:** Maintenance Page. **Hardware:** Hold button > 10s at boot. |

## 🧭 Optional IMU Features

By adding a **BNO085** (or BNO080) 9-Axis IMU, the system gains "Intelligence" and advanced safety features. The system automatically detects if the sensor is connected.

| Feature | Description | Benefit |
| :--- | :--- | :--- |
| **Garage Guard** | Detects if the bike is on the **Side Stand** (> 10° lean) or **Center Stand** (Calibrated position). | Prevents accidental oiling in the garage, even if the motor is running or GPS drifts. |
| **Crash Detection** | Detects tip-overs (> 70° lean) or accidents. | Immediate safety cutoff for the pump. **Requires restart to reset.** |
| **Smart Stop** | Detects standstill via accelerometer before GPS reacts. | More precise oiling stops at traffic lights. |
| **Telemetry Logger** | Records max lean angles (L/R), max acceleration, and braking G-forces. | Fun stats for the web interface ("How deep was I in that corner?"). |
| **Dynamic Intervals** | Analyzes riding style (Cruising vs. Racing). | Reduces oiling intervals during aggressive riding (high load on chain). |

*   **Calibration:** A dedicated Web Interface page allows zeroing the sensor and calibrating the "Parked" positions.
*   **Logging:** All IMU data will be logged to the SD card (if active) for analysis.

## 💾 Optional SD Card Logging

For advanced users and data analysis, you can connect a standard **MicroSD Card Module** (SPI). The system automatically detects the card at boot and starts logging telemetry data.

*   **File Format:** CSV (Comma Separated Values)
*   **Interval:** 1 Second
*   **Files:** `log_X.csv` (Increments automatically)

### Wiring (VSPI Default)

| Pin | ESP32 GPIO |
| :--- | :--- |
| **CS** | GPIO 5 |
| **MOSI** | GPIO 23 |
| **MISO** | GPIO 19 |
| **CLK** | GPIO 18 |
| **VCC** | 5V |
| **GND** | GND |

### Logged Data

| Column | Description |
| :--- | :--- |
| **Type** | Event type (DATA = Periodic, EVENT = System Event) |
| **Time_ms** | System uptime in milliseconds |
| **Speed_GPS** | Raw GPS speed (km/h) |
| **Speed_Smooth** | Smoothed speed used for calculation (km/h) |
| **Odo_Total** | Total odometer reading (km) |
| **Dist_Accum** | Distance accumulated towards next oiling (km) |
| **Target_Int** | Current target interval (km) |
| **Pump_State** | 1 = Pump running, 0 = Idle |
| **Rain_Mode** | 1 = Active, 0 = Inactive |
| **Temp_C** | Current temperature (°C) |
| **Sats** | Number of satellites |
| **HDOP** | GPS Precision (Lower is better) |
| **Message** | System messages or debug info |
| **Flush_Mode** | 1 = Active, 0 = Inactive |

## 🛠️ Hardware & Functionality Matrix

This table shows which features are available depending on the connected hardware components.

| Feature | Basic Setup | + Temp Sensor | + IMU (MPU6050) | + SD Card |
| :--- | :---: | :---: | :---: | :---: |
| **Speed-Dependent Oiling** | ✅ | ✅ | ✅ | ✅ |
| **Rain Mode** | ✅ | ✅ | ✅ | ✅ |
| **Chain Flush Mode** | ✅ | ✅ | ✅ | ✅ |
| **Cross-Country Mode** | ✅ | ✅ | ✅ | ✅ |
| **Emergency Mode** | ✅ | ✅ | ✅ | ✅ |
| **Night Mode** | ✅ | ✅ | ✅ | ✅ |
| **Tank Monitor** | ✅ | ✅ | ✅ | ✅ |
| **Temp. Compensation** | ❌ | ✅ | ✅ | ✅ |
| **Turn Safety (Cornering)** | ❌ | ❌ | ✅ | ✅ |
| **Crash Detection** | ❌ | ❌ | ✅ | ✅ |
| **Garage Guard** | ❌ | ❌ | ✅ | ✅ |
| **Smart Stop** | ❌ | ❌ | ✅ | ✅ |
| **Aux: Aux Power** | ✅ | ✅ | ✅ | ✅ |
| **Aux: Heated Grips** | ❌ | ✅ | ✅ | ✅ |
| **Data Logging** | ❌ | ❌ | ❌ | ✅ |

*   **Basic Setup:** ESP32, GPS Module, Pump, Button, LED.
*   **Temp Sensor:** DS18B20 (OneWire).
*   **IMU:** BNO085 (I2C).
*   **SD Card:** MicroSD Module (SPI).

## 🌡️ Automatic Temperature Compensation

This system uses a **DS18B20** temperature sensor to adjust the pump mechanics based on the viscosity of the oil.

**Why?**
Oil becomes thick like syrup in winter and thin like water in summer. Without compensation, the pump might fail to move cold oil or squirt too much hot oil.

**New Simplified Logic (Arrhenius):**
Instead of complex tables, the system uses the Arrhenius equation to model oil viscosity changes.

*   **Reference:** You set the optimal Pulse and Pause duration for **25°C** (Normal Temperature).
    *   *Recommendation:* Use a long pause (~2000ms) and adjust the pulse width. This ensures the pump always has enough time to refill, even with thick, cold oil.
*   **Oil Profile:** Select your oil type (Thin, Normal, Thick).
*   **Calculation:** The ESP32 automatically calculates the required energy for any temperature.
*   **Safety Limits:**
    *   **Minimum Pulse:** The system enforces a hard limit of **50ms** for the pulse duration to ensure the solenoid always moves, preventing "stuck" states at high temperatures.
*   **Update Interval:** Temperature is measured every 15 minutes to ensure stable operation.

**Configuration:**
*   **Web Interface:** Simply enter your 25°C values and select the oil type.
*   **Sensor Detection:** If no sensor is connected, the system defaults to 25°C.

**Wiring:**
*   **VCC:** Board dependent (e.g. 5V-12V for Wide Range boards, or 3.3V/5V for standard)
*   **GND:** GND
*   **Data:** GPIO 15 (Default) - Requires 4.7kΩ Pull-Up Resistor to VCC.

## 🧭 IMU Features (BNO085)

If an IMU is connected, the system enables advanced safety and convenience features:

*   **Turn Safety:** Prevents oiling while significant cornering.
    *   **Logic:** Oiling is paused if the bike leans > 20° towards the **tire side** (unsafe zone). Leaning towards the chain side is considered safe (oil drops on chain, not tire).
*   **Crash Detection:** Stops the pump immediately if the lean angle exceeds 60° (bike on ground).
*   **Garage Guard:** Detects if the bike is on the side stand or center stand and prevents oiling (even if GPS drifts).
*   **Smart Stop:** Uses accelerometer data to detect standstill faster than GPS.

## 🔌 Aux Port Manager (GPIO 17)

The system features a versatile Auxiliary Output (MOSFET/Relay driver) that can be configured for two main purposes:

### 1. Aux Power (Switched Live)
Turns on your accessories (Navigation, USB, Lights) automatically when the ignition is ON.
*   **Logic:** Activates the output after a configurable delay when the ESP boots.
*   **Start Delay:** Configurable delay (default 15s) to protect the battery during engine cranking.
*   **Benefit:** No need to tap into the bike's wiring harness or ignition switch. Connect directly to the battery via the Chain Juicer.

### 2. Automated Heated Grips
Advanced PWM control for heated grips, far superior to simple "Low/High" switches.
*   **Hardware Recommendation:** [Coolride Heating Cartridges](https://www.coolride.de/Neue-Heizpatronen-Paar-einzeln-ohne-Schalter) (52W).
*   **Base Level:** Set your preferred minimum heat.
*   **Speed Compensation:** Increases heat as you ride faster (Wind chill factor). Configurable: Low/Medium/High.
*   **Temp Compensation:** Increases heat as it gets colder (requires Temp Sensor). Configurable: Low/Medium/High.
*   **Start Temp:** Configurable threshold (default 20°C) below which the grips start working.
*   **Temp Offset:** Adjusts the sensor reading if placed near heat sources (e.g. engine).
*   **Rain Boost:** Automatically adds extra heat when Rain Mode is active.
*   **Startup Boost:** Heats up quickly (e.g. 80% for 60s) when you start the ride.
*   **Start Delay:** Configurable delay (default 15s) after boot before grips turn on to protect the battery.

**Configuration:**
All parameters (Base %, Speed Factor, Temp Factor, Boosts, Delays) are fully configurable via the new "Aux Config" web page.

## 🛠 Hardware

*   **MCU:** [LCTECH ESP32 Relay X1](http://www.chinalctech.com/cpzx/Programmer/Relay_Module/866.html).
    *   *Specs:* ESP32-WROOM-32E, Wide Range Input (7-30V), Onboard MOSFET (**NCE6020AK**).
*   **GPS:** ATGM336H or NEO-6M (UART, 9600 Baud)
*   **Pump:** [12V Dosing Pump](https://de.aliexpress.com/item/1005010375479436.html).
    *   *Durability:* Tested with 2 pumps, each > 100,000 strokes without failure.
    *   *Tip:* Use my [Smart Pump Calibrator](https://github.com/TechwriterSSchmidt/Smart-Pump-Calibrator) to find the perfect settings for your pump.
*   **Temp Sensor:** DS18B20 (Waterproof)
*   **IMU (Optional):** BNO085 (or BNO080) for lean angle detection and standstill logic.
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
| **Temp Sensor** | GPIO 15 | DS18B20 Data (with 4k7 resistor between VCC and Data!) |
| **IMU SDA** | GPIO 21 | I2C Data |
| **IMU SCL** | GPIO 22 | I2C Clock |

*(Configurable in `include/config.h`)*

### MOSFET Wiring (Direct Drive)

To prevent the pump from triggering briefly during boot, desolder the little driver transistor of one output MOSFET (NCE6020AK) and connect the Gate of the MOSFET via a 220 Ohm resistor directly to the GPIO of the ESP. Add a Pull-Down resistor to stabilize the MOSFET. This modification may not be necessary for the Aux Port - your choice!

**Recommended Fuses:**
*   **Main Fuse (Battery):** 15A (Blue) - Protects the entire system.
*   **Aux Circuit (Grips):** 7.5A (Brown) - Protects the heated grips wiring (Load ~5.2A).
*   **Pump Circuit:** 5A (Tan) - Protects the pump and its MOSFET (Load ~4A Peak).

**Important:** The 220Ω series resistor and 10kΩ Pull-Down resistor are **mandatory** to prevent the pump from running uncontrollably during the ESP32 boot sequence.

```ascii
ESP32 GPIO (the one that switches the MOSFET
)  ----[ 200R ]----+-----> MOSFET Gate
                               |
                              [existing SMD pull-down resistor with 10k]
                               |
                              GND
```

*   **200R (Series):** Protects the ESP32 pin from current spikes (Gate capacitance).
*   **10k (Pull-Down):** Keeps the Gate at GND (0V) while the ESP32 is booting.

## 📖 Operation

### Button Functions

| Action | Duration | Condition | Function |
| :--- | :--- | :--- | :--- |
| **1x Click** | < 1s | Not in Emergency Mode | **Rain Mode** On/Off (LED: Blue). **Note:** Toggles with 600ms delay. |
| **3x Click** | < 2s | Always | **Offroad Mode** On/Off (LED: Magenta Blink). Time based oiling. |
| **4x Click** | < 2s | Always | **Chain Flush Mode** On/Off (LED: Cyan Blink). Time-based (Configurable). |
| **5x Click** | < 2s | Always | Activate **WiFi & Web Interface** (LED: White pulsing). |
| **Hold** | > 2s | Always | **Aux Port / Grip Heating** Manual Toggle (Override). |
| **Hold at Boot** | > 10s | During Power-On | **Factory Reset** (LED: Yellow -> Red fast blink) |

### Mode Hierarchy & Interactions

If multiple modes are active simultaneously, the system follows this priority logic:

| Priority | Mode | LED Indication | Interaction Notes |
| :--- | :--- | :--- | :--- |
| **1 (Highest)** | **Chain Flush** | **Cyan Blink** | Runs in parallel with Offroad Mode. Requires Speed > 2 km/h. |
| **2** | **Offroad** | **Magenta Blink** | Runs in parallel with Flush Mode. Requires Speed > 7 km/h. |
| **3** | **Emergency** | **Cyan / Orange** | Active if GPS is lost. **Disables Rain Mode.** Note: Flush/Offroad will NOT oil because GPS speed is 0. |
| **4 (Lowest)** | **Rain** | **Blue** | Disabled by Emergency Mode. |

### LED Status Codes

| Color / Pattern | LED 1 (Main System) | LED 2 (Aux Port) |
| :--- | :--- | :--- |
| 🟢 **Green** | Normal Operation (GPS Fix) | Aux Power: **ON** (12V Active) |
| 🔵 **Blue** | Rain Mode Active | Heated Grips: **Level 1** (Low) |
| 🟡 **Yellow** | Oiling Event (Breathing) | Heated Grips: **Level 2** (Medium) |
| 🟠 **Orange** | Tank Warning (2x Blink) | Heated Grips: **Level 3** (High) |
| 🔴 **Red** | Bleeding Mode (Blink) | Heated Grips: **Level 4** (Max) |
| 🌈 **Cycle** | - | Heated Grips: **Boost** (Blue->Red) |
| 🟣 **Magenta** | No GPS (Solid) / Offroad (Blink) | - |
| 🔵 **Cyan** | Emergency Mode (Solid) / Flush (Blink) | - |
| ⚪ **White** | WiFi Config Mode (Pulse) | - |
| ⚫ **Off** | - | Aux Port: **OFF** |

## 📱 Web Interface

Connect to the WiFi network (Default SSID: `ChainJuicer`, no password) after activating it. Open `192.168.4.1` in your browser.

**Adjustable Parameters:**
*   **Intervals:** Distance and pump pulses for 5 speed ranges.
*   **Modes:** Rain Mode, Emergency Mode (Force), Night Mode (Times & Brightness).
*   **LED:** Brightness for Day and Night (in %).
*   **Statistics:**
    *   **Usage %:** Driving profile analysis to optimize intervals.
    *   **Juices:** Counter for oilings per range.
    *   **Reset:** Reset stats or refill tank.

👀 **Screenshots:** Check out the [web-ui examples](web-ui%20screenshots/)  folder to see what the interface looks like.

## 💡 Usage Scenarios & LED Status

The system uses two LEDs to communicate its status.
*   **LED 1 (Main):** System Status, Oiling, Warnings.
*   **LED 2 (Aux):** Status of the Auxiliary Port (Heated Grips / Aux Power).

| Scenario | Trigger / Action | LED 1 (Main) | LED 2 (Aux) | System Behavior |
| :--- | :--- | :--- | :--- | :--- |
| **Normal Ride** | GPS Fix, Speed > 7 km/h | **Green** | *State dependent* | System oils automatically based on speed and configured distance intervals. |
| **Oiling Event** | Distance reached | **Yellow Breathing** | *State dependent* | Pump releases oil pulse. |
| **Tunnel / Signal Loss** | No GPS signal > 3 min | **Cyan** | *State dependent* | Enters **Auto Emergency Mode**. Assumes 50 km/h for oiling. Returns to Green when GPS is back. |
| **Rain Ride** | 1x Click | **Blue** | *State dependent* | **Rain Mode** active. Oiling amount is doubled (or interval halved). Auto-off after 30 min or restart. |
| **Aux: Manual Toggle** | Hold > 2s | *State dependent* | **Off / On** | Manually toggles the Aux Port (Override). |
| **Offroad / Enduro** | 3x Click | **Magenta Blink** | *State dependent* | **Offroad Mode** active. Oils based on time (e.g. every 5 min) instead of distance. |
| **Dust / Flushing** | 4x Click | **Cyan Blink** | *State dependent* | **Chain Flush Mode** active. Oils based on time (e.g. every 60s). Good for flushing dust or after cleaning. |
| **Configuration** | 5x Click | **White Pulse** | *State dependent* | Activates WiFi AP `ChainJuicer`. Open `192.168.4.1` to config. |
| **Tank Empty** | Reserve reached | **Orange 2x Blink** | *State dependent* | **Tank Warning**. Refill tank and reset counter via Web Interface. |
| **Aux: Aux Power** | Ignition ON (Delay) | *State dependent* | **Green** | Aux Port is ON (12V). Powers accessories like Dashcam/Navi. |
| **Aux: Heated Grips** | Auto-Control | *State dependent* | **Blue &rarr; Red** | **Blue:** Low Heat<br>**Yellow:** Medium Heat<br>**Orange:** High Heat<br>**Red:** Max Heat |
| **Hardware Debug** | Pump runs at boot | **Check Wiring!** | **Check Wiring!** | Ensure 10k Pull-Down resistor is installed between Gate and GND. |

### Pin Assignment (Current Config)

| Pin | Function | Note |
| :--- | :--- | :--- |
| **GPIO 16** | Pump (MOSFET) | Active HIGH |
| **GPIO 17** | Aux Port (MOSFET) | Aux Power / Heated Grips |
| **GPIO 32** | LED (WS2812B) | Data In |
| **GPIO 27** | GPS RX | Connect to GPS TX |
| **GPIO 26** | GPS TX | Connect to GPS RX |
| **GPIO 4** | Button | Input Pullup (Active LOW) |
| **GPIO 15** | Temp Sensor | DS18B20 Data (Pullup 4.7k) |
| **GPIO 21** | I2C SDA | IMU Data |
| **GPIO 22** | I2C SCL | IMU Clock |
| **GPIO 34** | Rain Sensor | Analog Input (Reserved) |

## ⚙️ Technical Details

*   **Non-Blocking:** Pump control is asynchronous.
*   **Adaptive Smoothing:** Combination of Lookup Table and Low-Pass Filter.
*   **Smart Oiling (Hysteresis):** Oiling is triggered at **95% of the calculated distance**.
*   **Auto-Save:** The odometer is saved intelligently (at standstill < 7 km/h, but max. every 2 minutes).
*   **Timezone:** Automatic calculation of Central European Time (CET/CEST).

## 💻 Installation

### Option A: Web Installer (Recommended)
Flash the firmware directly from your browser (Chrome/Edge) without installing any software.
👉 **[Open Web Installer](https://TechwriterSSchmidt.github.io/ChainJuicer/)**

### Option B: PlatformIO (For Developers)

**⚠️ Important:** The recommended ESP32-32E board does **not** have a built-in USB-to-Serial converter. You need an external **USB-to-UART Adapter** (e.g., CP2102, CH340) to flash the initial firmware once. For updates, you can use the OTA page to upload new firmware via the webinterface.

*   **Connections:** TX->RX, RX->TX, GND->GND, 3.3V->3.3V (or 5V->5V).
*   **Boot Mode:** You might need to bridge GPIO0 to GND during power-up to enter bootloader mode.

1.  Clone or download the repository.
2.  Open in VS Code with installed **PlatformIO** Extension.
3.  Adjust Upload Port in `platformio.ini` if necessary.
4.  Compile and upload project (`Upload`).
5.  File system is not necessary (Data is saved in NVS/Preferences).

## 🛒 BOM & Costs (approx. 2025)

The total project cost is very low compared to commercial alternatives (~150€+).

| Component | Description | Link (Example) | Approx. Price |
| :--- | :--- | :--- | :--- |
| **ESP32 Board** | LCTECH Relay X1 (NCE6020AK) | [Manufacturer](http://www.chinalctech.com/cpzx/Programmer/Relay_Module/866.html) | ~ 6.00 € |
| **GPS Module** | ATGM336H (Small & Fast) | [AliExpress](https://de.aliexpress.com/item/1005007759554488.html) | ~ 4.50 € |
| **Dosing Pump** | 12V Pulse Pump | [AliExpress](https://de.aliexpress.com/item/1005010375479436.html) | ~ 12.00 € |
| **Oil Tank** | 120cc RC Tank | [AliExpress](https://de.aliexpress.com/item/1588019770.html) | ~ 4.00 € |
| **Dosing Needle** | 14G Needle (100mm) | [AliExpress](https://de.aliexpress.com/item/1005010382140407.html) | ~ 2.00 € |
| **Luer Lock Adapter** | Adapter for Needle | [AliExpress](https://de.aliexpress.com/item/1005003777344843.html) | ~ 1.00 € |
| **TPU Hose** | 2m Oil/Fuel Line | [AliExpress](https://de.aliexpress.com/item/1005005390968147.html) | ~ 3.00 € |
| **Button** | Waterproof Button | [AliExpress](https://de.aliexpress.com/item/1005009185830140.html) | ~ 0.70 € |
| **LED** | WS2812B (Code supports up to 2x) | Generic | ~ 1.00 € |
| **Fuses** | KFZ-Flachsicherungen (15A, 7.5A, 5A) | Generic | ~ 2.00 € |
| **Small Parts** | Automotive Connectors (2x 2pin, 1x 4pin), Wires | Generic | ~ 13.00 € |
| **Housing** | 3D Printed (PETG/ASA) | DIY | ~ 2.00 € |
| **Flasher** | USB-to-UART Adapter (CP2102/CH340) | Generic | ~ 3.00 € |
| **Total (Basic)** | | | **~ 52.50 €** |
| | | | |
| **Temp Sensor (Optional)** | DS18B20 (Waterproof) | [AliExpress](https://de.aliexpress.com/item/1005006090368473.html) | ~ 1.50 € |
| **IMU (Optional)** | BNO085 (9-Axis) | [AliExpress](https://de.aliexpress.com/item/1005009898088463.html) | ~ 15.00 € |
| **SD Card (Optional)** | MicroSD SPI Module | [AliExpress](https://de.aliexpress.com/item/1005005302035188.html) | ~ 1.00 € |
| **Total (Full)** | | | **~ 70.00 €** |

*Note: Prices vary by shipping location and seller.*

### Hardware Notes
*   **LED:** The code is configured for **2x WS2812B LEDs**. You can use one for the status display in the cockpit and a second one (optional) near the tank or pump for debugging/tank warning.
*   **LCTECH Board Modification (Critical):** If you are using the LCTECH Relay Board with the NCE6020AK MOSFET and J3Y driver circuit, you should modify the switching circuit of the pump (only the pump circuit!) to prevent the pump from triggering briefly during boot as follows:
    1.  **Remove:** Transistor `J3Y` (S8050) and Resistors `R8` & `R7`.
    2.  **Keep:** Resistor `R3` (10k Pull-Down).
    3.  **Add:** A **220 Ohm** resistor between GPIO 16 and the Gate of the MOSFET.
    *   *See `Docs/SCHEMATIC.txt` for details.*

## ⚠️ Disclaimer & Safety

**READ CAREFULLY BEFORE BUILDING OR USING THIS SYSTEM!**

*   **DIY Project:** This is a Do-It-Yourself project. You are solely responsible for the assembly, installation, and operation of the system.
*   **No Liability:** The author accepts **no liability** for any damage to your motorcycle, property, or person, nor for any accidents caused by the use or failure of this system.
*   **Oil on Tires:** Chain oilers involve liquids near moving parts. **Oil on the tire significantly reduces traction and can lead to crashes.**
    *   Ensure the dosing needle is positioned correctly so oil drips ONLY on the chain/sprocket, never on the tire or brake disc.
    *   Regularly check the system for leaks.
    *   **IMMEDIATELY clean any oil off the tire** if a spill occurs. Do not ride with oil on your tires.
*   **Road Safety:** Do not operate the configuration interface while riding. Focus on the road.





