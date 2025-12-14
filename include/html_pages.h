#ifndef HTML_PAGES_H
#define HTML_PAGES_H

// Simple Lemon SVG Icon
const char* lemonIcon = "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><path d='M15,50 Q15,15 50,15 Q85,15 85,50 Q85,85 50,85 Q15,85 15,50' fill='#FFEB3B' stroke='#FBC02D' stroke-width='3' transform='rotate(-45, 50, 50)'/><circle cx='35' cy='40' r='2' fill='#F9A825' opacity='0.6'/><circle cx='65' cy='60' r='2' fill='#F9A825' opacity='0.6'/><circle cx='50' cy='50' r='2' fill='#F9A825' opacity='0.6'/></svg>";

const char* htmlHeader = R"rawliteral(
<html>
<head>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Chain Juicer</title>
    <link rel="icon" href="data:image/svg+xml;utf8,%3Csvg%20xmlns%3D%27http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%27%20viewBox%3D%270%200%20100%20100%27%3E%3Cpath%20d%3D%27M15%2C50%20Q15%2C15%2050%2C15%20Q85%2C15%2085%2C50%20Q85%2C85%2050%2C85%20Q15%2C85%2015%2C50%27%20fill%3D%27%23FFEB3B%27%20stroke%3D%27%23FBC02D%27%20stroke-width%3D%273%27%20transform%3D%27rotate(-45%2C%2050%2C%2050)%27%2F%3E%3Ccircle%20cx%3D%2735%27%20cy%3D%2740%27%20r%3D%272%27%20fill%3D%27%23F9A825%27%20opacity%3D%270.6%27%2F%3E%3Ccircle%20cx%3D%2765%27%20cy%3D%2760%27%20r%3D%272%27%20fill%3D%27%23F9A825%27%20opacity%3D%270.6%27%2F%3E%3Ccircle%20cx%3D%2750%27%20cy%3D%2750%27%20r%3D%272%27%20fill%3D%27%23F9A825%27%20opacity%3D%270.6%27%2F%3E%3C%2Fsvg%3E">
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
        .btn{background:#007bff;color:white;padding:12px;border:none;width:100%;font-size:16px;border-radius:4px;margin-top:15px;cursor:pointer}
        .progress{text-align:center;margin-top:15px;color:#555}
        .time{text-align:center;color:#888;font-size:0.9em;margin-bottom:10px}
        .help-link{text-align:center;margin-bottom:15px}
        .help-link a{color:#007bff;text-decoration:none}
    </style>
</head>
<body>
    <h2>🍋 Chain Juicer</h2>
    <div class='help-link'><a href='/help'>Help & Manual</a></div>
    <div class='time'>Time: %TIME% | Sats: %SATS%</div>
    <form action='/save' method='POST'>
        <h3>Driving Profile</h3>
        <table>
            <tr>
                <th rowspan='2'>Speed</th>
                <th rowspan='2'>Km</th>
                <th colspan='2' style='text-align:center'>(Last 20 Cycles)</th>
                <th rowspan='2'>Pulses</th>
            </tr>
            <tr>
                <th style='text-align:center'>Usage %</th>
                <th style='text-align:center'>Cycles</th>
            </tr>
)rawliteral";

const char* htmlFooter = R"rawliteral(
        </table>
        <h3>General</h3>
        <table>
            <tr><td>Rain Mode (x2)</td><td><input type='checkbox' name='rain_mode' %RAIN_CHECKED%></td></tr>
            <tr><td>Force Emergency Mode (simulates 50km/h constant speed)</td><td><input type='checkbox' name='emerg_mode' %EMERG_CHECKED%></td></tr>
        </table>
        <h3>LED Settings (Day)</h3>
        <table>
            <tr><td>Brightness (%)</td><td><input type='number' min='0' max='100' name='led_dim' value='%LED_DIM%'></td></tr>
            <tr><td>Flash Brightness (%)</td><td><input type='number' min='0' max='100' name='led_high' value='%LED_HIGH%'></td></tr>
        </table>
        <h3>LED Settings (Night)</h3>
        <table>
            <tr><td>Enable</td><td><input type='checkbox' name='night_en' %NIGHT_CHECKED%></td></tr>
            <tr><td>Start (Hour)</td><td><input type='number' name='night_start' value='%NIGHT_START%'></td></tr>
            <tr><td>End (Hour)</td><td><input type='number' name='night_end' value='%NIGHT_END%'></td></tr>
            <tr><td>Brightness (%)</td><td><input type='number' min='0' max='100' name='night_bri' value='%NIGHT_BRI%'></td></tr>
            <tr><td>Flash Brightness (%)</td><td><input type='number' min='0' max='100' name='night_bri_h' value='%NIGHT_BRI_H%'></td></tr>
        </table>
        <h3>Tank Monitor</h3>
        <table>
            <tr><td>Enable</td><td><input type='checkbox' name='tank_en' %TANK_CHECKED%></td></tr>
            <tr><td>Capacity (ml)</td><td><input type='number' step='1' name='tank_cap' value='%TANK_CAP%'></td></tr>
            <tr><td>Drops/ml</td><td><input type='number' name='drop_ml' value='%DROP_ML%'></td></tr>
            <tr><td>Drops/Pulse</td><td><input type='number' name='drop_pls' value='%DROP_PLS%'></td></tr>
            <tr><td>Warning at (%)</td><td><input type='number' name='tank_warn' value='%TANK_WARN%'></td></tr>
            <tr><td>Current Level</td><td>%TANK_LEVEL% ml (%TANK_PCT%%)</td></tr>
        </table>
        <div style='margin-top:10px'><a href='/refill' style='color:green;text-decoration:none;font-size:0.9em'>[Refill Tank]</a></div>
        <h3>Statistics</h3>
        <table>
            <tr><td>Total Distance</td><td>%TOTAL_DIST% km</td></tr>
            <tr><td>Total Cycles</td><td>%PUMP_COUNT%</td></tr>
        </table>
        <div style='margin-top:10px'><a href='/reset_stats' style='color:red;text-decoration:none;font-size:0.9em'>[Reset Stats]</a></div>
        <div class='progress'>Current Progress: %PROGRESS%%</div>
        <input type='submit' value='Save' class='btn'>
    </form>
</body>
</html>
)rawliteral";

const char* htmlHelp = R"rawliteral(
<html>
<head>
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
        <li><b>Stats:</b> Tracks odometer, oiling cycles, and speed profile.</li>
    </ul>
    <h3>Configuration</h3>
    <p>Configure oiling intervals per speed range in the main table:</p>
    <ul>
        <li><b>Speed:</b> Speed range (e.g. 10-35 km/h).</li>
        <li><b>Km:</b> Distance interval (km) between oilings.</li>
        <li><b>Usage %:</b> Percentage of driving time in this range (helps optimizing).</li>
        <li><b>Cycles:</b> Number of oiling events triggered.</li>
        <li><b>Pulses:</b> Pump pulses per event.</li>
    </ul>
    <p>Use '[Reset Stats]' to clear 'Usage %' and 'Cycles'.</p>
    <h3>WiFi & Web Interface</h3>
    <p>WiFi is <b>OFF</b> by default.</p>
    <ul>
        <li><b>Activate:</b> Hold button (> 3s) at standstill.</li>
        <li><b>Deactivate:</b> Auto-off when driving (> 10 km/h) or after 5 min inactivity.</li>
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
    </ul>
    <a href='/' class='btn'>Back</a>
</body>
</html>
)rawliteral";

#endif
