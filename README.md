# GPS Chain Oiler (ESP32)

Ein fortschrittlicher, GPS-gesteuerter Kettenöler für Motorräder auf Basis des ESP32. Das System passt die Ölungsintervalle dynamisch an die gefahrene Geschwindigkeit an und bietet umfangreiche Konfigurationsmöglichkeiten über ein Webinterface.

## 🚀 Features

*   **Geschwindigkeitsabhängige Ölung:** 5 konfigurierbare Geschwindigkeitsbereiche mit individuellen Intervallen (km) und Pump-Impulsen.
*   **Intelligente Glättung:** Nutzt eine Lookup-Tabelle mit linearer Interpolation und einen Low-Pass Filter, um harte Sprünge bei den Schmierintervallen zu vermeiden.
*   **Smart GPS Filter:** Erkennt und ignoriert "Geister-Geschwindigkeiten" (Multipath-Reflexionen) in Innenräumen oder Tunneln (HDOP > 5.0 oder < 5 Satelliten).
*   **Startup Delay:** 10 Sekunden Sicherheitsverzögerung nach dem Booten, um Pumpen-Fehlfunktionen während der Initialisierung zu verhindern.
*   **GPS-Präzision:** Exakte Distanzmessung via GPS-Modul (TinyGPS++).
*   **Regenmodus:** Verdoppelt die Ölmenge bei Nässe. Aktivierbar per Taster. Automatische Abschaltung nach 30 Minuten.
*   **Notlauf-Modus (Emergency Mode):** Aktiviert sich automatisch, wenn länger als 5 Minuten kein GPS-Signal empfangen wird. Simuliert 50 km/h.
*   **Smart WiFi & Webinterface:**
    *   Konfiguration aller Parameter bequem per Smartphone.
    *   LED-Helligkeit in Prozent (0-100%) einstellbar.
    *   Aktivierung nur im Stand (< 7 km/h) durch langen Tastendruck (> 3s).
    *   Automatische Abschaltung bei Fahrtantritt (> 10 km/h) oder Inaktivität (5 Min).
*   **Nachtmodus:** Automatische Dimmung der Status-LED basierend auf der GPS-Uhrzeit. Separate Helligkeit für Events (Ölen, WiFi) einstellbar.
*   **Entlüftungsmodus (Bleeding):** Dauerpumpen zum Füllen der Ölleitung nach Wartungsarbeiten.
*   **Tank-Monitor:** Berechnet den Ölverbrauch und warnt (pulsierende LED), wenn der Vorrat zur Neige geht.
*   **Erweiterte Statistik:**
    *   **Fahrprofil (Time %):** Zeigt an, wie viel Prozent der Fahrzeit in welchem Geschwindigkeitsbereich verbracht wurde.
    *   **Ölungs-Zähler:** Zählt die Anzahl der ausgelösten Ölungen pro Geschwindigkeitsbereich.
    *   **Odometer:** Gesamtkilometerzähler.
*   **Datensicherheit:** Kilometerstand und Einstellungen werden dauerhaft im Flash-Speicher (NVS) gespeichert.

## 🛠 Hardware

*   **MCU:** ESP32 Development Board
*   **GPS:** NEO-6M oder kompatibles Modul (UART)
*   **Pumpe:** Dosierpumpe (angesteuert über MOSFET/Transistor)
*   **LED:** WS2812B (NeoPixel) für Statusanzeige (Unterstützt mehrere LEDs, z.B. Status + Gehäusebeleuchtung)
*   **Taster:** Schließer gegen GND (Input Pullup)

### Pinbelegung (Standard)

| Komponente | ESP32 Pin | Beschreibung |
| :--- | :--- | :--- |
| **Pumpe** | GPIO 27 | MOSFET Gate |
| **GPS RX** | GPIO 32 | Verbunden mit GPS TX |
| **GPS TX** | GPIO 33 | Verbunden mit GPS RX |
| **Taster (Lenker)** | GPIO 4 | Gegen GND geschaltet |
| **Taster (Gehäuse)** | GPIO 14 | Gegen GND geschaltet (Optional) |
| **LED** | GPIO 5 | WS2812B Data In |

*(Konfigurierbar in `include/config.h`)*

## 📖 Bedienung

### Taster-Funktionen

| Aktion | Dauer | Bedingung | Funktion |
| :--- | :--- | :--- | :--- |
| **Kurz drücken** | < 1.5s | Immer | **Regenmodus** Ein/Aus (LED: Blau) |
| **Halten** | > 3s | Im Stand (< 7 km/h) | **WiFi & Webinterface** aktivieren (LED: Weiß pulsierend) |
| **Lang halten** | > 10s | Im Stand (< 7 km/h) | **Entlüftungsmodus** (Bleeding) starten (LED: Rot blinkend, Pumpe läuft 10s) |

### LED Status-Codes

*   🟢 **Grün:** Normalbetrieb (GPS Fix vorhanden)
*   🔵 **Blau:** Regenmodus Aktiv
*   🟣 **Magenta:** Kein GPS Signal (Suche...)
*   🔵 **Cyan:** Emergency Mode (Kein GPS, Simulation aktiv)
*   🟡 **Gelb:** Ölung läuft (leuchtet für 3s)
*   ⚪ **Weiß (pulsierend):** WiFi Konfigurations-Modus aktiv
*   🔴 **Rot (pulsierend 2x):** Tankwarnung (Reserve erreicht)
*   🔴 **Rot (blinkend):** Entlüftungsmodus (Bleeding) aktiv

## 📱 Webinterface

Verbinden Sie sich mit dem WiFi-Netzwerk (Standard-SSID: `ChainJuicer`, kein Passwort), nachdem Sie es aktiviert haben. Rufen Sie im Browser `192.168.4.1` auf.

**Einstellbare Parameter:**
*   **Intervalle:** Distanz und Pumpstöße für 5 Geschwindigkeitsbereiche.
*   **Modi:** Regenmodus, Emergency Mode, Nachtmodus (Zeiten & Helligkeit).
*   **LED:** Helligkeit für Tag und Nacht (in %).
*   **Statistik:**
    *   **Time %:** Fahrprofil-Analyse zur Optimierung der Intervalle.
    *   **Oilings:** Zähler für Ölungen pro Bereich.
    *   Gesamtkilometerzähler und Pump-Zyklen (Reset möglich).

## ⚙️ Technische Details

*   **Non-Blocking:** Die Ansteuerung der Pumpe erfolgt asynchron.
*   **Adaptive Glättung:** Kombination aus Lookup-Tabelle und Low-Pass Filter.
*   **Smart Oiling (Hysterese):** Die Ölung wird bereits bei **95% der errechneten Distanz** ausgelöst.
*   **Speicherschutz:** Der Kilometerstand wird intelligent gespeichert (bei Stillstand < 7 km/h, aber max. alle 2 Minuten).
*   **Zeitzone:** Automatische Berechnung der Mitteleuropäischen Zeit (MEZ/MESZ).

## 💻 Installation (PlatformIO)

1.  Repository klonen oder herunterladen.
2.  In VS Code mit installierter **PlatformIO** Extension öffnen.
3.  Ggf. Upload-Port in `platformio.ini` anpassen.
4.  Projekt kompilieren und hochladen (`Upload`).
5.  Dateisystem ist nicht notwendig (Daten werden im NVS/Preferences gespeichert).



