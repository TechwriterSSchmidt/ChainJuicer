# Verdrahtungs-Anleitung / Wiring Guide

## ChainJuicer-light - Hardwareaufbau

### Komponenten-Liste

1. **ESP32 Development Board**
2. **GPS-Modul** (z.B. NEO-6M oder NEO-7M)
3. **OLED Display** (SSD1306, 128x64, I2C)
4. **Pumpe oder Servo** (5V)
5. **LED** (optional, ESP32 hat meist eine onboard)
6. **Kabel und Steckverbinder**
7. **Stromversorgung** (5V, mindestens 1A)

### Detaillierte Verdrahtung

#### GPS-Modul zu ESP32

```
GPS-Modul          ESP32
---------          -----
VCC      <------>  5V
GND      <------>  GND
TX       <------>  GPIO 16 (RX2)
RX       <------>  GPIO 17 (TX2)
```

**Wichtig:** TX vom GPS geht zu RX am ESP32 und umgekehrt!

#### OLED Display zu ESP32 (I2C)

```
OLED Display       ESP32
------------       -----
VCC      <------>  3.3V (oder 5V, je nach Display)
GND      <------>  GND
SDA      <------>  GPIO 21 (I2C SDA)
SCL      <------>  GPIO 22 (I2C SCL)
```

#### Pumpe/Servo zu ESP32

```
Pumpe/Servo        ESP32
-----------        -----
VCC      <------>  5V (externe Stromversorgung empfohlen!)
GND      <------>  GND (gemeinsame Masse mit ESP32)
Signal   <------>  GPIO 25 (PWM)
```

**Wichtig:** Bei größeren Pumpen unbedingt externe Stromversorgung verwenden und nur das Signal-Kabel mit dem ESP32 verbinden. Die Masse (GND) muss gemeinsam sein!

#### Status LED

```
LED                ESP32
---                -----
+        <------>  GPIO 2 (mit Vorwiderstand ~220Ω)
-        <------>  GND
```

**Hinweis:** Die meisten ESP32 Boards haben bereits eine LED an GPIO2 (onboard).

### Stromversorgung

#### Option 1: USB-Versorgung
- ESP32 über USB mit 5V versorgen
- Geeignet für Tests und kleine Pumpen

#### Option 2: Externe Versorgung
- 5V Spannungsregler oder Powerbank
- Mindestens 1A für ESP32 + GPS + Display
- Mindestens 2A wenn Pumpe mit versorgt wird

**Empfehlung:** Separate Stromversorgung für die Pumpe!

### Schematischer Aufbau

```
                    +5V
                     |
                     |
    +----------------+----------------+
    |                |                |
    |                |                |
  [GPS]          [Display]        [Pumpe]
    |                |                |
    |                |                |
  GPIO16,17       GPIO21,22        GPIO25
    |                |                |
    +-------[ESP32]--+----------------+
                     |
                    GND
```

### Montage-Tipps

1. **Wasserdicht:** Verwenden Sie ein wasserdichtes Gehäuse
2. **GPS-Antenne:** Muss freie Sicht zum Himmel haben
3. **Kabelführung:** Verwenden Sie ausreichend lange Kabel
4. **Vibrationsfest:** Sichere Befestigung am Motorrad
5. **Ölbehälter:** Leicht zugänglich für Nachfüllen
6. **Schlauchführung:** Direkt zur Kette führen

### Testaufbau auf dem Breadboard

Für erste Tests empfiehlt sich ein Aufbau auf einem Breadboard:

```
1. ESP32 in Breadboard stecken
2. GPS-Modul mit Jumper-Kabeln verbinden
3. OLED Display mit I2C-Kabeln verbinden
4. Pumpe mit PWM-Kabel verbinden (Servo-Tester hilft beim Test)
5. USB-Kabel für Stromversorgung und Programmierung
```

### Fehlersuche Hardware

| Problem | Mögliche Ursache | Lösung |
|---------|-----------------|--------|
| GPS findet keine Satelliten | Keine Sicht zum Himmel | GPS-Antenne nach außen |
| Display bleibt dunkel | Falsche I2C-Adresse | I2C Scanner verwenden |
| Pumpe läuft nicht | Zu wenig Strom | Externe Stromversorgung |
| ESP32 startet nicht | Überlast | Pumpe separat versorgen |
| Instabile Verbindung | Lockere Kabel | Löten statt Steckverbinder |

### Sicherheitshinweise

⚠️ **WICHTIG:**

1. Niemals Pumpe direkt vom ESP32 mit Strom versorgen
2. Immer gemeinsame Masse (GND) verwenden
3. Keine Kurzschlüsse riskieren
4. Richtige Polung beachten
5. Wasserdichte Installation am Motorrad

### Erweiterte Optionen

Optional können weitere Sensoren hinzugefügt werden:

- **Temperatursensor** (z.B. DS18B20) -> GPIO 4
- **Regensensor** -> GPIO 34 (ADC)
- **Zusätzliche Taster** -> GPIO 35, 36 (Eingänge)
- **Zweite LED** -> GPIO 13

## Viel Erfolg beim Aufbau!
