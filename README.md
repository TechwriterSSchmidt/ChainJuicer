# ChainJuicer-light

GPS-gesteuertes automatisches Kettenöler-System für Motorräder auf Basis des ESP32

## Beschreibung

ChainJuicer-light ist ein fortschrittliches, GPS-gesteuertes automatisches Kettenöler-System für Motorräder. Das System nutzt GPS-Geschwindigkeitsdaten, um die Ölzufuhr zur Kette automatisch und dynamisch anzupassen. Bei höheren Geschwindigkeiten wird häufiger geölt, bei niedrigen Geschwindigkeiten seltener.

## Features

- 🛰️ **GPS-gesteuert**: Automatische Anpassung der Ölintervalle basierend auf Geschwindigkeit
- 📊 **OLED-Display**: Zeigt Geschwindigkeit, GPS-Status, Ölstatus und Statistiken
- ⚡ **PWM-Pumpensteuerung**: Präzise Steuerung der Ölmenge
- 📍 **Distanzverfolgung**: Automatische Berechnung der gefahrenen Strecke
- 📈 **Statistiken**: Zählt die Anzahl der Ölschüsse
- 🔧 **Konfigurierbar**: Einfache Anpassung aller Parameter über config.h
- 💻 **Serial Debug**: Ausführliche Debugging-Ausgaben über serielle Schnittstelle

## Hardware-Anforderungen

### Erforderliche Komponenten

- **ESP32 Development Board** (z.B. ESP32-DevKitC, NodeMCU-32S)
- **GPS-Modul** (z.B. NEO-6M, NEO-7M, BN-880)
  - UART-Schnittstelle
  - Verbindung: RX -> GPIO16, TX -> GPIO17
- **OLED-Display** (SSD1306, 128x64 Pixel)
  - I2C-Schnittstelle (0x3C)
  - Verbindung: SDA -> GPIO21, SCL -> GPIO22
- **Pumpe/Servo** für Ölabgabe
  - PWM-gesteuert
  - Verbindung: GPIO25
- **Status-LED**: GPIO2 (meist onboard)
- **Stromversorgung**: 5V (USB oder Batterie)

### Optionale Komponenten

- Gehäuse (wasserdicht empfohlen)
- Ölbehälter
- Schläuche zur Kette

## Pin-Belegung

```
ESP32 Pin    | Komponente        | Beschreibung
-------------|-------------------|---------------------------
GPIO 16      | GPS RX            | GPS-Modul Receive
GPIO 17      | GPS TX            | GPS-Modul Transmit
GPIO 21      | OLED SDA          | Display I2C Data
GPIO 22      | OLED SCL          | Display I2C Clock
GPIO 25      | Pump PWM          | Pumpen-/Servo-Steuerung
GPIO 2       | Status LED        | Status-Anzeige
```

## Software-Anforderungen

