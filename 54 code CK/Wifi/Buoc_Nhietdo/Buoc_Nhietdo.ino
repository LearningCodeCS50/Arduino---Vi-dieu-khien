#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Stepper.h>
#include "DHT.h" // Thư viện cho DHT11

// Thông tin WiFi
const char *ssid = "Tran Thi Kim Loan";
const char *password = "01229333995";

// Định nghĩa chân cho cảm biến nhiệt độ, độ ẩm, và động cơ bước
#define DHT_PIN D5
#define DHT_TYPE DHT11
#define IN1 D1
#define IN2 D2
#define IN3 D3
#define IN4 D4

// Tạo đối tượng DHT
DHT dht(DHT_PIN, DHT_TYPE);

// Các biến cho động cơ bước
const int stepperRevolution = 2048; // Số bước trong một vòng quay
Stepper myStepper(stepperRevolution, IN1, IN3, IN2, IN4);

// Các thông số và trạng thái của động cơ
int steps = 0;
String direction = "Stopped"; // Chiều quay ban đầu là dừng

// Tạo web server
AsyncWebServer server(80);

// Hàm đọc dữ liệu từ cảm biến DHT
float temperature = 0;
float humidity = 0;

void readDHTSensor() {
  temperature = dht.readTemperature(); // Đọc nhiệt độ
  humidity = dht.readHumidity();       // Đọc độ ẩm
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    temperature = 0;
    humidity = 0;
  }
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
  <p>Temperature: <span id="temperature">%TEMP%</span> &deg;C</p>
  <p>Humidity: <span id="humidity">%HUM%</span> %</p>
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
          document.getElementById("temperature").innerHTML = data.temperature;
          document.getElementById("humidity").innerHTML = data.humidity;
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
  if (var == "TEMP") {
    return String(temperature);
  } else if (var == "HUM") {
    return String(humidity);
  } else if (var == "DIRECTION") {
    return direction; // Hiển thị chiều quay của động cơ
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

  // Khởi tạo cảm biến DHT
  dht.begin();

  // Khởi tạo kết nối WiFi
  initWiFi();

  // Cấu hình động cơ bước
  myStepper.setSpeed(10);

  // Web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = "{\"temperature\":\"" + String(temperature) + "\",\"humidity\":\"" + String(humidity) +
                          "\",\"direction\":\"" + direction + "\"}";
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
  static unsigned long lastDHTRead = 0;
  const unsigned long DHTInterval = 2000; // Đọc cảm biến DHT mỗi 2 giây
  const unsigned long motorInterval = 10; // Khoảng thời gian giữa các bước động cơ (ms)

  static int currentStep = 0;
  static String prevDirection = "";

  // Đọc cảm biến DHT theo chu kỳ
  if (millis() - lastDHTRead >= DHTInterval) {
    lastDHTRead = millis();
    readDHTSensor();

    // Điều chỉnh chiều quay của động cơ dựa trên nhiệt độ
    if (temperature > 28) {
      direction = "Counterclockwise";  // Ngược chiều
    } else {
      direction = "Clockwise";         // Thuận chiều
    }
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
    Serial.print("Motor Direction: ");
    Serial.println(direction);

    if (direction != prevDirection) {
      prevDirection = direction;
      currentStep = 0;
    }
  }

  // Điều khiển động cơ bước
  if (steps > 0) {
    int totalSteps = map(steps, 0, 360, 0, stepperRevolution);
    if (currentStep < totalSteps) {
      myStepper.step(direction == "Clockwise" ? 1 : -1);
      currentStep++;
      yield();  // Nhường CPU cho các tác vụ khác
      ESP.wdtFeed();
    } else {
      currentStep = 0;
      delay(500);
    }
  }
}
