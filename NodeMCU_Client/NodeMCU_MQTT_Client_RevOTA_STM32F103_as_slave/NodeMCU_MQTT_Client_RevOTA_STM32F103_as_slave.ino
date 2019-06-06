#include <ESP8266WiFi.h>
#include <PubSubClient.h>
//#include <Wire.h> //STM32F103 
#include "wifiConf.h"
#include "comms.h"
#include "mqttfunc.h"
#include "OTAfunc.h"

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

unsigned long previousMillis = 0, currentMillis;
unsigned int interval = 15000;

String ledR, ledG, ledB;

void setup() 
{
  //pinMode(2, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  //Wire.begin(4, 5); //SDA, SCL
  
  Serial.begin(115200);
  Serial1.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  OTAfunc();
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  String topicstr;
  String payloadstr;
  
  for (int i = 0; i < length; i++) 
  {
    payloadstr = String(payloadstr + (char)payload[i]);  //convert payload to string
  }
  
  for(int i = 0; i <= 50; i++)
  {
    topicstr = String(topicstr + (char)topic[i]);  //convert topic to string
  }

  Serial1.print("Message arrived: ");
  Serial1.print(topicstr);
  Serial1.print(" - ");
  Serial1.print(payloadstr);
  Serial1.println();
  
  if(topicstr=="/adalight/statecmd")
  { 
    if(payloadstr=="1") //On Adalight
    {
      client.publish("/adalight/state", "1"); //publish to topic
      Serial.print("<state, 1>");
    }
    else if(payloadstr=="0") //Off Adalight
    {
      client.publish("/adalight/state", "0"); //publish to topic 
      Serial.print("<state, 0>");
    } 
  }
  
  if(topicstr=="/adalight/mode")
  {
    Serial.print("<mode, " + payloadstr + ">");
  }

  if(topicstr=="/adalight/brightness")
  { 
    Serial.print("<brightness, " + payloadstr + ">");
  }
  
  if(topicstr=="/adalight/R")
  {
    ledR = payloadstr;
  }
  if(topicstr=="/adalight/G")
  {
    ledG = payloadstr;
  }
  if(topicstr=="/adalight/B")
  {
    ledB = payloadstr;
    Serial.print("<ledRGB, " + ledR + "," + ledG + "," + ledB + ">");
  }

  if(topicstr=="/adalight/welcomemessage")
  { 
    Serial.print("<welcomemsg, 55>");
  }
}

void loop() 
{
  currentMillis = millis();
  
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();
  ArduinoOTA.handle();

  if (currentMillis - previousMillis >= interval) 
  {
    previousMillis = currentMillis;
    client.publish("/nodemcu/heartbeat", "Hi");
  }
  
  if(currentMillis> 4094967296)
  {
    //Serial.println("reboot");
    //Serial.println(currentMillis);
    ESP.restart();
  }
}
