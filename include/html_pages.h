#ifndef HTML_PAGES_H
#define HTML_PAGES_H

const char* htmlHeader = R"rawliteral(
<html>
<head>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Chain Oiler</title>
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
    <h2>Chain Oiler Config</h2>
    <div class='help-link'><a href='/help'>Help & Manual</a></div>
    <div class='time'>Time (GPS): %TIME% | Sats: %SATS%</div>
    <form action='/save' method='POST'>
        <h3>Recent Driving Profile</h3>
        <table>
            <tr>
                <th rowspan='2'>Range</th>
                <th rowspan='2'>Km</th>
                <th colspan='2' style='text-align:center'>(Last 20 Lubes)</th>
                <th rowspan='2'>Pulses</th>
            </tr>
            <tr>
                <th style='text-align:center'>Distrib.</th>
                <th style='text-align:center'>Lubes  </th>
            </tr>
)rawliteral";

const char* htmlFooter = R"rawliteral(
        </table>
        <h3>General</h3>
        <table>
            <tr><td>Rain Mode (x2)</td><td><input type='checkbox' name='rain_mode' %RAIN_CHECKED%></td></tr>
            <tr><td>Force Emergency Mode (Simulate 50km/h)</td><td><input type='checkbox' name='emerg_mode' %EMERG_CHECKED%></td></tr>
        </table>
        <h3>LED Settings (daylight)</h3>
        <table>
            <tr><td>Brightness (%)</td><td><input type='number' min='0' max='100' name='led_dim' value='%LED_DIM%'></td></tr>
            <tr><td>Event Brightness (%)</td><td><input type='number' min='0' max='100' name='led_high' value='%LED_HIGH%'></td></tr>
        </table>
        <h3>LED Settings (night mode)</h3>
        <table>
            <tr><td>Enable</td><td><input type='checkbox' name='night_en' %NIGHT_CHECKED%></td></tr>
            <tr><td>Start (Hour)</td><td><input type='number' name='night_start' value='%NIGHT_START%'></td></tr>
            <tr><td>End (Hour)</td><td><input type='number' name='night_end' value='%NIGHT_END%'></td></tr>
            <tr><td>Brightness (%)</td><td><input type='number' min='0' max='100' name='night_bri' value='%NIGHT_BRI%'></td></tr>
            <tr><td>Event Brightness (%)</td><td><input type='number' min='0' max='100' name='night_bri_h' value='%NIGHT_BRI_H%'></td></tr>
        </table>
        <h3>Tank Monitor</h3>
        <table>
            <tr><td>Enable</td><td><input type='checkbox' name='tank_en' %TANK_CHECKED%></td></tr>
            <tr><td>Capacity (ml)</td><td><input type='number' step='1' name='tank_cap' value='%TANK_CAP%'></td></tr>
            <tr><td>Drops/ml</td><td><input type='number' name='drop_ml' value='%DROP_ML%'></td></tr>
            <tr><td>Drops/Pulse</td><td><input type='number' name='drop_pls' value='%DROP_PLS%'></td></tr>
            <tr><td>Warning (%)</td><td><input type='number' name='tank_warn' value='%TANK_WARN%'></td></tr>
            <tr><td>Level</td><td>%TANK_LEVEL% ml (%TANK_PCT%%)</td></tr>
        </table>
        <div style='margin-top:10px'><a href='/refill' style='color:green;text-decoration:none;font-size:0.9em'>[Refill Tank]</a></div>
        <h3>Statistics</h3>
        <table>
            <tr><td>Total Distance</td><td>%TOTAL_DIST% km</td></tr>
            <tr><td>Oiling Cycles</td><td>%PUMP_COUNT%</td></tr>
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
        <li><b>Smart Filter:</b> Ignores indoor 'ghost speeds' (Multipath protection).</li>
        <li><b>Rain Mode:</b> Double oil amount, Auto-Off after 30 min.</li>
        <li><b>Emergency Mode:</b> Auto-active if no GPS > 3 min. Simulates 50 km/h. Disables Rain Mode.</li>
        <li><b>Night Mode:</b> Automatic LED dimming.</li>
        <li><b>Web Config:</b> Settings via Smartphone.</li>
        <li><b>Memory:</b> Settings and progress are saved automatically at standstill (< 7 km/h).</li>
        <li><b>Statistics:</b> Odometer, oiling counter, and driving profile.</li>
    </ul>
    <h3>Configuration</h3>
    <p>The main table allows you to configure the oiling intervals for different speed ranges.</p>
    <ul>
        <li><b>Range:</b> Speed range (e.g. 10-35 km/h).</li>
        <li><b>Km:</b> Distance interval for this range.</li>
        <li><b>Time %:</b> Percentage of total driving time spent in this range. Helps to optimize intervals.</li>
        <li><b>Oilings:</b> Number of oiling events triggered in this range.</li>
        <li><b>Pulses:</b> Number of pump pulses per oiling event.</li>
    </ul>
    <p>Use the <b>[Reset Stats]</b> link below the table to reset the Time % and Oilings statistics.</p>
    <h3>WiFi & Web Interface</h3>
    <p>WiFi is <b>OFF</b> by default.</p>
    <ul>
        <li><b>Activate:</b> Hold button (> 3s) while standing still (< 7 km/h).</li>
        <li><b>Deactivate:</b> Automatically when driving (> 10 km/h) or after 5 min inactivity.</li>
        <li><b>Signal:</b> LED pulses white when WiFi is active.</li>
    </ul>
    <h3>Modes & Button</h3>
    <ul>
        <li><b>Short Press (< 1.5s):</b> Toggle Rain Mode (Half interval).</li>
        <li><b>Long Press (> 10s):</b> Bleeding Mode (Pump runs 10s). Only at standstill (< 7 km/h).</li>
    </ul>
    <h3>LED Color Scheme</h3>
    <ul>
        <li><span class='color-box' style='background:green'></span> <b>Green:</b> Normal Operation (GPS OK).</li>
        <li><span class='color-box' style='background:blue'></span> <b>Blue:</b> Rain Mode Active.</li>
        <li><span class='color-box' style='background:magenta'></span> <b>Magenta:</b> No GPS Signal.</li>
        <li><span class='color-box' style='background:cyan'></span> <b>Cyan:</b> Emergency Mode (No GPS > 3 min).</li>
        <li><span class='color-box' style='background:yellow'></span> <b>Yellow:</b> Oiling in progress (3s).</li>
        <li><span class='color-box' style='border:1px solid #ccc'></span> <b>White pulsing:</b> WiFi Config Active.</li>
        <li><span class='color-box' style='background:red'></span> <b>Red pulsing:</b> Tank Warning (2x).</li>
        <li><span class='color-box' style='background:red'></span> <b>Red blinking:</b> Bleeding Mode Active.</li>
    </ul>
    <h3>LED Brightness</h3>
    <p>The brightness of the status LED can be adjusted in percent.</p>
    <ul>
        <li><b>Range:</b> 0% (Off) to 100% (Max).</li>
    </ul>
    <h3>Night Mode</h3>
    <p>Night mode automatically reduces LED brightness during a defined time window.</p>
    <ul>
        <li><b>Start/End:</b> Hour (0-23) for night mode.</li>
        <li><b>Brightness:</b> Brightness value during night (recommended: 5-10%).</li>
        <li><b>Event Brightness:</b> Brightness for events (Oiling, WiFi) during night.</li>
    </ul>
    <a href='/' class='btn'>Back</a>
</body>
</html>
)rawliteral";

#endif
