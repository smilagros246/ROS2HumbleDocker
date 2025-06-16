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

// Offsets de dirección
const int offset1A = 1;
const int offset1B = 1;
const int offset2A = 1;
const int offset2B = 1;

// Motores
Motor motor1 = Motor(A1_1, A1_2, PWM_A, offset1A, STBY);
Motor motor2 = Motor(B1_1, B1_2, PWM_B, offset1B, STBY);
Motor motor3 = Motor(A2_1, A2_2, PWM_C, offset2A, STBY);
Motor motor4 = Motor(B2_1, B2_2, PWM_D, offset2B, STBY);

// Velocidades individuales
int speeds[4] = {0, 0, 0, 0};

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println(" Arduino con TB6612 listo. Esperando comandos...");

  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    parseAndSetSpeeds(input);
  }
}

void parseAndSetSpeeds(String input) {
  input.trim();
  int lastIndex = 0;

  Serial.print(" Recibido: ");
  Serial.println(input);

  for (int i = 0; i < 4; i++) {
    int index = input.indexOf(',', lastIndex);
    String valStr = (index == -1) ? input.substring(lastIndex) : input.substring(lastIndex, index);
    speeds[i] = valStr.toInt();
    lastIndex = index + 1;
  }

  // Mostrar velocidades
  Serial.print(" Velocidades aplicadas: ");
  for (int i = 0; i < 4; i++) {
    Serial.print("M");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(speeds[i]);
    Serial.print("  ");
  }
  Serial.println();

  // Aplicar velocidades directamente
  if (speeds[0] == 0) motor1.brake(); else motor1.drive(speeds[0]);
  if (speeds[1] == 0) motor2.brake(); else motor2.drive(speeds[1]);
  if (speeds[2] == 0) motor3.brake(); else motor3.drive(speeds[2]);
  if (speeds[3] == 0) motor4.brake(); else motor4.drive(speeds[3]);
}
