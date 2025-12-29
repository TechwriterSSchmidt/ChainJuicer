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
    <div class='help-link'>
        <a href='/help'>Help & Manual</a> | 
        <a href='/imu'>IMU Calibration</a>
    </div>
    <div class='time'>Time: %TIME% | Sats: %SATS% | Temp: %TEMP%&deg;C</div>
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
            <tr><td>Pulse Duration (ms) @ 25&deg;C</td><td><input type='number' min='50' name='tc_pulse' value='%TC_PULSE%' class='pulse-input'></td></tr>
            <tr><td>Pause Duration (ms) @ 25&deg;C</td><td><input type='number' name='tc_pause' value='%TC_PAUSE%' class='num-input'></td></tr>
            <tr><td colspan='2'><b>Oil Viscosity Profile:</b></td></tr>
            <tr><td><input type='radio' name='oil_type' value='0' %OIL_THIN%> Thin Oil</td><td>(e.g. ATF / Bio)</td></tr>
            <tr><td><input type='radio' name='oil_type' value='1' %OIL_NORMAL%> Normal Oil</td><td>(e.g. Mineral Oil 80w90)</td></tr>
            <tr><td><input type='radio' name='oil_type' value='2' %OIL_THICK%> Thick Oil</td><td>(e.g. Gear Oil SAE 90)</td></tr>
            <tr><td colspan='2' style='font-size:0.8em;color:#666'>Current Temp: %TEMP_C% &deg;C</td></tr>
        </table>
        <h3>General</h3>
        <table>
            <tr><td>Rain Mode (x2)</td><td><input type='checkbox' name='rain_mode' %RAIN_CHECKED%></td></tr>
            <tr><td>Force Emergency Mode (simulates 50km/h constant speed)</td><td><input type='checkbox' name='emerg_mode' %EMERG_CHECKED%></td></tr>
            <tr><td>Start Delay (km)</td><td><input type='number' step='0.1' name='start_dly' value='%START_DLY%' class='num-input'></td></tr>
            <tr><td>Cross-Country Interval (min)</td><td><input type='number' name='cc_int' value='%CC_INT%' class='num-input'></td></tr>
            <tr><td colspan='2'><b>Chain Flush Mode:</b></td></tr>
            <tr><td>Events (Total)</td><td><input type='number' name='flush_ev' value='%FLUSH_EV%' class='num-input'></td></tr>
            <tr><td>Pulses per Event</td><td><input type='number' name='flush_pls' value='%FLUSH_PLS%' class='num-input'></td></tr>
            <tr><td>Interval (Seconds)</td><td><input type='number' name='flush_int' value='%FLUSH_INT%' class='num-input'></td></tr>
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
        <li><b>Chain Flush Mode:</b>
            <ul>
                <li>Oils periodically (Time based) for cleaning.</li>
                <li>Useful for cleaning the chain or after heavy rain.</li>
            </ul>
        </li>
        <li><b>Cross-Country Mode:</b>
            <ul>
                <li>Activates via <b>6x Button Click</b>.</li>
                <li>Oils purely based on time (e.g. every 5 min).</li>
                <li>Useful for slow offroad riding where distance is short but chain needs oil.</li>
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
        <li><b>Pulses:</b> Pump pulses per event (Default: 2).</li>
    </ul>
    <p>Use '[Reset Stats]' to clear 'Usage %' and 'Juices'.</p>
    <h3>Temperature Compensation</h3>
    <p>Adjusts pump mechanics based on oil viscosity (Temperature).</p>
    <ul>
        <li><b>Sensor:</b> Requires temperature sensor. If missing, defaults to settings for normal temperature.</li>
        <li><b>Reference:</b> Set Pulse and Pause for normal temperature.</li>
        <li><b>Oil Type:</b> Select your oil viscosity profile (Thin, Normal, Thick).</li>
        <li><b>Logic:</b> The system automatically calculates the required energy for colder/warmer temperatures based on the selected profile.</li>
        <li><b>Hysteresis:</b> 3&deg;C buffer prevents rapid switching.</li>
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
        <li><b>Short Press (< 1.5s):</b> Toggle 'Rain Mode' (with 400ms delay).</li>
        <li><b>3x Click:</b> Toggle 'Chain Flush Mode' (Configurable).</li>
        <li><b>6x Click:</b> Toggle 'Cross-Country Mode' (Time based oiling).</li>
        <li><b>Long Press (> 10s):</b> 'Bleeding Mode' (15s pump). Only at standstill.</li>
    </ul>
    <h3>LED Status</h3>
    <ul>
        <li><span class='color-box' style='background:green'></span> <b>Green:</b> GPS OK (Ready).</li>
        <li><span class='color-box' style='background:blue'></span> <b>Blue:</b> 'Rain Mode' Active.</li>
        <li><span class='color-box' style='background:cyan'></span> <b>Cyan (blink):</b> 'Chain Flush Mode' Active.</li>
        <li><span class='color-box' style='background:magenta'></span> <b>Magenta:</b> No GPS Signal.</li>
        <li><span class='color-box' style='background:cyan'></span> <b>Cyan:</b> 'Emergency Mode' (No GPS > 3 min).</li>
        <li><span class='color-box' style='background:yellow'></span> <b>Yellow:</b> Oiling (3s, breathes 3x).</li>
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

