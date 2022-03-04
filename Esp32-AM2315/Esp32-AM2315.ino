/******************************************************************************
 * Copyright 2022 otori.id
 *****************************************************************************/
// Connect RED of the AM2315 sensor to 5.0V
// Connect BLACK to Ground
// Connect WHITE to i2c clock
// Connect YELLOW to i2c data


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

#include <Adafruit_AM2315.h>
Adafruit_AM2315 am2315;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  setupCloudIoT();

  while (!Serial) {
    delay(10);
  }
  Serial.println("AM2315 Test!");

  if (! am2315.begin()) {
      Serial.println("Sensor not found, check wiring & pullups!");
      while (1);
  }
  delay(2000);
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
         
    float temperature, humidity;

    if (! am2315.readTemperatureAndHumidity(&temperature, &humidity)) {
        Serial.println("Failed to read data from AM2315");
        return;
    }

    Serial.print("Temp *C: "); Serial.println(temperature);
    Serial.print("Hum %: "); Serial.println(humidity);
    
    String payload_data = String("{\"name parameter 1\":")+temperature+
                          String(",\"name parameter 2\":")+humidity+
                          String("}");

    publishTelemetry(payload_data);
    Serial.println("Publishing value = "+payload_data);

   }
   
    lastMillis = millis();
    
  }
}

#endif
