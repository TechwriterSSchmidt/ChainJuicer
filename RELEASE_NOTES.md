# Release Notes

## v1.1.0 - UI & UX Refinement (Work in Progress)

This update focuses on streamlining the user experience, cleaning up the Web Interface, and refining system feedback.

### Web Interface Overhaul
*   **New Maintenance Page:** Created a dedicated area for system tools to declutter the main settings page.
    *   Includes: Pump Test, Bleeding Mode, IMU Configuration, Serial Console, Firmware Update, and System Restart.
*   **Factory Reset:** Added a software-based Factory Reset button to the Maintenance page (no need to hold the physical button at boot).
*   **Layout Improvements:** Moved the "Statistics" card to the top of the Settings page for immediate visibility of key metrics.

### UX & Feedback
*   **Heated Grips Boost:** The LED now cycles through colors (Blue &rarr; Red) during the startup boost phase, providing a more intuitive "heating up" visualization.
*   **Button Logic:** Consolidated button commands for a simplified control scheme.
*   **Documentation:** Added a clear "Mode Hierarchy" section to the manual, explaining how Chain Flush, Offroad, Emergency, and Rain modes interact.

## v1.0.0 - Initial Release (2026-01-01)

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
