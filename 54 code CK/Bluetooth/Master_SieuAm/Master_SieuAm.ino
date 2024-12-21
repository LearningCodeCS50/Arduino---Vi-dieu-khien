// Connect the HC-05 and communicate using the serial monitor
// When first powered on. you need to manually enter AT mode
// The default baud rate for AT mode is 38400
#include <SoftwareSerial.h>
int count0 = 0;
int count1 = 0;
SoftwareSerial BTSerial(10, 11); // RX | TX
// Connect the HC-05 TX to Arduino pin 10
// Connect the HC-05 RX to Arduino pin 11

const int trigPin = 7;
const int echoPin = 6;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);  
  Serial.begin(9600);
  Serial.println("Arduino is ready:");
  Serial.println("Remember to select Both BL & CR in the serial monitor");
  BTSerial.begin(38400); // HC-05 default speed in AT command
}

void loop() {
  int duration; // biến đo thời gian
  int distance;           // biến lưu khoảng cách

  digitalWrite(trigPin, LOW); // tắt chân trig
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); // phát xung từ chân trig
  delayMicroseconds(10); // xung có độ dài 10 microSeconds
  digitalWrite(trigPin, LOW); // tắt chân trig
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2.0;

  Serial.print(distance);
  Serial.println("cm"); 
  if (distance < 20) {    
    Serial.println("YES");
    BTSerial.write("1");
  }
  else{
    Serial.println("NO");
    BTSerial.write("0");
  } 
  delay(2000);
}
