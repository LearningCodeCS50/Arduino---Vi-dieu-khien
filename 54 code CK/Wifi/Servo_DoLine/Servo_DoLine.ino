#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h>

// WiFi thông tin
const char *ssid = "Tran Thi Kim Loan";
const char *password = "01229333995";

// Cảm biến dò line và servo
#define lineSensorPin D5  // Chân cảm biến dò line (cảm biến digital)
#define servoPin D1       // Chân servo

Servo myServo;
String lineSensorStateStr = "White";  

int angleConditionMet = 90;  // Góc servo khi có đường
int angleConditionNotMet = 0; // Góc servo khi không có đường

// Tạo web server
AsyncWebServer server(80);

// HTML giao diện web
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Servo Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <h1>Servo Control</h1>
  <p>Line Sensor State: <span id="sensorState">%SENSORSTATE%</span></p>
  <form id="servoForm">
    <label for="angleMet">Angle (Line Detected): </label>
    <input type="number" name="angleMet" min="0" max="180" required>
    <br>
    <label for="angleNotMet">Angle (No Line Detected): </label>
    <input type="number" name="angleNotMet" min="0" max="180" required>
    <br><br>
    <input type="submit" value="Update">
  </form>
  <script>
    document.getElementById('servoForm').addEventListener('submit', function (e) {
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
          document.getElementById("sensorState").innerHTML = data.state;
        })
        .catch(error => console.error("Error:", error));
    }, 2000);
  </script>
</body>
</html>
)rawliteral";

// Đọc trạng thái cảm biến dò line (Digital hoặc Analog)
int readLineSensor() {
  return digitalRead(lineSensorPin);  // Trạng thái HIGH hoặc LOW
}

String processor(const String &var) {
  if (var == "SENSORSTATE") {
    return String(lineSensorStateStr);
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
  Serial.println("\nConnected to WiFi! IP Address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  // Khởi tạo Serial Monitor
  Serial.begin(9600);

  // Khởi tạo WiFi
  initWiFi();

  // Khởi tạo cảm biến và servo
  pinMode(lineSensorPin, INPUT);
  myServo.attach(servoPin);
  myServo.write(0);

  // Cấu hình Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = "{\"state\":\"" + String(lineSensorStateStr) + "\"}";
    request->send(200, "application/json", jsonResponse);
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("angleMet", true) && request->hasParam("angleNotMet", true)) {
      angleConditionMet = request->getParam("angleMet", true)->value().toInt();
      angleConditionNotMet = request->getParam("angleNotMet", true)->value().toInt();
      Serial.print("Updated angles: ");
      Serial.print("Line Detected: ");
      Serial.print(angleConditionMet);
      Serial.print(", No Line Detected: ");
      Serial.println(angleConditionNotMet);
    }
    request->send(200, "text/plain", "Settings updated");
  });

  server.begin();
}

void loop() {
  int lineSensorState = readLineSensor();
  if (lineSensorState == HIGH) { // Line (Black)
    lineSensorStateStr = "Black";
    myServo.write(angleConditionMet);
  } else { // Line (White)
    lineSensorStateStr = "White";
    myServo.write(angleConditionNotMet);
  }
  Serial.print("Line Sensor State: ");
  Serial.println(lineSensorStateStr);

  delay(500);
  myServo.write(0);
  delay(500);
}
