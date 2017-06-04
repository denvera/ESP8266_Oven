#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>

#include <RingBuf.h>
#include <time.h>
#include <PubSubClient.h>

#include "max6675.h"

#define EXTCONFIG

#ifdef EXTCONFIG
#include "config.h"
#else
#define USE_SSD1306
#define OTA_NAME "ESP8266Oven"
#endif

#define RINGBUG_LEN 5

int thermoDO = D5;
int thermoCS = D6;
int thermoCLK = D7;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

char timestamp[26];
unsigned short current_index = 0;
unsigned long  last_event_ts = 0;

struct TempSample
{
  unsigned short index;
  double temp;
  char timestamp[26];
  bool output;
};
RingBuf *temps = RingBuf_new(sizeof(struct TempSample), RINGBUF_LEN);


char * ICACHE_FLASH_ATTR asctime_iso(struct tm *tim_p ,char *result);

void setup() {  
  Serial.begin(115200);
  initScreen();
  Serial.println("\n");
  Serial.print("connecting to ");
  Serial.print(SSID);
  WiFi.begin(SSID, KEY);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  setupOTA();
  statusScreen("Set time");
  Serial.print("Setting time using SNTP");
  configTime(2 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // GMT + 2 no DST
  time_t now = time(nullptr);
  while (now < 1000) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  statusScreen(asctime(&timeinfo));
  Serial.print("Current time: ");
  asctime_iso(&timeinfo, timestamp);
  Serial.println(timestamp);
  if (!temps)
  {
    Serial.println("Not enough memory to store temps");
  }
  Serial.println("Waiting 1s for MAX6675 to stabilize");
  delay(1000);
}

void loop() {
  idleScreen();
  struct tm timeinfo;
  time_t now = time(nullptr);
  gmtime_r(&now, &timeinfo);
  asctime_iso(&timeinfo, timestamp);
  double tempc = thermocouple.readCelsius();
  delay(1000);
  ArduinoOTA.handle();
  if (millis() - last_event_ts > 10000) {    
      last_event_ts = millis();
      Serial.print(timestamp);  
      if (isnan(tempc)) { 
        Serial.print(" Thermocouple Disconnected!");
      } else {
        Serial.print(" C = ");
        Serial.print(tempc);
      }
      struct TempSample current_sample;
      current_sample.index = (current_index++) % 270;
      current_sample.temp = tempc;
      current_sample.output = false;
      strncpy(current_sample.timestamp, timestamp, strlen(timestamp)+1);
      temps->add(temps, &current_sample);      
      Serial.print(" [RingBuf Len: ");
      Serial.print(RingBufNumElements(temps));
      Serial.println("]");
  }

}


char * ICACHE_FLASH_ATTR
asctime_iso(struct tm *tim_p ,char *result)
{
  os_sprintf (result, "%02d-%02d-%02dT%02d:%02d:%02d+02:00",
     1900 + tim_p->tm_year, tim_p->tm_mon+1, tim_p->tm_mday, tim_p->tm_hour, tim_p->tm_min,
     tim_p->tm_sec);
  return result;
}
