const int sensorPin = A0;

int sensorValue = 0;  // value read from the sensor
int minSensorValue = 670;  // sensor value at minimum distance (10 cm)
int maxSensorValue = 87;   // sensor value at maximum distance (80 cm) //SE DEBE COMPROBAR

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
}

void loop() {
  // read the analog sensor value:
  sensorValue = analogRead(sensorPin);

  // map the sensor value to the corresponding distance in centimeters:
  int distance = map(sensorValue, minSensorValue, maxSensorValue, 10, 80);

  // print the results to the Serial Monitor:
  Serial.print("sensor value = ");
  Serial.print(sensorValue);
  Serial.print(", distance = ");
  Serial.print(distance);
  Serial.println(" cm");

  // wait 2 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:
  delay(2);
}