- [PlatformIO](https://platformio.org/)
- ESP32 Arduino Framework
- Bibliotheken (werden automatisch installiert):
  - TinyGPSPlus
  - Adafruit SSD1306
  - Adafruit GFX Library

## Installation

### 1. Repository klonen

```bash
git clone https://github.com/TechwriterSSchmidt/ChainJuicer-light.git
cd ChainJuicer-light
```

### 2. Mit PlatformIO öffnen

```bash
pio project init
```

Oder öffnen Sie das Projekt in VS Code mit der PlatformIO-Erweiterung.

### 3. Projekt kompilieren

```bash
pio run
```

### 4. Auf ESP32 hochladen

```bash
pio run --target upload
```

### 5. Serial Monitor öffnen (optional)

```bash
pio device monitor
```

## Konfiguration

Alle Einstellungen können in `include/config.h` angepasst werden:

### Geschwindigkeitsschwellen

```cpp
#define SPEED_THRESHOLD_MIN 5.0f    // Minimale Geschwindigkeit für Ölung (km/h)
#define SPEED_THRESHOLD_MAX 120.0f  // Maximale Geschwindigkeit (km/h)
```

### Pumpensteuerung

```cpp
#define PUMP_MIN_DUTY 50    // Minimale Pumpenleistung (0-255)
#define PUMP_MAX_DUTY 200   // Maximale Pumpenleistung (0-255)
```

### Ölintervalle

```cpp
#define OIL_INTERVAL_MIN 5000   // Minimales Intervall bei hoher Geschwindigkeit (ms)
#define OIL_INTERVAL_MAX 30000  // Maximales Intervall bei niedriger Geschwindigkeit (ms)
#define OIL_PULSE_DURATION 200  // Dauer eines Ölschusses (ms)
```

## Funktionsweise

### GPS-gesteuerte Ölung

Das System berechnet automatisch die optimalen Ölintervalle basierend auf der aktuellen Geschwindigkeit:

- **Niedrige Geschwindigkeit** (5-30 km/h): Seltene Ölung (alle 20-30 Sekunden)
- **Mittlere Geschwindigkeit** (30-70 km/h): Moderate Ölung (alle 10-15 Sekunden)
- **Hohe Geschwindigkeit** (70-120 km/h): Häufige Ölung (alle 5-10 Sekunden)

Die Pumpenleistung wird ebenfalls automatisch angepasst:
- Bei höheren Geschwindigkeiten wird mehr Öl pro Schuss abgegeben
- Bei niedrigeren Geschwindigkeiten weniger

### Display-Anzeige

Das OLED-Display zeigt folgende Informationen:

```
GPS: 12 SAT         <- GPS-Status und Satellitenanzahl
                    
  85 km/h           <- Aktuelle Geschwindigkeit (groß)
                    
OILING: 65%         <- Aktueller Ölstatus oder Zeit bis zum nächsten Schuss
                    
Dist:45.2km Oil:234 <- Gefahrene Distanz und Anzahl Ölschüsse
```

## Projekt-Struktur

```
ChainJuicer-light/
├── include/
│   ├── config.h          # Konfigurationseinstellungen
│   ├── ChainOiler.h      # Hauptklasse Deklaration
│   └── Display.h         # Display-Verwaltung Deklaration
├── src/
│   ├── main.cpp          # Hauptprogramm
│   ├── ChainOiler.cpp    # Hauptklasse Implementierung
│   └── Display.cpp       # Display-Verwaltung Implementierung
├── platformio.ini        # PlatformIO-Konfiguration
└── README.md            # Diese Datei
```

## Entwicklung

### Build-Befehle

```bash
# Projekt kompilieren
pio run

# Hochladen auf ESP32
pio run --target upload

# Serial Monitor
pio device monitor

# Alles zusammen
pio run --target upload && pio device monitor

# Clean build
pio run --target clean
```

### Debug-Ausgaben

Aktivieren Sie Serial Debug in `config.h`:

```cpp
#define SERIAL_DEBUG true
```

Die Debug-Ausgaben zeigen:
- GPS-Status (Geschwindigkeit, Satelliten, Distanz)
- Ölschüsse (Leistung, Anzahl)
- System-Status

## Fehlerbehebung

### GPS findet keine Satelliten

- Prüfen Sie die GPS-Antenne (sollte freie Sicht zum Himmel haben)
- Warten Sie 2-5 Minuten für ersten Fix (Cold Start)
- Prüfen Sie die Verkabelung (RX/TX könnten vertauscht sein)

### Display zeigt nichts

- Prüfen Sie die I2C-Adresse (Standard: 0x3C)
- Prüfen Sie die Verkabelung (SDA/SCL)
- Testen Sie mit I2C-Scanner

### Pumpe läuft nicht

- Prüfen Sie die PWM-Verbindung (GPIO25)
- Stellen Sie sicher, dass die Pumpe genug Spannung bekommt
- Überprüfen Sie die Duty Cycle-Einstellungen in config.h

## Lizenz

Dieses Projekt ist Open Source und steht unter der MIT-Lizenz.

## Autor

TechwriterSSchmidt

## Beiträge

Beiträge sind willkommen! Bitte erstellen Sie einen Pull Request oder öffnen Sie ein Issue.

## Version

Version 1.0 - Erste vollständige Implementierung
