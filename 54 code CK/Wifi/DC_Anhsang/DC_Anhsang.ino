#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// WiFi thông tin
const char *ssid = "Tran Thi Kim Loan";
const char *password = "01229333995";

// Cảm biến ánh sáng và động cơ DC
#define lightSensor A0
#define in1 D1 // Motor direction pin 1
#define in2 D2 // Motor direction pin 2
#define motorSpeedPin D4 // Motor speed PWM control

int lightValue = 0; // Giá trị cảm biến ánh sáng
int motorSpeed = 0; // Tốc độ động cơ từ web

// Trạng thái quay của động cơ
String motorState = "Stopped";

// Tạo web server
AsyncWebServer server(80);

// HTML giao diện web
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Motor Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <h1>Motor Control</h1>
  <p>Light Sensor Value: <span id="light">%LIGHT%</span></p>
  <p>Motor State: <span id="state">%STATE%</span></p>
  
  <form id="motorForm">
    <label for="speed">Speed (0-255): </label>
    <input type="number" name="speed" min="0" max="255" value="0" required>
    <br><br>
    <input type="submit" value="Update">
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
          document.getElementById("light").innerHTML = data.light;
          document.getElementById("state").innerHTML = data.state;
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
  if (var == "STATE") {
    return motorState;
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

  // Khởi tạo cảm biến và động cơ DC
  pinMode(lightSensor, INPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(motorSpeedPin, OUTPUT);

  // Cấu hình Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = "{\"light\":\"" + String(lightValue) + "\",\"state\":\"" + motorState + "\"}";
    request->send(200, "application/json", jsonResponse);
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("speed", true)) {
      motorSpeed = request->getParam("speed", true)->value().toInt();
      Serial.print("Updated motor speed: ");
      Serial.println(motorSpeed);
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

   // Kiểm tra tốc độ động cơ và điều chỉnh trạng thái
  if (motorSpeed == 0) {
    motorState = "Stopped";
    digitalWrite(in1, LOW);  // Dừng động cơ
    digitalWrite(in2, LOW);  // Dừng động cơ
  } else {
    // Điều khiển động cơ DC dựa trên giá trị cảm biến ánh sáng
    if (lightValue > 700) {  // Điều kiện ánh sáng yếu
      motorState = "Running Clockwise";
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
    } else {  // Điều kiện ánh sáng mạnh
      motorState = "Running Counterclockwise";
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
    }
  }

  // Điều chỉnh tốc độ động cơ DC theo giá trị từ web
  analogWrite(motorSpeedPin, motorSpeed);
}
