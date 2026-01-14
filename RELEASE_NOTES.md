# Release Notes

## v2.0.3 - Logging & State Logic Fixes (2026-01-14)
*   **Enhanced WebConsole Logging:** Added detailed logging for every pump actuation, specifying the active mode (e.g., "normal-mode juicing started", "offroad-mode juicing started"). Flush Mode deactivation is now also logged.
*   **Logic Fix - Mode Transition:** Fixed a bug where the system would get stuck in a "Searching" state after Flush, Bleeding, or Offroad mode ended if no GPS signal was present. It now correctly hands over to Emergency Mode immediately if the timeout condition is met.
*   **Stability:** Fixed compilation errors related to variable scope in LED update logic.

## v2.0.2 - New Color Scheme & Offroad Config (2026-01-14)
*   **New Intuitive LED Colors:**
    *   **Green:** Normal (Ready).
    *   **Yellow (Flash):** Active Oiling Event (Pump Stroke).
    *   **Blue:** Rain Mode.
    *   **Amber/Orange:** Offroad Mode.
    *   **Cyan (Breathing):** GPS Searching.
    *   **Red (Pulsing):** Emergency Mode (No GPS / Forced).
    *   **Red/Yellow (Alternating):** Tank Empty!
    *   **Magenta (Blink):** Maintenance (Flush / Bleed).
*   **Offroad Configuration:** Added "Pulses per Event" setting to Offroad Mode.
*   **Emergency Fix:** Emergency Mode (Forced) no longer auto-disables Offroad Mode (allowing manual timer-based oiling in deep woods).

## v2.0.1 - Bleeding Timing Fix (2026-01-06)
*   **Bleeding Mode:** Fixed a millis underflow race that made the pump run at ~6 Hz despite 60/320 ms settings. Bleeding now uses a simple hard-coded scheduler (pulse 60 ms, pause 320 ms) with an underflow guard. Logging noise was reduced for the web/serial console.

## v2.0.0 - Core Refactoring & Smart Stop (2026-01-05)

This update introduces significant architectural changes to support future platforms (HelLo Juicer) while enhancing the current user experience.

### Core Architecture
*   **Multi-Platform Support:** Refactored the core logic (`Oiler`, `AuxManager`) to be platform-agnostic.
    *   Decoupled storage logic using a new `IPersistence` interface, allowing the same core code to run on ESP32 (using NVS) and nRF52 (using FileSystem).
    *   Abstracted hardware pin definitions from the core library.
*   **ESP32 Compatibility:** Updated the codebase to support **ESP32 Arduino Core 3.x**.
    *   Migrated PWM generation from the deprecated `ledcSetup`/`ledcAttachPin` to the new `ledcAttach` API.
    *   Updated Watchdog Timer (WDT) initialization to the new configuration structure.

## v1.1.2 - IMU Calibration Update + Smart Stop Mode Enhancement (2026-01-04)

Improved the IMU calibration process for better usability and accuracy, especially for solo riders.

*   **Non-Blocking Calibration:** The calibration process no longer freezes the web interface.
*   **Countdown Timer:** Added a 5-second countdown before calibration starts, giving the rider time to stabilize the bike (hands on handlebars).
*   **Averaging:** Calibration now averages sensor data over 3 seconds (instead of a single snapshot) to filter out rider movement and vibrations.
*   **Visual Feedback:** The Web Interface automatically redirects to the Serial Console to display the countdown and calibration status in real-time.
*   
*   ### User Experience (Smart Stop)
Enhanced the visual feedback when the motorcycle is stationary (e.g., at traffic lights) to provide useful status information without distraction.

*   **Smart Stop Mode:** When the bike is stopped (< 3 km/h), the LED switches to a "Pulsing" mode to distinguish it from normal operation.
    *   **Green Pulsing:** System OK.
    *   **Blue Pulsing:** Rain Mode Active.
    *   **Orange Blinking (2x):** Tank Low (Warning Level).
    *   **Red Pulsing:** Tank Empty.