const char* htmlIMU = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>IMU Calibration</title>
    <style>
        body{font-family:sans-serif;margin:0;padding:10px;background:#f4f4f9}
        h2{text-align:center;color:#333}
        h3{color:#555;margin-top:20px}
        .card{background:#fff;padding:15px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);margin-bottom:15px}
        table{width:100%;border-collapse:collapse}
        td{padding:8px 5px;border-bottom:1px solid #eee}
        .btn{background:#007bff;color:white;padding:12px;border:none;width:100%;font-size:16px;border-radius:4px;margin-top:10px;cursor:pointer}
        .btn-cal{background:#28a745}
        .val{font-weight:bold;font-family:monospace}
        .note{font-size:0.9em;color:#666;margin-top:5px}
    </style>
</head>
<body>
    <h2>🧭 IMU Calibration</h2>
    
    <div class='card'>
        <h3>Sensor Status</h3>
        <table>
            <tr><td>Model:</td><td class='val'>%IMU_MODEL%</td></tr>
            <tr><td>Status:</td><td class='val'>%IMU_STATUS%</td></tr>
        </table>
    </div>

    <div class='card'>
        <h3>Live Orientation</h3>
        <table>
            <tr><td>Pitch (Nose Up/Down):</td><td class='val'>%PITCH%&deg;</td></tr>
            <tr><td>Roll (Lean Left/Right):</td><td class='val'>%ROLL%&deg;</td></tr>
        </table>
        <div class='note'>Values update on refresh.</div>
    </div>

    <div class='card'>
        <h3>Calibration</h3>
        <p>Park the bike on the <b>Center Stand</b> (level ground) and press 'Set Zero'.</p>
        <form action='/imu_zero' method='POST'>
            <input type='submit' value='Set Zero Position' class='btn btn-cal'>
        </form>
        
        <p style='margin-top:20px'>Park the bike on the <b>Side Stand</b> and press 'Set Side Stand'.</p>
        <form action='/imu_side' method='POST'>
            <input type='submit' value='Set Side Stand Position' class='btn btn-cal'>
        </form>
    </div>

    <div class='card'>
        <h3>Configuration</h3>
        <form action='/imu_config' method='POST'>
            <table>
                <tr>
                    <td>Chain Side:</td>
                    <td>
                        <select name='chain_side'>
                            <option value='0' %CHAIN_LEFT%>Left (Standard)</option>
                            <option value='1' %CHAIN_RIGHT%>Right</option>
                        </select>
                    </td>
                </tr>
            </table>
            <input type='submit' value='Save Config' class='btn'>
        </form>
    </div>

    <div class='card'>
        <h3>Features</h3>
        <ul>
            <li><b>Garage Guard:</b> Prevents oiling if lean angle > 10&deg; (Side Stand).</li>
            <li><b>Crash Detect:</b> Stops pump if lean angle > 60&deg;.</li>
            <li><b>Smart Stop:</b> Detects stops faster than GPS.</li>
            <li><b>Turn Safety:</b> Stops oiling in turns towards the chain side (> 20&deg;).</li>
        </ul>
    </div>

    <a href='/' class='btn'>Back to Main</a>
</body>
</html>
)rawliteral";

#endif
