void ICACHE_FLASH_ATTR setupOTA() {
  // Setup OTA
  Serial.println(F("Initializing OTA..."));  
#ifdef OTA_NAME  
  #warning "Setting OTA name"
  ArduinoOTA.setHostname(OTA_NAME);
#endif
#ifdef OTA_PASSWORD
  #warning "Setting OTA password"
  ArduinoOTA.setPassword((const char *)OTA_PASSWORD);
#endif
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    }
    else { // U_SPIFFS
      type = "filesystem";
      //SPIFFS.end();
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    }
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}
