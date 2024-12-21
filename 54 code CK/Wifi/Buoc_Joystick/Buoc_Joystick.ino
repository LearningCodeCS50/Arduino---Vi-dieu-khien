#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Stepper.h>

// Thông tin WiFi
const char *ssid = "Tran Thi Kim Loan";
const char *password = "01229333995";

// Định nghĩa chân cho cảm biến joystick và động cơ bước
#define X_PIN A0  // Chân X của joystick
#define KEY_PIN D5 // Chân KEY (nút nhấn)

#define IN1 D1
#define IN2 D2
#define IN3 D3
#define IN4 D4

// Các biến cho động cơ bước
const int stepperRevolution = 2048; // Số bước trong một vòng quay
Stepper myStepper(stepperRevolution, IN1, IN3, IN2, IN4);

// Thông số và trạng thái của động cơ
int steps = 0;
String direction = "Stopped";  // Chiều quay ban đầu là dừng

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
  <title>Stepper Motor Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <h1>Stepper Motor Control</h1>
  <p>Joystick X Value: <span id="x">%X%</span></p>
  <p>Motor Direction: <span id="direction">%DIRECTION%</span></p>
  <form id="motorForm">
    <label for="steps">Number of steps (0-360): </label>
    <input type="number" name="steps" min="0" max="360" required>
    <input type="submit" value="GO!">
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

  // Khởi tạo chân cảm biến joystick
  pinMode(KEY_PIN, INPUT_PULLUP);

  // Cấu hình động cơ bước
  myStepper.setSpeed(10);

  // Web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = "{\"x\":\"" + String(readX()) + "\",\"direction\":\"" + direction + "\"}";
    request->send(200, "application/json", jsonResponse);
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("steps", true)) {
      steps = request->getParam("steps", true)->value().toInt();
      Serial.print("Number of steps set to: ");
      Serial.println(steps);
    }
    request->send(200, "text/plain", "Settings updated");
  });

  server.begin();
}

void loop() {
  static unsigned long lastRead = 0;
  const unsigned long motorInterval = 10;  // Khoảng thời gian giữa các bước động cơ (ms)
  const unsigned long joystickInterval = 100;  // Khoảng thời gian đọc cảm biến joystick (ms)

  static int currentStep = 0;        // Current step count
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

    // Nếu động cơ không quay, giữ trạng thái là "Stopped"
    if (steps == 0) {
      direction = "Stopped";
    }

    // In thông tin thay đổi nếu có
    if (direction != prevDirection) {
      prevDirection = direction;
      Serial.print("Joystick X Value: ");
      Serial.println(xValue);
      Serial.print("Motor Direction: ");
      Serial.println(direction);
      Serial.println();
      currentStep = 0;
    }
  }

  // Điều khiển động cơ bước
  if (steps > 0) {
      int totalSteps = map(steps, 0, 360, 0, stepperRevolution);
      if (currentStep < totalSteps) {
        myStepper.step(direction == "Clockwise" ? 1 : -1);
        ESP.wdtFeed();  // Feed watchdog để tránh reset
        currentStep++;
      } else {
        currentStep = 0;  // Reset step count after completing a full rotation
        delay(500);
      }
  }

  // Kiểm tra nút nhấn để dừng động cơ
  if (readKey()) {
    direction = "Stopped";  // Nếu nhấn nút, dừng động cơ
    Serial.println("Motor stopped");
    steps = 0;
  }
}
