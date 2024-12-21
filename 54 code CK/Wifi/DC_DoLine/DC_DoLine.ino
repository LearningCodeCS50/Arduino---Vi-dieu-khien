#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Replace with your network credentials
const char *ssid = "Tran Thi Kim Loan";
const char *password = "01229333995";

// Define GPIO pins for DC Motor
#define lineSensor D5
#define motorPin1 D1
#define motorPin2 D2
#define motorSpeedPin D4

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Variables to save values from HTML form
const char *PARAM_INPUT = "speed";

String direction = "Stopped";  // Motor direction
int motorSpeed = 0;           // Motor speed (0-255)

// Read line sensor
String readLineSensor() {
  return (digitalRead(lineSensor) == HIGH) ? "black" : "white";
}

// HTML content
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>DC Motor & Sensor</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <h1>DC Motor Control</h1>
  <h2>Sensor Reading</h2>
  <p>Line Sensor Status: <span id="line">%LINE%</span></p>
  <h2>Motor Status</h2>
  <p>Motor Direction: <span id="direction">%DIRECTION%</span></p>
  <form id="motorForm">
    <label for="speed">Motor Speed (0-255): </label>
    <input type="number" name="speed" min="0" max="255" required>
    <input type="submit" value="Set Speed">
  </form>
  <script>
    document.getElementById('motorForm').addEventListener('submit', function (e) {
      e.preventDefault();
      const formData = new FormData(this);
      fetch("/", {
        method: "POST",
        body: formData
      }).then(response => response.text()).catch(error => console.error("Error:", error));
    });

    setInterval(function() {
      fetch("/status")
        .then(response => response.json())
        .then(data => {
          document.getElementById("line").innerHTML = data.line;
          document.getElementById("direction").innerHTML = data.direction;
        })
        .catch(error => console.error("Error:", error));
    }, 2000);
  </script>
</body>
</html>
)rawliteral";

// Replace placeholder in HTML
String processor(const String &var) {
  if (var == "LINE") {
    return readLineSensor();
  } else if (var == "DIRECTION") {
    return direction;
  }
  return String();
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("\nConnected! IP Address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);

  // Initialize WiFi
  initWiFi();

  // Set up line sensor
  pinMode(lineSensor, INPUT);

  // Set up motor control pins
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorSpeedPin, OUTPUT);

  // Web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = "{\"line\":\"" + readLineSensor() + "\",\"direction\":\"" + direction + "\"}";
    request->send(200, "application/json", jsonResponse);
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam(PARAM_INPUT, true)) {
      motorSpeed = request->getParam(PARAM_INPUT, true)->value().toInt();
      Serial.print("Motor speed set to: ");
      Serial.println(motorSpeed);
    }
    request->send(200, "text/plain", "Settings updated");
  });

  // Start server
  server.begin();
}

void loop() {
  static String prevDirection = "";   // Previous motor direction
  static String prevSensorStatus = ""; // Previous sensor status

  // Read sensor status
  String sensorStatus = readLineSensor();

  // Determine motor direction based on sensor status
  if (sensorStatus == "white" && motorSpeed > 0) {
    direction = "Clockwise";
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
  } else if (sensorStatus == "black" && motorSpeed > 0) {
    direction = "Counterclockwise";
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
  } else if (motorSpeed == 0) {
    direction = "Stopped";
    // Stop the motor
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
  }

  // Update motor speed
  analogWrite(motorSpeedPin, motorSpeed);

  // Print info if sensor status or motor direction changes
  if (sensorStatus != prevSensorStatus || direction != prevDirection) {
    Serial.print("Sensor Status: ");
    Serial.println(sensorStatus);
    Serial.print("Motor Direction: ");
    Serial.println(direction);
    Serial.println();
    prevSensorStatus = sensorStatus;
    prevDirection = direction;
  }

  // Optionally, you can add delays or other logic here as needed
}
