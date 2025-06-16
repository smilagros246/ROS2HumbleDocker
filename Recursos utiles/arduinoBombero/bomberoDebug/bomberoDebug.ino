/**
 * @file bomberoDebug.cpp
 * @author Sofía Milagros Castaño Vanegas 
 * @date 2025-05-15
 * @version 1.0
 * @brief Código para comunicacion serial entre raspberry y arduino para control de robot bombero uao usando ros2.
 */

#include <Wire.h>
#include <Servo.h>
#include <SparkFun_TB6612.h>
#include <Adafruit_PWMServoDriver.h>

// Dirección del BMI160 IMU 
#define BMI160_I2C_ADDRESS 0x68
#define ACCEL_SENSITIVITY 16384.0   // ±2g (LSB/g)
#define DEG_TO_RAD 0.0174533        // Conversión de grados a radianes

// Sensibilidad del giroscopio según rango, se actualiza dinámicamente
float GYRO_SENSITIVITY = 16.4;      // Default: ±2000 °/s → 16.4 LSB/(°/s)

struct Vector3 {
  float x;
  float y;
  float z;
};

// Pines sensores sharp
const int irPins[3] = {A0, A1, A6};

// Inicializa el driver PCA9685 (con dirección 0x40 por defecto)
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// Definir pines de puentes H
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

// Rango de pulsos en "ticks"
const uint8_t SERVO_FREQ = 50;
const uint16_t SERVO_MIN = 102;
const uint16_t SERVO_MAX = 512;
int currentAngles[4] = {90, 90, 90, 90};  // posiciones iniciales

#define pinBomba 9
const float voltajeFuente = 11.0; // Voltaje que alimenta a la bomba

// Buffers para comunicación serial
const byte TX_HEADER = 'A';
const byte TX_END = '#';
const byte RX_MAX_LEN = 64;
byte rxBuffer[RX_MAX_LEN];
int rxIndex = 0;

// Frecuencia de actualización
const unsigned long SEND_INTERVAL_MS = 50;
unsigned long lastSendTime = 0;

const byte CMD_LEN = 36; // Tamaño esperado comando: 1 modo + 4*4 posiciones + 4*4 velocidades + 1 bomba + 1 checksum + 1 fin

void moveSmooth(int newAngles[4], int steps = 100, int delayTime = 200);
void startSmoothMove(int newAngles[4], int steps =30, int delayTime =20);

struct SmoothMoveState {
  int startAngles[4];
  int targetAngles[4];
  int steps;
  int currentStep;
  unsigned long lastUpdate;
  int delayTime;
  bool active;
};

SmoothMoveState smoothMove;

void setup() {
  
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);
  analogWrite(pinBomba, 0);
  Wire.begin();
  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQ);  // Establece la frecuencia de los servos
  delay(10); 

  // Inicializar acelerómetro en modo normal
  Wire.beginTransmission(BMI160_I2C_ADDRESS);
  Wire.write(0x7E);
  Wire.write(0x11);
  Wire.endTransmission();
  delay(100);

  // Inicializar giroscopio en modo normal
  Wire.beginTransmission(BMI160_I2C_ADDRESS);
  Wire.write(0x7E);
  Wire.write(0x15);
  Wire.endTransmission();
  delay(100);
  
  // Configurar rango del giroscopio a ±500 °/s (puedes cambiar a 125, 250, 1000 o 2000)
  configurarGiroscopio(500);

  autoCalibrateAccelerometer();
  
  analogWrite(pinBomba, 0);


  // Configura los servos en su posición inicial
  moveSmooth(currentAngles, 300, 200);  // 30 pasos, 15 ms por paso
  
}

void loop() {
  readSerial();
  updateSmoothMove();  // movimiento no bloqueante

  if (millis() - lastSendTime >= SEND_INTERVAL_MS) {
    lastSendTime = millis();
    sendSensorData();
  }
}

