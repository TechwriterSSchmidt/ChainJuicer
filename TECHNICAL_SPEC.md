# ChainJuicer-light - Technische Spezifikation

## System-Übersicht

ChainJuicer-light ist ein vollständig autonomes, GPS-gesteuertes Kettenöler-System für Motorräder, basierend auf dem ESP32 Mikrocontroller und dem Arduino Framework.

## Technische Spezifikationen

### Mikrocontroller
- **MCU:** ESP32 (Dual-Core Tensilica LX6, 240 MHz)
- **RAM:** 520 KB SRAM
- **Flash:** 4 MB (Standard)
- **Framework:** Arduino (PlatformIO)
- **Entwicklungsumgebung:** PlatformIO 6.x

### GPS-System
- **Modul:** Kompatibel mit NEO-6M, NEO-7M, BN-880 und anderen UART GPS
- **Schnittstelle:** Hardware Serial 2 (GPIO 16/17)
- **Baudrate:** 9600 Baud (konfigurierbar)
- **Update-Rate:** 1 Hz (1 Sekunde)
- **Fix-Validierung:** Minimum 4 Satelliten für Genauigkeit
- **Positionsgenauigkeit:** ~2.5m CEP (mit guten Bedingungen)
- **Geschwindigkeitsgenauigkeit:** ±0.1 km/h

### Display-System
- **Typ:** OLED SSD1306
- **Auflösung:** 128x64 Pixel
- **Schnittstelle:** I2C (GPIO 21/22)
- **I2C-Adresse:** 0x3C (Standard)
- **Farbtiefe:** Monochrom (Weiß auf Schwarz)
- **Update-Rate:** 2 Hz (500 ms Intervall)

### Pumpen-Steuerung
- **Schnittstelle:** PWM (GPIO 25)
- **PWM-Frequenz:** 1000 Hz
- **PWM-Auflösung:** 8-bit (0-255)
- **Duty Cycle Range:** 0% - 100%
- **Konfigurierbar:** Min/Max Duty Cycle
- **Pulse-Dauer:** 100-300 ms (konfigurierbar)

### Status-Anzeige
- **LED:** GPIO 2 (onboard)
- **Funktion:** GPS Fix-Indikator (blinkt bei aktivem Fix)

## Funktionale Spezifikationen

### GPS-gesteuerte Ölung

#### Geschwindigkeitsbereiche
- **Minimum:** 5 km/h (konfigurierbar)
- **Maximum:** 120 km/h (konfigurierbar)
- **Granularität:** Kontinuierliche lineare Interpolation

#### Ölintervalle
- **Bei niedriger Geschwindigkeit:** 30 Sekunden (Standard)
- **Bei hoher Geschwindigkeit:** 5 Sekunden (Standard)
- **Algorithmus:** Inverse lineare Funktion (schneller = häufiger)

#### Pumpenleistung
- **Bei niedriger Geschwindigkeit:** 20% Duty Cycle (Standard)
- **Bei hoher Geschwindigkeit:** 78% Duty Cycle (Standard)
- **Algorithmus:** Direkte lineare Funktion (schneller = mehr Öl)

### Distanz-Tracking
- **Methode:** Haversine-Formel (TinyGPSPlus)
- **Genauigkeit:** ±1-3 Meter
- **Speicherung:** Volatile (Reset bei Neustart)
- **Anzeige:** Kilometer mit einer Dezimalstelle

### Statistiken
- **Gefahrene Distanz:** Akkumuliert seit Start
- **Ölschuss-Zähler:** Anzahl der Ölabgaben seit Start
- **GPS-Satelliten:** Live-Anzahl der verfolgten Satelliten
- **Aktuelle Geschwindigkeit:** In km/h

## Software-Architektur

### Klassenstruktur

#### ChainOiler-Klasse
**Verantwortlichkeiten:**
- GPS-Daten-Verarbeitung
- Geschwindigkeitsberechnung und -validierung
- Satelliten-Zählung und Fix-Validierung
- Distanz-Berechnung
- Ölinterval-Berechnung
- Pumpen-PWM-Steuerung
- Timing-Management

