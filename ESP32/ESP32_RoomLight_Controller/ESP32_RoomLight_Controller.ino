#include "communications.h"
#include "led.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();
  otaSetup();

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(BRIGHTNESS);

  int core = xPortGetCoreID();
  Serial.print("LED code running on core ");
  Serial.println(core);

  // -- Create the FastLED show task
  xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 2048, NULL, 2, &FastLEDshowTaskHandle, FASTLED_SHOW_CORE);
}

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop() {
  ArduinoOTA.handle();
  
  FastLED.delay(1000/FRAMES_PER_SECOND); 
  //fill_rainbow( leds, NUM_LEDS, gHue, 7);

  switch(curMode)
  {
    case 1:
      for(int i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CRGB::Black;
      }
      FastLED.setBrightness(brightness);
      FastLEDshowESP32();
    break;
      
    case 2:
      for(int i = 0; i < NUM_LEDS; i++)
      {
        leds[i].setHSV(gHue, 255, 255);
      }
      FastLED.setBrightness(brightness);
      FastLEDshowESP32();
      EVERY_N_MILLISECONDS( 100 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    break;

    case 3:
      fill_rainbow( leds, NUM_LEDS, gHue, 7);
      FastLED.setBrightness(brightness);
      EVERY_N_MILLISECONDS( 25 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    break;

    case 4:
      if(rgbReady)
      {
        for(int i = 0; i < NUM_LEDS; i++)
        {
          leds[i].setRGB(ledR, ledG, ledB);
        }
        FastLEDshowESP32();
        rgbReady = false;
      }
      FastLED.setBrightness(brightness);
    break;
  }
}
