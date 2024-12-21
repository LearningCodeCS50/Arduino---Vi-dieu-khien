#include <IRremote.h>
#define PIN_RECV 2

IRrecv receiver(PIN_RECV);
decode_results output;

// Kết nối động cơ A
int enableA = 3;
int input1 = 5;
int input2 = 6;

void setup() {
  pinMode(enableA, OUTPUT);
  pinMode(input1, OUTPUT);
  pinMode(input2, OUTPUT);
  Serial.begin(9600);
  IrReceiver.begin(PIN_RECV, ENABLE_LED_FEEDBACK);
}

void loop() {
  //  Serial.println("Received something...");
  //  Serial.println();
  //  Serial.println(IrReceiver.decode());
  if (IrReceiver.decode()) {
    Serial.print("Mã IR nhận được: 0x");
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);  // In ra mã IR nhận được
    Serial.print("Giao thức: ");
    Serial.println(IrReceiver.decodedIRData.protocol, DEC);
    if (IrReceiver.decodedIRData.decodedRawData == 0x101) {
      Serial.println("decode");
      analogWrite(enableA, 255);
      // Động cơ chạy thuận 5s
      digitalWrite(input1, HIGH);
      digitalWrite(input2, LOW);
      delay(500);
    } else if (IrReceiver.decodedIRData.decodedRawData == 0x102) {
      Serial.println("decoode2");
      analogWrite(enableA, 255);
      digitalWrite(input1, LOW);
      digitalWrite(input2, HIGH);
      delay(500);
    }
    IrReceiver.resume();
  }
}
