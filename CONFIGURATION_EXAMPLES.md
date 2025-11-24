# Konfigurations-Beispiele

## ChainJuicer-light Konfigurationen für verschiedene Szenarien

### Beispiel 1: Stadtfahrer (niedrige Geschwindigkeiten)

Für überwiegend Stadtfahrten mit Geschwindigkeiten bis 60 km/h:

```cpp
// include/config.h

// Geschwindigkeitsschwellen
#define SPEED_THRESHOLD_MIN 10.0f   // Start bei 10 km/h
#define SPEED_THRESHOLD_MAX 60.0f   // Maximum 60 km/h

// Ölintervalle - seltener ölen
#define OIL_INTERVAL_MIN 10000      // 10 Sekunden bei max. Speed
#define OIL_INTERVAL_MAX 45000      // 45 Sekunden bei min. Speed
#define OIL_PULSE_DURATION 150      // Kürzere Pulse

// Pumpenleistung - weniger Öl
#define PUMP_MIN_DUTY 40
#define PUMP_MAX_DUTY 120
```

### Beispiel 2: Langstreckenfahrer (Autobahn)

Für Autobahnfahrten mit höheren Geschwindigkeiten:

```cpp
// include/config.h

// Geschwindigkeitsschwellen
#define SPEED_THRESHOLD_MIN 30.0f   // Start bei 30 km/h
#define SPEED_THRESHOLD_MAX 180.0f  // Maximum 180 km/h

// Ölintervalle - häufiger ölen bei hoher Geschwindigkeit
#define OIL_INTERVAL_MIN 3000       // 3 Sekunden bei max. Speed
#define OIL_INTERVAL_MAX 20000      // 20 Sekunden bei min. Speed
#define OIL_PULSE_DURATION 250      // Längere Pulse

// Pumpenleistung - mehr Öl
#define PUMP_MIN_DUTY 80
#define PUMP_MAX_DUTY 220
```

### Beispiel 3: Offroad/Enduro

Für Offroad-Einsatz mit viel Staub und Schmutz:

```cpp
// include/config.h

// Geschwindigkeitsschwellen
#define SPEED_THRESHOLD_MIN 5.0f    // Start bei 5 km/h (langsames Gelände)
#define SPEED_THRESHOLD_MAX 80.0f   // Maximum 80 km/h

// Ölintervalle - sehr häufig ölen wegen Verschmutzung
#define OIL_INTERVAL_MIN 2000       // 2 Sekunden bei max. Speed
#define OIL_INTERVAL_MAX 10000      // 10 Sekunden bei min. Speed
#define OIL_PULSE_DURATION 300      // Längere Pulse für mehr Öl

// Pumpenleistung - viel Öl
#define PUMP_MIN_DUTY 100
#define PUMP_MAX_DUTY 240
```

### Beispiel 4: Sparsamer Modus (Ölverbrauch minimieren)

Für maximale Ölersparnis:

```cpp
// include/config.h

// Geschwindigkeitsschwellen
#define SPEED_THRESHOLD_MIN 20.0f   // Start erst bei 20 km/h
#define SPEED_THRESHOLD_MAX 120.0f  // Standard Maximum

// Ölintervalle - sehr selten ölen
#define OIL_INTERVAL_MIN 15000      // 15 Sekunden bei max. Speed
#define OIL_INTERVAL_MAX 60000      // 60 Sekunden bei min. Speed
#define OIL_PULSE_DURATION 100      // Sehr kurze Pulse

// Pumpenleistung - minimal
#define PUMP_MIN_DUTY 30
#define PUMP_MAX_DUTY 100
```

### Beispiel 5: Rennstrecke (maximale Leistung)

Für Track Days und Rennstrecke:

```cpp
// include/config.h

// Geschwindigkeitsschwellen
#define SPEED_THRESHOLD_MIN 50.0f   // Start erst bei 50 km/h
#define SPEED_THRESHOLD_MAX 250.0f  // Sehr hohes Maximum

// Ölintervalle - sehr häufig bei hoher Belastung
#define OIL_INTERVAL_MIN 2000       // 2 Sekunden bei max. Speed
#define OIL_INTERVAL_MAX 8000       // 8 Sekunden bei min. Speed
#define OIL_PULSE_DURATION 200      // Moderate Pulse

// Pumpenleistung - hoch
#define PUMP_MIN_DUTY 120
#define PUMP_MAX_DUTY 250
```

### Beispiel 6: Winter/Kalte Bedingungen

