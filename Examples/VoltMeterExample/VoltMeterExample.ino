/* Volt Meter EXAMPLE - Joshua.Morrisett */
const int ANALOG_PIN = A0;

int offset = -22;

void setup() {
  Serial.begin(9600);
}

void loop() {
  int volt = analogRead(A0); //read the analog pin's input
  double voltage = map(volt,0.00489,1023,0,2500) + offset;

  voltage /=100;
  Serial.print("Voltage: ");
  Serial.print(voltage);
  Serial.println("V");

  delay(1000);
}
