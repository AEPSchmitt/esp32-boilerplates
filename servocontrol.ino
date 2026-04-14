#include <ESP32Servo.h>

Servo myServo;

const int servoPin = 0; // change accordingly

void setup() {
  myServo.attach(servoPin);
}

void loop() {
  myServo.write(0);
  delay(2000);

  myServo.write(185);
  delay(2000);
}