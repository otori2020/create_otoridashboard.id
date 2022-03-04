/******************************************************************************
 * Copyright 2022 otori.id
 *****************************************************************************/

#include <CloudIoTCore.h>

#include "esp8266_mqtt.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("setup iot core");
  pinMode(LED_BUILTIN, OUTPUT);
  setupCloudIoT(); // Creates globals for MQTT
  
}

unsigned long lastMillis = 0;
void loop()
{
  mqtt->loop();
  delay(10); // <- fixes some issues with WiFi stability

  if (!mqttClient->connected())
  {
    ESP.wdtDisable();
    connect();
    ESP.wdtEnable(0);
  }

  // TODO: Replace with your code here
  if (millis() - lastMillis > 60000)
  {
    lastMillis = millis();
    String payload_data = String("{\"name parameter 1\":")+24+
                          String(",\"name parameter 2\":")+70+
                          String("}");
    publishTelemetry(payload_data);
  }
}
