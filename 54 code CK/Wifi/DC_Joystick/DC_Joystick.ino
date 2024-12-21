#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Thông tin WiFi
const char *ssid = "Tran Thi Kim Loan";
const char *password = "01229333995";

// Định nghĩa chân cho cảm biến joystick và động cơ DC
#define X_PIN A0  // Chân X của joystick
#define KEY_PIN D5 // Chân KEY (nút nhấn)

#define MOTOR_PIN1 D1
#define MOTOR_PIN2 D2
#define SPEED_PIN D4  // Chân D4 điều khiển tốc độ động cơ (PWM)

// Các biến cho động cơ DC
String direction = "Stopped";  // Chiều quay ban đầu là dừng
int motorSpeed = 0;  // Biến lưu trữ tốc độ động cơ

// Tạo web server
AsyncWebServer server(80);

// Đọc giá trị từ cảm biến joystick
int readX() {
  return analogRead(X_PIN);  // Đọc giá trị từ chân X
}

bool readKey() {
  return digitalRead(KEY_PIN) == LOW;  // Kiểm tra xem nút có được nhấn hay không (nút nhấn là LOW)
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
  <p>Joystick X Value: <span id="x">%X%</span></p>
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
          document.getElementById("x").innerHTML = data.x;
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
  if (var == "X") {
    return String(readX());  // Hiển thị giá trị của X
  } else if (var == "DIRECTION") {
    return direction;  // Hiển thị chiều quay của động cơ
  } else if (var == "SPEED") {
    return String(motorSpeed);  // Hiển thị tốc độ động cơ
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

  // Khởi tạo kết nối WiFi
  initWiFi();

  // Khởi tạo chân cảm biến joystick và KEY
  pinMode(KEY_PIN, INPUT_PULLUP);
  pinMode(MOTOR_PIN1, OUTPUT);
  pinMode(MOTOR_PIN2, OUTPUT);
  pinMode(SPEED_PIN, OUTPUT);  // Khởi tạo chân PWM

  // Web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = "{\"x\":\"" + String(readX()) + "\",\"direction\":\"" + direction + "\"}";
    request->send(200, "application/json", jsonResponse);
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("speed", true)) {
      motorSpeed = request->getParam("speed", true)->value().toInt();
      Serial.print("Motor speed set to: ");
      Serial.println(motorSpeed);

      // Điều chỉnh tốc độ động cơ
      if (motorSpeed > 0) {
        if (direction == "Clockwise") {
          digitalWrite(MOTOR_PIN1, HIGH);
          digitalWrite(MOTOR_PIN2, LOW);
        } else if (direction == "Counterclockwise") {
          digitalWrite(MOTOR_PIN1, LOW);
          digitalWrite(MOTOR_PIN2, HIGH);
        }
        analogWrite(SPEED_PIN, motorSpeed);  // Điều chỉnh tốc độ động cơ qua PWM
      } 
      if (readKey() || motorSpeed == 0){
        digitalWrite(MOTOR_PIN1, LOW);
        digitalWrite(MOTOR_PIN2, LOW);  // Tắt động cơ khi tốc độ bằng 0
        analogWrite(SPEED_PIN, 0);      // Tắt PWM khi không có tốc độ
      }
    }
    request->send(200, "text/plain", "Settings updated");
  });

  server.begin();
}

void loop() {
  static unsigned long lastRead = 0;
  const unsigned long joystickInterval = 100;  // Khoảng thời gian đọc cảm biến joystick (ms)

  static String prevDirection = "";   // Lưu chiều quay trước đó

  // Đọc giá trị cảm biến joystick theo chu kỳ
  if (millis() - lastRead >= joystickInterval) {
    lastRead = millis();
    int xValue = readX();
    
    // Điều chỉnh chiều quay của động cơ dựa trên giá trị joystick
    if (xValue > 542) {
      direction = "Clockwise";         // Quay thuận nếu X lớn
    } else if (xValue <= 542){
      direction = "Counterclockwise";  // Quay ngược nếu X nhỏ
    }

    // In thông tin thay đổi nếu có
    if (direction != prevDirection) {
      prevDirection = direction;
      Serial.print("Joystick X Value: ");
      Serial.println(xValue);
      Serial.print("Motor Direction: ");
      Serial.println(direction);
      Serial.println();
    }
  }

  // Điều khiển tốc độ động cơ dựa trên giá trị motorSpeed
  if (motorSpeed > 0) {
    if (direction == "Clockwise") {
      digitalWrite(MOTOR_PIN1, HIGH);
      digitalWrite(MOTOR_PIN2, LOW);
    } else if (direction == "Counterclockwise") {
      digitalWrite(MOTOR_PIN1, LOW);
      digitalWrite(MOTOR_PIN2, HIGH);
    }
    analogWrite(SPEED_PIN, motorSpeed);  // Điều chỉnh tốc độ động cơ qua PWM
  } 
  // Kiểm tra nút nhấn để dừng động cơ
  if (readKey() || motorSpeed == 0) {
    motorSpeed = 0;
    direction = "Stopped";  // Nếu nhấn nút, dừng động cơ
    Serial.println("Motor stopped");
    digitalWrite(MOTOR_PIN1, LOW);
    digitalWrite(MOTOR_PIN2, LOW);
  }
  analogWrite(SPEED_PIN, motorSpeed);  // Điều chỉnh tốc độ động cơ qua PWM
}
