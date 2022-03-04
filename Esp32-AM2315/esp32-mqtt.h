
#include <Client.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <MQTT.h>

#include <CloudIoTCore.h>
#include <CloudIoTCoreMqtt.h>
#include "ciotc_config.h" // Update this file with your configuration

// !!REPLACEME!!
// The MQTT callback function for commands and configuration updates
// Place your message handler code here.
void messageReceived(String &topic, String &payload){
  Serial.println("incoming: " + topic + " - " + payload);
  
  
}
///////////////////////////////

// Initialize WiFi and MQTT for this board
WiFiClientSecure *netClient;
CloudIoTCoreDevice *device;
CloudIoTCoreMqtt *mqtt;
MQTTClient *mqttClient;
unsigned long iat = 0;
String jwt;

///////////////////////////////
// Helpers specific to this board
///////////////////////////////
String getDefaultSensor(){
  return "Wifi: " + String(WiFi.RSSI()) + "db";
}


String getJwt(){
  iat = time(nullptr);
  Serial.println("Refreshing JWT");
  jwt = device->createJWT(iat, jwt_exp_secs);
  return jwt;
}

void setupWifi(){
  Serial.println("Starting wifi");

  WiFi.mode(WIFI_STA);
  // WiFi.setSleep(false); // May help with disconnect? Seems to have been removed from WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED){
    delay(100);
  }

  configTime(0, 0, ntp_primary, ntp_secondary);
  Serial.println("Waiting on time sync...");
  while (time(nullptr) < 1510644967){
    delay(10);
  }
}

void connectWifi(){
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(1000);
  }
}

void detachDelegate(String delegateId) {
  //subscribe to delegate configuration
  mqttClient->unsubscribe("/devices/"+ delegateId +"/config");

  //subscribe to delegate commands
  mqttClient->unsubscribe("/devices/"+ delegateId +"/commands/#");

    //attach to delegate device
  String dat = "{}";
  mqttClient->publish(
      String("/devices/"+ delegateId +"/detach").c_str(),
      dat.c_str(), false, 1);
}


void attachAndSubscribe(String delegateId) {
  //attach to delegate device
  String dat = "{}";
  mqttClient->publish(String("/devices/"+ delegateId +"/attach").c_str(),
      dat.c_str(), false, 1);

  //subscribe to delegate configuration
  mqttClient->subscribe("/devices/"+ delegateId +"/config", 1);

  //subscribe to delegate commands
  mqttClient->subscribe("/devices/"+ delegateId +"/commands/#", 0);
}

///////////////////////////////
// Orchestrates various methods from preceeding code.
///////////////////////////////
bool publishTelemetry(String data){
  return mqtt->publishTelemetry(data);
}

bool publishTelemetry(const char *data, int length){
  return mqtt->publishTelemetry(data, length);
}

bool publishTelemetry(String subfolder, String data){
  return mqtt->publishTelemetry(subfolder, data);
}

bool publishTelemetry(String subfolder, const char *data, int length){
  return mqtt->publishTelemetry(subfolder, data, length);
}

bool publishDelegateTelemetry(String delegateId,String data) {
  return mqttClient->publish(
      String("/devices/"+ delegateId +"/events").c_str(),
      String(data).c_str(), false, 1);
}

bool publishDelegateState(String delegateId,String data) {
  return mqttClient->publish(
      String("/devices/"+ delegateId +"/state").c_str(),
      String(data).c_str(), false, 1);
}



void connect(){
  connectWifi();
  mqtt->mqttConnect();
}

void setupCloudIoT(){
  device = new CloudIoTCoreDevice(
      project_id, location, registry_id, device_id,
      private_key_str);

  setupWifi();
  netClient = new WiFiClientSecure();
  netClient->setCACert(root_cert);
  mqttClient = new MQTTClient(512);
  mqttClient->setOptions(180, true, 10000); // keepAlive, cleanSession, timeout
  mqtt = new CloudIoTCoreMqtt(mqttClient, netClient, device);
  mqtt->setUseLts(true);
  mqtt->startMQTT();
}
