#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define FSR_PIN A0
#define WIFI_SSID "Ap04"
#define WIFI_PASSWORD "16253404"

const char *SERVER_MQTT = "test.mosquitto.org";
const int MQTT_PORT = 1883;
const char *PUBLISH_TOPIC = "teste/2";

WiFiClient espClient;
PubSubClient client(espClient);

bool is_wifi_connected()
{
  return WiFi.status() == WL_CONNECTED;
}

void conectarWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Conectando ao Wi-Fi...");
  while (!is_wifi_connected())
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Conectado à rede: ");
  Serial.println(WiFi.SSID());

  Serial.print("IP atribuído: ");
  Serial.println(WiFi.localIP());
}

void displayMqttNotConnected()
{
  Serial.print("Falha na conexão MQTT. Estado: ");
  Serial.print(client.state());
  Serial.println(". Tentando novamente em 5 segundos...");
}

void reconnect()
{
  byte willQoS = 0;
  const char *willTopic = "esp/status";
  const char *willMessage = "OFF_LINE";
  boolean willRetain = true;
  const char *message = "ON_LINE";
  boolean retained = true;
  String clientId = "ESP8266Client01p";

  while (!client.connected())
  {
    Serial.print("Tentando estabelecer conexão MQTT...");

    if (client.connect(clientId.c_str(), willTopic, willQoS, willRetain, willMessage))
    {
      client.publish("esp/status", message, retained);
      Serial.print("Conectado ao MQTT e tópico: ");
      Serial.println(willTopic);
    }
    else
    {
      displayMqttNotConnected();
      delay(5000);
    }
  }
}

void setupMqtt()
{
  client.setServer(SERVER_MQTT, MQTT_PORT);
}

void setup()
{
  Serial.begin(115200);
  pinMode(FSR_PIN, INPUT);
  conectarWifi();
  setupMqtt();
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Conexão Wi-Fi perdida! Tentando reconectar...");
    conectarWifi();
  }

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  int sensorValue = analogRead(FSR_PIN);
  float voltage = sensorValue * (1.0 / 1023.0);

  if (sensorValue > 100)
  {
    Serial.println("Pressionado");
    Serial.print("Valor bruto: ");
    Serial.print(sensorValue);
    Serial.print(" | Tensão: ");
    Serial.println(voltage);

    String payload = String("{\"value\": ") + sensorValue + ", \"voltage\": " + voltage + "}";
    client.publish(PUBLISH_TOPIC, payload.c_str());
  }
  delay(100);
}