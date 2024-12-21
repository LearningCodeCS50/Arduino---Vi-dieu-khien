#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Thông tin WiFi
const char *ssid = "Tran Thi Kim Loan";
const char *password = "01229333995";

// Định nghĩa chân cho cảm biến siêu âm và động cơ DC
const int trigPin = D5;  // Chân trigger
const int echoPin = D6;  // Chân echo
const int motorPin1 = D1; // Chân điều khiển chiều quay 1
const int motorPin2 = D2; // Chân điều khiển chiều quay 2
const int motorSpeedPin = D4; // Chân PWM điều khiển tốc độ động cơ

// Tạo web server
AsyncWebServer server(80);

// Các biến để lưu giá trị
int motorSpeed = 255;  // Tốc độ động cơ (0-255)
String direction = "Stopped";  // Chiều quay ban đầu là dừng

// Đọc giá trị từ cảm biến siêu âm
long readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);  // Đọc thời gian
  long distance = duration * 0.034 / 2;   // Tính khoảng cách
  return distance;
}

// HTML giao diện web
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>DC Motor Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <h1>DC Motor Control</h1>
  <p>Distance from sensor: <span id="distance">%DISTANCE%</span> cm</p>
  <p>Motor Direction: <span id="direction">%DIRECTION%</span></p>
  <form id="motorForm">
    <label for="steps">Speed (0-255): </label>
    <input type="number" name="steps" min="0" max="255" required>
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
          document.getElementById("distance").innerHTML = data.distance;
          document.getElementById("direction").innerHTML = data.direction;
          document.getElementById("speed").innerHTML = data.speed;
        })
        .catch(error => console.error("Error:", error));
    }, 2000);
  </script>
</body>
</html>
)rawliteral";

// Hàm xử lý HTML
String processor(const String &var) {
  if (var == "DISTANCE") {
    return String(readDistance());  // Hiển thị giá trị khoảng cách
  } else if (var == "DIRECTION") {
    return direction;  // Hiển thị chiều quay của động cơ
  } else if (var == "SPEED") {
    return String(motorSpeed);  // Hiển thị tốc độ của động cơ
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
  // Khởi tạo Serial Monitor
  Serial.begin(9600);

  // Khởi tạo kết nối WiFi
  initWiFi();

  // Khởi tạo chân cho cảm biến siêu âm
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Khởi tạo chân điều khiển động cơ DC
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorSpeedPin, OUTPUT);

  // Web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = "{\"distance\":\"" + String(readDistance()) + "\",\"direction\":\"" + direction + "\"}";
    request->send(200, "application/json", jsonResponse);
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("steps", true)) {
      motorSpeed = request->getParam("steps", true)->value().toInt();
      motorSpeed = constrain(motorSpeed, 0, 255);  // Đảm bảo tốc độ nằm trong khoảng 0-255
      Serial.print("Motor speed set to: ");
      Serial.println(motorSpeed);

      // Điều chỉnh chiều quay và tốc độ động cơ dựa trên khoảng cách
      long distance = readDistance();
      if (distance > 20) {
        direction = "Clockwise";
        digitalWrite(motorPin1, HIGH);
        digitalWrite(motorPin2, LOW);
      } else {
        direction = "Counterclockwise";
        digitalWrite(motorPin1, LOW);
        digitalWrite(motorPin2, HIGH);
      }

      // Nếu tốc độ là 0, dừng động cơ
      if (motorSpeed == 0) {
        direction = "Stopped";
        digitalWrite(motorPin1, LOW);
        digitalWrite(motorPin2, LOW);  // Tắt cả hai chân điều khiển chiều quay
        analogWrite(motorSpeedPin, 0); // Tắt tín hiệu PWM
      } else {
        analogWrite(motorSpeedPin, motorSpeed); // Điều chỉnh tốc độ động cơ
      }
    }
    request->send(200, "text/plain", "Settings updated");
  });

  server.begin();
}

void loop() {
 long distance = readDistance();
  if (distance > 20) {
    direction = "Clockwise";
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
  } else {
    direction = "Counterclockwise";
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
  }

  // Nếu tốc độ là 0, dừng động cơ
  if (motorSpeed == 0) {
    direction = "Stopped";
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);  // Tắt cả hai chân điều khiển chiều quay
    analogWrite(motorSpeedPin, 0); // Tắt tín hiệu PWM
  } else {
    analogWrite(motorSpeedPin, motorSpeed); // Điều chỉnh tốc độ động cơ
  }
}
