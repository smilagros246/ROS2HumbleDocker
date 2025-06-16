// SCL -> A5
// SDA -> A4

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// Rango de pulsos en "ticks"
const uint8_t SERVO_FREQ = 50;
const uint16_t SERVO_MIN = 102;
const uint16_t SERVO_MAX = 512;

// Ángulos para servos 0 a 4
uint8_t angulos[5] = {0, 45, 90, 135, 180};  // Puedes cambiarlos según desees

void setup() {
  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQ);

  for (uint8_t i = 0; i < 5; i++) {
    uint16_t pwm_val = map(angulos[i], 0, 180, SERVO_MIN, SERVO_MAX);
    pwm.setPWM(i, 0, pwm_val);  // Canal i, desde 0 hasta 4
  }
}

void loop() {
 
}
