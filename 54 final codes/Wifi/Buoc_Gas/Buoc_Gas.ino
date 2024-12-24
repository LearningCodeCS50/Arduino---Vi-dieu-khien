#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Stepper.h>

// Thông tin WiFi
const char *ssid = "Tran Thi Kim Loan";
const char *password = "01229333995";

// Định nghĩa chân cho cảm biến khí gas và động cơ bước
#define gasSensor A0
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

// Đọc giá trị từ cảm biến khí gas
int readGasSensor() {
  return analogRead(gasSensor);  // Đọc giá trị từ cảm biến khí gas
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
  <p>Gas Sensor Value: <span id="gas">%GAS%</span></p>
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
          document.getElementById("gas").innerHTML = data.gas;
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
  if (var == "GAS") {
    return String(readGasSensor());  // Hiển thị giá trị cảm biến khí gas
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

  // Khởi tạo chân cảm biến khí gas
  pinMode(gasSensor, INPUT);

  // Cấu hình động cơ bước
  myStepper.setSpeed(10);

  // Web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = "{\"gas\":\"" + String(readGasSensor()) + "\",\"direction\":\"" + direction + "\"}";
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
  static unsigned long lastGasRead = 0;
  const unsigned long motorInterval = 10;  // Khoảng thời gian giữa các bước động cơ (ms)
  const unsigned long gasInterval = 500;  // Khoảng thời gian đọc cảm biến khí gas (ms)

  static int currentStep = 0;        // Current step count
  static String prevDirection = "";   // Lưu chiều quay trước đó

  // Đọc giá trị cảm biến khí gas theo chu kỳ
  if (millis() - lastGasRead >= gasInterval) {
    lastGasRead = millis();
    int gasValue = readGasSensor();
    
    // Điều chỉnh chiều quay của động cơ dựa trên giá trị cảm biến khí gas
    if (gasValue > 200) {
      direction = "Counterclockwise";  // Quay ngược khi giá trị khí gas cao
    } else {
      direction = "Clockwise";         // Quay thuận khi giá trị khí gas thấp hoặc bằng 700
    }

    // In thông tin thay đổi nếu có
    if (direction != prevDirection) {
      prevDirection = direction;
      Serial.print("Gas Sensor Value: ");
      Serial.println(gasValue);
      Serial.print("Motor Direction: ");
      Serial.println(direction);
      Serial.println();
      currentStep = 0;
    }
  }

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
}

