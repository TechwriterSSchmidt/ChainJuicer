# 🍋 Chain Juicer (ESP32)

An advanced, GPS-controlled chain oiler for motorcycles based on the ESP32. The system dynamically adjusts oiling intervals based on the driven speed and offers extensive configuration options via a web interface.

## Support my projects

Your tip motivates me to continue developing cool stuff for the DIY community. Thank you very much for your support!

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/D1D01HVT9A)

## 🚀 Features

| Feature | Description | Details |
| :--- | :--- | :--- |
| **Speed-Dependent Oiling** | 5 configurable speed ranges with individual intervals. | Intervals down to **0.1 km**. Pre-configured progression (Base 5km, reducing >100km/h). Default: 2 pulses/event. |
| **Smart Smoothing** | Linear interpolation & low-pass filter. | Avoids harsh jumps in lubrication intervals. |
| **Drift Filter** | Ignores GPS multipath reflections. | Prevents "ghost mileage" indoors/tunnels (HDOP > 5.0 or < 5 Sats). |
| **Safety Cutoff** | Hard limit for pump runtime. | Max 30s continuous run to prevent hardware damage. |
| **Start Delay** | Distance driven before first oiling. | Default **2.0 km**. Keeps garage floor clean. |
| **GPS Precision** | Exact distance measurement. | Uses TinyGPS++ library. |
| **Rain Mode** | Doubles oil amount in wet conditions. | **Button:** Short Press. **Auto-Off:** 30 min or restart. **Rain Flush:** 6 pulses when turning off (if moving). |
| **Turbo Mode** | Intensive oiling for cleaning/re-lubing. | **Button:** 3x Click. **Action:** Every 1 km for 15 min. LED: Cyan Blink. |
| **Cross-Country Mode** | Time-based oiling for slow offroad riding. | **Button:** 6x Click. **Action:** Time-based (e.g. 5 min). LED: Magenta Blink. |
| **Emergency Mode** | Simulates speed if GPS fails. | **Auto:** After 3 min no signal (50 km/h sim). **Forced:** Manual activation (resets on reboot). |
| **WiFi & WebUI** | Configuration via Smartphone. | **Activation:** Hold button >3s at standstill. **Features:** OTA Update, LED config, Stats, Test functions. |
| **Night Mode** | Auto-dimming of LED. | Based on GPS time. Separate brightness for events. |
| **Bleeding Mode** | Continuous pumping for maintenance. | Fills oil line. |
| **Tank Monitor** | Virtual oil level tracking. | Warns (Red 2x blink) when low. Configurable capacity & consumption. |
| **Advanced Stats** | Usage analysis. | Usage % per speed range, total juice counts, odometer. |
| **Auto-Save** | Persistent storage. | Saves settings & odometer to NVS at standstill (< 7 km/h). |
| **Factory Reset** | Reset to defaults. | Hold button at boot. |

## 🧭 Optional IMU Features

By adding a **BNO085** (or BNO080) 9-Axis IMU, the system gains "Intelligence" and advanced safety features. The system automatically detects if the sensor is connected.

| Feature | Description | Benefit |
| :--- | :--- | :--- |
| **Garage Guard** | Detects if the bike is on the **Side Stand** (> 10° lean) or **Center Stand** (Calibrated position). | Prevents accidental oiling in the garage, even if the motor is running or GPS drifts. |
| **Crash Detection** | Detects tip-overs (> 60° lean) or accidents. | Immediate safety cutoff for the pump. |
| **Smart Stop** | Detects standstill via accelerometer before GPS reacts. | More precise oiling stops at traffic lights. |
| **Telemetry Logger** | Records max lean angles (L/R), max acceleration, and braking G-forces. | Fun stats for the web interface ("How deep was I in that corner?"). |
| **Dynamic Intervals** | Analyzes riding style (Cruising vs. Racing). | Reduces oiling intervals during aggressive riding (high load on chain). |

*   **Calibration:** A dedicated Web Interface page allows zeroing the sensor and calibrating the "Parked" positions.
*   **Logging:** All IMU data will be logged to the SD card (if active) for analysis.

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
*   **Hysteresis:** A 3.0°C buffer prevents rapid switching.

