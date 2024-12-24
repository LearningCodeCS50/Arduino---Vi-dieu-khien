#include <IRremote.h>
#include <Servo.h>
#define PIN_RECV 2
// #define PIN_RECV 2
Servo myservo;
IRrecv receiver(PIN_RECV);
decode_results output;
int servo_pin = 9;
void setup() {
  myservo.attach(servo_pin);
  Serial.begin(9600);
  IrReceiver.begin(PIN_RECV, ENABLE_LED_FEEDBACK);
   myservo.write(0);
}

void loop() {
  //  Serial.println("Received something...");
  //  Serial.println();
  if (IrReceiver.decode()) {
    Serial.print("Mã IR nhận được: 0x");
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);  // In ra mã IR nhận được
    Serial.print("Giao thức: ");
    Serial.println(IrReceiver.decodedIRData.protocol, DEC);
    if (IrReceiver.decodedIRData.decodedRawData == 0x101) {
      Serial.println("decode");
      myservo.write(150);
      delay(500);
      myservo.write(0);
      delay(500);
    } else if (IrReceiver.decodedIRData.decodedRawData == 0x102) {
      Serial.println("decode2");
      myservo.write(50);
      delay(500);
      myservo.write(0);
      delay(500);
    }
    IrReceiver.resume();
  } 
}
