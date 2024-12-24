#define PIN_SEND 3
#include <IRremote.h> 
#include <DHT.h>
#define DHT11Pin 4
#define DHTType DHT11
DHT HT(DHT11Pin, DHTType);

float tempC;
float humidity;

void setup() { 
  HT.begin();
  Serial.begin(9600);
  IrSender.begin(PIN_SEND); 
} 
  
void loop() { 
  delay(1000);
  tempC = HT.readTemperature();
  humidity = HT.readHumidity();
  Serial.print("Temperature: ");
  Serial.println(tempC);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  if (tempC > 28 && humidity > 80) {
    IrSender.sendRC5(0x1, 8);  // Truyền mã 0x1 với độ dài 8 bit
    delay(200);
  } else {
    IrSender.sendRC5(0x2, 8);  // Truyền mã 0x2 với độ dài 8 bit
    delay(200);
  }  
}