void sendSensorData() {
  uint16_t irValues[3];
  for (int i = 0; i < 3; i++) {
    irValues[i] = analogRead(irPins[i]);
  }

  // Leer datos crudos de IMU
  Vector3 accel, gyro;
  leerIMU(accel, gyro);
  
  // Enviar paquete binario
  Serial.write(TX_HEADER);

  // Enviar IR como uint16_t (6 bytes)
  for (int i = 0; i < 3; i++) {
    sendUInt16(irValues[i]);
  }

  // Acelerómetro en miligravedad (mg)
  int16_t accelX_mg = accel.x * 1000;
  int16_t accelY_mg = accel.y * 1000;
  int16_t accelZ_mg = accel.z * 1000;

  // Giroscopio en milliradianes por segundo (mrad/s)
  int16_t gyroX_mrad = gyro.x * 1000;
  int16_t gyroY_mrad = gyro.y * 1000;
  int16_t gyroZ_mrad = gyro.z * 1000;

  // Enviar IMU como int16_t (12 bytes)
  sendInt16(accelX_mg);
  sendInt16(accelY_mg);
  sendInt16(accelZ_mg);
  sendInt16(gyroX_mrad);
  sendInt16(gyroY_mrad);
  sendInt16(gyroZ_mrad);
  // Checksum (simple suma de bytes)
  uint8_t checksum = 0;
  checksum += byteSum(irValues, 3);
  checksum += int16Sum(accelX_mg,accelY_mg,accelZ_mg,gyroX_mrad,gyroY_mrad,gyroZ_mrad);
  Serial.write(checksum);

  // Fin del mensaje
  Serial.write(TX_END);

  // -------- LOGS en texto para depuración ----------
  
  // Serial.print("IR: ");
  // for (int i = 0; i < 3; i++) {
  //   Serial.print(irValues[i]);
  //   Serial.print(" ");
  // }
  // Serial.print("Accel (m/s^2): ");
  // Serial.print(accel.x, 2); Serial.print(", ");
  // Serial.print(accel.y, 2); Serial.print(", ");
  // Serial.println(accel.z, 2);

  // Serial.print("Gyro (rad/s): ");
  // Serial.print(gyro.x, 3); Serial.print(", ");
  // Serial.print(gyro.y, 3); Serial.print(", ");
  // Serial.println(gyro.z, 3);
}

void sendUInt16(uint16_t val) {
  Serial.write(lowByte(val));   // primero el byte bajo
  Serial.write(highByte(val));  // luego el alto
}

void sendInt16(int16_t val) {
  Serial.write(lowByte(val));
  Serial.write(highByte(val));
}

uint8_t byteSum(uint16_t vals[], int len) {
  uint8_t sum = 0;
  for (int i = 0; i < len; i++) {
    sum += lowByte(vals[i]) + highByte(vals[i]);
  }
  return sum;
}


uint8_t int16Sum(int16_t ax, int16_t ay, int16_t az, int16_t gx, int16_t gy, int16_t gz) {
  int16_t arr[] = {ax, ay, az, gx, gy, gz};
  uint8_t sum = 0;
  for (int i = 0; i < 6; i++) {
    sum += highByte(arr[i]) + lowByte(arr[i]);
  }
  return sum;
}

void readSerial() {
  while (Serial.available()) {
    byte incoming = Serial.read();

    // Ignorar paquetes sensores que comienzan con 'A'
    if (rxIndex == 0 && incoming == 'A') {
      // Si quieres procesar paquete sensores recibido, hazlo aquí
      // Por ahora, solo descartar todo hasta '#'
      rxBuffer[rxIndex++] = incoming;
    } else {
      if (rxIndex < RX_MAX_LEN) {
        rxBuffer[rxIndex++] = incoming;

        if (incoming == '#') {
          if (rxIndex == CMD_LEN) {
            processCommand(rxBuffer, rxIndex);
          } else {
            Serial.println("Error: trama comando tamaño incorrecto");
          }
          rxIndex = 0;
        }
      } else {
        // Buffer overflow
        rxIndex = 0;
      }
    }
  }
}