*   **Tank Warning Logic:** The "Smart Stop" mode now correctly respects the user-configured tank warning threshold instead of a fixed value.

## v1.1.1 - Stability & Robustness (2026-01-03)

This patch focuses on system reliability and failsafe mechanisms to ensure the ChainJuicer operates autonomously without hanging, even in case of hardware glitches.

*   **Watchdog Timer (WDT):** Added a system watchdog that automatically resets the ESP32 if the main loop freezes for more than 5 seconds. This prevents the system from hanging indefinitely due to software bugs or electrical interference.
*   **I2C Bus Recovery:** Implemented a recovery sequence for the IMU connection. If the BNO085 sensor locks up the I2C bus (e.g., due to voltage spikes), the system attempts to clear the bus by toggling the clock line before giving up.
*   **Robust Initialization:** Improved the startup sequence to handle missing or malfunctioning sensors gracefully without blocking the rest of the system.

## v1.1.0 - UI & UX Refinement (Released)

This update focuses on streamlining the user experience, cleaning up the Web Interface, and refining system feedback.

### Web Interface Overhaul
*   **New Maintenance Page:** Created a dedicated area for system tools to declutter the main settings page.
    *   **Visual Safety:** "Bleeding Mode" is color-coded yellow (Warning), "Factory Reset" is red (Danger), while "Firmware Update" and "Restart System" are gray (Neutral).
    *   **Functionality:** Includes Pump Test, Bleeding Mode, IMU Configuration, Serial Console, Firmware Update, and System Restart.
*   **Heated Grips Settings:** Reorganized the settings page. Parameters are now logically grouped (Compensation factors vs. Base settings) with visual separation for better usability.
*   **Factory Reset:** Removed the hardware-based Factory Reset (Hold > 10s) to prevent accidental resets. Factory Reset is now exclusively available via the Web Interface.
*   **Layout Improvements:** Moved the "Statistics" card to the top of the Settings page for immediate visibility of key metrics.

### UX & Feedback
*   **Heated Grips Boost:** The LED now cycles through colors (Blue &rarr; Red) during the startup boost phase, providing a more intuitive "heating up" visualization.
*   **Heated Grips Reaction Speed:** Added a configurable "Reaction Speed" setting (Slow/Medium/Fast) to control how quickly the heating adjusts to speed changes. "Slow" (default) provides a very smooth, high-end feel.
*   **Button Logic:** Consolidated button commands for a simplified control scheme.
*   **WiFi Safety:** The 5-click WiFi command now only activates WiFi or extends the timer. It prevents accidental deactivation via button (deactivation is handled automatically by timeout or driving).
*   **Documentation:** Added a clear "Mode Hierarchy" section to the manual, explaining how Chain Flush, Offroad, Emergency, and Rain modes interact.

### Logic Improvements
*   **Aux Port Persistence:** The ON/OFF state of the Aux Port (Heated Grips / Power) is now saved to non-volatile memory. The system remembers if you turned it off manually and restores the state after a reboot.
*   **Heated Grips Startup:** Fixed the startup logic. The "Startup Boost" now correctly begins *after* the configured "Start Delay" has elapsed, ensuring the boost phase isn't consumed while waiting for the engine to stabilize.
*   **Temperature Compensation:** Changed from hysteresis-based to a fixed 15-minute update interval for more stable and predictable viscosity adjustments.
*   **IMU Turn Safety:** Corrected the logic to suppress oiling when leaning towards the **tire** (unsafe side). When leaning towards the chain, oiling is now permitted.
*   **Console Logging:** Added comprehensive logging of all physical button presses and WebUI actions to the Web Console for better diagnostics.
*   **Bleeding Stats:** Fixed an issue where oil consumption during Bleeding Mode was not logged. The total amount consumed is now displayed in the console upon completion.
*   **Persistent Emergency Mode:** The "Forced Emergency Mode" setting is now saved to non-volatile memory. This allows the mode to persist across system restarts, which is critical for long trips with a broken GPS module.
*   **System Tools Feedback:** Fixed "Restart System" and "Factory Reset" buttons. They now require a **3x click confirmation** to prevent accidental activation. The buttons display a countdown (e.g., "1/3") and are reordered for better safety.
*   **Additive Bleeding Mode:** The Bleeding Mode can now be extended by triggering it again while active. Each trigger adds 20 seconds to the duration (up to a maximum of 60 seconds / 3x). This allows for easier priming of long oil lines without restarting the process repeatedly.
*   **Pump Noise Reduction:** Optimized the PWM ramp-up/ramp-down logic to start at ~50% duty cycle. This eliminates the motor "whining" sound at low power levels where torque is insufficient to move the pump.
*   **UI Visibility:** Updated the "Home" button arrow in the Web Interface to a bold `<` symbol for better rendering on iOS devices.
*   **Bleeding Feedback:** Added a live countdown timer to the Serial Console during Bleeding Mode (e.g., "Bleeding... 15s").
*   **Heated Grips Boost Logic:** Improved the startup boost logic. The boost timer now resets correctly when the grips are manually enabled during a ride, ensuring the "quick heat" feature works every time they are switched on, not just at boot.
*   **Garage Guard Logic:** Replaced the complex lean-angle based "Parked" detection with a robust "Stability Check". The system now detects if the bike is stationary (variance check) for 5 seconds, regardless of lean angle (Side Stand vs. Center Stand).

