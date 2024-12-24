#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h>

// WiFi thông tin
const char *ssid = "Tran Thi Kim Loan";
const char *password = "01229333995";

// Cảm biến ánh sáng và servo
#define lightSensor A0
#define servoPin D1

Servo myServo;
int lightValue = 0;      // Giá trị cảm biến ánh sáng
int angleConditionMet = 0;     // Góc khi thỏa điều kiện ánh sáng
int angleConditionNotMet = 0;  // Góc khi không thỏa điều kiện ánh sáng

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
  <p>Light Sensor Value: <span id="light">%LIGHT%</span></p>
  <form id="servoForm">
    <label for="angleMet">Angle (Condition Met): </label>
    <input type="number" name="angleMet" min="0" max="180" required>
    <br>
    <label for="angleNotMet">Angle (Condition Not Met): </label>
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
          document.getElementById("light").innerHTML = data.light;
        })
        .catch(error => console.error("Error:", error));
    }, 2000);
  </script>
</body>
</html>
)rawliteral";

// Đọc giá trị cảm biến ánh sáng
int readLightSensor() {
  return analogRead(lightSensor);
}

// Xử lý HTML
String processor(const String &var) {
  if (var == "LIGHT") {
    return String(lightValue);
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
  pinMode(lightSensor, INPUT);
  myServo.attach(servoPin);
  myServo.write(0);

  // Cấu hình Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = "{\"light\":\"" + String(lightValue) + "\"}";
    request->send(200, "application/json", jsonResponse);
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("angleMet", true) && request->hasParam("angleNotMet", true)) {
      angleConditionMet = request->getParam("angleMet", true)->value().toInt();
      angleConditionNotMet = request->getParam("angleNotMet", true)->value().toInt();
      Serial.print("Updated angles: ");
      Serial.print("Condition Met: ");
      Serial.print(angleConditionMet);
      Serial.print(", Condition Not Met: ");
      Serial.println(angleConditionNotMet);
    }
    request->send(200, "text/plain", "Settings updated");
  });

  server.begin();
}

void loop() {
  // Đọc giá trị cảm biến ánh sáng
  lightValue = readLightSensor();
  Serial.print("Light Sensor Value:");
  Serial.println(lightValue);
  // Điều khiển servo dựa trên giá trị cảm biến ánh sáng
  if (lightValue > 800) { // Điều kiện ánh sáng mạnh
    myServo.write(angleConditionMet);
  } else { // Điều kiện ánh sáng yếu
    myServo.write(angleConditionNotMet);
  }
  delay(500);
  myServo.write(0);
  delay(500);
}
