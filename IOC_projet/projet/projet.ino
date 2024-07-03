#include <WiFi.h>
#include <PubSubClient.h>
#include <WebServer.h>

//Info pour le WIFI
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

//Info MQTT
#define MQTT_SERVER ""
#define MQTT_PORT 1883
WiFiClient espClient;
PubSubClient client(espClient);

#define MAX_MSG_LEN 128
WebServer server(80);

const int led = 2;
bool etatLed = 0;
char texteEtatLed[2][10] = {"ÉTEINTE!","ALLUMÉE!"};


void handleRoot()
{
    String page = "<!DOCTYPE html>";

    page += "<html lang='fr'>";

    page += "<head>";
    page += "    <title>Serveur ESP32</title>";
    page += "    <meta http-equiv='refresh' content='60' name='viewport' content='width=device-width, initial-scale=1' charset='UTF-8' />";
    page += "    <link rel='stylesheet' href='https://www.w3schools.com/w3css/4/w3.css'>";
    page += "</head>";

    page += "<body>";
    page += "    <div class='w3-card w3-blue w3-padding-small w3-jumbo w3-center'>";
    page += "        <p>ÉTAT LED: "; page += texteEtatLed[etatLed]; + "</p>";
    page += "    </div>";

    page += "    <div class='w3-bar'>";
    page += "        <a href='/on' class='w3-bar-item w3-button w3-border w3-jumbo' style='width:50%; height:50%;'>ON</a>";
    page += "        <a href='/off' class='w3-bar-item w3-button w3-border w3-jumbo' style='width:50%; height:50%;'>OFF</a>";
    page += "    </div>";

    page += "    <div class='w3-center w3-padding-16'>";
    page += "    </div>";

    page += "</body>";

    page += "</html>";

    server.setContentLength(page.length());
    server.send(200, "text/html", page);
}

void handleOn()
{
    etatLed = 1;
    digitalWrite(led, HIGH);
    server.sendHeader("Location","/");
    server.send(303);
}

void handleOff()
{
    etatLed = 0;
    digitalWrite(led, LOW);
    server.sendHeader("Location","/");
    server.send(303);
}

void handleNotFound()
{
    server.send(404, "text/plain", "404: Not found");
}


void connect_WIFI(){
   // Set software serial baud to 115200;
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.print("WiFi connected on IP address ");
  Serial.println(WiFi.localIP());

}

void connect_MQTT(){  
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("esp32_Danny")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      digitalWrite(led, HIGH);
      Serial.println("on");
    }
    else if(messageTemp == "off"){
      digitalWrite(led, LOW);
      Serial.println("off");
    }
  }
}


void setup() {
  pinMode(led, OUTPUT);
  connect_WIFI();
  
  client.setServer(MQTT_SERVER,MQTT_PORT);
  client.setCallback(callback);

  connect_MQTT();

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("Serveur web actif!");
}

void loop() {
  server.handleClient();
  if (!client.connected()) {
    Serial.println("Connecting ...");
    delay(5000);
    if (client.connect("esp32_Danny")) {
      client.subscribe("esp32/output");
      client.setCallback(callback);
    } 
  }
  client.loop();
  delay(1);
}
