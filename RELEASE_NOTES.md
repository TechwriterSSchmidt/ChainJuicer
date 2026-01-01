# Release Notes

## v1.1.0 - UI & UX Refinement (Work in Progress)

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
*   **System Tools Feedback:** Fixed "Restart System" and "Factory Reset" buttons. They now redirect to the console and display a 3-second countdown before executing, providing clear visual confirmation of the action.

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
