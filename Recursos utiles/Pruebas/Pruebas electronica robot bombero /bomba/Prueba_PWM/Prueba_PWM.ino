const int pwmPin = 9; // Pin PWM de salida
const float voltajeFuente = 11.0; // Voltaje que alimenta al motor

void setup() {
  Serial.begin(9600);
  pinMode(pwmPin, OUTPUT);
  mostrarMenu();
}

void loop() {
  if (Serial.available() > 0) {
    char opcion = Serial.read();
    opcion = toupper(opcion); // Convierte a mayúscula

    int valorPWM = 0;
    float voltajeAplicado = 0;

    switch(opcion) {
      case 'B':
        valorPWM = 64; // 25% de 255
        Serial.println(">> Modo BAJO seleccionado");
        break;
      case 'M':
        valorPWM = 128; // 50% de 255
        Serial.println(">> Modo MEDIO seleccionado");
        break;
      case 'A':
        valorPWM = 230; // 90% de 255
        Serial.println(">> Modo ALTO seleccionado");
        break;
      case 'S':
        valorPWM = 0;
        Serial.println(">> Motor detenido");
        break;
      default:
        Serial.println(">> Opción inválida. Intente nuevamente.");
        mostrarMenu();
        return;
    }

    analogWrite(pwmPin, valorPWM);

    // Mostrar voltaje estimado solo si el motor no está detenido
    if (valorPWM > 0) {
      voltajeAplicado = (valorPWM / 255.0) * voltajeFuente;
      Serial.print(">> Voltaje estimado aplicado al motor: ");
      Serial.print(voltajeAplicado, 2);
      Serial.println(" V");
    }

    mostrarMenu(); // Se vuelve a mostrar el menú al final
  }
}

void mostrarMenu() {
  Serial.println("\n=== Seleccione un modo de operación ===");
  Serial.println("B - Velocidad BAJA");
  Serial.println("M - Velocidad MEDIA");
  Serial.println("A - Velocidad ALTA");
  Serial.println("S - DETENER el motor");
  Serial.println("======================================");
}


