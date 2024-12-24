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

struct DC{
  int speed;
  int curspeed = 0;
  int v1,v2;
  void tov(int speed){
    if (speed>=0){
      v2 = 0;
      v1 = speed;
    }
    else{
      v1 = 0;
      v2 = -speed;
    }
  }
  void apply_speed(){
    tov(curspeed);
    if (v1==0){
      digitalWrite(input1,LOW);
    }
    else{
      analogWrite(input1,v1);
    }
    if (v2==0){
      digitalWrite(input2,LOW);
    }
    else{
      analogWrite(input2,v2);
    }
    
  }
  //nhanh dần : 0~255 ->0 ~255
  // chậm dần : 255 ~ 0 -> ...
  // Thuận chiều:  
  // hàm gọi sẽ gán giá trị speed 
  
  //Gọi hàm trong vòng loop, 
  void thuan_chieu_nhanh_dan(){
  
    if (curspeed<0) curspeed=0;
    curspeed+=25;
    if (curspeed>255){
      curspeed = 0;
    }
    apply_speed();
  }
  void thuan_chieu_cham_dan(){
    if (curspeed>255) curspeed = 255;
    curspeed -= 25;
    if (curspeed<0) curspeed = 255;
    apply_speed();
  }
  void nguoc_chieu_nhanh_dan(){
    if (curspeed>0) curspeed = 0;
    if (curspeed< -255) curspeed = 0;
    curspeed -=25;
    apply_speed();
  }
  void nguoc_chieu_cham_dan(){
    if (curspeed>0) curspeed = -255;
    if (curspeed<-255) curspeed = -255;
    curspeed +=25;
    apply_speed();
  }
  void thuan_chieu_quay_deu(){
    curspeed = 255;
    apply_speed();
  }
  void nguoc_chieu_quay_deu(){
    curspeed = -255;
    apply_speed();
  } 
  void loop(int type) {
    switch (type)
    {
      case 1: // thuan_chieu_nhanh_dan
        thuan_chieu_nhanh_dan();
      break;
      case 2: // thuan_chieu_cham_dan
        thuan_chieu_cham_dan();
      break;
      case 3: // nguoc_chieu_nhanh_dan
        nguoc_chieu_nhanh_dan();
      break;
      case 4: // nguoc_chieu_cham_dan
        nguoc_chieu_cham_dan();
      break;
      case 5: // thuan_chieu_quay_deu
        thuan_chieu_quay_deu();
      break;
      case 6: // nguoc_chieu_quay_deu
        nguoc_chieu_quay_deu();
      break;
    default:
      break;
    }
    Serial.print(v1);
    Serial.print("-");
    Serial.println(v2);
    delay(255);
  }
} DCTask;

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
    char c = BTSerial.read();
    Serial.println(c);
    if (c== '1')
    {
      DCTask.thuan_chieu_nhanh_dan();
      delay(255);
    }
    else {
      // Điều khiển tốc độ động cơ
      DCTask.nguoc_chieu_cham_dan();
      delay(255);  
    }
  }
  if (Serial.available())
  {
    BTSerial.write(Serial.read());
  }
}
