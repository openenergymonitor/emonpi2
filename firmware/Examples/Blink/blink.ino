#define Serial Serial3

void setup() {
  pinMode(PIN_PC2, OUTPUT);
  Serial.begin(115200);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(PIN_PC2, HIGH);    // turn the LED on (HIGH is the voltage level)
  delay(100);                     // wait 100 milliseconds
  digitalWrite(PIN_PC2, LOW);     // turn the LED off by making the voltage LOW
  delay(1000);                    // wait for a second
  Serial.println("Hello World");
}
