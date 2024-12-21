// Connect the HC-05 and communicate using the serial monitor
// When first powered on. you need to manually enter AT mode
// The default baud rate for AT mode is 38400
#include <DHT.h>
#include <SoftwareSerial.h>
int count0 = 0;
int count1 = 0;
SoftwareSerial BTSerial(10, 11); // RX | TX
// Connect the HC-05 TX to Arduino pin 10
// Connect the HC-05 RX to Arduino pin 11
#define DHT11Pin 4
#define DHTType DHT11
DHT HT(DHT11Pin, DHTType);

float humi;
float temp;

void setup() {
  HT.begin();
  Serial.begin(9600);
  Serial.println("Arduino is ready:");
  Serial.println("Remember to select Both BL & CR in the serial monitor");
  BTSerial.begin(38400); // HC-05 default speed in AT command
}

void loop() {
  humi = HT.readHumidity();
  temp = HT.readTemperature();
  Serial.print("Humidity: ");
  Serial.println(humi);
  Serial.print("Temperature: ");
  Serial.println(temp);
  if (humi > 90 && temp > 29) {    
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
