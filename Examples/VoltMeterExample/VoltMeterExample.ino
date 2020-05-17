/* Volt Meter EXAMPLE - Joshua.Morrisett */
const int ANALOG_PIN = A0;

int offset = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  int volt = analogRead(A0); //read the analog pin's input
//  volt += -9;
  Serial.print("volt: ");
  Serial.print(volt);
  Serial.println("V(x100)");
  double voltage = map(volt,9,1023,0,1640) + offset;
//  double voltage = map(volt,0,1023,0,1650) + offset;

  voltage /=100;
  Serial.print("Voltage: ");
  Serial.print(voltage);
  Serial.println("V");

  delay(1000);
}
