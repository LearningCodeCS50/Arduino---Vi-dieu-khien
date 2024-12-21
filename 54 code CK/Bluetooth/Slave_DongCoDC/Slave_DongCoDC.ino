// Connect the HC-05 and communicate using the serial monitor
// When first powered on. you need to manually enter AT mode
// The default baud rate for AT mode is 38400
#include <SoftwareSerial.h> 
SoftwareSerial BTSerial(10, 11); // RX | TX
// Connect the HC-05 TX to Arduino pin 10
// Connect the HC-05 RX to Arduino pin 11

int enableA = 3;
int input1 = 5;
int input2 = 6;

void setup() {
  pinMode(enableA, OUTPUT);
  pinMode(input1, OUTPUT);
  pinMode(input2, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  Serial.begin(9600);
  Serial.println("Arduino is ready:");
  Serial.println("Remember to select Both BL & CR in the serial monitor");
  BTSerial.begin(38400); // HC-05 default speed in AT command
  pinMode(7, OUTPUT);
}

void loop() {
  if (BTSerial.available())
  {
    // Điều khiển tốc độ động cơ
    analogWrite(enableA, 200);
    char c = BTSerial.read();
    Serial.println(c);
    if (c== '1')
    {
      digitalWrite(input1, HIGH);
      digitalWrite(input2, LOW);
      delay(1000);
    }
    else {
      // Động cơ chạy nghịch 5s
      digitalWrite(input1, LOW);
      digitalWrite(input2, HIGH);
      delay(1000);  
    }
    analogWrite(enableA, 0);
    delay(1000);
  }
  if (Serial.available())
  {
    BTSerial.write(Serial.read());
  }
}
