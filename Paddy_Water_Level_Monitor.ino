/*
  Smart Paddy Field Water-Level Monitoring System
  Board: ESP32
  Sensor: JSN-SR04T
  Interface: ESP32 WebServer dashboard
  Optional: relay-driver control output and SIM800L SMS alert

  IMPORTANT:
  - This is generated prototype firmware. Compile and bench-test before field use.
  - Confirm your JSN-SR04T version and ECHO voltage. ESP32 GPIO is NOT 5V tolerant.
  - Pump output is a LOW-VOLTAGE control signal only; never connect a mains pump directly.
*/

#include <WiFi.h>
#include <WebServer.h>

// ---------------- USER CONFIGURATION ----------------
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Example GPIOs only. Change to match your board and wiring.
const int TRIG_PIN = 5;
const int ECHO_PIN = 18;
const int PUMP_CONTROL_PIN = 26;

// Most relay modules are active LOW. Set according to your driver.
const bool PUMP_ACTIVE_LEVEL = LOW;
const bool PUMP_INACTIVE_LEVEL = HIGH;

// Keep false until output wiring and safety have been verified.
const bool ENABLE_PUMP_OUTPUT = false;

// Sensor mounting height measured from sensor face to the zero-water datum.
const float SENSOR_MOUNT_HEIGHT_CM = 100.0;

// Configure thresholds for your field after calibration.
const float LOW_LEVEL_THRESHOLD_CM = 2.0;
const float HIGH_LEVEL_THRESHOLD_CM = 5.0;

// Optional SIM800L SMS. Keep disabled unless wired and tested.
const bool ENABLE_SIM800L = false;
const int SIM800L_RX_PIN = 16;  // ESP32 RX2 <- SIM800L TX
const int SIM800L_TX_PIN = 17;  // ESP32 TX2 -> SIM800L RX (level compatibility required)
const char* ALERT_PHONE_NUMBER = "+91XXXXXXXXXX";

// ----------------------------------------------------
WebServer server(80);
HardwareSerial sim800(2);

float distanceCm = -1.0;
float waterLevelCm = 0.0;
bool sensorValid = false;
bool pumpOn = false;
bool automaticMode = true;
unsigned long lastMeasureMs = 0;
unsigned long lastSmsMs = 0;
const unsigned long MEASURE_INTERVAL_MS = 1500;
const unsigned long SMS_COOLDOWN_MS = 10UL * 60UL * 1000UL;

