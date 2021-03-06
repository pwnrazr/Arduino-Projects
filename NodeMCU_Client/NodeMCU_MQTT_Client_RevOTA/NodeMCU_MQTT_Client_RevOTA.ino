#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "wifiConf.h"
#include "comms.h"
#include "mqttfunc.h"
#include "OTAfunc.h"

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

unsigned long previousMillis = 0, currentMillis;
unsigned int interval = 15000;

void setup() 
{
  //pinMode(2, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Wire.begin(4, 5); //SDA, SCL
  
  Serial.begin(115200);
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

  Serial.print("Message arrived: ");
  Serial.print(topicstr);
  Serial.print(" - ");
  Serial.print(payloadstr);
  Serial.println();
  
  if(topicstr=="/myroom/lights/mosfet0")
  {
    if(payloadstr=="1") //On desk LED
    {
      //digitalWrite(2, LOW); 
      mosfet0On(); //Toggle desk LED
      client.publish("/myroom/lights/mosfet0/state", "1"); //publish to topic 
    }
    else if(payloadstr=="0") //Off desk LED
    {
      //digitalWrite(2, HIGH);
      mosfet0Off();
      client.publish("/myroom/lights/mosfet0/state", "0"); //publish to topic 
    }
  }

  if(topicstr=="/myroom/lights/mosfet0/brightness                  ")
  {
    brightnessled = payloadstr.toInt();
    mosfet0brightness();
  }
  
  if(topicstr=="/nodemcu/request/temperature")
  {
    reqtemp();
  }

  if(topicstr=="/myroom/lights/mosfet0/breathe")
  {
    if(payloadstr=="1") //On breathe
    {
      mosfet0breatheOn();
      client.publish("/myroom/lights/mosfet0/breathe/state", "1"); //publish to topic 
    }
    else if(payloadstr=="0") //Off breathe
    {
      mosfet0breatheOff();
      client.publish("/myroom/lights/mosfet0/breathe/state", "0"); //publish to topic 
    }
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
    Serial.println("reboot");
    Serial.println(currentMillis);
    ESP.restart();
  }
}

void reqtemp()
{
    getTemp();  //call temperature receive function
    
    snprintf (tempchar, 75, "%ld.%ld", (int)temp, (int)(temp*100)%100);  //Send temperature

    client.publish("/myroom/temperature", tempchar); //publish to topic and tempchar as payload
}

