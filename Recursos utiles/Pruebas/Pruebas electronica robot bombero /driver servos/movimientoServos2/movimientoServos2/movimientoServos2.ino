#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// Configuración del servo
const uint8_t SERVO_FREQ = 50;
const uint16_t SERVO_MIN = 102;
const uint16_t SERVO_MAX = 512;

int ultimo_angulo[16];  // Guarda el último ángulo de cada canal

void setup() {
  Serial.begin(115200);
  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQ);

  for (int i = 0; i < 16; i++) {
    ultimo_angulo[i] = 90;  // Asumimos posición media inicial
    uint16_t pwm_val = map(90, 0, 180, SERVO_MIN, SERVO_MAX);
    pwm.setPWM(i, 0, pwm_val);
  }

  Serial.println("Listo. Envia: <canal>,<angulo>");
  Serial.println("Ejemplo: 3,120");
}

void moverSuave(int canal, int desde, int hasta, int paso = 1, int retardo = 30) {
  if (desde < hasta) {
    for (int ang = desde; ang <= hasta; ang += paso) {
      uint16_t pwm_val = map(ang, 0, 180, SERVO_MIN, SERVO_MAX);
      pwm.setPWM(canal, 0, pwm_val);
      delay(retardo);
    }
  } else {
    for (int ang = desde; ang >= hasta; ang -= paso) {
      uint16_t pwm_val = map(ang, 0, 180, SERVO_MIN, SERVO_MAX);
      pwm.setPWM(canal, 0, pwm_val);
      delay(retardo);
    }
  }
  ultimo_angulo[canal] = hasta;
}

void loop() {
  if (Serial.available()) {
    String entrada = Serial.readStringUntil('\n');
    entrada.trim();

    int comaIndex = entrada.indexOf(',');
    if (comaIndex > 0) {
      int canal = entrada.substring(0, comaIndex).toInt();
      int angulo = entrada.substring(comaIndex + 1).toInt();

      if (canal >= 0 && canal <= 15 && angulo >= 0 && angulo <= 180) {
        moverSuave(canal, ultimo_angulo[canal], angulo);
        Serial.print("Servo ");
        Serial.print(canal);
        Serial.print(" → ");
        Serial.print(angulo);
        Serial.println("°");
      } else {
        Serial.println("⚠️ Valores fuera de rango.");
      }
    } else {
      Serial.println("Formato incorrecto. Usa: <canal>,<angulo>");
    }
  }
}
