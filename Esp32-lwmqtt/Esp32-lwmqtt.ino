/******************************************************************************
 * Copyright 2022 otori.id
 *****************************************************************************/

#if defined(ARDUINO_SAMD_MKR1000) or defined(ESP8266)
#define __SKIP_ESP32__
#endif
#if defined(ESP32)
#define __ESP32_MQTT_H__
#endif
#ifdef __SKIP_ESP32__
#include <Arduino.h>
#endif
#ifdef __ESP32_MQTT_H__
#include "esp32-mqtt.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  setupCloudIoT();
  
}

unsigned long lastMillis = 0;

void loop() {
  
  mqtt->loop();
  delay(10);  // <- fixes some issues with WiFi stability

  // TODO: replace with your code
  // publish a message roughly every second.
  
  if (millis() - lastMillis > 60000) {
    
   if (!mqttClient->connected()) {
      Serial.println("mqtt cant connect");      
      connect();
      
    }
   if(mqttClient->connected()){
    Serial.println("mqtt connected");      

    String payload_data = String("{\"name parameter 1\":")+24+
                          String(",\"name parameter 2\":")+70+
                          String("}");

    publishTelemetry(payload_data);
    Serial.println("Publishing value = "+payload_data);

   }
   
    lastMillis = millis();
    
  }
}

#endif
