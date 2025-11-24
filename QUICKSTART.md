# Quick Start Guide - ChainJuicer-light

## Schnellstart in 5 Minuten

### Voraussetzungen

- ✅ PlatformIO installiert
- ✅ ESP32 mit USB verbunden
- ✅ GPS-Modul angeschlossen
- ✅ (Optional) OLED Display angeschlossen
- ✅ (Optional) Pumpe angeschlossen

### Schritt 1: Repository klonen

```bash
git clone https://github.com/TechwriterSSchmidt/ChainJuicer-light.git
cd ChainJuicer-light
```

### Schritt 2: Konfiguration prüfen

Öffnen Sie `include/config.h` und passen Sie bei Bedarf an:

```cpp
// Geschwindigkeitsbereich
#define SPEED_THRESHOLD_MIN 5.0f    // Minimum für Ölung
#define SPEED_THRESHOLD_MAX 120.0f  // Maximum

// Ölintervalle
#define OIL_INTERVAL_MIN 5000   // Schnell: alle 5 Sekunden
#define OIL_INTERVAL_MAX 30000  // Langsam: alle 30 Sekunden
```

### Schritt 3: Projekt kompilieren

```bash
pio run
```

### Schritt 4: Auf ESP32 hochladen

```bash
pio run --target upload
```

### Schritt 5: Serial Monitor starten

```bash
pio device monitor
```

## Erste Inbetriebnahme

### Was Sie sehen sollten:

```
=================================
ChainJuicer-light v1.0
GPS-Controlled Chain Oiler
ESP32 + Arduino Framework
=================================

ChainOiler: Initialized
Waiting for GPS fix...

System ready!
```

### GPS Fix warten

Der erste GPS Fix kann 2-5 Minuten dauern (Cold Start).
Bei GPS Fix sehen Sie:

```
GPS: Speed=0.0 km/h, Sats=8, Dist=0.00 km
```

### Display-Anzeige

Nach erfolgreicher Initialisierung zeigt das OLED Display:

```
GPS: 8 SAT
  
  0 km/h
  
Next: ---
  
Dist:0.0km Oil:0
```

## Testmodus

### Manuelle Ölabgabe testen

Fügen Sie in `src/main.cpp` in der `loop()` Funktion hinzu:

```cpp
// Testcode: Drücken Sie Reset-Taste für manuellen Ölschuss
static unsigned long lastTest = 0;
if (millis() - lastTest > 10000) {  // Alle 10 Sekunden
    chainOiler.manualOilShot();
    lastTest = millis();
}
```

### GPS-Simulation (Ohne GPS-Modul)

Für Tests ohne GPS können Sie in `ChainOiler.cpp` die Geschwindigkeit hardcoded setzen:

```cpp
void ChainOiler::updateGPS() {
    // TEST: Simuliere Geschwindigkeit
    currentSpeed = 50.0f;  // 50 km/h simulieren
    gpsFixValid = true;
    satellites = 8;
    // ... Rest auskommentieren
}
```

## Fehlerbehebung

### Problem: "Upload failed"

```bash
# COM-Port finden
pio device list

# Manuell hochladen mit Port
pio run --target upload --upload-port /dev/ttyUSB0
```

### Problem: "GPS: NO FIX"

1. GPS-Antenne nach außen/ans Fenster
2. 2-5 Minuten warten (Cold Start)
3. Verkabelung prüfen (RX/TX vertauscht?)

### Problem: Display bleibt dunkel

```bash
# I2C Scanner in Serial Monitor
# Prüfe ob Adresse 0x3C erkannt wird
```

In `config.h` ggf. Adresse ändern:
```cpp
#define OLED_ADDR 0x3D  // Statt 0x3C
```

### Problem: Pumpe reagiert nicht

1. Externe Stromversorgung prüfen
2. PWM-Signal mit Oszilloskop/LED testen
3. Duty Cycle in config.h erhöhen:
   ```cpp
   #define PUMP_MIN_DUTY 100  // Statt 50
   ```

## Erweiterte Funktionen

### Debug-Level erhöhen

In `platformio.ini`:
```ini
build_flags = 
    -D CORE_DEBUG_LEVEL=5  ; Mehr Debug-Ausgaben
```

### GPS Baudrate ändern

Manche GPS-Module verwenden 115200 statt 9600:

In `platformio.ini`:
```ini
build_flags = 
    -D GPS_BAUDRATE=115200
```

### Display deaktivieren

In `include/config.h`:
```cpp
#define DISPLAY_ENABLED false
```

## Optimierung für den Einsatz

### 1. Ölintervalle anpassen

Testen Sie verschiedene Werte für:
- `OIL_INTERVAL_MIN`: Wie oft bei hoher Geschwindigkeit ölen?
- `OIL_INTERVAL_MAX`: Wie oft bei niedriger Geschwindigkeit?
- `OIL_PULSE_DURATION`: Wie lange Pumpe aktivieren?

### 2. Pumpenleistung kalibrieren

Testen Sie verschiedene Duty Cycles:
- `PUMP_MIN_DUTY`: Minimum für sichtbare Ölabgabe
- `PUMP_MAX_DUTY`: Maximum (nicht über Pumpen-Limit!)

### 3. Geschwindigkeitsschwellen

Passen Sie an Ihr Fahrverhalten an:
- `SPEED_THRESHOLD_MIN`: Ab welcher Geschwindigkeit ölen?
- `SPEED_THRESHOLD_MAX`: Maximale Berücksichtigung

## Nächste Schritte

1. ✅ Hardware aufbauen (siehe WIRING.md)
2. ✅ Software flashen
3. ✅ Testen auf dem Tisch
4. ✅ Parameter optimieren
5. ✅ Am Motorrad montieren
6. ✅ Testfahrt durchführen
7. ✅ Feintuning

## Support

Bei Problemen:
1. README.md lesen
2. WIRING.md prüfen
3. Serial Monitor Ausgabe prüfen
4. Issue auf GitHub erstellen

Viel Erfolg! 🏍️
