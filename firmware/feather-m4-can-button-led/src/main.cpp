#include <Arduino.h>

void setup() {
  pinMode(13, OUTPUT);
  // put your setup code here, to run once:
}

void loop() {
  digitalWrite(13, HIGH);
  delay(50);
  digitalWrite(13, LOW);
  delay(50);
  // put your main code here, to run repeatedly:
}