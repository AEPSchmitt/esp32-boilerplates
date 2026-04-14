#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// WIFI
const char* SSID = "ssid";
const char* PASSWORD = "password";

const int SERVO_PIN   = 0;

// HEAD LEFT/RIGHT RANGE
const int SAFE_LEFT  = 70;
const int SAFE_RIGHT = 20;

// FULL SERVO RANGE
const int FULL_MIN = 0;
const int FULL_MAX = 180;

const int POS_CENTER = 45;

// SERVO SPEED
int speedPercent = 50;  // default
int stepDelay = 32;
const int MIN_DELAY = 5;
const int MAX_DELAY = 60;

// CO2 SENSOR
HardwareSerial co2Serial(1);
#define RX_PIN 17
#define TX_PIN 16

// Webserver and other shit
WebServer server(80);
Servo myServo;

enum MotionMode { IDLE, MOVE_TO_TARGET, SHAKE, CIRCLE };
MotionMode motion = IDLE;

int currentPos = POS_CENTER;
int targetPos  = POS_CENTER;

bool infiniteShake = false;
int cyclesRemaining = 0;

int direction = 1; // for circle

unsigned long lastStep = 0;

// sorry about this. seems to be the easiest way to serve html from esp.
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>🐾 Muffibot 2000</title>
<link href="https://fonts.googleapis.com/css2?family=Fredoka+One&display=swap" rel="stylesheet">

<style>
body {
    font-family: 'Arial', sans-serif;
    background: #111;
    color: #fff;
    text-align: center;
    padding: 20px;
    margin: 0;
}

h1 {
    font-family: 'Fredoka One', cursive;
    color: #fff;
    text-shadow: 2px 2px 5px #000;
    margin-bottom: 30px;
}

/* BUTTON SECTIONS */
.button-group, .button-other {
    display: flex;
    justify-content: center;
    gap: 15px;
    margin: 15px 0;
}

button {
    background-color: #000;
    color: #fff;
    border: 2px solid #fff;
    border-radius: 12px;
    padding: 15px 25px;
    font-size: 1rem;
    cursor: pointer;
    transition: all 0.2s ease-in-out;
    width:25%;
}

button:hover {
    background-color: #fff;
    color: #000;
}

/* WHITE PATCH AREAS */
.section {
    background-color: #fff;
    color: #000;
    border-radius: 15px;
    padding: 15px;
    margin: 15px auto;
    width: 90%;
    max-width: 500px;
}

.section label {
    font-weight: bold;
}

input[type="range"], input[type="number"] {
    width: 100%;
    padding: 5px;
    font-size: 1rem;
    border-radius: 6px;
    border: 1px solid #000;
    background-color: #eee;
    color: #000;
    margin: 10px 0;
}

input[type="number"] {
    width: 80px;
}

.slider-container span {
    margin-left: 10px;
    font-weight: bold;
}

.status {
    margin-top: 20px;
    padding: 10px;
    background-color: #222;
    border-radius: 12px;
    font-family: monospace;
    white-space: pre-wrap;
    position: fixed;
    bottom:0;
    right:0;
}
.section canvas {
    width: 100% !important;
    max-height: 200px;
}
</style>
</head>
<body>

<h1>Muffibot 2000 🐱⚙️</h1>

<!-- LEFT / CENTER / RIGHT -->
<div class="button-group section">
    <button onclick="sendCommand('left')">←←</button>
    <img src="http://192.168.0.230:1337/images/muffihead.png" onclick="sendCommand('center')" style="width: 50%;">
    <button onclick="sendCommand('right')">→→</button>
</div>

<!-- SPEED -->
<div class="section">
    <label>Speed </label><label id="speedValue">50%</label>
    <input type="range" id="speedInput" min="0" max="100" value="50" oninput="speedDisplay(this.value)">

</div>

<!-- GRAPH plotly.js -->
<div class="section">
    <label>CO2 Levels (ppm)</label>
    <div id="co2Graph" style="width:100%;height:300px;"></div>
     <a href="http://192.168.0.230:1337">full history</a>
</div>

<!-- SHAKE BUTTON -->
<div class="button-other section">
    <button onclick="shake()">Shake</button>
    <input type="number" id="shakeCount" min="1" placeholder="Times">
</div>

<!-- OTHER BUTTONS -->
<div class="button-other section">
    <button onclick="sendCommand('circle')">Circle</button>
    <button onclick="sendCommand('stop')">Stop</button>
</div>

