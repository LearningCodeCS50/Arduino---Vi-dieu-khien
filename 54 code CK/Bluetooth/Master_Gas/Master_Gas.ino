#include <SoftwareSerial.h>

SoftwareSerial BTSerial(10, 11); // RX | TX
int GasPin = A0; // Chân cảm biến khí
int val;

void setup() {
  Serial.begin(9600);
  pinMode(GasPin, INPUT);
  Serial.println("Master is ready:");
  BTSerial.begin(38400); // HC-05 baud rate
}

void loop() {
  val = analogRead(GasPin); // Đọc giá trị cảm biến khí
  Serial.println(val);      // Hiển thị giá trị trên Serial Monitor

  if (val > 120) {
    Serial.println("Sending: 1");
    BTSerial.write('1'); // Gửi ký tự '1' qua Bluetooth
  } else {
    Serial.println("Sending: 0");
    BTSerial.write('0'); // Gửi ký tự '0' qua Bluetooth
  }
  delay(2000); // Độ trễ giữa các lần đọc cảm biến

  // Đọc dữ liệu từ Slave
  if (BTSerial.available()) {
    char received = BTSerial.read();
    Serial.println("Received from Slave: " + String(received));
  }
}
