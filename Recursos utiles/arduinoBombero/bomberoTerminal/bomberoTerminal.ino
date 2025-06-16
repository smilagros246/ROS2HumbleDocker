/**
 * @file bomberoTerminal.cpp
 * @author Sofía Milagros Castaño Vanegas 
 * @date 2025-05-15
 * @version 1.0
 * @brief Código para comunicacion serial entre raspberry y arduino para control de robot bombero uao desde teclado.
 */
#include <Servo.h>
#include <Wire.h> 
#include <SparkFun_TB6612.h>
#include <Adafruit_PWMServoDriver.h>
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
float ultimo_angulo[16];  // Guarda el último ángulo de cada canal

int wheels[4] = {0, 0, 0, 0};   // Valores para las ruedas
int currentAngles[4] = {90, 90, 90, 90};  // posiciones iniciales
int servos[4];  // objetivo de los servos
String pumpLevel = "APAGADO";    // Nivel de la bomba

#define pinPrueba 13
// Definición para PWM bomba
#define pinBomba 9
const float voltajeFuente = 13.0; // Voltaje que alimenta a la bomba

// Pines de los sensores IR
int sharpPins[3] = {A0, A1, A4};  // Pines analógicos para los sensores IR

// Dirección del BMI160 y otros parámetros
#define BMI160_I2C_ADDRESS 0x68
#define ACCEL_SENSITIVITY 16384.0 // ±2g
#define GYRO_SENSITIVITY 131.0    // ±250°/s

void setup() {
  Serial.begin(115200);  // Establece la velocidad del puerto serial
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);
  
  // Inicializa I2C
  Wire.begin();          // Inicializa la comunicación I2C
  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQ);  // Establece la frecuencia de los servos
  delay(10);  
  
  // Configura los servos en su posición inicial
  for (uint8_t i = 0; i < 4; i++) {
    uint16_t pwm_val = map(servos[i], 0, 180, SERVO_MIN, SERVO_MAX);
    pwm.setPWM(i+12, 0, pwm_val);  // Canal i, desde 0 hasta 4
  }
  // Inicializa el IMU (BMI160)
  initBMI160();
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');  // Lee la línea completa del comando
    processCommand(command);  // Procesa el comando recibido
  }

  // Enviar datos de los sensores a Python
  enviarDatosSensores();
  delay(100);  // Agregar un pequeño retraso para evitar saturar la comunicación serial
}


void processCommand(String command) {
  // Extraer datos de las ruedas
  int wheelsStart = command.indexOf("WHEELS:") + 7;
  int wheelsEnd = command.indexOf(";", wheelsStart);
  String wheelsData = command.substring(wheelsStart, wheelsEnd);
  parseWheels(wheelsData);

  // Extraer datos de los servos
  int servosStart = command.indexOf("SERVOS:") + 7;
  int servosEnd = command.indexOf(";", servosStart);
  String servosData = command.substring(servosStart, servosEnd);
  parseServos(servosData);

  // Extraer estado de la bomba
  int pumpStart = command.indexOf("PUMP:") + 5;
  String pumpData = command.substring(pumpStart);
  pumpLevel = pumpData;
  controlPump();
}

