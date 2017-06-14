#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
//#include <FS.h>

#include <RingBuf.h>
//#include <ArduinoJson.h>
//#include <PubSubClient.h>
#include <Encoder.h>

#include <time.h>

#include "max6675.h"

#define EXTCONFIG

#ifdef EXTCONFIG
#include "config.h"
#else
#define USE_SSD1306
#define OTA_NAME "ESP8266Oven"
#endif

#define RINGBUF_LEN 480
#define SAMPLE_INTERVAL 15000
#define STATE_TIMEOUT 30000
#define START_TEMP 50
#define DEBOUNCE_TIME 50
#define BUTTON D0
ESP8266WebServer server(80);

int thermoDO = D5;
int thermoCS = D6;
int thermoCLK = D7;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

Encoder enc(D4, D3);

char timestamp[26];
unsigned short current_index = 0;
unsigned long last_event_ts = 0, last_state_ts = 0, debounce_ts = 0;
short set_temp = 0;
enum states { STATE_IDLE, STATE_STATUS, STATE_SET_TEMP, STATE_SET_TIME };
enum states current_state = STATE_STATUS, old_state = STATE_IDLE;
byte lastBtnState = HIGH, btnState = HIGH;

struct PROGMEM TempSample
{
  unsigned short index;
  double temp;
  char timestamp[26];
  bool output;
};
RingBuf *temps = RingBuf_new(sizeof(struct TempSample), RINGBUF_LEN);
long oldPosition  = 0, lastPosition = 0;

char * ICACHE_FLASH_ATTR asctime_iso(struct tm *tim_p ,char *result);
void ICACHE_FLASH_ATTR setupWebServer();
void ICACHE_FLASH_ATTR initScreen();
void ICACHE_FLASH_ATTR statusScreen(String msg);
void ICACHE_FLASH_ATTR idleScreen();
void ICACHE_FLASH_ATTR setupOTA();

void ICACHE_FLASH_ATTR setup() {  
  pinMode(BUTTON, INPUT);
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
  Serial.print(F("WiFi connected [IP: "));
  Serial.print(WiFi.localIP());
  Serial.println("]");
  setupOTA();
  statusScreen("Set time");
  Serial.print(F("Setting time using SNTP"));
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
    Serial.println(F("Not enough memory to store temps"));
  }
  setupWebServer();
  server.begin();
  Serial.println(F("HTTP server started"));
  Serial.println(F("Waiting 1s for MAX6675 to stabilize"));
  delay(1000);
}

void check_timeout() {
  if (millis() - last_state_ts > STATE_TIMEOUT) {
    last_state_ts = millis();
    old_state = current_state;
    current_state = STATE_IDLE;
  }  
}

void state_idle() {
  idleScreen();
}

void state_status() {
  check_timeout();
  statusScreen("Current Temp: " + String(set_temp));
}
void state_set_temp() {
  check_timeout();
  statusScreen("Set Temp: " + String(set_temp));
}

void state_set_time() {  
  check_timeout();
  statusScreen("Set Time: ????");
}

void ICACHE_FLASH_ATTR loop() {
  ArduinoOTA.handle();
  server.handleClient();
  byte btnNow = digitalRead(BUTTON);
  if (btnNow != lastBtnState) {
    debounce_ts = millis();
  }
  if (millis() - debounce_ts > DEBOUNCE_TIME) {
    if (btnNow != btnState) {
      btnState = btnNow;
      if (btnState == LOW) {
        switch (current_state) {
          case STATE_IDLE:
            Serial.println("IDLE -> STATUS");
            current_state = STATE_STATUS;
            break;
          case STATE_STATUS:
            Serial.println("STATUS -> SET_TEMP");
            current_state = STATE_SET_TEMP;
            break;
          case STATE_SET_TEMP:
            Serial.println("SET_TEMP -> SET_TIME");
            current_state = STATE_SET_TIME;
            break;
          case STATE_SET_TIME:
            Serial.println("SET_TIME -> STATUS");
            current_state = STATE_STATUS;
            break;
        }
      }
    }
  }
  if (millis() - last_event_ts > SAMPLE_INTERVAL) {    
      struct tm timeinfo;
      time_t now = time(nullptr);
      gmtime_r(&now, &timeinfo);
      asctime_iso(&timeinfo, timestamp);
      double tempc = thermocouple.readCelsius();  
      last_event_ts = millis();
      Serial.print(timestamp);  
      if (isnan(tempc)) { 
        Serial.print(" Thermocouple Disconnected!");
      } else {
        Serial.print(" C = ");
        Serial.print(tempc);
      }
      struct TempSample current_sample;
      current_sample.index = (current_index++) % RINGBUF_LEN;
      current_sample.temp = tempc;
      current_sample.output = false;
      strncpy(current_sample.timestamp, timestamp, strlen(timestamp)+1);
      if (RingBufIsFull(temps)) {
        struct TempSample temp_temp;
        RingBufPull(temps, &temp_temp);
      }
      temps->add(temps, &current_sample);      
      Serial.print(" [RingBuf Len: ");
      Serial.print(RingBufNumElements(temps));
      Serial.println("]");
  }
  long newPosition = enc.read();
  if (newPosition != oldPosition) {
    Serial.printf("Old Pos: %d, New Pos: %d\n", oldPosition, newPosition);
    if (abs(lastPosition - newPosition) >= 4) {    
      last_state_ts = millis();    
      if (current_state == STATE_IDLE) {         
        old_state = current_state;
        current_state = STATE_STATUS;
      } else if (current_state == STATE_STATUS) {
        old_state = current_state;
        current_state = STATE_SET_TEMP;
      } else if (current_state == STATE_SET_TEMP) {
        if (newPosition > oldPosition) {
          set_temp += (set_temp == 0) ? START_TEMP : 10;
        } else {
          if ((set_temp-10) >= START_TEMP) {
            set_temp -= 10;
          } else {
            set_temp = 0;
          }
        }
      }
      lastPosition = newPosition;
    }
    oldPosition = newPosition;

  }
    switch (current_state) {
      case STATE_IDLE:
          state_idle();
          break;
      case STATE_STATUS:
          state_status();
          break;
      case STATE_SET_TEMP:
          state_set_temp();
          break;
      case STATE_SET_TIME:
          state_set_time();
          break;
      default:
          break;
  }
  lastBtnState = btnNow;
}


char * ICACHE_FLASH_ATTR
asctime_iso(struct tm *tim_p ,char *result)
{
  os_sprintf (result, "%02d-%02d-%02dT%02d:%02d:%02d+02:00",
     1900 + tim_p->tm_year, tim_p->tm_mon+1, tim_p->tm_mday, tim_p->tm_hour, tim_p->tm_min,
     tim_p->tm_sec);
  return result;
}