**Öffentliche Methoden:**
- `begin()` - Initialisierung
- `update()` - Hauptschleife
- `getSpeed()` - Aktuelle Geschwindigkeit
- `hasGPSFix()` - GPS-Fix-Status
- `getSatellites()` - Satellitenanzahl
- `getPumpDuty()` - Aktueller Pumpen-Duty-Cycle
- `getTimeUntilNextOil()` - Zeit bis zur nächsten Ölung
- `manualOilShot()` - Manuelle Ölabgabe (Test)
- `getTotalDistance()` - Gefahrene Strecke
- `getOilShotCount()` - Anzahl Ölschüsse

#### Display-Klasse
**Verantwortlichkeiten:**
- OLED-Initialisierung
- UI-Rendering
- Statusanzeige
- Splash-Screen

**Öffentliche Methoden:**
- `begin()` - Display-Initialisierung
- `update(oiler)` - Display aktualisieren
- `showSplash()` - Splash-Screen anzeigen
- `clear()` - Display löschen

### Datenfluss

```
GPS-Modul (UART)
    ↓
[TinyGPSPlus Parser]
    ↓
[ChainOiler::updateGPS()]
    ↓
├─→ Geschwindigkeit
├─→ Position
├─→ Satelliten
└─→ Distanz
    ↓
[ChainOiler::updatePump()]
    ↓
├─→ Intervall-Berechnung
└─→ Duty-Cycle-Berechnung
    ↓
[PWM-Output] → Pumpe
    ↓
[Display::update()] → OLED
```

## Konfigurationsoptionen

Alle Optionen in `include/config.h`:

### GPIO-Pins (änderbar)
```cpp
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define PUMP_PIN 25
#define OLED_SDA_PIN 21
#define OLED_SCL_PIN 22
#define STATUS_LED_PIN 2
```

### GPS-Einstellungen
```cpp
#define GPS_BAUDRATE 9600
#define GPS_UPDATE_INTERVAL 1000
#define GPS_FIX_TIMEOUT 10000
#define MIN_SATELLITES 4
```

### Ölungs-Parameter
```cpp
#define SPEED_THRESHOLD_MIN 5.0f
#define SPEED_THRESHOLD_MAX 120.0f
#define OIL_INTERVAL_MIN 5000
#define OIL_INTERVAL_MAX 30000
#define OIL_PULSE_DURATION 200
#define PUMP_MIN_DUTY 50
#define PUMP_MAX_DUTY 200
```

### Display-Einstellungen
```cpp
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define DISPLAY_UPDATE_INTERVAL 500
#define DISPLAY_ENABLED true
```

### Debug-Optionen
```cpp
#define SERIAL_DEBUG true
#define SERIAL_BAUDRATE 115200
```

## Berechnungsformeln

### Ölintervall-Berechnung

```cpp
speedRatio = (speed - MIN) / (MAX - MIN)
speedRatio = constrain(speedRatio, 0.0, 1.0)
interval = MAX_INTERVAL - (MAX_INTERVAL - MIN_INTERVAL) * speedRatio
```

**Beispiel bei 60 km/h (MIN=5, MAX=120):**
```
speedRatio = (60 - 5) / (120 - 5) = 0.478
interval = 30000 - (30000 - 5000) * 0.478 = 18050 ms ≈ 18 Sekunden
```

### Pumpen-Duty-Cycle-Berechnung

```cpp
speedRatio = (speed - MIN) / (MAX - MIN)
speedRatio = constrain(speedRatio, 0.0, 1.0)
duty = MIN_DUTY + (MAX_DUTY - MIN_DUTY) * speedRatio
```

**Beispiel bei 60 km/h (MIN_DUTY=50, MAX_DUTY=200):**
```
speedRatio = 0.478
duty = 50 + (200 - 50) * 0.478 = 121.7 ≈ 122 (≈48% Duty Cycle)
```

### Distanz-Berechnung (Haversine)

```cpp
distance = TinyGPSPlus::distanceBetween(
    lastLat, lastLng,
    currentLat, currentLng
)
totalDistance += distance / 1000.0  // Meter → Kilometer
```

## Stromverbrauch

