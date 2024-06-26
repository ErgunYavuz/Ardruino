#define SENSORPIN 2

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println("value: ");
  Serial.println(analogRead(SENSORPIN));
  delay(2000);
}