<!-- SAFE Motion Slider -->
<div class="section">
    <label>Left ↔ Right</label>
    <input type="range" id="safeSlider" min="0" max="100" value="50"
       oninput="safeSliderChange(this.value)"
       onchange="safeSliderChange(this.value)">
</div>

<!-- FULL Motion Slider -->
<div class="section">
    <label>0–360°</label>
    <input type="range" id="fullSlider" min="0" max="100" value="50"
       oninput="fullSliderChange(this.value)"
       onchange="fullSliderChange(this.value)">
</div>

<!-- STATUS -->
<div class="status" id="statusDisplay">
    Loading status...
</div>

<script>
let apiBase = ''

// Send generic commands
function sendCommand(endpoint) {
    let speed = document.getElementById('speedInput').value;
    fetch(`${apiBase}/${endpoint}?speed=${speed}`)
        .then(() => updateStatus());
}

// Shake
function shake() {
    let times = document.getElementById('shakeCount').value;
    let speed = document.getElementById('speedInput').value;
    let url = `${apiBase}/shake${times ? '?n=' + times + '&speed=' + speed : '?speed=' + speed}`;
    fetch(url).then(() => updateStatus());
}

// Safe slider
function safeSliderChange(value) {
    let speed = document.getElementById('speedInput').value;
    fetch(`${apiBase}/safe?value=${value}&speed=${speed}`);
}

// Full slider
function fullSliderChange(value) {
    let speed = document.getElementById('speedInput').value;
    fetch(`${apiBase}/goto?value=${value}&speed=${speed}`);
}

// Speed slider
function speedDisplay(value) {
    let speedVal = document.getElementById('speedValue');
    speedVal.innerText = value + "%";

    // Optionally, send speed immediately to server (optional)
    // let sliderSpeed = value;
    // fetch(`/speed?value=${sliderSpeed}`);
}

// Status polling
function updateStatus() {
    fetch(`${apiBase}/status`)
        .then(response => response.json())
        .then(data => {
            document.getElementById('statusDisplay').innerText =
                `Motion: ${data.motion}\nPosition: ${data.position}\nSpeed: ${data.speed}%`;

            // Set sliders to current positions on first load
            if (!window.initialized) {
                // Map SAFE slider to SAFE range
                let safeValue = Math.round((data.position - 20) / (70 - 20) * 100);
                safeValue = Math.max(0, Math.min(100, safeValue));
                document.getElementById('safeSlider').value = safeValue;

                // Full slider
                let fullValue = Math.round((data.position - 0) / (180 - 0) * 100);
                fullValue = Math.max(0, Math.min(100, fullValue));
                document.getElementById('fullSlider').value = fullValue;

                window.initialized = true;
            }
        });
}

setInterval(updateStatus, 1000);
updateStatus();

// Gamepad support (optional)
let lastSendTime = 0
let GAMEPAD_DEADZONE = 0.1
let GAMEPAD_RATE = 50

function pollGamepad() {
    let gp = navigator.getGamepads()[0]
    if (gp) {
        let xAxis = gp.axes[0];
        if (Math.abs(xAxis) > GAMEPAD_DEADZONE) {
            let slider = document.getElementById("fullSlider");
            let currentValue = parseInt(slider.value);
            let delta = Math.round(-xAxis * 2); // flipped direction
            let newValue = Math.max(0, Math.min(100, currentValue + delta));

            let now = Date.now();
            if (now - lastSendTime > GAMEPAD_RATE) {
                lastSendTime = now;
                slider.value = newValue;
                //document.getElementById("fullValue").innerText = newValue;
                fullSliderChange(newValue);
            }
        }

        let speed = document.getElementById('speedInput').value

        // Buttons: 0=A,1=B,2=X,3=Y,4=LB,5=RB
        if (gp.buttons[0].pressed) sendCommand('center');
        if (gp.buttons[1].pressed) sendCommand('stop');
        if (gp.buttons[2].pressed) shake();
        if (gp.buttons[3].pressed) sendCommand('circle');
        if (gp.buttons[4].pressed) sendCommand('left');
        if (gp.buttons[5].pressed) sendCommand('right');
    }
    requestAnimationFrame(pollGamepad);
}

window.addEventListener("gamepadconnected", () => pollGamepad());
window.addEventListener("gamepaddisconnected", () => console.log("Gamepad disconnected"));

</script>

<script src="https://cdn.plot.ly/plotly-3.4.0.min.js" charset="utf-8"></script>
<script>
let trace = {
    x: [],
    y: [],
    mode: "lines",
    line: {color: "black"},
    type: "scatter"
};

