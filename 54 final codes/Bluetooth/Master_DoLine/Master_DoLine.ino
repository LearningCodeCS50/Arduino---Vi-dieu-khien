// Connect the HC-05 and communicate using the serial monitor
// When first powered on. you need to manually enter AT mode
// The default baud rate for AT mode is 38400
#include <SoftwareSerial.h>
#define LINE 2
int count0 = 0;
int count1 = 0;
int val = 0;
SoftwareSerial BTSerial(10, 11); // RX | TX
// Connect the HC-05 TX to Arduino pin 10
// Connect the HC-05 RX to Arduino pin 11

void setup() {
  
  Serial.begin(9600);
  Serial.println("Arduino is ready:");
  Serial.println("Remember to select Both BL & CR in the serial monitor");
  BTSerial.begin(38400); // HC-05 default speed in AT command
}

void loop() {
  val = digitalRead(LINE);
  Serial.println(val);
  if (val == 0) {    
      Serial.println("YES");
      BTSerial.write("1");
      delay(2000);
    }
    else{
      Serial.println("NO");
      BTSerial.write("0");
      delay(2000);
    } 
  if (BTSerial.available())
  {
    Serial.write(BTSerial.read());
  }
  if (Serial.available())
  {
    BTSerial.write(Serial.read());
  }
  

}