**Configuration:**
*   **Web Interface:** Simply enter your 25°C values and select the oil type.
*   **Sensor Detection:** If no sensor is connected, the system defaults to 25°C.

**Wiring:**
*   **VCC:** Board dependent (e.g. 5V-12V for Wide Range boards, or 3.3V/5V for standard)
*   **GND:** GND
*   **Data:** GPIO 15 (Default) - Requires 4.7kΩ Pull-Up Resistor to VCC.

## 🛠 Hardware

*   **MCU:** [ESP32-32E Board](https://de.aliexpress.com/item/1005007481509548.html?spm=a2g0o.detail.pcDetailTopMoreOtherSeller.1.7ee51GVe1GVen4&gps-id=pcDetailTopMoreOtherSeller&scm=1007.40050.354490.0&scm_id=1007.40050.354490.0&scm-url=1007.40050.354490.0&pvid=7a27ce65-0636-4528-ad57-32ebfd2405a5&_t=gps-id:pcDetailTopMoreOtherSeller,scm-url:1007.40050.354490.0,pvid:7a27ce65-0636-4528-ad57-32ebfd2405a5,tpp_buckets:668%232846%238109%231935&pdp_ext_f=%7B%22order%22%3A%2233%22%2C%22eval%22%3A%221%22%2C%22sceneId%22%3A%2230050%22%2C%22fromPage%22%3A%22recommend%22%7D&pdp_npi=6%40dis%21CHF%212.32%212.09%21%21%2120.23%2118.21%21%402103867617669674178075350e6907%2112000043231287453%21rec%21CH%211702792198%21X%211%210%21n_tag%3A-29919%3Bd%3A94cdc6e3%3Bm03_new_user%3A-29895&utparam-url=scene%3ApcDetailTopMoreOtherSeller%7Cquery_from%3A%7Cx_object_id%3A1005007481509548%7C_p_origin_prod%3A) (with MOSFET & Wide Range Input).
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
| **Temp Sensor** | GPIO 15 | DS18B20 Data |
| **IMU SDA** | GPIO 21 | I2C Data |
| **IMU SCL** | GPIO 22 | I2C Clock |

*(Configurable in `include/config.h`)*

### MOSFET Wiring (Direct Drive)

To prevent the pump from triggering briefly during boot (floating pin), use a Logic Level MOSFET (e.g., NCE6020AK) and a Pull-Down resistor. Do **not** use an NPN driver stage (Emitter Follower) as it may cause threshold voltage issues with 3.3V logic.

**Important:** The 200Ω series resistor and 10kΩ Pull-Down resistor are **mandatory** to prevent the pump from running uncontrollably during the ESP32 boot sequence.

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

## 📖 Operation

### Button Functions

| Action | Duration | Condition | Function |
| :--- | :--- | :--- | :--- |
| **Short Press** | < 1.5s | Not in Emergency Mode | **Rain Mode** On/Off (LED: Blue). **Note:** Toggles with 400ms delay. Turning OFF triggers **Rain Flush** (6 pulses) if enabled and moving. |
| **3x Click** | < 2s | Always | **Turbo Mode** On/Off (LED: Cyan Blink). 15 min @ 1km. |
| **6x Click** | < 3s | Always | **Cross-Country Mode** On/Off (LED: Magenta Blink). Time based oiling. |
| **Hold** | > 3s | At Standstill (< 7 km/h) | Activate **WiFi & Web Interface** (LED: White pulsing) |
| **Long Hold** | > 10s | At Standstill (< 7 km/h) & No Emergency | Start **Bleeding Mode** (LED: Red blinking, pump runs 15s @ 80ms/250ms) |
| **Hold at Boot** | > 10s | During Power-On | **Factory Reset** (LED: Yellow -> Red fast blink) |

### LED Status Codes

*   🟢 **Green:** Normal Operation (GPS Fix available)
*   🔵 **Blue:** Rain Mode Active
*   🔵 **Cyan (blinking):** Turbo Mode Active
*   🟣 **Magenta (blinking):** Cross-Country Mode Active
*   🟣 **Magenta:** No GPS Signal (Searching...)
*   🔵 **Cyan:** Emergency Mode (Auto: No GPS > 3 min)
*   🟠 **Orange (pulsing):** Emergency Mode (Forced: GPS available)
*   🟡 **Yellow:** Oiling in progress (lit for 3s, breathes 3x)
*   ⚪ **White (pulsing):** WiFi Configuration Mode active
*   🟠 **Orange (2x blink):** Tank Warning (Reserve reached)
*   🔴 **Red (blinking):** Bleeding Mode active
*   🔵 **Cyan (fast blink):** Firmware Update in progress

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
2.  Check MOSFET: Ensure you are using a Logic Level MOSFET (Vgs(th) < 2.5V) and not a standard MOSFET or BJT Darlington without proper bias.

## ⚙️ Technical Details

*   **Non-Blocking:** Pump control is asynchronous.
*   **Adaptive Smoothing:** Combination of Lookup Table and Low-Pass Filter.
*   **Smart Oiling (Hysteresis):** Oiling is triggered at **95% of the calculated distance**.
*   **Auto-Save:** The odometer is saved intelligently (at standstill < 7 km/h, but max. every 2 minutes).
*   **Timezone:** Automatic calculation of Central European Time (CET/CEST).

## 💻 Installation

### Option A: Web Installer (Recommended)
Flash the firmware directly from your browser (Chrome/Edge) without installing any software.
👉 **[Open Web Installer](https://TechwriterSSchmidt.github.io/ChainJuicer-light/)**

### Option B: PlatformIO (For Developers)

**⚠️ Important:** The recommended ESP32-32E board usually does **not** have a built-in USB-to-Serial converter. You need an external **USB-to-UART Adapter** (e.g., CP2102, CH340) to flash the firmware!
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
| **ESP32 Board** | ESP32-32E (MOSFET & Wide Range) | Generic | ~ 6.00 € |
| **GPS Module** | ATGM336H (Small & Fast) | [AliExpress](https://de.aliexpress.com/item/1005007759554488.html) | ~ 4.50 € |
| **Dosing Pump** | 12V Pulse Pump | [AliExpress](https://de.aliexpress.com/item/1005010375479436.html) | ~ 12.00 € |
| **Oil Tank** | 120cc RC Tank | [AliExpress](https://de.aliexpress.com/item/1588019770.html) | ~ 4.00 € |
| **Dosing Needle** | 14G Needle (100mm) | [AliExpress](https://de.aliexpress.com/item/1005010382140407.html) | ~ 2.00 € |
| **Luer Lock Adapter** | Adapter for Needle | [AliExpress](https://de.aliexpress.com/item/1005003777344843.html) | ~ 1.00 € |
| **TPU Hose** | 2m Oil/Fuel Line | [AliExpress](https://de.aliexpress.com/item/1005005390968147.html) | ~ 3.00 € |
| **Temp Sensor** | DS18B20 (Waterproof) | [AliExpress](https://de.aliexpress.com/item/1005006090368473.html) | ~ 1.50 € |
| **Button** | Waterproof Button | [AliExpress](https://de.aliexpress.com/item/1005009185830140.html) | ~ 0.70 € |
| **LED** | WS2812B (Code supports up to 2x) | Generic | ~ 1.00 € |
| **Small Parts** | Automotive Connectors (2x 2pin, 1x 4pin), Wires, Fuse | Generic | ~ 15.00 € |
| **Housing** | 3D Printed (PETG/ASA) | DIY | ~ 2.00 € |
| **Flasher** | USB-to-UART Adapter (CP2102/CH340) | Generic | ~ 3.00 € |
| **Total (Basic)** | | | **~ 54.00 €** |
| | | | |
| **IMU (Optional)** | BNO085 (9-Axis) | [AliExpress](https://de.aliexpress.com/item/1005009898088463.html) | ~ 15.00 € |
| **Total (Full)** | | | **~ 69.00 €** |

*Note: Prices vary by shipping location and seller.*

### Hardware Notes
*   **LED:** The code is configured for **2x WS2812B LEDs**. You can use one for the status display in the cockpit and a second one (optional) near the tank or pump for debugging/tank warning.
*   **Resistors:** Don't forget the **200R** (Series) and **10k** (Pull-Down) for the MOSFET gate if your board doesn't have them integrated!




