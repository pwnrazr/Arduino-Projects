#include <PubSubClient.h>
#include <ESP8266WiFi.h>

WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = "192.168.1.75";
const char* mqtt_username = "admin";
const char* mqtt_password = "pwnrazr123";

char ipaddr[16];

void subscribemqtt()
{
  client.subscribe("/myroom/lights/mosfet0");
  client.subscribe("/nodemcu/request/temperature");
  client.subscribe("/myroom/lights/mosfet0/brightness");
  client.subscribe("/myroom/lights/mosfet0/breathe");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
      Serial.println("Connected");
      // Once connected, publish an announcement...
      client.publish("/nodemcu/status", "Connected");
      //client.publish("myroom/lights/deskled/brightness/lastval", "reqbrightness");
      
      sprintf(ipaddr, "IP:%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
      
      client.publish("/nodemcu/ip", ipaddr);
      
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
