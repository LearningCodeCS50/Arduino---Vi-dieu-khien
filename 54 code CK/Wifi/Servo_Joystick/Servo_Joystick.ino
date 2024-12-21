#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h>

// WiFi thông tin
const char *ssid = "D320";
const char *password = "20012005";

// Cảm biến joystick và servo
#define joystickX A0     // Chân analog cho trục X của joystick
#define servoPin D1      // Chân servo

Servo myServo;
int joystickValue = 0;       // Giá trị trục X của joystick
int angleConditionMet = 0;   // Góc khi joystick vượt ngưỡng
int angleConditionNotMet = 0; // Góc khi joystick nhỏ hơn ngưỡng
int joystickThreshold = 550; // Ngưỡng giá trị của joystick

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
  <p>Joystick X Value: <span id="joystick">%JOYSTICK%</span></p>
  <form id="servoForm">
    <label for="angleMet">Angle (Condition Met): </label>
    <input type="number" id="angleMet" name="angleMet" min="0" max="180" required>
    <br>
    <label for="angleNotMet">Angle (Condition Not Meet): </label>
    <input type="number" id="angleNotMet" name="angleNotMet" min="0" max="180" required>
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
          document.getElementById("joystick").innerHTML = data.joystick;
        })
        .catch(error => console.error("Error:", error));
    }, 2000);
  </script>
</body>
</html>
)rawliteral";

// Đọc giá trị trục X của joystick
int readJoystickX() {
  return analogRead(joystickX);
}

// Xử lý HTML
String processor(const String &var) {
  if (var == "JOYSTICK") {
    return String(joystickValue);
  } else return String();
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

  // Khởi tạo joystick và servo
  pinMode(joystickX, INPUT);
  myServo.attach(servoPin);
  myServo.write(90); // Servo bắt đầu ở góc giữa

  // Cấu hình Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = "{\"joystick\":\"" + String(joystickValue) + "\"}";
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
  // Đọc giá trị trục X của joystick
  joystickValue = readJoystickX();
  Serial.print("Joystick X Value: ");
  Serial.println(joystickValue);

  // Điều khiển servo dựa trên giá trị joystick
  if (joystickValue > joystickThreshold) {
    myServo.write(angleConditionMet);
  } else {
    myServo.write(angleConditionNotMet);
  }
  delay(500);
  myServo.write(0);
  delay(500);
}
