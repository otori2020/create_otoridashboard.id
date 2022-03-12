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

#include "RTClib.h"

RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


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

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
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
    
    delay(100);
    DateTime now = rtc.now();
    String year_now = String(now.year(),DEC); 
    String month_now = "";
    int month_dtg =now.month();
    if(month_dtg < 10){
      month_now += '0';
    }
    month_now += month_dtg;  
    String day_now = "";
    int day_dtg =now.day();
    if(day_dtg < 10){
      day_now += '0';
    }
    day_now += day_dtg; 
    String hour_now = "";
    int hour_dtg =now.hour();
    if(hour_dtg < 10){
      hour_now += '0';
    }
    hour_now += hour_dtg;
    String minute_now = "";
    int minute_dtg =now.minute();
    if(minute_dtg < 10){
      minute_now += '0';
    }
    minute_now += minute_dtg;
    String second_now = "";
    int second_dtg =now.second();
    if(second_dtg < 10){
      second_now += '0';
    }
  
    second_now += second_dtg;
    String date_rtc = String(now.year(),DEC) + "-" + month_now + "-" + day_now;
    String time_rtc = hour_now + ":" + minute_now + ":" + second_now;   
    String date_time = String(date_rtc)+"T"+String(time_rtc);

         
    float temperature, humidity;

    if (! am2315.readTemperatureAndHumidity(&temperature, &humidity)) {
        Serial.println("Failed to read data from AM2315");
        return;
    }

    Serial.print("Temp *C: "); Serial.println(temperature);
    Serial.print("Hum %: "); Serial.println(humidity);
    
    String payload_data = String("{\"date_device\":\"")+date_time+  //format date : 2022-03-12T19:43:33
                            String("\",\"temperature\":\"")+temperature+
                            String("\",\"humidity\":\"")+humidity+
                            String("\"}");

    publishTelemetry(payload_data);
    Serial.println("Publishing value = "+payload_data);

   }
   
    lastMillis = millis();
    
  }
}

#endif
