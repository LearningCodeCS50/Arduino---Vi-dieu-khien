#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h>
#include "DHT.h" // Thư viện cho DHT11

// Thông tin WiFi
const char *ssid = "D320";
const char *password = "20012005";

// Định nghĩa chân cho cảm biến DHT và servo
#define DHT_PIN D5
#define DHT_TYPE DHT11
#define SERVO_PIN D1

// Tạo đối tượng DHT và Servo
DHT dht(DHT_PIN, DHT_TYPE);
Servo myServo;

// Biến nhiệt độ và độ ẩm
float temperature = 0;
float humidity = 0;

// Góc servo cho các điều kiện
int angleConditionMet = 90;   // Góc khi điều kiện thỏa
int angleConditionNotMet = 0; // Góc khi điều kiện không thỏa

// Tạo web server
AsyncWebServer server(80);

// Hàm đọc dữ liệu từ cảm biến DHT
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
  <title>Servo Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <h1>Servo Control</h1>
  <p>Temperature: <span id="temperature">%TEMP%</span> &deg;C</p>
  <p>Humidity: <span id="humidity">%HUM%</span> %</p>
  <form id="servoForm">
    <label for="angleMet">Angle when condition met (0-180): </label>
    <input type="number" name="angleMet" min="0" max="180" required>
    <br>
    <label for="angleNotMet">Angle when condition not met (0-180): </label>
    <input type="number" name="angleNotMet" min="0" max="180" required>
    <br>
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
          document.getElementById("temperature").innerHTML = data.temperature;
          document.getElementById("humidity").innerHTML = data.humidity;
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

  // Khởi tạo Servo
  myServo.attach(SERVO_PIN);
  myServo.write(angleConditionNotMet); // Servo khởi đầu ở góc không thỏa

  // Web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = "{\"temperature\":\"" + String(temperature) + "\",\"humidity\":\"" + String(humidity) + "\"}";
    request->send(200, "application/json", jsonResponse);
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("angleMet", true) && request->hasParam("angleNotMet", true)) {
      angleConditionMet = request->getParam("angleMet", true)->value().toInt();
      angleConditionNotMet = request->getParam("angleNotMet", true)->value().toInt();

      angleConditionMet = constrain(angleConditionMet, 0, 180);
      angleConditionNotMet = constrain(angleConditionNotMet, 0, 180);

      Serial.print("Updated angles -> Met: ");
      Serial.print(angleConditionMet);
      Serial.print(", Not Met: ");
      Serial.println(angleConditionNotMet);
    }
    request->send(200, "text/plain", "Angles updated");
  });

  server.begin();
}

void loop() {
  static unsigned long lastDHTRead = 0;
  const unsigned long DHTInterval = 2000; // Đọc cảm biến DHT mỗi 2 giây

  // Đọc cảm biến DHT theo chu kỳ
  if (millis() - lastDHTRead >= DHTInterval) {
    lastDHTRead = millis();
    readDHTSensor();

    // Điều chỉnh servo dựa trên nhiệt độ
    if (temperature > 27) {
      myServo.write(angleConditionMet); // Góc khi điều kiện thỏa
    } else {
      myServo.write(angleConditionNotMet); // Góc khi điều kiện không thỏa
    }

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
    Serial.print("Servo Condition: ");
    Serial.println(temperature > 27 ? "Condition Met" : "Condition Not Met");
  }
  delay(500);
  myServo.write(0);
  delay(500);
}