const char PAGE[] PROGMEM = R"HTML(
<!doctype html><html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Paddy Water Monitor</title>
<style>
body{font-family:Arial,sans-serif;background:#eef6f7;color:#123;margin:0;padding:18px}
main{max-width:760px;margin:auto}.card{background:white;padding:18px;margin:12px 0;border-radius:14px;box-shadow:0 2px 10px #0001}
h1{color:#075985}.value{font-size:2rem;font-weight:bold;color:#087e8b}
button{padding:12px 18px;margin:5px;border:0;border-radius:8px;background:#087e8b;color:white;font-weight:bold}
button.alt{background:#475569}.small{color:#52616b;font-size:.9rem}
</style></head><body><main>
<h1>Smart Paddy Field Water Monitor</h1>
<div class="card"><div class="small">Water level from configured datum</div><div class="value" id="level">-- cm</div>
<div>Sensor distance: <b id="distance">--</b> cm</div><div>Status: <b id="valid">--</b></div></div>
<div class="card"><h3>Control mode</h3><div id="mode">--</div>
<button onclick="cmd('/mode?auto=1')">Automatic</button><button class="alt" onclick="cmd('/mode?auto=0')">Manual</button>
<h3>Pump output</h3><div id="pump">--</div>
<button onclick="cmd('/pump?state=1')">Pump ON</button><button class="alt" onclick="cmd('/pump?state=0')">Pump OFF</button>
<p class="small">Pump control is disabled in firmware unless ENABLE_PUMP_OUTPUT is set true.</p></div>
<div class="card small">Local ESP32 dashboard. Refreshes every 2 seconds.</div>
</main><script>
async function refresh(){try{let r=await fetch('/data');let d=await r.json();
document.getElementById('level').textContent=d.valid?d.level.toFixed(1)+' cm':'No reading';
document.getElementById('distance').textContent=d.distance>=0?d.distance.toFixed(1):'--';
document.getElementById('valid').textContent=d.valid?'Valid':'Invalid echo';
document.getElementById('mode').textContent=d.auto?'Automatic':'Manual';
document.getElementById('pump').textContent=d.pump?'ON':'OFF';
}catch(e){}}
async function cmd(u){await fetch(u);refresh()}setInterval(refresh,2000);refresh();
</script></body></html>
)HTML";

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 35000UL);
  if (duration == 0) return -1.0f;
  float d = (duration * 0.0343f) / 2.0f;
  if (d < 2.0f || d > 600.0f) return -1.0f;
  return d;
}

void applyPump(bool on) {
  pumpOn = on;
  if (ENABLE_PUMP_OUTPUT) {
    digitalWrite(PUMP_CONTROL_PIN, on ? PUMP_ACTIVE_LEVEL : PUMP_INACTIVE_LEVEL);
  } else {
    // Software state only; physical output remains inactive.
    digitalWrite(PUMP_CONTROL_PIN, PUMP_INACTIVE_LEVEL);
  }
}

void sendSms(const String& message) {
  if (!ENABLE_SIM800L) return;
  if (millis() - lastSmsMs < SMS_COOLDOWN_MS) return;
  sim800.println("AT+CMGF=1");
  delay(300);
  sim800.print("AT+CMGS=\"");
  sim800.print(ALERT_PHONE_NUMBER);
  sim800.println("\"");
  delay(300);
  sim800.print(message);
  sim800.write(26);
  delay(1000);
  lastSmsMs = millis();
}

void handleRoot() {
  server.send_P(200, "text/html", PAGE);
}

void handleData() {
  String json = "{";
  json += "\"valid\":" + String(sensorValid ? "true" : "false") + ",";
  json += "\"distance\":" + String(distanceCm, 2) + ",";
  json += "\"level\":" + String(waterLevelCm, 2) + ",";
  json += "\"pump\":" + String(pumpOn ? "true" : "false") + ",";
  json += "\"auto\":" + String(automaticMode ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void handleMode() {
  if (server.hasArg("auto")) {
    automaticMode = (server.arg("auto") == "1");
  }
  server.send(200, "text/plain", automaticMode ? "Automatic mode" : "Manual mode");
}

void handlePump() {
  if (!server.hasArg("state")) {
    server.send(400, "text/plain", "Missing state");
    return;
  }
  automaticMode = false;
  applyPump(server.arg("state") == "1");
  server.send(200, "text/plain", pumpOn ? "Pump state ON" : "Pump state OFF");
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  unsigned long started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < 15000) {
    delay(400);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected");
    Serial.print("Dashboard: http://");
    Serial.println(WiFi.localIP());
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Paddy-Water-Monitor", "Paddy1234");
    Serial.println("\nWi-Fi unavailable; fallback AP started.");
    Serial.println("Connect to Paddy-Water-Monitor (password: Paddy1234)");
    Serial.print("Dashboard: http://");
    Serial.println(WiFi.softAPIP());
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PUMP_CONTROL_PIN, OUTPUT);
  digitalWrite(PUMP_CONTROL_PIN, PUMP_INACTIVE_LEVEL);
  applyPump(false);

  if (ENABLE_SIM800L) {
    sim800.begin(9600, SERIAL_8N1, SIM800L_RX_PIN, SIM800L_TX_PIN);
  }

  connectWiFi();
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/mode", handleMode);
  server.on("/pump", handlePump);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  if (millis() - lastMeasureMs >= MEASURE_INTERVAL_MS) {
    lastMeasureMs = millis();
    distanceCm = readDistanceCm();
    sensorValid = (distanceCm > 0.0f);

    if (sensorValid) {
      waterLevelCm = SENSOR_MOUNT_HEIGHT_CM - distanceCm;
      Serial.printf("Distance: %.1f cm | Water level: %.1f cm\n",
                    distanceCm, waterLevelCm);

      if (automaticMode) {
        // Example hysteresis: turn output ON below low threshold,
        // turn it OFF at/above high threshold. Adapt to pump direction
        // and field design before enabling a physical output.
        if (waterLevelCm < LOW_LEVEL_THRESHOLD_CM) applyPump(true);
        else if (waterLevelCm >= HIGH_LEVEL_THRESHOLD_CM) applyPump(false);
      }

      if (waterLevelCm < LOW_LEVEL_THRESHOLD_CM) {
        sendSms("Paddy field water level is below configured threshold.");
      }
    } else {
      Serial.println("Invalid ultrasonic reading. Pump output forced OFF.");
      applyPump(false);
      sendSms("Paddy field sensor reading invalid. Check sensor.");
    }
  }
}
