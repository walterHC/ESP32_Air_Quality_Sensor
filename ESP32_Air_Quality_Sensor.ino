#include "secrets.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define MQ135_PIN 34
#define LED_PIN 2

#define MQTT_PUBLISH_TOPIC "esp32/JKhmMHTpo6c3KsiLAp5yV/pub"
#define MQTT_SUBSCRIBE_TOPIC "esp32/JKhmMHTpo6c3KsiLAp5yV/sub"

int airQuality;

WiFiClient net;
PubSubClient client(net);

void connectMQTT()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Conectar al broker MQTT en la Raspberry Pi
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(messageHandler);

  Serial.println("Connecting to MQTT broker");
  while (!client.connect(THINGNAME, MQTT_USER, MQTT_PASSWORD))
  {
    Serial.print(".");
    delay(1000);
  }

  if (!client.connected())
  {
    Serial.println("MQTT broker Timeout!");
    return;
  }

  // Suscribirse a un topic
  client.subscribe(MQTT_SUBSCRIBE_TOPIC);

  Serial.println("MQTT broker Connected!");
}

void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["airQuality"] = airQuality;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
  Serial.println(F("Message Send To Broker Mosquitto"));

  client.publish(MQTT_PUBLISH_TOPIC, jsonBuffer);
}

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* command = doc["command"];
  Serial.println(command);

  if (strcmp(command, "on") == 0) {
    digitalWrite(LED_PIN, HIGH); // Enciende el LED
  } else if (strcmp(command, "off") == 0) {
    digitalWrite(LED_PIN, LOW); // Apaga el LED
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  connectMQTT();
}

void loop()
{
  airQuality = analogRead(MQ135_PIN);

  Serial.print(F("Air Quality: "));
  Serial.println(airQuality);

  publishMessage();
  client.loop();
  delay(10000);
}