void moveSmooth(int newAngles[4], int steps = 30, int delayTime = 20) {
  for (int s = 1; s <= steps; s++) {
    for (int i = 0; i < 4; i++) {
      float interpolated = currentAngles[i] + (newAngles[i] - currentAngles[i]) * (float(s) / steps);
      uint16_t pwm_val = map(interpolated, 0, 180, SERVO_MIN, SERVO_MAX);
      pwm.setPWM(i + 12, 0, pwm_val);  // ⚠️ Nota el +12 si usas canales 12 a 15
    }
    delay(delayTime);
  }

  for (int i = 0; i < 4; i++) {
    currentAngles[i] = newAngles[i];  // actualiza posición actual
    Serial.print("Servo ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(currentAngles[i], 1);
    Serial.println("°");
  }
}



void parseWheels(String wheelsData) {
  int i = 0;
  int start = 0;
  int end = wheelsData.indexOf(",");
  while (end != -1) {
    int vel = wheelsData.substring(start, end).toInt();
    wheels[i] = vel;
    start = end + 1;
    end = wheelsData.indexOf(",", start);
    i++;
  }
  int vel = wheelsData.substring(start).toInt();
  wheels[i] = vel;

  if (wheels[1] == - 200){
    digitalWrite(LED_BUILTIN, HIGH);
  }else{
    digitalWrite(LED_BUILTIN, LOW);
  }
  
  // Imprime los valores de las ruedas (para depuración)
  for (int i = 0; i < 4; i++) {
    Serial.print("Wheel ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(wheels[i]);
  }
  // Aplicar velocidades directamente
  if (wheels[0] == 0) motor1.brake(); else motor1.drive(wheels[0]);
  if (wheels[1] == 0) motor2.brake(); else motor2.drive(wheels[1]);
  if (wheels[2] == 0) motor3.brake(); else motor3.drive(wheels[2]);
  if (wheels[3] == 0) motor4.brake(); else motor4.drive(wheels[3]);
}


void parseServos(String servosData) {
  int i = 0;
  int start = 0;
  int end = servosData.indexOf(",");

  while (end != -1 && i < 4) {
    servos[i] = servosData.substring(start, end).toFloat();
    start = end + 1;
    end = servosData.indexOf(",", start);
    i++;
  }
  if (i < 4) servos[i] = servosData.substring(start).toFloat();  // último valor

  Serial.println("Moviendo servos suavemente:");
  moveSmooth(servos);  
}


void controlPump() {
  int valorPWM = 0;
  float voltajeAplicado = 0;
  if (pumpLevel == "PWM_BAJO") {
    Serial.println("Bomba en baja potencia");
    valorPWM = 64; // 25% de 255
  } else if (pumpLevel == "PWM_MEDIO") {
    Serial.println("Bomba en media potencia");
    valorPWM = 128; // 50% de 255
  } else if (pumpLevel == "PWM_ALTO") {
    Serial.println("Bomba en alta potencia");
    valorPWM = 230; // 90% de 255
  } else {
    Serial.println("Bomba apagada");
    valorPWM = 0;
  }

  analogWrite(pinBomba, valorPWM);

  // Mostrar voltaje estimado solo si el motor no está detenido
    // if (valorPWM > 0) {
    //   voltajeAplicado = (valorPWM / 255.0) * voltajeFuente;
    //   Serial.print(">> Voltaje estimado aplicado al motor: ");
    //   Serial.print(voltajeAplicado, 2);
    //   Serial.println(" V");
    // }
}

// === ENVÍO DE DATOS DE SENSORES ===
void enviarDatosSensores() {
  // Leer los valores de los sensores IR
  String irData = "IR:";
  for (int i = 0; i < 3; i++) {
    irData += String(analogRead(sharpPins[i]));
    if (i < 2) irData += ",";
  }
  irData += "\n";
  Serial.print(irData);  // Enviar los datos IR
  
  // Leer y enviar los datos del IMU
  float ax, ay, az;
  int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
  leerIMU(ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw);

  // Convertir a valores legibles
  ax = ax_raw * (9.81 / ACCEL_SENSITIVITY);
  ay = ay_raw * (9.81 / ACCEL_SENSITIVITY);
  az = az_raw * (9.81 / ACCEL_SENSITIVITY);

  float gx = gx_raw / GYRO_SENSITIVITY;
  float gy = gy_raw / GYRO_SENSITIVITY;
  float gz = gz_raw / GYRO_SENSITIVITY;

  // Enviar los datos del IMU
  String imuData = "IMU:" + String(gx, 2) + "," + String(gy, 2) + "," + String(gz, 2) + "," +String(ax, 2) + "," + String(ay, 2) + "," + String(az, 2)  + "\n";
  Serial.print(imuData);  // Enviar los datos IMU
}

void initBMI160() {
  Wire.beginTransmission(BMI160_I2C_ADDRESS);
  Wire.write(0x7E); // Comando
  Wire.write(0x11); // Modo normal acelerómetro
  Wire.endTransmission();
  delay(50);

  Wire.beginTransmission(BMI160_I2C_ADDRESS);
  Wire.write(0x7E);
  Wire.write(0x15); // Modo normal giroscopio
  Wire.endTransmission();
  delay(50);
}

void leerIMU(int16_t &ax, int16_t &ay, int16_t &az, int16_t &gx, int16_t &gy, int16_t &gz) {
  // Leer acelerómetro y giroscopio
  Wire.beginTransmission(BMI160_I2C_ADDRESS);
  Wire.write(0x12);  // Registro de acelerómetro
  Wire.endTransmission(false);
  Wire.requestFrom(BMI160_I2C_ADDRESS, 12); // 6 accel + 6 gyro

  if (Wire.available() >= 12) {
    ax = Wire.read() | (Wire.read() << 8);
    ay = Wire.read() | (Wire.read() << 8);
    az = Wire.read() | (Wire.read() << 8);
    gx = Wire.read() | (Wire.read() << 8);
    gy = Wire.read() | (Wire.read() << 8);
    gz = Wire.read() | (Wire.read() << 8);
  }
}