let lastTimestamp = null
let initialViewSet = false

Plotly.newPlot("co2Graph", [trace], {
    dragmode: "pan",
    uirevision: "co2",
    xaxis: {
        title: "Time",
        rangeslider: {visible: true}
    },
    yaxis: {
        title: "CO₂ (ppm)"
    },
    margin: {t:20}
})

function addPoints(xVals, yVals){

    Plotly.extendTraces(
        "co2Graph",
        {x:[xVals], y:[yVals]},
        [0],
        5000   // keep last 5000 points
    )
}

function setInitialRange() {

    if (initialViewSet) return;

    let x = co2Data.x;
    if (x.length === 0) return;

    let last = new Date(x[x.length-1]);
    let first = new Date(last.getTime() - 3600000); // 1 hour

    Plotly.relayout('co2Graph', {
        'xaxis.range': [first, last]
    });

    initialViewSet = true
}
let lastLine = 1

function updateCO2(){

    fetch(`${apiBase}/co2`)
    .then(res => res.text())
    .then(d => {
        console.log(d)
        let x = [] 
        let y = []
        x.push(new Date())
        y.push(Number(d))
        //y.push(Math.random() * (1500 - 100))
        if(x.length){
            addPoints(x, y)
        }
    })
}

setInterval(updateCO2,5000)
updateCO2()
</script>
</body>
</html>
)rawliteral";

// ===== SPEED HELPERS =====
void setSpeed(int percent)
{
    percent = constrain(percent, 1, 100);
    speedPercent = percent;
    stepDelay = MAX_DELAY - ((percent - 1) * (MAX_DELAY - MIN_DELAY) / 99);
}

void handleSpeedArg()
{
    if (server.hasArg("speed"))
    {
        int s = server.arg("speed").toInt();
        if (s >= 1 && s <= 100)
            setSpeed(s);
    }
}

// ===== MOTION HELPERS =====
String motionName()
{
    switch (motion)
    {
        case IDLE: return "idle";
        case MOVE_TO_TARGET: return "move";
        case SHAKE: return "shake";
        case CIRCLE: return "circle";
    }
    return "unknown";
}

void moveTo(int pos)
{
    targetPos = pos;
    motion = MOVE_TO_TARGET;
}

void startShake(int cycles)
{
    motion = SHAKE;
    // Do NOT write the servo instantly; let updateServo handle it at current speed
    currentPos = currentPos;      // stay at current position
    targetPos  = SAFE_LEFT;       // start moving to left at proper speed

    if (cycles <= 0)
        infiniteShake = true;
    else
    {
        infiniteShake = false;
        cyclesRemaining = cycles;
    }
}

void startCircle()
{
    motion = CIRCLE;
    direction = (currentPos < FULL_MAX) ? 1 : -1;  // start sweeping in correct direction
    // No instant write, so movement respects speedPercent
}

void stopServo()
{
    motion = MOVE_TO_TARGET;
    targetPos = POS_CENTER;  // move toward center
    // Do NOT write instantly; updateServo() will move it at current speedPercent
}

// ===== SERVO UPDATE LOOP =====
void updateServo()
{
    if (millis() - lastStep < stepDelay) return;
    lastStep = millis();

    switch (motion)
    {
        case MOVE_TO_TARGET:
            if (currentPos < targetPos) currentPos++;
            else if (currentPos > targetPos) currentPos--;
            myServo.write(currentPos);
            if (currentPos == targetPos) motion = IDLE;
            break;

        case SHAKE:
            if (currentPos < targetPos) currentPos++;
            else if (currentPos > targetPos) currentPos--;
            myServo.write(currentPos);

            if (currentPos == targetPos)
            {
                if (targetPos == SAFE_RIGHT)
                    targetPos = SAFE_LEFT;
                else
                    targetPos = SAFE_RIGHT;

                if (!infiniteShake)
                {
                    cyclesRemaining--;
                    if (cyclesRemaining <= 0) stopServo();
                }
            }
            break;

        case CIRCLE:
            currentPos += direction;
            if (currentPos >= FULL_MAX) direction = -1;
            if (currentPos <= FULL_MIN) direction = 1;
            myServo.write(currentPos);
            break;

        case IDLE:
            break;
    }
}

