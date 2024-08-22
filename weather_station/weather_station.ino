#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <ctime>

#define MQTT_SERVER "XX"
#define MQTT_PORT 1883

#define WIFI_SSID "XX"
#define WIFI_PASSWORD "XX"

#define DHTPIN 4
#define PHOTOPIN 5

//Time sync server
const char* ntpServer = "pool.ntp.org";
// UTC+1 offset 
const long gmtOffset_sec = 3600;
// Daylight saving
const int daylightOffset_sec = 3600;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
DHT dht(DHTPIN, DHT22);

void setupWiFi();
void reconnectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);
  while(!Serial) continue;
  
  dht.begin();
  setupWiFi();
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.print("Waiting for NTP time sync");
  while (time(nullptr) < 1609459200) { // Check against January 1, 2021
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nTime has been synchronized!");

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  
  int lightLevel = analogRead(PHOTOPIN);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(1000);
    return;  
  }

  time_t timestamp;
  time(&timestamp);

  StaticJsonDocument<256> jsonDoc; 
  char output[256];

  jsonDoc["date"] = ctime(&timestamp);
  jsonDoc["Temperature (°C)"] = temperature;
  jsonDoc["Humidity (%)"] = humidity;
  jsonDoc["Light level"] = lightLevel;
  
  serializeJsonPretty(jsonDoc, output);
  mqttClient.publish("/test/message", output);

  mqttClient.loop();
  sleep(10);
}

void setupWiFi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  int cpt = 0;
  while (!mqttClient.connected()) {
    if(cpt == 3){
      Serial.println("Connection failed. Fixe server issue and reboot device");
      sleep(3600);
    }
    Serial.println("Connecting to MQTT Broker...");

    String clientId = "ESP32Client-1";

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("Connected to MQTT Broker");
      mqttClient.publish("/test/message", "hello world");
      mqttClient.subscribe("/test/message");
    } else {
      Serial.print("Failed to connect, rc=");
      Serial.print(mqttClient.state());
      Serial.println(". Retrying in 5 seconds...");
      cpt++;
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
