#ifndef HTML_PAGES_H
#define HTML_PAGES_H

const char* htmlCss = R"rawliteral(
body{font-family:sans-serif;margin:0;padding:10px;background:#121212;color:#e0e0e0}
h2{text-align:center;color:#ffffff;font-weight:normal;text-transform:uppercase;letter-spacing:1px}
h3{color:#ffc107;margin-top:0;border-bottom:1px solid #333;padding-bottom:5px;font-weight:bold}
.card{background:#1e1e1e;padding:15px;border-radius:4px;border:1px solid #333;margin-bottom:15px}
table{width:100%;border-collapse:collapse}
th{text-align:left;color:#ccc;font-size:1.1em}
td{padding:10px 5px;border-bottom:1px solid #333}
input{width:100%;padding:8px;border:1px solid #444;background:#333;color:#fff;border-radius:2px;font-size:16px;box-sizing:border-box}
input[type=checkbox]{width:20px;height:20px;accent-color:#ffc107}
select{background:#2d2d2d;color:#fff;border:1px solid #444;padding:5px;appearance:none;-webkit-appearance:none;background-image:url('data:image/svg+xml;charset=utf-8,%3Csvg%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%20width%3D%2224%22%20height%3D%2224%22%20viewBox%3D%220%200%2024%2024%22%20fill%3D%22%23ffc107%22%3E%3Cpath%20d%3D%22M7%2010l5%205%205-5z%22%2F%3E%3C%2Fsvg%3E');background-repeat:no-repeat;background-position:right 8px center;background-size:24px;padding-right:30px;width:100%;box-sizing:border-box}
.num-input{width:80px;text-align:center}
.km-input{width:70px;text-align:center}
.pulse-input{width:50px;text-align:center}
.btn{display:block;width:100%;box-sizing:border-box;background:#333;color:#fff;padding:12px;border:1px solid #555;font-size:16px;border-radius:4px;margin-top:15px;cursor:pointer;text-align:center;text-decoration:none}
.btn:active{background:#555}
.btn-sec{background:#222;color:#aaa;border:1px solid #444}
.btn-danger{background:#500;color:#fff;border:1px solid #800}
.btn-cal{background:#28a745;color:#fff;border:none}
.back-btn{display:block;width:100%;box-sizing:border-box;background:#ffc107;color:#000;text-align:center;padding:12px;text-decoration:none;border-radius:4px;margin-bottom:15px;border:1px solid #e0a800;font-size:20px;font-weight:bold}
.stat-box{background:#1e1e1e;padding:15px;border-radius:4px;border:1px solid #333;margin-bottom:15px;text-align:left}
.stat-row{display:flex;justify-content:space-between;margin-bottom:5px;border-bottom:1px solid #333;padding-bottom:5px}
.stat-row:last-child{border-bottom:none}
.val{font-weight:bold;color:#ffffff;font-family:monospace}
.status-bar{font-size:1.1em;color:#ccc;margin-bottom:20px;text-align:center}
.tank-bar{width:100%;height:10px;background:#333;border-radius:2px;overflow:hidden;margin-top:8px;border:1px solid #444}
.tank-fill{height:100%;background:#ccc;transition:width 0.3s}
.progress{text-align:center;margin-top:15px;color:#ccc}
.time{text-align:center;color:#ccc;font-size:1.1em;margin-bottom:10px}
.disabled{opacity:0.5;pointer-events:none}
.note{font-size:1.1em;color:#ccc;margin-top:5px}
.color-box{display:inline-block;width:12px;height:12px;margin-right:5px;border-radius:50%}
ul{padding-left:20px}
#console{width:100%;height:400px;background:#000;border:1px solid #444;padding:10px;overflow-y:scroll;white-space:pre-wrap;font-size:14px;box-sizing:border-box;color:#0f0;font-family:monospace}
)rawliteral";

const char* htmlLanding = R"rawliteral(
<html>
<head>
    <meta charset="UTF-8">
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Chain Juicer</title>
    <link rel="stylesheet" href="/style.css">
    <style>body{text-align:center} h2{margin-bottom:5px}</style>
</head>
<body>
    <h2>🍋 Chain Juicer</h2>
    <div class='status-bar'>Time: %TIME% | Sats: %SATS% | Temp: %TEMP%&deg;C</div>

    <div class='stat-box'>
        <h3>Statistics</h3>
        <div class='stat-row'><span>Odometer:</span> <span class='val'>%TOTAL_DIST% km</span></div>
        <div class='stat-row'><span>Juice Events:</span> <span class='val'>%PUMP_COUNT%</span></div>
        <div class='stat-row'><span>Progress:</span> <span class='val'>%PROGRESS%%</span></div>
    </div>

    <div class='stat-box'>
        <h3>Tank Monitor</h3>
        <div class='stat-row'><span>Level:</span> <span class='val'>%TANK_LEVEL% / %TANK_CAP% ml</span></div>
        <div class='tank-bar'><div class='tank-fill' style='width:%TANK_PCT%%;background-color:%TANK_COLOR%'></div></div>
    </div>

    <a href='/toggle_emerg' class='btn %EMERG_CLASS%'>⚠️ Emergency Mode: %EMERG_STATUS%</a>
    <a href='/settings' class='btn'>Juicer Settings</a>
    <a href='/led_settings' class='btn btn-sec'>LED Settings</a>
    <a href='/aux' class='btn btn-sec'>Aux Port</a>
    <a href='/maintenance' class='btn btn-sec'>Maintenance</a>
    <a href='/help' class='btn btn-sec'>Manual</a>
</body>
</html>
)rawliteral";

const char* htmlMaintenance = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Maintenance</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <a href='/' class='back-btn'>&larr; Home</a>
    <h2>Maintenance</h2>
    
    <div class='card'>
        <h3>System Tools</h3>
        <a href='/test_pump' class='btn' style='background:#555; margin-bottom:10px'>Test Pump (1 Pulse)</a>
        <a href='/imu' class='btn btn-sec' style='margin-bottom:10px'>IMU Configuration</a>
        <a href='/console' class='btn btn-sec' style='margin-bottom:10px'>Serial Console</a>
    </div>

    <div class='card'>
        <h3>Danger Zone</h3>
        <div class='note' style='font-weight:bold; margin-bottom:15px'>
            ⚠️ Reset and restart actions are executed immediately without confirmation!
        </div>
        <a href='/restart' class='btn' style='background:#555; margin-bottom:10px'>Restart System</a>        
        <a href='/update' class='btn' style='background:#555; color:#000; margin-bottom:10px'>Firmware Update</a>
        <a href='/factory_reset' class='btn btn-sec' style='background:#800; color:#fff; margin-bottom:10px'>Factory Reset</a>
        <a href='/bleeding' class='btn' style='background:#ffc107; color:#000; margin-bottom:10px'>Start Bleeding Mode</a>
    </div>
</body>
</html>
)rawliteral";

const char* htmlHeader = R"rawliteral(
<html>
<head>
    <meta charset="UTF-8">
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Juicer Settings</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <a href='/' class='back-btn'>&larr; Home</a>
    <h2>Juicer Settings</h2>
    <div class='time'>Time: %TIME% | Sats: %SATS% | Temp: %TEMP%&deg;C</div>
    <form action='/save' method='POST'>
        <div class='card'>
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
        </div>
        <div class='card'>
        <h3>Statistics</h3>
        <table>
            <tr><td>Total Distance</td><td>%TOTAL_DIST% km</td></tr>
            <tr><td>Total Juices</td><td>%PUMP_COUNT%</td></tr>
        </table>
        <div style='margin-top:10px'><a href='/reset_stats' style='color:#d32f2f;text-decoration:none;font-size:1.1em'>[Reset Stats]</a></div>
        <div class='progress'>Current Progress: %PROGRESS%%</div>
        </div>
        <div class='card'>
        <h3>General</h3>
        <table>
            <tr><td>Force Emergency Mode (simulates 50km/h constant speed)</td><td><input type='checkbox' name='emerg_mode' %EMERG_CHECKED%></td></tr>
            <tr><td>Start Delay (m)</td><td><input type='number' step='1' name='start_dly' value='%START_DLY%' class='num-input'></td></tr>
            <tr><td>Offroad Interval (min)</td><td><input type='number' name='offroad_int' value='%OFFROAD_INT%' class='num-input'></td></tr>
            <tr><td colspan='2'><b>Chain Flush Mode:</b></td></tr>
            <tr><td>Events (Total)</td><td><input type='number' name='flush_ev' value='%FLUSH_EV%' class='num-input'></td></tr>
            <tr><td>Pulses per Event</td><td><input type='number' name='flush_pls' value='%FLUSH_PLS%' class='num-input'></td></tr>
            <tr><td>Interval (Seconds)</td><td><input type='number' name='flush_int' value='%FLUSH_INT%' class='num-input'></td></tr>
        </table>
        </div>
        <div class='card'>
        <h3>Temperature Compensation</h3>
        <table>
            <tr><td>Pulse Duration (ms) @ 25&deg;C</td><td><input type='number' min='50' name='tc_pulse' value='%TC_PULSE%' class='pulse-input'></td></tr>
            <tr><td>Pause Duration (ms) @ 25&deg;C</td><td><input type='number' name='tc_pause' value='%TC_PAUSE%' class='num-input'></td></tr>
            <tr><td colspan='2'><b>Oil Viscosity Profile:</b></td></tr>
            <tr><td><input type='radio' name='oil_type' value='0' %OIL_THIN%> Thin Oil</td><td>(e.g. ATF / Bio)</td></tr>
            <tr><td><input type='radio' name='oil_type' value='1' %OIL_NORMAL%> Normal Oil</td><td>(e.g. Mineral Oil 80w90)</td></tr>
            <tr><td><input type='radio' name='oil_type' value='2' %OIL_THICK%> Thick Oil</td><td>(e.g. Gear Oil SAE 90)</td></tr>
            <tr><td colspan='2' style='font-size:1.1em;color:#888'>Current Temp: %TEMP_C% &deg;C</td></tr>
        </table>
        </div>
        <div class='card'>
        <h3>Tank Monitor</h3>
        <table>
            <tr><td>Enable</td><td><input type='checkbox' name='tank_en' %TANK_CHECKED%></td></tr>
            <tr><td>Capacity (ml)</td><td><input type='number' step='1' name='tank_cap' value='%TANK_CAP%' class='num-input'></td></tr>
            <tr><td>Drops/ml</td><td><input type='number' name='drop_ml' value='%DROP_ML%' class='num-input'></td></tr>
            <tr><td>Drops/Pulse</td><td><input type='number' name='drop_pls' value='%DROP_PLS%' class='num-input'></td></tr>
            <tr><td>Warning at (%)</td><td><input type='number' name='tank_warn' value='%TANK_WARN%' class='num-input'></td></tr>
            <tr><td>Current Level</td><td>%TANK_LEVEL% ml (%TANK_PCT%%)</td></tr>
        </table>
        <div style='margin-top:10px'><a href='/refill' style='color:#28a745;text-decoration:none;font-size:1.1em'>[Refill Tank]</a></div>
        </div>
        <input type='submit' value='Save' class='btn'>
        <div style='margin-top:20px;text-align:center'><a href='/update' style='color:#888;text-decoration:none;font-size:1.1em'>[Firmware Update]</a></div>
    </form>
</body>
</html>
)rawliteral";

const char* htmlLEDSettings = R"rawliteral(
<html>
<head>
    <meta charset="UTF-8">
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>LED Settings</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <a href='/' class='back-btn'>&larr; Home</a>
    <h2>LED Settings</h2>
    <form action='/save_led' method='POST'>
        <div class='card'>
            <h3>Daytime Settings</h3>
            <table>
                <tr><td>Brightness (%)</td><td><input type='number' min='0' max='100' name='led_dim' value='%LED_DIM%' class='num-input'></td></tr>
                <tr><td>Flash Brightness (%)</td><td><input type='number' min='0' max='100' name='led_high' value='%LED_HIGH%' class='num-input'></td></tr>
            </table>
        </div>
        <div class='card'>
            <h3>Nighttime Settings</h3>
            <table>
                <tr><td>Enable</td><td><input type='checkbox' name='night_en' %NIGHT_CHECKED%></td></tr>
                <tr><td>Start (Hour)</td><td><input type='number' name='night_start' value='%NIGHT_START%' class='num-input'></td></tr>
                <tr><td>End (Hour)</td><td><input type='number' name='night_end' value='%NIGHT_END%' class='num-input'></td></tr>
                <tr><td>Brightness (%)</td><td><input type='number' min='0' max='100' name='night_bri' value='%NIGHT_BRI%' class='num-input'></td></tr>
                <tr><td>Flash Brightness (%)</td><td><input type='number' min='0' max='100' name='night_bri_h' value='%NIGHT_BRI_H%' class='num-input'></td></tr>
            </table>
        </div>
        <input type='submit' value='Save' class='btn'>
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
    <link rel="stylesheet" href="/style.css">
    <style>body{line-height:1.6}</style>
</head>
<body>
    <a href='/' class='back-btn'>&larr; Home</a>
    <h2>Manual</h2>
    <div style='text-align:center;color:#888;margin-bottom:15px'>Version: %VERSION%</div>
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
        <li><b>Bleeding Mode:</b>
            <ul>
                <li>Activates via Web Interface.</li>
                <li>Runs pump continuously for 15s to fill the line.</li>
                <li>Only works at standstill.</li>
            </ul>
        </li>
        <li><b>Offroad Mode:</b>
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
        <li><b>Update Interval:</b> Temperature is measured every 15 minutes.</li>
    </ul>
    <h3>Aux Port Manager</h3>
    <p>Controls the Auxiliary Output for accessories. Set <b>Start Delay</b> to protect battery after boot before cranking the motor.</p>
    <ul>
        <li><b>Aux Power:</b> Turns ON after a the start delay when the ESP boots (Ignition ON).</li>
        <li><b>Heated Grips:</b> Automated PWM control.
            <ul>
                <li><b>Base:</b> Minimum heat level.</li>
                <li><b>Speed:</b> Increases heat with speed (Wind chill).</li>
                <li><b>Temp:</b> Increases heat with cold (requires Temp Sensor).</li>
                <li><b>Rain:</b> Adds boost in Rain Mode.</li>
                <li><b>Startup:</b> High power for 60s after start.</li>
                <li><b>Offset:</b> Corrects sensor reading (e.g. if placed near hot engine).</li>
            </ul>
        </li>
    </ul>
    <h3>WiFi & Web Interface</h3>
    <p>WiFi is <b>OFF</b> by default.</p>
    <ul>
        <li><b>Activate:</b> 5x Button Click.</li>
        <li><b>Deactivate:</b> Auto-off when driving (> 10 km/h) or after 5 min inactivity.</li>
    </ul>
    <h3>Button Functions</h3>
    <ul>
        <li><b>1x Click:</b> Toggle 'Rain Mode'.</li>
        <li><b>Hold > 2s:</b> Toggle 'Aux Port' (Manual Override).</li>
        <li><b>1x Click:</b> Toggle 'Rain Mode' (600ms delay).</li>
        <li><b>3x Click:</b> Toggle 'Offroad Mode'.</li>
        <li><b>4x Click:</b> Toggle 'Chain Flush Mode'.</li>
        <li><b>5x Click:</b> Toggle 'WiFi'.</li>
        <li><b>Hold > 2s:</b> Toggle 'Aux Port'.</li>
        <li><b>Hold > 10s (at Boot):</b> Factory Reset.</li>
    </ul>
    <h3>Maintenance</h3>
    <ul>
        <li><b>Factory Reset:</b> Resets all settings to default. Available via Web Interface (Maintenance Page) or Hardware Button (Hold > 10s at Boot).</li>
    </ul>
    <h3>Mode Hierarchy</h3>
    <div class='note'>If multiple modes are active:</div>
    <ul>
        <li><b>1. Chain Flush:</b> Highest Priority. Runs with Offroad.</li>
        <li><b>2. Offroad:</b> Runs with Flush.</li>
        <li><b>3. Emergency:</b> Active if GPS lost. Disables Rain Mode.</li>
        <li><b>4. Rain:</b> Lowest Priority.</li>
    </ul>
    <h3>LED Status</h3>
    <ul>
        <li><span class='color-box' style='background:green'></span> <b>Green:</b> GPS OK (Ready).</li>
        <li><span class='color-box' style='background:blue'></span> <b>Blue:</b> 'Rain Mode' Active.</li>
        <li><span class='color-box' style='background:cyan'></span> <b>Cyan (blink):</b> 'Chain Flush Mode' Active.</li>
        <li><span class='color-box' style='background:magenta'></span> <b>Magenta (blink):</b> 'Offroad Mode' Active.</li>
        <li><span class='color-box' style='background:magenta'></span> <b>Magenta:</b> No GPS Signal.</li>
        <li><span class='color-box' style='background:cyan'></span> <b>Cyan:</b> 'Emergency Mode' (No GPS > 3 min).</li>
        <li><span class='color-box' style='background:yellow'></span> <b>Yellow:</b> Oiling (3s, breathes 3x).</li>
        <li><span class='color-box' style='border:1px solid #ccc'></span> <b>White pulse:</b> 'WiFi Config' Active.</li>
        <li><span class='color-box' style='background:red'></span> <b>Red (2x blink):</b> 'Tank Warning'.</li>
        <li><span class='color-box' style='background:red'></span> <b>Red blink:</b> 'Bleeding Mode'.</li>
        <li><span class='color-box' style='background:cyan'></span> <b>Cyan (fast blink):</b> Firmware Update.</li>
    </ul>
</body>
</html>
)rawliteral";

const char* htmlUpdate = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Firmware Update</title>
    <link rel="stylesheet" href="/style.css">
    <style>body{text-align:center} form{display:inline-block}</style>
</head>
<body>
    <a href='/' class='back-btn'>&larr; Home</a>
    <h2>Firmware Update</h2>
    <form method='POST' action='/update' enctype='multipart/form-data'>
        <input type='file' name='update' accept='.bin'>
        <br>
        <input type='submit' value='Update Firmware' class='btn'>
    </form>
</body>
</html>
)rawliteral";

const char* htmlIMU = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>IMU Calibration</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <a href='/' class='back-btn'>&larr; Home</a>
    <h2>IMU Calibration</h2>
    
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
        <h3>Chain position</h3>
        <form action='/imu_config' method='POST'>
            <table>
                <tr>
                    <td>Chain Side:</td>
                    <td>
                        <select name='chain_side'>
                            <option value='0' %CHAIN_LEFT%>Left</option>
                            <option value='1' %CHAIN_RIGHT%>Right (Standard)</option>
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
            <li><b>Crash Detect:</b> Stops pump if lean angle > 70&deg;. Requires restart to reset.</li>
            <li><b>Smart Stop:</b> Detects stops faster than GPS.</li>
            <li><b>Turn Safety:</b> Stops oiling in turns towards the tire side (> 20&deg;).</li>
        </ul>
    </div>
</body>
</html>
)rawliteral";

const char* htmlAuxConfig = R"rawliteral(
<html>
<head>
    <meta charset="UTF-8">
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Aux Config</title>
    <link rel="stylesheet" href="/style.css">
    <script>
        function toggleFields() {
            var mode = document.getElementById('mode').value;
            var grips = document.getElementById('grips_settings');
            var desc = document.getElementById('grips_desc');
            var aux = document.getElementById('aux_desc');
            var delay = document.getElementById('common_delay');
            
            if (mode == '2') { // Heated Grips
                grips.style.display = 'block';
                desc.style.display = 'block';
                aux.style.display = 'none';
                delay.style.display = 'table-row';
            } else if (mode == '1') { // Aux Power
                grips.style.display = 'none';
                desc.style.display = 'none';
                aux.style.display = 'block';
                delay.style.display = 'table-row';
            } else {
                grips.style.display = 'none';
                desc.style.display = 'none';
                aux.style.display = 'none';
                delay.style.display = 'none';
            }
        }
    </script>
</head>
<body onload="toggleFields()">
    <a href='/' class='back-btn'>&larr; Home</a>
    <h2>Aux Port Configuration</h2>
    <form action='/save_aux' method='POST'>
        <div class='card'>
            <h3>Mode Selection</h3>
            <table>
                <tr>
                    <td>Mode</td>
                    <td>
                        <select id='mode' name='mode' onchange="toggleFields()">
                            <option value='0' %MODE_OFF%>OFF</option>
                            <option value='1' %MODE_AUX%>Aux Power (Switched)</option>
                            <option value='2' %MODE_GRIPS%>Heated Grips (Auto)</option>
                        </select>
                    </td>
                </tr>
                <tr id='common_delay' style='display:none'>
                    <td>Start Delay (sec)</td>
                    <td><input type='number' name='startD' value='%STARTD%' min='0'></td>
                </tr>
            </table>
        </div>
        
        <div id='grips_settings' style='display:none'>
            <div class='card'>
                <h3>Heated Grips Settings</h3>
                <table>
                    <tr>
                        <td>Wind Chill Compensation</td>
                        <td>
                            <select name='speedF'>
                                <option value='0.2' %SPEED_LOW%>Low</option>
                                <option value='0.5' %SPEED_MED%>Medium</option>
                                <option value='1.0' %SPEED_HIGH%>High</option>
                            </select>
                        </td>
                    </tr>
                    <tr>
                        <td>Temp Compensation</td>
                        <td>
                            <select name='tempF'>
                                <option value='1.0' %TEMP_LOW%>Low</option>
                                <option value='2.0' %TEMP_MED%>Medium</option>
                                <option value='3.0' %TEMP_HIGH%>High</option>
                            </select>
                        </td>
                    </tr>
                    <tr>
                        <td>Rain Boost (%)</td>
                        <td><input type='number' name='rainB' value='%RAINB%' min='0' max='100'></td>
                    </tr>
                    <tr>
                        <td>Base Level (%)</td>
                        <td><input type='number' name='base' value='%BASE%' min='0' max='100'></td>
                    </tr>
                    <tr>
                        <td>Reaction Speed</td>
                        <td>
                            <select name='reaction'>
                                <option value='0' %REACT_SLOW%>Slow (Smooth)</option>
                                <option value='1' %REACT_MED%>Medium</option>
                                <option value='2' %REACT_FAST%>Fast (Responsive)</option>
                            </select>
                        </td>
                    </tr>
                    <tr><td colspan="2" style="height:15px"></td></tr>
                    <tr>
                        <td>Startup Boost Level (%)</td>
                        <td><input type='number' name='startL' value='%STARTL%' min='0' max='100'></td>
                    </tr>
                    <tr>
                        <td>Startup Boost Time (sec)</td>
                        <td><input type='number' name='startS' value='%STARTS%' min='0'></td>
                    </tr>
                    <tr>
                        <td>Start Temp (&deg;C)</td>
                        <td><input type='number' name='startT' value='%STARTT%' step='1'></td>
                    </tr>
                    <tr>
                        <td>Temp Sensor Offset (&deg;C)</td>
                        <td><input type='number' name='tempO' value='%TEMPO%' step='0.1'></td>
                    </tr>
                    <tr>
                        <td style="font-size:1.1em; color:#ccc">Current Reading:</td>
                        <td style="font-size:1.1em; color:#ffc107; font-weight:bold">%CURRENT_TEMP% &deg;C</td>
                    </tr>
                    <tr>
                        <td colspan="2" class="note" style="padding-bottom: 10px;">
                            Correction for sensor placement. Use <b>negative values</b> (e.g. -5.0) if sensor is near a hot engine to lower the reading.
                        </td>
                    </tr>
                </table>
            </div>
        </div>
        
        <input type='submit' value='Save Configuration' class='btn'>
    </form>

    <div id='grips_desc' class='card' style='display:none; margin-top:15px'>
        <h3>Feature Description</h3>
        <div class='note' style='margin-bottom:15px; line-height:1.4'>
            The Heated Grips mode automatically adjusts the power (PWM) based on speed, temperature and rain mode settings. For faster heat up, a higher initial power leel can be configured.<br>
        </div>
    </div>

    <div id='aux_desc' class='card' style='display:none; margin-top:15px'>
        <h3>Aux Power Description</h3>
        <div class='note' style='margin-bottom:15px; line-height:1.4'>
            <b>Aux Power</b> automatically turns on the Aux Port (12V) after a configurable delay (Ignition ON).<br>
            <br>
            <span style='color:red; font-weight:bold'>⚠️ WARNING:</span><br>
            Do not exceed a continuous load of <b>5 Amps</b> on this port to prevent overheating of the traces.
        </div>
    </div>
</body>
</html>
)rawliteral";

const char* htmlConsole = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Serial Console</title>
    <link rel="stylesheet" href="/style.css">
    <script>
        function fetchLogs() {
            fetch('/console/data')
                .then(response => response.text())
                .then(data => {
                    var consoleDiv = document.getElementById('console');
                    var isScrolledToBottom = consoleDiv.scrollHeight - consoleDiv.clientHeight <= consoleDiv.scrollTop + 1;
                    
                    consoleDiv.innerText = data;
                    
                    if(isScrolledToBottom){
                        consoleDiv.scrollTop = consoleDiv.scrollHeight;
                    }
                });
        }
        setInterval(fetchLogs, 200);
    </script>
</head>
<body>
    <a href='/' class='back-btn'>&larr; Home</a>
    <h2>Serial Console</h2>
    <div id='console'>Loading...</div>
    <form action='/console/clear' method='POST'>
        <input type='submit' value='Clear Log' class='btn'>
    </form>
</body>
</html>
)rawliteral";

#endif