Für Fahrten bei kaltem Wetter (Öl ist dickflüssiger):

```cpp
// include/config.h

// Geschwindigkeitsschwellen
#define SPEED_THRESHOLD_MIN 5.0f
#define SPEED_THRESHOLD_MAX 120.0f

// Ölintervalle - normal
#define OIL_INTERVAL_MIN 5000
#define OIL_INTERVAL_MAX 30000
#define OIL_PULSE_DURATION 300      // Längere Pulse wegen zähflüssigerem Öl

// Pumpenleistung - höher wegen dickflüssigerem Öl
#define PUMP_MIN_DUTY 100
#define PUMP_MAX_DUTY 240

// Temperaturkompensation aktivieren (zukünftige Funktion)
#define TEMP_COMPENSATION_ENABLED true
```

## Kalibrierungs-Anleitung

### Schritt 1: Basis-Test

1. Starten Sie mit Standardwerten
2. Machen Sie eine Testfahrt (30 Minuten)
3. Prüfen Sie die Kettenölung visuell
4. Lesen Sie die Statistiken aus (Dist, Oil Count)

### Schritt 2: Pump Duty Kalibrierung

Test für minimalen Duty Cycle:

```cpp
// Test verschiedene Werte
#define PUMP_MIN_DUTY 30   // Zu wenig?
// oder
#define PUMP_MIN_DUTY 50   // Gerade richtig?
// oder
#define PUMP_MIN_DUTY 80   // Zu viel?
```

**Ziel:** Gerade genug für sichtbare Ölabgabe

### Schritt 3: Intervall-Optimierung

Berechnen Sie die optimale Frequenz:

```
Beispiel:
- Testfahrt: 50 km, 1 Stunde
- Oil Count: 120 Schüsse
- Durchschnitt: 120 / 60 = 2 Schüsse/Minute

Zu oft? -> Intervalle erhöhen
Zu selten? -> Intervalle verringern
```

### Schritt 4: Feintuning

Testen Sie verschiedene Geschwindigkeitsbereiche:

- Langsame Fahrt (10-30 km/h): Wie oft wird geölt?
- Mittlere Fahrt (50-70 km/h): Wie oft wird geölt?
- Schnelle Fahrt (100+ km/h): Wie oft wird geölt?

Passen Sie die Kurve an:

```cpp
// Flachere Kurve (weniger Unterschied zwischen langsam/schnell)
#define OIL_INTERVAL_MIN 10000
#define OIL_INTERVAL_MAX 20000

// Steilere Kurve (großer Unterschied)
#define OIL_INTERVAL_MIN 3000
#define OIL_INTERVAL_MAX 60000
```

## Debug-Konfiguration

Für Entwicklung und Fehlersuche:

```cpp
// include/config.h

#define SERIAL_DEBUG true
#define SERIAL_BAUDRATE 115200

// platformio.ini
build_flags = 
    -D CORE_DEBUG_LEVEL=5
```

## Minimale Konfiguration (nur GPS, kein Display)

```cpp
// include/config.h

#define DISPLAY_ENABLED false
#define SERIAL_DEBUG false  // Strom sparen
```

## Performance-Optimierung

Für maximale Akkulaufzeit:

```cpp
// include/config.h

#define GPS_UPDATE_INTERVAL 2000     // Weniger häufig GPS lesen
#define DISPLAY_UPDATE_INTERVAL 1000 // Weniger Display-Updates
```

## Tipps zur Konfiguration

1. **Starten Sie konservativ**: Lieber zu wenig Öl als zu viel
2. **Dokumentieren Sie Änderungen**: Notieren Sie welche Werte funktionieren
3. **Testen Sie schrittweise**: Ändern Sie nur einen Parameter gleichzeitig
4. **Berücksichtigen Sie die Jahreszeit**: Winter braucht andere Einstellungen
5. **Messen Sie den Ölverbrauch**: Wie viel ml pro 100 km?

## Häufige Anpassungen

| Situation | Anpassung | Parameter |
|-----------|-----------|-----------|
| Kette zu trocken | Mehr ölen | Intervalle verringern |
| Kette zu nass | Weniger ölen | Intervalle erhöhen |
| Öl spritzt | Weniger Druck | PUMP_MAX_DUTY reduzieren |
| Keine Wirkung | Mehr Druck | PUMP_MIN_DUTY erhöhen |
| Batterie leer | Strom sparen | Update-Intervalle erhöhen |

Viel Erfolg beim Optimieren!
