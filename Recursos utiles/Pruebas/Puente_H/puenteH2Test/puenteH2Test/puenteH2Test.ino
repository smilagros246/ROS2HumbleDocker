#include <SparkFun_TB6612.h>

// Pines Motor A (driver 1)
#define A1_1 A3
#define A1_2 A2
#define PWM_A 3

// Pines Motor B (driver 1)
#define B1_1 2
#define B1_2 4
#define PWM_B 5

// Pines Motor C (driver 2)
#define A2_1 8
#define A2_2 7
#define PWM_C 6

// Pines Motor D (driver 2)
#define B2_1 10
#define B2_2 12
#define PWM_D 11

// Pin STBY compartido
#define STBY A7

// Offsets (ajustar si algún motor gira al revés)
const int offset1A = 1;
const int offset1B = 1;
const int offset2A = 1;
const int offset2B = 1;

// Crear motores
Motor motor1 = Motor(A1_1, A1_2, PWM_A, offset1A, STBY); // M1
Motor motor2 = Motor(B1_1, B1_2, PWM_B, offset1B, STBY); // M2
Motor motor3 = Motor(A2_1, A2_2, PWM_C, offset2A, STBY); // M3
Motor motor4 = Motor(B2_1, B2_2, PWM_D, offset2B, STBY); // M4

void setup() {
  pinMode(STBY, OUTPUT);
  pinMode(A1_1, OUTPUT);
  pinMode(A1_2, OUTPUT);
  digitalWrite(STBY, HIGH);  // Habilita el driver
}

void loop() {
  // Movimiento adelante individual
  motor1.drive(200, 1000);
  motor2.drive(200, 1000);
  motor3.drive(200, 1000);
  motor4.drive(200, 1000);
  brakeAll();
  // Reversa
  motor1.drive(-200, 1000);
  motor2.drive(-200, 1000);
  motor3.drive(-200, 1000);
  motor4.drive(-200, 1000);
  brakeAll();
  // Frenar todos
  motor1.brake();
  motor2.brake();
  motor3.brake();
  motor4.brake();
  delay(1000);
  brakeAll();
  // Girar izquierda (ejemplo)
  motor1.drive(-200, 1000);
  motor2.drive(-200, 1000);
  motor3.drive(200, 1000);
  motor4.drive(200, 1000);
  brakeAll();
  // Girar derecha (ejemplo)
  motor1.drive(200, 1000);
  motor2.drive(200, 1000);
  motor3.drive(-200, 1000);
  motor4.drive(-200, 1000);
  brakeAll();

  // Probar motor1 y motor2
  motor1.drive(200);
  motor2.drive(200);
  delay(2000);
  motor1.brake();
  motor2.brake();
  delay(1000);

  // Luego motor3 y motor4
  motor3.drive(200);
  motor4.drive(200);
  delay(2000);
  motor3.brake();
  motor4.brake();
  delay(1000);

  brakeAll();
  delay(1000);
}

// Función para frenar todos los motores
void brakeAll() {
  motor1.brake();
  motor2.brake();
  motor3.brake();
  motor4.brake();
}
