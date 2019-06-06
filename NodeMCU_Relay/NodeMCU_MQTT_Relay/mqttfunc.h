#include <PubSubClient.h>
#include <ESP8266WiFi.h>

WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = "192.168.1.75";
const char* mqtt_username = "admin";
const char* mqtt_password = "pwnrazr123";

void subscribemqtt()
{
  client.subscribe("/myroom/relay/0");
  client.subscribe("/myroom/relay/1");
  client.subscribe("/myroom/relay/2");
  client.subscribe("/myroom/relay/3");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    digitalWrite(2, LOW);
    // Attempt to connect
    if (client.connect("ESPRelay", mqtt_username, mqtt_password)) {
      Serial.println("Connected");
      digitalWrite(2, HIGH);
      // Once connected, publish an announcement...

      //publish on connect insert here
      client.publish("/myroom/relay/boot", "0"); //publish to topic on boot
      //client.publish("NodeMCU/IP", ip);
      // ... and resubscribe
      subscribemqtt();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
