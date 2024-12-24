#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Thông tin WiFi
const char *ssid = "Tran Thi Kim Loan";
const char *password = "01229333995";

// Định nghĩa chân cho cảm biến khí gas và động cơ DC
#define gasSensor A0
#define motorPin1 D1
#define motorPin2 D2
#define motorSpeedPin D3

// Các biến cho động cơ DC
int motorSpeed = 0;  // Tốc độ động cơ
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
  <title>DC Motor Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <h1>DC Motor Control</h1>
  <p>Gas Sensor Value: <span id="gas">%GAS%</span></p>
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

  // Khởi tạo chân cảm biến khí gas và động cơ DC
  pinMode(gasSensor, INPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorSpeedPin, OUTPUT);

  // Web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = "{\"gas\":\"" + String(readGasSensor()) + "\",\"direction\":\"" + direction + "\"}";
    request->send(200, "application/json", jsonResponse);
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("speed", true)) {
      motorSpeed = request->getParam("speed", true)->value().toInt();
      Serial.print("Motor speed set to: ");
      Serial.println(motorSpeed);
    }
    request->send(200, "text/plain", "Settings updated");
  });

  server.begin();
}

void loop() {
  static unsigned long lastGasRead = 0;
  const unsigned long gasInterval = 500;  // Khoảng thời gian đọc cảm biến khí gas (ms)

  static String prevDirection = "";   // Lưu chiều quay trước đó

  // Đọc giá trị cảm biến khí gas theo chu kỳ
  if (millis() - lastGasRead >= gasInterval) {
    lastGasRead = millis();
    int gasValue = readGasSensor();
    
    // Điều chỉnh chiều quay của động cơ dựa trên giá trị cảm biến khí gas
    if (gasValue > 200) {
      direction = "Counterclockwise";  // Quay ngược khi giá trị khí gas cao
      digitalWrite(motorPin1, LOW);
      digitalWrite(motorPin2, HIGH);
    } else {
      direction = "Clockwise";         // Quay thuận khi giá trị khí gas thấp
      digitalWrite(motorPin1, HIGH);
      digitalWrite(motorPin2, LOW);
    }

    // In thông tin thay đổi nếu có
    if (direction != prevDirection) {
      prevDirection = direction;
      Serial.print("Gas Sensor Value: ");
      Serial.println(gasValue);
      Serial.print("Motor Direction: ");
      Serial.println(direction);
      Serial.println();
    }
  }

  // Nếu tốc độ là 0, dừng động cơ
  if (motorSpeed == 0) {
    direction = "Stopped";
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
  } else {
    // Điều chỉnh tốc độ động cơ
    analogWrite(motorSpeedPin, motorSpeed);
  }
}