// CO2 Sensor reading
int readCO2() {
  byte cmd[9] = {0xFF, 0x01, 0x86, 0, 0, 0, 0, 0, 0x79};
  byte response[9];

  // Clear buffer
  while (co2Serial.available()) co2Serial.read();

  // Send command
  co2Serial.write(cmd, 9);

  delay(100);

  if (co2Serial.available() >= 9) {
    co2Serial.readBytes(response, 9);

    // Validate response
    if (response[0] == 0xFF && response[1] == 0x86) {
      int ppm = (response[2] << 8) | response[3];
      return ppm;
    }
  }

  return -1; // error
}

// ===== ENDPOINT HANDLERS =====
void handleLeft()
{
    handleSpeedArg();
    moveTo(SAFE_LEFT);  // physically left
    server.send(200, "text/plain", "Moving left (safe range) at speed " + String(speedPercent) + "%");
}

void handleRight()
{
    handleSpeedArg();
    moveTo(SAFE_RIGHT); // physically right
    server.send(200, "text/plain", "Moving right (safe range) at speed " + String(speedPercent) + "%");
}

void handleCenter()
{
    handleSpeedArg();
    moveTo(POS_CENTER);
    server.send(200, "text/plain", "Centering at speed " + String(speedPercent) + "%");
}

void handleShake()
{
    handleSpeedArg();
    if (server.hasArg("n"))
    {
        int cycles = server.arg("n").toInt();
        startShake(cycles);
        server.send(200, "text/plain", "Shaking " + String(cycles) + " times at speed " + String(speedPercent) + "%");
    }
    else
    {
        startShake(0);
        server.send(200, "text/plain", "Shaking indefinitely at speed " + String(speedPercent) + "%");
    }
}

void handleCircle()
{
    handleSpeedArg();
    startCircle();
    server.send(200, "text/plain", "Continuous circle started at speed " + String(speedPercent) + "%");
}

void handleStop()
{
    handleSpeedArg();
    stopServo();
    server.send(200, "text/plain", "Stopping and centering at speed " + String(speedPercent) + "%");
}

void handleStatus()
{
    String msg = "{";
    msg += "\"motion\":\"" + motionName() + "\",";
    msg += "\"position\":" + String(currentPos) + ",";
    msg += "\"speed\":" + String(speedPercent);
    msg += "}";
    server.send(200, "application/json", msg);
}

void handleSafe()
{
    handleSpeedArg();

    if (!server.hasArg("value"))
    {
        server.send(400, "text/plain", "Missing value parameter (0-100)");
        return;
    }

    int val = constrain(server.arg("value").toInt(), 0, 100);
    int angle = SAFE_LEFT + (SAFE_RIGHT - SAFE_LEFT) * val / 100;
    moveTo(angle);
    server.send(200, "text/plain", 
        "Safe motion to value " + String(val) + " (angle " + String(angle) + ") at speed " + String(speedPercent) + "%");
}

void handleGoto()
{
    handleSpeedArg();
    if (!server.hasArg("value"))
    {
        server.send(400, "text/plain", "Missing value parameter (0-100)");
        return;
    }
    int val = constrain(server.arg("value").toInt(), 0, 100);
    int angle = FULL_MIN + (FULL_MAX - FULL_MIN) * val / 100;
    moveTo(angle);
    server.send(200, "text/plain", "Moving to value " + String(val) + " (angle " + String(angle) + ") at speed " + String(speedPercent) + "%");
}

void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

void handleCO2() {
    int co2ppm = readCO2();
    String co2_string = String(co2ppm);
    server.send(200, "application/json", co2_string);
}

// ===== API SETUP =====
void setup_api()
{
    server.on("/", handleRoot);
    server.on("/left", handleLeft);
    server.on("/right", handleRight);
    server.on("/center", handleCenter);

    server.on("/shake", handleShake);
    server.on("/circle", handleCircle);

    server.on("/stop", handleStop);
    server.on("/status", handleStatus);
    server.on("/goto", handleGoto);
    server.on("/safe", handleSafe);

    server.on("/co2", handleCO2);
    server.begin();
}

// ===== SETUP =====
void setup()
{
    delay(200);
    Serial.begin(115200);

    myServo.attach(SERVO_PIN);

    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected. IP: ");
    Serial.println(WiFi.localIP());
    setup_api();

    co2Serial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
}

// ===== LOOP =====
void loop()
{
    server.handleClient();
    updateServo();

    // default setup only reads co2 if /co2 endpoint is called.
    // uncomment below to continously read and send via serial
    //int co2ppm = readCO2();
    //Serial.println(co2ppm);
}