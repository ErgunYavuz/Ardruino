#include <DHT.h>

#define DHTPIN 4
#define PHOTOPIN 2


DHT dht(DHTPIN, DHT11);

void setup() {
  // put your setup code here, to run once:
  dht.begin();
  delay(2000);

  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  getDHT();
  getPhotoSensor();
  delay(10000); 
}

void getDHT(){
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temp) || isnan(humidity)) {
    Serial.println(F("Failed to read from DHT sensor!"));
  return;
  }

  Serial.print("Temp : ");
  Serial.print(temp);
  Serial.print(" C, ");
   
  Serial.print("Humidity : ");
  Serial.print(humidity);
  Serial.print(" %");
  Serial.println();
}

void getPhotoSensor(){
  int value = analogRead(PHOTOPIN);
  Serial.print("light value : ");
  Serial.println(value);
}
