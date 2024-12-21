#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h>

// WiFi thông tin
const char *ssid = "D320";
const char *password = "20012005";

// Cảm biến khí gas và servo
#define gasSensor A0      // Chân analog để đọc giá trị cảm biến khí gas
#define servoPin D1       // Chân servo

Servo myServo;
int gasValue = 0;        // Giá trị cảm biến khí gas
int angleConditionMet = 0;     // Góc khi có khí gas
int angleConditionNotMet = 0;  // Góc khi không có khí gas

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
  <p>Gas Sensor Value: <span id="gas">%GAS%</span></p>
  <form id="servoForm">
    <label for="angleMet">Angle (Gas Detected): </label>
    <input type="number" name="angleMet" min="0" max="180" required>
    <br>
    <label for="angleNotMet">Angle (No Gas Detected): </label>
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
          document.getElementById("gas").innerHTML = data.gas;
        })
        .catch(error => console.error("Error:", error));
    }, 2000);
  </script>
</body>
</html>
)rawliteral";

// Đọc giá trị cảm biến khí gas
int readGasSensor() {
  return analogRead(gasSensor);
}

// Xử lý HTML
String processor(const String &var) {
  if (var == "GAS") {
    return String(gasValue);
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
  pinMode(gasSensor, INPUT);
  myServo.attach(servoPin);
  myServo.write(0);

  // Cấu hình Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = "{\"gas\":\"" + String(gasValue) + "\"}";
    request->send(200, "application/json", jsonResponse);
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("angleMet", true) && request->hasParam("angleNotMet", true)) {
      angleConditionMet = request->getParam("angleMet", true)->value().toInt();
      angleConditionNotMet = request->getParam("angleNotMet", true)->value().toInt();
      Serial.print("Updated angles: ");
      Serial.print("Gas Detected: ");
      Serial.print(angleConditionMet);
      Serial.print(", No Gas Detected: ");
      Serial.println(angleConditionNotMet);
    }
    request->send(200, "text/plain", "Settings updated");
  });

  server.begin();
}

void loop() {
  // Đọc giá trị cảm biến khí gas
  gasValue = readGasSensor();
  Serial.print("Gas Sensor Value:");
  Serial.println(gasValue);
  
  // Điều khiển servo dựa trên giá trị cảm biến khí gas
  if (gasValue > 120) { // Điều kiện phát hiện khí gas
    myServo.write(angleConditionMet);
  } else { // Không phát hiện khí gas
    myServo.write(angleConditionNotMet);
  }
  delay(500);
  myServo.write(0);
  delay(500);
}