void processCommand(byte* buffer, int len) {
  // buffer[0]: modo ('S' o 'W')
  // buffer[1..16]: 4 floats posiciones
  // buffer[17..32]: 4 floats velocidades
  // buffer[33]: bomba
  // buffer[34]: checksum
  // buffer[35]: fin '#'

  // Validar checksum
  uint8_t checksum = 0;
  for (int i = 1; i < len - 2; ++i) {
    checksum += buffer[i];
  }
  if (checksum != buffer[len - 2]) {
    Serial.println("Checksum invalido");
    return;
  }

  char modo = (char)buffer[0];

  float posiciones[4];
  memcpy(posiciones, buffer + 1, 4 * sizeof(float));  // radianes

  float velocidades[4];
  memcpy(velocidades, buffer + 1 + 4 * sizeof(float), 4 * sizeof(float));

  uint8_t bomba = buffer[1 + 4 * sizeof(float) + 4 * sizeof(float)];

  // Comandos
  // Aplicar velocidades a motores
  for (int i = 0; i < 4; i++) {
  velocidades[i] = rad_s_to_pwm(velocidades[i]);  
  }
  if (velocidades[0] == 0) motor1.brake(); else motor1.drive(velocidades[0]);
  if (velocidades[1] == 0) motor2.brake(); else motor2.drive(velocidades[1]);
  if (velocidades[2] == 0) motor3.brake(); else motor3.drive(velocidades[2]);
  if (velocidades[3] == 0) motor4.brake(); else motor4.drive(velocidades[3]);

  //Aplicar movimeintos de servos
  // Convertir de radianes a grados limitados entre 0 y 180
  int grados[4];
  for (int i = 0; i < 4; i++) {
    float deg = posiciones[i] * 180.0 / PI;
    grados[i] = constrain((int)round(deg), 0, 180);
  }
  // Aplicar movimiento suave
  // moveSmooth(grados);
  startSmoothMove(grados);


  int valorPWM = 0;
  float voltajeAplicado = 0;
  if (bomba == 1) {
    // Serial.println("Bomba en baja potencia");
    valorPWM = 64; // 25% de 255
  } else if (bomba == 2) {
    // Serial.println("Bomba en media potencia");
    valorPWM = 128; // 50% de 255
  } else if (bomba == 3) {
    // Serial.println("Bomba en alta potencia");
    valorPWM = 230; // 90% de 255
  } else {
    // Serial.println("Bomba apagada");
    valorPWM = 0;
  }
  analogWrite(pinBomba, valorPWM);

  // Serial.print("Modo: ");
  // Serial.println(modo);
  // for (int i = 0; i < 4; i++) {
  //   Serial.print("Posicion ");
  //   Serial.print(i);
  //   Serial.print(": ");
  //   Serial.println(grados[i], 3);
  // }
  // for (int i = 0; i < 4; i++) {
  //   Serial.print("Velocidad ");
  //   Serial.print(i);
  //   Serial.print(": ");
  //   Serial.println(velocidades[i], 3);
  // }
  // Serial.print("Bomba: ");
  // Serial.println(bomba);

  if (velocidades[0] == 255) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else if (valorPWM != 0){
    digitalWrite(LED_BUILTIN, HIGH);
  } else if (grados[0]!=90) {
    digitalWrite(LED_BUILTIN, HIGH);  } else {
    // Serial.println("Modo desconocido");
    digitalWrite(LED_BUILTIN, LOW);
  }
}

int rad_s_to_pwm(float vel_rad_s) {
  const float max_rad_s = 9.0; // ajustá según tu motor
  const int max_pwm = 255;
  vel_rad_s = constrain(vel_rad_s, -max_rad_s, max_rad_s);
  return (int)(vel_rad_s / max_rad_s * max_pwm);
}

void moveSmooth(int newAngles[4], int steps = 100, int delayTime = 200) {
  for (int s = 1; s <= steps; s++) {
    for (int i = 0; i < 4; i++) {
      float interpolated = currentAngles[i] + (newAngles[i] - currentAngles[i]) * (float(s) / steps);
      uint16_t pwm_val = map(interpolated, 0, 180, SERVO_MIN, SERVO_MAX);
      pwm.setPWM(i + 12, 0, pwm_val);  
    }
    delay(delayTime);
  }

  for (int i = 0; i < 4; i++) {
    currentAngles[i] = newAngles[i];  // actualiza posición actual
    // Serial.print("Servo ");
    // Serial.print(i);
    // Serial.print(": ");
    // Serial.print(currentAngles[i], 1);
    // Serial.println("°");
  }
}
void startSmoothMove(int newAngles[4], int steps =30, int delayTime = 20) {
  for (int i = 0; i < 4; i++) {
    smoothMove.startAngles[i] = currentAngles[i];
    smoothMove.targetAngles[i] = newAngles[i];
  }
  smoothMove.steps = steps;
  smoothMove.currentStep = 0;
  smoothMove.delayTime = delayTime;
  smoothMove.lastUpdate = millis();
  smoothMove.active = true;
}

