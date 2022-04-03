#include <Arduino.h>
#include <Wire.h>

#define CLK_LED_PIN A0 

//#define ONE 

#ifdef ONE 
#define ADDRESS_SELF 12
#define ADDRESS_FRIEND 14 
#else 
#define ADDRESS_SELF 14
#define ADDRESS_FRIEND 12 
#endif 

uint8_t datagram[16] = { 15, 16, 17 };

void onWireData(int count){
  digitalWrite(A2, !digitalRead(A2));
  Serial.println("rx " + String(count));
}

void transmit(void){
  digitalWrite(A1, HIGH);
  // become host 
  Wire.begin();
  // transmit... 
  Wire.beginTransmission(ADDRESS_FRIEND);
  Wire.write(datagram, 16);
  uint8_t res = Wire.endTransmission();
  Serial.println("res: " + String(res));
  // become guest 
  Wire.begin(ADDRESS_SELF);
  digitalWrite(A1, LOW);
}

void setup() {
  Serial.begin(9600);
  Wire.begin(ADDRESS_SELF);
  Wire.onReceive(onWireData);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A2, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(CLK_LED_PIN, OUTPUT);
}

unsigned long lastTx = 0;

void loop() {
  if(lastTx + 50 < millis()){
    digitalWrite(CLK_LED_PIN, !digitalRead(CLK_LED_PIN));
    lastTx = millis();
    transmit();
  }
}