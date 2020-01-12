#include <PubSubClient.h>
#include <ESP8266WiFi.h>

WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = "192.168.1.184";
const char* mqtt_username = "pwnrazr";
const char* mqtt_password = "pwnrazr123";

char ipaddr[16];

void subscribemqtt()
{
  client.subscribe("/adalight/statecmd");
  client.subscribe("/adalight/mode");
  client.subscribe("/adalight/brightness");
  client.subscribe("/adalight/R");
  client.subscribe("/adalight/G");
  client.subscribe("/adalight/B");
  client.subscribe("/adalight/welcomemessage");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial1.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
      Serial1.println("Connected");
      // Once connected, publish an announcement...
      client.publish("/nodemcu/status", "Connected");
      //client.publish("myroom/lights/deskled/brightness/lastval", "reqbrightness");
      
      sprintf(ipaddr, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
      
      client.publish("/nodemcu/ip", ipaddr);
      
      // ... and resubscribe
      subscribemqtt();
    } else {
      Serial1.print("failed, rc=");
      Serial1.print(client.state());
      Serial1.println(" try again in 5 seconds");
      // Wait 1 seconds before retrying
      delay(1000);
    }
  }
}