### Typischer Verbrauch
- **ESP32 (aktiv):** ~160 mA @ 3.3V
- **GPS-Modul:** ~40 mA @ 3.3V
- **OLED Display:** ~20 mA @ 3.3V
- **Gesamt (ohne Pumpe):** ~220 mA

### Pumpen-Verbrauch
- **Abhängig von Pumpe/Servo:** 100-500 mA
- **Empfehlung:** Separate Stromversorgung für Pumpe

## Speicherverbrauch (geschätzt)

### Flash (Programmspeicher)
- **ChainOiler-Code:** ~15 KB
- **Display-Code:** ~8 KB
- **Main-Code:** ~3 KB
- **TinyGPSPlus:** ~10 KB
- **Adafruit Libraries:** ~20 KB
- **Arduino Framework:** ~200 KB
- **Gesamt:** ~256 KB (von 4 MB)

### RAM (Laufzeitspeicher)
- **ChainOiler-Objekt:** ~200 Bytes
- **Display-Objekt:** ~1024 Bytes (Display-Buffer)
- **TinyGPS-Objekt:** ~128 Bytes
- **Stack/Heap:** ~50 KB
- **Gesamt:** ~52 KB (von 520 KB)

## Performance-Charakteristiken

### Reaktionszeiten
- **GPS-Update:** 1 Sekunde
- **Display-Update:** 500 Millisekunden
- **Pumpen-Reaktion:** < 10 Millisekunden
- **Main-Loop:** ~10 Millisekunden

### Genauigkeit
- **Geschwindigkeit:** ±0.1 km/h (GPS-abhängig)
- **Distanz:** ±2-3% (GPS-abhängig)
- **Ölintervall:** ±50 ms
- **PWM-Duty:** ±0.4% (8-bit Auflösung)

## Umgebungsbedingungen

### Empfohlene Betriebsbedingungen
- **Temperatur:** -10°C bis +60°C
- **Luftfeuchtigkeit:** 0-95% (nicht kondensierend)
- **Versorgungsspannung:** 4.5V - 5.5V
- **Schutzart:** IP65+ empfohlen (Gehäuse)

### GPS-Anforderungen
- **Freie Sicht zum Himmel:** Optimal
- **Urban Canyon:** Reduzierte Genauigkeit
- **Indoor:** Kein GPS-Fix möglich

## Sicherheitsfeatures

### GPS-Validierung
- Minimum 4 Satelliten erforderlich
- Fix-Timeout nach 10 Sekunden ohne Signal
- Ungültige Geschwindigkeiten werden gefiltert

### Pumpen-Schutz
- Maximaler Duty-Cycle begrenzt
- Pulse-Dauer limitiert
- Automatisches Abschalten bei GPS-Verlust

### Fehlerbehandlung
- Graceful Degradation bei Display-Ausfall
- Weiterbetrieb möglich ohne Display
- Serial Debug für Fehlerdiagnose

## Erweiterungsmöglichkeiten

### Hardware
- Temperatursensor (DS18B20)
- Regensensor (ADC)
- Zusätzliche Taster
- WiFi/Bluetooth-Konfiguration
- SD-Karte für Logging

### Software
- Webinterface für Konfiguration
- OTA-Updates (Over-The-Air)
- Datenlogging auf SD-Karte
- Temperatur-Kompensation
- Adaptive Lernalgorithmen
- Smartphone-App-Integration

## Wartung und Kalibrierung

### Regelmäßige Checks
- Ölstand im Behälter prüfen
- GPS-Antenne auf Verschmutzung prüfen
- Schlauch-Verbindungen prüfen
- Display-Anzeige verifizieren

### Kalibrierung
- Pumpen-Duty-Cycle an Viskosität anpassen
- Intervalle für Fahrstil optimieren
- Satelliten-Minimum bei Bedarf anpassen

## Lizenz

MIT License - Open Source

## Version

**v1.0** - Initiale Release-Version

- Vollständige GPS-Integration
- OLED Display-Unterstützung
- Konfigurierbare Parameter
- Umfassende Dokumentation
- Produktionsreif

---

**Entwickelt für:** ChainJuicer-light Project  
**Autor:** TechwriterSSchmidt  
**Datum:** November 2025
