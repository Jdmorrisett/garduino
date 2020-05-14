/* RELAY EXAMPLE - Joshua.Morrisett */
 
#if defined(ESP8266)
const int SDA_PIN = D2;
const int SDC_PIN = D1;
const int RELAY_PIN_1 = D5; //GPIO 14 (SCLK)
const int RELAY_PIN_2 = D6; //GPIO 12 (MISO)
const int RELAY_PIN_3 = D7; //GPIO 13 (MOSI)
const int RELAY_PIN_4 = D8; //GPIO 15 (CS)
#else
const int SDA_PIN = 4; //D2;
const int SDC_PIN = 5; //D1;
const int RELAY_PIN_1 = 14; //D5
const int RELAY_PIN_2 = 12;  //D6
const int RELAY_PIN_3 = 13; //D7
const int RELAY_PIN_4 = 15 //D8
#endif

void setup() {
  pinMode(RELAY_PIN_1, OUTPUT);  // initialize RELEAY_PIN_1 as OUTPUT
  pinMode(RELAY_PIN_2, OUTPUT);  // initialize RELEAY_PIN_2 as OUTPUT
  pinMode(RELAY_PIN_3, OUTPUT);  // initialize RELEAY_PIN_3 as OUTPUT
  pinMode(RELAY_PIN_4, OUTPUT);  // initialize RELEAY_PIN_4 as OUTPUT
}

void loop() {
  digitalWrite(RELAY_PIN_1, HIGH);  // turn on RELAY with voltage HIGH
  delay(1000);                      // wait one second
  digitalWrite(RELAY_PIN_1, LOW);   // turn off RELAY with voltage LOW
  delay(1000);                      // wait one 

  //RELAY_PIN_2
  digitalWrite(RELAY_PIN_2, HIGH);  // turn on RELAY with voltage HIGH
  delay(1000);                      // wait one second
  digitalWrite(RELAY_PIN_2, LOW);   // turn off RELAY with voltage LOW
  delay(1000);                      // wait one second

  //RELAY_PIN_3
  digitalWrite(RELAY_PIN_3, HIGH);  // turn on RELAY with voltage HIGH
  delay(1000);                      // wait one second
  digitalWrite(RELAY_PIN_3, LOW);   // turn off RELAY with voltage LOW
  delay(1000);                      // wait one second

  //RELAY_PIN_4
  digitalWrite(RELAY_PIN_4, HIGH);  // turn on RELAY with voltage HIGH
  delay(1000);                      // wait one second
  digitalWrite(RELAY_PIN_4, LOW);   // turn off RELAY with voltage LOW
  delay(1000);                      // wait one second
}
