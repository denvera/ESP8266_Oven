#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>

#include <PubSubClient.h>

#include "max6675.h"

#define OTA_NAME "ESP8266Oven"

int thermoDO = D1;
int thermoCS = D2;
int thermoCLK = D4;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

void setup() {  
  Serial.begin(115200);
  setupOTA();
  Serial.println("Waiting 1s for MAX6675 to stabilize");
  delay(1000);
}

void loop() {
  Serial.print("C = "); 
  Serial.println(thermocouple.readCelsius());
  delay(1000);
  ArduinoOTA.handle();
}