void updateSmoothMove() {
  if (!smoothMove.active) return;

  if (millis() - smoothMove.lastUpdate >= smoothMove.delayTime) {
    smoothMove.lastUpdate = millis();
    smoothMove.currentStep++;

    if (smoothMove.currentStep > smoothMove.steps) {
      smoothMove.active = false;
      return;
    }

    for (int i = 0; i < 4; i++) {
      float interpolated = smoothMove.startAngles[i] + 
        (smoothMove.targetAngles[i] - smoothMove.startAngles[i]) * 
        ((float)smoothMove.currentStep / smoothMove.steps);

      uint16_t pwm_val = map(interpolated, 0, 180, SERVO_MIN, SERVO_MAX);
      pwm.setPWM(i + 12, 0, pwm_val);
      currentAngles[i] = interpolated;  // actualizar para referencia futura
    }
  }
}


void leerIMU(Vector3 &accel, Vector3 &gyro) {
  int16_t ax, ay, az;
  int16_t gx, gy_, gz;

  // Leer acelerómetro
  Wire.beginTransmission(BMI160_I2C_ADDRESS);
  Wire.write(0x12); // Dirección de datos de acelerómetro
  Wire.endTransmission(false);
  Wire.requestFrom(BMI160_I2C_ADDRESS, 6);
  if (Wire.available() == 6) {
    ax = Wire.read() | (Wire.read() << 8);
    ay = Wire.read() | (Wire.read() << 8);
    az = Wire.read() | (Wire.read() << 8);
  }

  // Leer giroscopio
  Wire.beginTransmission(BMI160_I2C_ADDRESS);
  Wire.write(0x0C); // Dirección de datos de giroscopio
  Wire.endTransmission(false);
  Wire.requestFrom(BMI160_I2C_ADDRESS, 6);
  if (Wire.available() == 6) {
    gx  = Wire.read() | (Wire.read() << 8);
    gy_ = Wire.read() | (Wire.read() << 8);
    gz  = Wire.read() | (Wire.read() << 8);
  }

  // Convertir a unidades físicas
  accel.x = ax * (9.81 / ACCEL_SENSITIVITY);
  accel.y = ay * (9.81 / ACCEL_SENSITIVITY);
  accel.z = az * (9.81 / ACCEL_SENSITIVITY);

  gyro.x = (gx / GYRO_SENSITIVITY) * DEG_TO_RAD;
  gyro.y = (gy_ / GYRO_SENSITIVITY) * DEG_TO_RAD;
  gyro.z = (gz / GYRO_SENSITIVITY) * DEG_TO_RAD;
}

// Configurar rango del giroscopio y actualizar sensibilidad
void configurarGiroscopio(int rango_dps) {
  uint8_t valor_registro = 0x00;

  switch (rango_dps) {
    case 125:
      valor_registro = 0x04;
      GYRO_SENSITIVITY = 262.4;
      break;
    case 250:
      valor_registro = 0x03;
      GYRO_SENSITIVITY = 131.2;
      break;
    case 500:
      valor_registro = 0x02;
      GYRO_SENSITIVITY = 65.6;
      break;
    case 1000:
      valor_registro = 0x01;
      GYRO_SENSITIVITY = 32.8;
      break;
    case 2000:
    default:
      valor_registro = 0x00;
      GYRO_SENSITIVITY = 16.4;
      break;
  }

  // Escribir en el registro GYR_RANGE (0x43)
  Wire.beginTransmission(BMI160_I2C_ADDRESS);
  Wire.write(0x43);            // Dirección del registro GYR_RANGE
  Wire.write(valor_registro); // Valor correspondiente al rango
  Wire.endTransmission();
  delay(50);

  Serial.print("Rango de giroscopio configurado a ±");
  Serial.print(rango_dps);
  Serial.println(" °/s");
}

// Calibración automática del acelerómetro
void autoCalibrateAccelerometer() {
  Wire.beginTransmission(BMI160_I2C_ADDRESS);
  Wire.write(0x7E);
  Wire.write(0x37); // Comando de auto-calibración
  Wire.endTransmission();
  delay(1000);
  Serial.println("Calibración automática del acelerómetro completada");
}
