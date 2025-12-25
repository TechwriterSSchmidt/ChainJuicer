#ifndef HTML_PAGES_H
#define HTML_PAGES_H

const char* htmlHeader = R"rawliteral(
<html>
<head>
    <meta charset="UTF-8">
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Chain Juicer v0.9</title>
    <style>
        body{font-family:sans-serif;margin:0;padding:10px;background:#f4f4f9}
        h2{text-align:center;color:#333}
        h3{color:#555;margin-top:20px}
        form{background:#fff;padding:15px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1)}
        table{width:100%;border-collapse:collapse}
        th{text-align:left;color:#666;font-size:0.9em}
        td{padding:10px 5px;border-bottom:1px solid #eee}
        input{width:100%;padding:8px;border:1px solid #ddd;border-radius:4px;font-size:16px;box-sizing:border-box}
        input[type=checkbox]{width:20px;height:20px}
        .num-input{width:80px;text-align:center}
        .km-input{width:70px;text-align:center}
        .pulse-input{width:50px;text-align:center}
        .btn{background:#007bff;color:white;padding:12px;border:none;width:100%;font-size:16px;border-radius:4px;margin-top:15px;cursor:pointer}
        .progress{text-align:center;margin-top:15px;color:#555}
        .time{text-align:center;color:#888;font-size:0.9em;margin-bottom:10px}
        .help-link{text-align:center;margin-bottom:15px}
        .help-link a{color:#007bff;text-decoration:none}
        .disabled{opacity:0.5;pointer-events:none}
    </style>
</head>
<body>
    <h2>🍋 Chain Juicer v1.0</h2>
    <div class='help-link'><a href='/help'>Help & Manual</a></div>
    <div class='time'>Time: %TIME% | Sats: %SATS%</div>
    <form action='/save' method='POST'>
        <h3>Driving Profile</h3>
        <table>
            <tr>
                <th rowspan='2'>Speed</th>
                <th rowspan='2'>km</th>
                <th colspan='3' style='text-align:left'>(Last 20 juices only)</th>
            </tr>
            <tr>
                <th style='text-align:center'>Usage %</th>
                <th style='text-align:center'>Juices</th>
                <th style='text-align:center'>Pulses</th>
            </tr>
)rawliteral";

const char* htmlFooter = R"rawliteral(
        </table>
        <h3>Temperature Compensation</h3>
        <table>
            <tr><th>Range</th><th>Max Temp</th><th>Pulse</th><th>Pause</th></tr>
            <tr class='%ROW_CLASS_0%'><td>Very Cold</td><td><input type='number' step='0.1' name='t0_m' value='%T0_MAX%' class='km-input'></td><td><input type='number' name='t0_p' value='%T0_P%' class='pulse-input'></td><td><input type='number' name='t0_w' value='%T0_W%' class='num-input'></td></tr>
            <tr class='%ROW_CLASS_1%'><td>Cold</td><td><input type='number' step='0.1' name='t1_m' value='%T1_MAX%' class='km-input'></td><td><input type='number' name='t1_p' value='%T1_P%' class='pulse-input'></td><td><input type='number' name='t1_w' value='%T1_W%' class='num-input'></td></tr>
            <tr class='%ROW_CLASS_2%'><td>Normal</td><td><input type='number' step='0.1' name='t2_m' value='%T2_MAX%' class='km-input'></td><td><input type='number' name='t2_p' value='%T2_P%' class='pulse-input'></td><td><input type='number' name='t2_w' value='%T2_W%' class='num-input'></td></tr>
            <tr class='%ROW_CLASS_3%'><td>Warm</td><td><input type='number' step='0.1' name='t3_m' value='%T3_MAX%' class='km-input'></td><td><input type='number' name='t3_p' value='%T3_P%' class='pulse-input'></td><td><input type='number' name='t3_w' value='%T3_W%' class='num-input'></td></tr>
            <tr class='%ROW_CLASS_4%'><td>Hot</td><td>&gt; Warm</td><td><input type='number' name='t4_p' value='%T4_P%' class='pulse-input'></td><td><input type='number' name='t4_w' value='%T4_W%' class='num-input'></td></tr>
        </table>
        <h3>General</h3>
        <table>
            <tr><td>Rain Mode (x2)</td><td><input type='checkbox' name='rain_mode' %RAIN_CHECKED%></td></tr>
            <tr><td>Force Emergency Mode (simulates 50km/h constant speed)</td><td><input type='checkbox' name='emerg_mode' %EMERG_CHECKED%></td></tr>
        </table>
        <h3>LED Settings (Day)</h3>
        <table>
            <tr><td>Brightness (%)</td><td><input type='number' min='0' max='100' name='led_dim' value='%LED_DIM%' class='num-input'></td></tr>
            <tr><td>Flash Brightness (%)</td><td><input type='number' min='0' max='100' name='led_high' value='%LED_HIGH%' class='num-input'></td></tr>
        </table>
        <h3>LED Settings (Night)</h3>
        <table>
            <tr><td>Enable</td><td><input type='checkbox' name='night_en' %NIGHT_CHECKED%></td></tr>
            <tr><td>Start (Hour)</td><td><input type='number' name='night_start' value='%NIGHT_START%' class='num-input'></td></tr>
            <tr><td>End (Hour)</td><td><input type='number' name='night_end' value='%NIGHT_END%' class='num-input'></td></tr>
            <tr><td>Brightness (%)</td><td><input type='number' min='0' max='100' name='night_bri' value='%NIGHT_BRI%' class='num-input'></td></tr>
            <tr><td>Flash Brightness (%)</td><td><input type='number' min='0' max='100' name='night_bri_h' value='%NIGHT_BRI_H%' class='num-input'></td></tr>
        </table>
        <h3>Tank Monitor</h3>
        <table>
            <tr><td>Enable</td><td><input type='checkbox' name='tank_en' %TANK_CHECKED%></td></tr>
            <tr><td>Capacity (ml)</td><td><input type='number' step='1' name='tank_cap' value='%TANK_CAP%' class='num-input'></td></tr>
            <tr><td>Drops/ml</td><td><input type='number' name='drop_ml' value='%DROP_ML%' class='num-input'></td></tr>
            <tr><td>Drops/Pulse</td><td><input type='number' name='drop_pls' value='%DROP_PLS%' class='num-input'></td></tr>
            <tr><td>Warning at (%)</td><td><input type='number' name='tank_warn' value='%TANK_WARN%' class='num-input'></td></tr>
            <tr><td>Current Level</td><td>%TANK_LEVEL% ml (%TANK_PCT%%)</td></tr>
        </table>
        <div style='margin-top:10px'><a href='/refill' style='color:green;text-decoration:none;font-size:0.9em'>[Refill Tank]</a></div>
        <h3>Statistics</h3>
        <table>
            <tr><td>Total Distance</td><td>%TOTAL_DIST% km</td></tr>
            <tr><td>Total Juices</td><td>%PUMP_COUNT%</td></tr>
        </table>
        <div style='margin-top:10px'><a href='/reset_stats' style='color:red;text-decoration:none;font-size:0.9em'>[Reset Stats]</a></div>
        <div class='progress'>Current Progress: %PROGRESS%%</div>
        <input type='submit' value='Save' class='btn'>
        <div style='margin-top:20px;text-align:center'><a href='/update' style='color:#999;text-decoration:none;font-size:0.8em'>[Firmware Update]</a></div>
    </form>
</body>
</html>
)rawliteral";

const char* htmlHelp = R"rawliteral(
<html>
<head>
    <meta charset="UTF-8">
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Help</title>
    <style>
        body{font-family:sans-serif;margin:0;padding:15px;background:#fff;line-height:1.6}
        h2{color:#333}
        h3{color:#007bff;border-bottom:1px solid #eee;padding-bottom:5px}
        ul{padding-left:20px}
        .color-box{display:inline-block;width:12px;height:12px;margin-right:5px;border-radius:50%}
        .btn{display:block;background:#6c757d;color:white;text-align:center;padding:10px;text-decoration:none;border-radius:4px;margin-top:20px}
    </style>
</head>
<body>
    <h2>Manual</h2>
    <h3>Features</h3>
    <ul>
        <li><b>GPS Control:</b> Speed-dependent intervals (Active > 10 km/h).</li>
        <li><b>Drift Filter:</b> Ignores GPS signal reflections (multipath) to prevent ghost mileage.</li>
        <li><b>Rain Mode:</b>
            <ul>
                <li>Doubles oil amount.</li>
                <li>Turns off automatically after 30 min or on reboot.</li>
            </ul>
        </li>
        <li><b>Emergency Mode:</b>
            <ul>
                <li>Auto-activates if no GPS > 3 min.</li>
                <li>Simulates a constant speed of 50 km/h and adds to odometer.</li>
                <li>Disables 'Rain Mode'.</li>
            </ul>
        </li>
        <li><b>Night Mode:</b> Dims LED during defined hours.</li>
        <li><b>Auto-Save:</b> Settings and progress saved at standstill (< 7 km/h).</li>
        <li><b>Stats:</b> Tracks odometer, juices, and speed profile.</li>
    </ul>
    <h3>Configuration</h3>
    <p>Configure oiling intervals per speed range in the main table:</p>
    <ul>
        <li><b>Speed:</b> Speed range (e.g. 10-35 km/h).</li>
        <li><b>km:</b> Distance interval (km) between oilings.</li>
        <li><b>Usage %:</b> Percentage of driving time in this range (helps optimizing).</li>
        <li><b>Juices:</b> Number of oiling events triggered.</li>
        <li><b>Pulses:</b> Pump pulses per event.</li>
    </ul>
    <p>Use '[Reset Stats]' to clear 'Usage %' and 'Juices'.</p>
    <h3>Temperature Compensation</h3>
    <p>Adjusts pump mechanics based on oil viscosity (Temperature).</p>
    <ul>
        <li><b>Sensor:</b> Requires DS18B20 sensor. If missing, only "Normal" range is active.</li>
        <li><b>Ranges:</b> 5 temperature zones (Very Cold to Hot).</li>
        <li><b>Pulse:</b> Duration of the pump stroke (ms). Higher = More power for thick oil.</li>
        <li><b>Pause:</b> Wait time after stroke (ms). Higher = More time for refill.</li>
        <li><b>Max Temp:</b> Upper limit for the range.</li>
    </ul>
    <h3>WiFi & Web Interface</h3>
    <p>WiFi is <b>OFF</b> by default.</p>
    <ul>
        <li><b>Activate:</b> Hold button (> 3s) at standstill.</li>
        <li><b>Deactivate:</b> Auto-off when driving (> 10 km/h) or after 5 min inactivity.</li>
        <li><b>Update:</b> Upload new firmware (.bin) via the web interface.</li>
    </ul>
    <h3>Button Functions</h3>
    <ul>
        <li><b>Short Press (< 1.5s):</b> Toggle 'Rain Mode'.</li>
        <li><b>Long Press (> 10s):</b> 'Bleeding Mode' (10s pump). Only at standstill.</li>
    </ul>
    <h3>LED Status</h3>
    <ul>
        <li><span class='color-box' style='background:green'></span> <b>Green:</b> GPS OK (Ready).</li>
        <li><span class='color-box' style='background:blue'></span> <b>Blue:</b> 'Rain Mode' Active.</li>
        <li><span class='color-box' style='background:magenta'></span> <b>Magenta:</b> No GPS Signal.</li>
        <li><span class='color-box' style='background:cyan'></span> <b>Cyan:</b> 'Emergency Mode' (No GPS > 3 min).</li>
        <li><span class='color-box' style='background:yellow'></span> <b>Yellow:</b> Oiling (3s).</li>
        <li><span class='color-box' style='border:1px solid #ccc'></span> <b>White pulse:</b> 'WiFi Config' Active.</li>
        <li><span class='color-box' style='background:red'></span> <b>Red (2x blink):</b> 'Tank Warning'.</li>
        <li><span class='color-box' style='background:red'></span> <b>Red blink:</b> 'Bleeding Mode'.</li>
        <li><span class='color-box' style='background:cyan'></span> <b>Cyan (fast blink):</b> Firmware Update.</li>
    </ul>
    <a href='/' class='btn'>Back</a>
</body>
</html>
)rawliteral";

const char* htmlUpdate = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Firmware Update</title>
    <style>
        body{font-family:sans-serif;margin:0;padding:20px;background:#f4f4f9;text-align:center}
        h2{color:#333}
        form{background:#fff;padding:20px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);display:inline-block}
        input[type=file]{margin-bottom:15px}
        input[type=submit]{background:#007bff;color:white;padding:10px 20px;border:none;border-radius:4px;cursor:pointer;font-size:16px}
    </style>
</head>
<body>
    <h2>Firmware Update</h2>
    <form method='POST' action='/update' enctype='multipart/form-data'>
        <input type='file' name='update'>
        <br>
        <input type='submit' value='Update Firmware'>
    </form>
</body>
</html>
)rawliteral";

#endif
