// SCL -> A5
// SDA -> A4

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// Rango de pulsos en "ticks" (basado en 50 Hz)
const uint8_t SERVO_FREQ = 50;
const uint16_t SERVO_MIN = 102;  // ≈ 0.5 ms
const uint16_t SERVO_MAX = 512;  // ≈ 2.5 ms

// Servo a controlar
const uint8_t canal_servo = 0;  // puedes cambiarlo entre 0 y 15
const uint8_t angulo = 90;      // cambia el ángulo entre 0 y 180

void setup() {
  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQ);

  // Convertimos ángulo a "ticks"
  uint16_t pwm_val = map(angulo, 0, 180, SERVO_MIN, SERVO_MAX);
  pwm.setPWM(canal_servo, 0, pwm_val);
}

void loop() {
  //Vacio
}