## v1.0.0 - Initial Release (2025-12-29)

**Chain Juicer** is a multi-tool for motorcycle chain maintenance. This initial release brings a complete suite of features for intelligent chain lubrication and accessory management, designed for the ESP32 platform.

### Key Features

*   **Speed-Dependent Oiling:** 5 configurable speed ranges with individual intervals (down to 0.1 km). Includes the pre-configured "Swiss Alpine Profile".
*   **Smart Smoothing:** Linear interpolation and low-pass filtering for smooth operation.
*   **GPS Precision:** Accurate distance measurement with a **Drift Filter** to ignore multipath reflections (tunnels/indoors).
*   **Rain Mode:** One-click activation to double the oil amount. Auto-off after 30 minutes or reboot.
*   **Chain Flush Mode:** Intensive, time-based oiling for cleaning the chain or post-rain maintenance.
*   **Offroad Mode:** Time-based oiling intervals for slow, technical riding where distance is minimal.
*   **Emergency Mode:** Automatic fallback to simulated speed (50 km/h) if GPS signal is lost for > 3 minutes.
*   **Temperature Compensation:** Automatically adjusts pump pulse duration based on ambient temperature and selected oil viscosity profile (Thin, Normal, Thick).

### Aux Port Manager

*   **Heated Grips Control:** Automated PWM control based on speed (wind chill), temperature, and rain status. Includes a "Boost" mode for rapid heating.
*   **Aux Power:** Switched 12V output with configurable start delay to protect the battery during cranking.

### Web Interface & Connectivity

*   **Responsive WebUI:** Dark mode interface for configuration, statistics, and monitoring.
*   **WiFi Management:** WiFi is off by default to save power; activated via button sequence (5x click). Auto-off when driving.
*   **Maintenance Page:** Dedicated tools for pump testing, system bleeding, and factory reset.
*   **OTA Updates:** Wireless firmware updates directly from the browser.
*   **Web Console:** Live serial logging via the web interface for debugging without USB.

### Safety & Monitoring

*   **Tank Monitor:** Virtual oil level tracking with LED warning (Red 2x blink) when low.
*   **Safety Cutoff:** Hard limit on pump runtime to prevent hardware damage.
*   **IMU Integration (Optional):**
    *   **Garage Guard:** Prevents oiling when parked on the side stand (> 10° lean).
    *   **Crash Detection:** Stops the pump immediately if a tip-over (> 70° lean) is detected.
    *   **Smart Stop:** Detects standstill via accelerometer for faster response than GPS.

### Logging

*   **Statistics:** Tracks total distance, pump cycles, and usage percentage per speed range.
*   **SD Card Logging (Optional):** Records detailed telemetry (Speed, GPS, IMU, Pump State) to CSV files for analysis.
