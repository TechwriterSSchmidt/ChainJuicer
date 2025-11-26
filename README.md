# GPS Chain Oiler (ESP32)

Ein fortschrittlicher, GPS-gesteuerter Kettenöler für Motorräder auf Basis des ESP32. Das System passt die Ölungsintervalle dynamisch an die gefahrene Geschwindigkeit an und bietet umfangreiche Konfigurationsmöglichkeiten über ein Webinterface.

## 🚀 Features

*   **Geschwindigkeitsabhängige Ölung:** 5 konfigurierbare Geschwindigkeitsbereiche mit individuellen Intervallen (km) und Pump-Impulsen.
*   **GPS-Präzision:** Exakte Distanzmessung via GPS-Modul (TinyGPS++).
*   **Regenmodus:** Verdoppelt die Ölmenge bei Nässe. Aktivierbar per Taster. Automatische Abschaltung nach 30 Minuten.
*   **Notlauf-Modus (Emergency Mode):** Simuliert eine Geschwindigkeit von 50 km/h bei GPS-Ausfall, um die Kettenschmierung sicherzustellen.
*   **Smart WiFi & Webinterface:**
    *   Konfiguration aller Parameter bequem per Smartphone.
    *   Aktivierung nur im Stand (Sicherheitsfeature).
    *   Automatische Abschaltung bei Fahrtantritt (> 10 km/h) oder Inaktivität.
*   **Nachtmodus:** Automatische Dimmung der Status-LED basierend auf der GPS-Uhrzeit (inkl. Sommerzeit-Korrektur).
*   **Entlüftungsmodus (Bleeding):** Dauerpumpen zum Füllen der Ölleitung nach Wartungsarbeiten.
*   **Datensicherheit:** Kilometerstand und Einstellungen werden dauerhaft im Flash-Speicher (NVS) gespeichert.
*   **Sicherheit:** Watchdog-Timer (WDT) und Schutz vor ungewolltem Pumpen beim Booten.

## 🛠 Hardware

*   **MCU:** ESP32 Development Board
*   **GPS:** NEO-6M oder kompatibles Modul (UART)
*   **Pumpe:** Dosierpumpe (angesteuert über MOSFET/Transistor)
*   **LED:** WS2812B (NeoPixel) für Statusanzeige
*   **Taster:** Schließer gegen GND (Input Pullup)

### Pinbelegung (Standard)

| Komponente | ESP32 Pin | Beschreibung |
| :--- | :--- | :--- |
| **Pumpe** | GPIO 23 | MOSFET Gate |
| **GPS RX** | GPIO 16 | Verbunden mit GPS TX |
| **GPS TX** | GPIO 17 | Verbunden mit GPS RX |
| **Taster** | GPIO 4 | Gegen GND geschaltet |
| **LED** | GPIO 5 | WS2812B Data In |

*(Konfigurierbar in `include/config.h`)*

## 📖 Bedienung

### Taster-Funktionen

| Aktion | Dauer | Bedingung | Funktion |
| :--- | :--- | :--- | :--- |
| **Kurz drücken** | < 1.5s | Immer | **Regenmodus** Ein/Aus (LED: Blau) |
| **Halten** | > 3s | Im Stand (< 5 km/h) | **WiFi & Webinterface** aktivieren (LED: Weiß blinkend) |
| **Lang halten** | > 10s | Immer | **Entlüftungsmodus** (Bleeding) starten (LED: Rot blinkend) |

### LED Status-Codes

*   🟢 **Grün:** Normalbetrieb (GPS Fix vorhanden)
*   🔵 **Blau:** Regenmodus Aktiv
*   🟣 **Magenta:** Kein GPS Signal (Suche...)
*   🔵 **Cyan:** Emergency Mode (Kein GPS, Simulation aktiv)
*   🟡 **Gelb:** Ölung läuft (Pumpe aktiv)
*   ⚪ **Weiß (blinkend):** WiFi Konfigurations-Modus aktiv
*   🔴 **Rot (blinkend):** Entlüftungsmodus (Bleeding) aktiv

## 📱 Webinterface

Verbinden Sie sich mit dem WiFi-Netzwerk (Standard-SSID: `MotoOiler_Config`, kein Passwort), nachdem Sie es aktiviert haben. Rufen Sie im Browser `192.168.4.1` auf (oder warten Sie auf das Captive Portal).

**Einstellbare Parameter:**
*   **Intervalle:** Distanz und Pumpstöße für 5 Geschwindigkeitsbereiche.
*   **Modi:** Regenmodus, Emergency Mode, Nachtmodus (Zeiten & Helligkeit).
*   **LED:** Helligkeit für Tag und Nacht.
*   **Statistik:** Gesamtkilometerzähler und Pump-Zyklen (Reset möglich).

## ⚙️ Technische Details

*   **Non-Blocking:** Die Ansteuerung der Pumpe erfolgt asynchron. GPS-Daten werden auch während des Pumpens weiter verarbeitet.
*   **Smart Oiling (Hysterese):** Die Ölung wird bereits bei **95% der errechneten Distanz** ausgelöst. Diese Sicherheitsmarge verhindert "Double Oiling" bei Geschwindigkeitswechseln und stellt sicher, dass Intervalle zuverlässig eingehalten werden.
*   **Speicherschutz:** Der Kilometerstand wird intelligent gespeichert (bei Stillstand, aber max. alle 2 Minuten), um den Flash-Speicher zu schonen.
*   **Zeitzone:** Automatische Berechnung der Mitteleuropäischen Zeit (MEZ/MESZ) basierend auf dem GPS-Datum.

## 💻 Installation (PlatformIO)

1.  Repository klonen oder herunterladen.
2.  In VS Code mit installierter **PlatformIO** Extension öffnen.
3.  Ggf. Upload-Port in `platformio.ini` anpassen.
4.  Projekt kompilieren und hochladen (`Upload`).
5.  Dateisystem ist nicht notwendig (Daten werden im NVS/Preferences gespeichert).



