#include <Wire.h>

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

void setup() {
  Serial.begin(9600);
  Wire.begin();

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

  Serial.println("BMI160 inicializado y calibrado");
}

void loop() {
  Vector3 accel, gyro;
  leerIMU(accel, gyro);

  // Serial.print("Accel (m/s^2): ");
  // Serial.print(accel.x, 2); Serial.print(", ");
  // Serial.print(accel.y, 2); Serial.print(", ");
  // Serial.println(accel.z, 2);

  Serial.print("Gyro (rad/s): ");
  Serial.print(gyro.x, 3); Serial.print(", ");
  Serial.print(gyro.y, 3); Serial.print(", ");
  Serial.println(gyro.z, 3);

  delay(50);
}

// 🔹 Función para leer acelerómetro y giroscopio
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

// 🔹 Configurar rango del giroscopio y actualizar sensibilidad
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

// 🔹 Calibración automática del acelerómetro
void autoCalibrateAccelerometer() {
  Wire.beginTransmission(BMI160_I2C_ADDRESS);
  Wire.write(0x7E);
  Wire.write(0x37); // Comando de auto-calibración
  Wire.endTransmission();
  delay(1000);
  Serial.println("Calibración automática del acelerómetro completada");
}
