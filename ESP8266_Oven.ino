#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
//#include <FS.h>
#include <EEPROM.h>

#include <RingBuf.h>
//#include <ArduinoJson.h>
//#include <PubSubClient.h>
#include <Encoder.h>
#include <PID_v1.h>

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
#define PID_INTERVAL 1000
#define START_TEMP 20
#define DEBOUNCE_TIME 50
#define BUTTON D0
#define RELAY D8
#define TC_DO D5
#define TC_CS D6
#define TC_CLK D7
#define BANDGAP 5

ESP8266WebServer server(80);


MAX6675 thermocouple(TC_CLK, TC_CS, TC_DO);

Encoder enc(D4, D3);

char timestamp[26];
unsigned short current_index = 0;
unsigned long last_event_ts = 0, last_state_ts = 0, debounce_ts = 0, pid_ts = 0, min_ts = 0;
short set_temp = 0, set_time = 0;
double current_temp = 0;
enum states { STATE_IDLE, STATE_STATUS, STATE_SET_TEMP, STATE_SET_TIME };
enum controllers { CTRL_PID, CTRL_DUMB };
enum states current_state = STATE_STATUS, old_state = STATE_IDLE;
enum controllers current_controller = CTRL_PID;
byte lastBtnState = HIGH, btnState = HIGH;
bool output = false;
struct tm timeinfo;

struct PROGMEM TempSample {
//  unsigned short index;
  float temp;
  short set_temp;
  char timestamp[26];
  bool output;
};
struct PROGMEM Config {
  double Kp;
  double Ki;
  double Kd;
  enum controllers controller;  
};
struct Config config;
RingBuf *temps = RingBuf_new(sizeof(struct TempSample), RINGBUF_LEN);
long oldPosition  = 0, lastPosition = 0;

double Setpoint, Input, Output;
double Kp=2 , Ki=0.5, Kd=0.2;
PID OvenPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

int WindowSize = 5000;
unsigned long windowStartTime;



char * ICACHE_FLASH_ATTR asctime_iso(struct tm *tim_p ,char *result);
void ICACHE_FLASH_ATTR setupWebServer();
void ICACHE_FLASH_ATTR initScreen();
void ICACHE_FLASH_ATTR statusScreen(String msg, int currentTemp, int setTemp);
void ICACHE_FLASH_ATTR statusScreen(String msg);
void ICACHE_FLASH_ATTR setTempScreen(String temp);
void ICACHE_FLASH_ATTR setTimeScreen(String sTime);
void ICACHE_FLASH_ATTR idleScreen();
void ICACHE_FLASH_ATTR displayWake();
void ICACHE_FLASH_ATTR idleScreen(String msg);
void ICACHE_FLASH_ATTR setupOTA();

void ICACHE_FLASH_ATTR setup() {  
  pinMode(BUTTON, INPUT);
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, false);
  Serial.begin(115200);
  initScreen();
  Serial.println("\n");
  Serial.print("connecting to ");
  Serial.print(SSID);
  WiFi.hostname("ESP8266Oven");
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
  statusScreen("Load config");
  EEPROM.begin(512);
  getConfig();
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
  //statusScreen(asctime(&timeinfo));
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
  current_temp = thermocouple.readCelsius();  
  windowStartTime = millis();
  OvenPID.SetOutputLimits(0, WindowSize);
  OvenPID.SetMode(AUTOMATIC);
}

void check_timeout() {
  if (millis() - last_state_ts > STATE_TIMEOUT) {
    last_state_ts = millis();
    old_state = current_state;
    current_state = STATE_IDLE;
  }  
}

void state_idle() {
  //idleScreen(String(current_temp) + " °C");
  idleScreen();
}

void state_status() {
  check_timeout();
  idleScreen();
  // The status screen was a bit meh :|
  //statusScreen("Set: " + String(set_temp) + " °C", current_temp, set_temp);
}
void state_set_temp() {
  check_timeout();
  setTempScreen(String(set_temp));
  Setpoint = set_temp;
}

void state_set_time() {  
  check_timeout();
  if (set_time != 0) {   
    setTimeScreen(String(set_time) + " min");
  } else {
    setTimeScreen("Disabled");
  }
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
        last_state_ts = millis();
        switch (current_state) {
          case STATE_IDLE:
            Serial.println("IDLE -> STATUS");
            displayWake();
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
    } else {
      if (millis() - debounce_ts > 1000) {
        btnState = HIGH;
        debounce_ts = millis();
      }
    }
  }
  if (millis() - last_event_ts > SAMPLE_INTERVAL) {     
      if (set_temp <= 10) {
        set_temp = 0;
        output = false;
        digitalWrite(RELAY, output);     
      }
      time_t now = time(nullptr);
      gmtime_r(&now, &timeinfo);
      asctime_iso(&timeinfo, timestamp);
      double tempc = thermocouple.readCelsius();  
      current_temp = tempc;
      last_event_ts = millis();
      Serial.print(timestamp);  
      if (isnan(tempc)) { 
        Serial.print(" Thermocouple Disconnected!");
      } else {
        Serial.print(" C = ");
        Serial.print(tempc);
      }
      struct TempSample current_sample;
      //current_sample.index = (current_index++) % RINGBUF_LEN;
      current_sample.temp = tempc;
      current_sample.output = output;
      current_sample.set_temp = set_temp;
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
    Serial.printf("Old Pos: %d, Last pos: %d, New Pos: %d\n", oldPosition, lastPosition, newPosition);
    if (abs(lastPosition - newPosition) >= 4) {    
      last_state_ts = millis();    
      if (current_state == STATE_IDLE) {         
        old_state = current_state;
        current_state = STATE_STATUS;
      } else if (current_state == STATE_STATUS) {
        Serial.println("Cur Temp: " + String(current_temp) + " Set: " + String(set_temp) + " %: " + String(float(current_temp)/float(set_temp)));
        old_state = current_state;
        current_state = STATE_SET_TEMP;
      } else if (current_state == STATE_SET_TEMP) {
        if (newPosition > oldPosition) {
          if (set_temp == 0) { 
            //OvenPID.Reset();
            OvenPID.SetMode(MANUAL);
            OvenPID.SetMode(AUTOMATIC);
          }
          set_temp += (set_temp == 0) ? START_TEMP : 10;
          OvenPID.SetMode(MANUAL);
          OvenPID.SetMode(AUTOMATIC);
        } else {
          if ((set_temp-10) >= START_TEMP) {
            set_temp -= 10;
            OvenPID.SetMode(MANUAL);
            OvenPID.SetMode(AUTOMATIC);
          } else {
            set_temp = 0;
            output = false;
            OvenPID.SetMode(MANUAL);
          }
        }
        Setpoint = set_temp;
      } else if (current_state == STATE_SET_TIME) {
        if (newPosition > oldPosition) {
          min_ts = millis(); 
          set_time += 1;
        } else {
          min_ts = millis(); 
          if (set_time >= 1) {
            set_time -= 1;
          } else {
            set_time = 0;
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
  if (millis() - pid_ts >= PID_INTERVAL) {
    pid_ts = millis();
    time_t now = time(nullptr);
    gmtime_r(&now, &timeinfo);
    current_temp = thermocouple.readCelsius();
    Serial.println("Temp: " + String(thermocouple.readCelsius()));
    if (set_temp >= START_TEMP) {
      Input = current_temp;      
      if (millis() - windowStartTime > WindowSize)
      { //time to shift the Relay Window
        windowStartTime += WindowSize;
      }
      if (current_controller == CTRL_PID) {
        if (Output < millis() - windowStartTime) {
          //digitalWrite(RELAY_PIN, HIGH);
          if (output != false) {
            Serial.println("[" + String(Output) + "] ELEMENT OFF!");
            output = false;
          }
        } else {
          //digitalWrite(RELAY_PIN, LOW);
          if (output != true) {
            Serial.println("[" + String(Output) + "] ELEMENT ON!");
            output = true;
          }
        }
      } else {
        dumbSwitch();
        Serial.println("Dumb switch: " + String(output));
      }
    } else {
      // output off
      output = false;
    }
    digitalWrite(RELAY, output);
  }
  OvenPID.Compute();
  if (millis() - min_ts >= 60000) {
    min_ts = millis();
    if (set_time == 1) {
      set_time--;
      set_temp = 0;
      Serial.println("Time up - switching off");
    } else if (set_time > 1) {
      set_time--;
      Serial.println("Time left: " + String(set_time) + " min");
    } else {
      // Nothing
    }    
  }
}

void saveConfig() {    
  config.Kp = Kp;
  config.Ki = Ki;
  config.Kd = Kd;
  config.controller = current_controller;
  Serial.println("Saving config...");
  printConfig();
  EEPROM.write(0, 0xDD);
  EEPROM.put(1, config);
  EEPROM.commit();
  Serial.println("Done");
}
bool getConfig() {
  byte confOk = 0;
  confOk = EEPROM.read(0);
  if (confOk == 0xDD) {
    Serial.println("Getting config:");
    EEPROM.get(1, config);
    current_controller = config.controller;
    Kp = config.Kp;
    Ki = config.Ki;
    Kd = config.Kd;    
    printConfig();
    Serial.println("Done");
  } else {
    Serial.println("[!] Not loading config - bad flag");
  }
}

void printConfig() {
  Serial.println("Kp: " + String(Kp) + " Ki: " + String(Ki) + " Kd: " + String(Kd));
  Serial.print("Controller: ");
  Serial.println((current_controller == CTRL_PID) ? "PID" : "DUMB");
}

void dumbSwitch() {
  if (current_temp <= set_temp - BANDGAP)
    output = true;
  else if (current_temp >= set_temp + BANDGAP)
    output = false;
}

char * ICACHE_FLASH_ATTR
asctime_iso(struct tm *tim_p ,char *result)
{
  os_sprintf (result, "%02d-%02d-%02dT%02d:%02d:%02d+02:00",
     1900 + tim_p->tm_year, tim_p->tm_mon+1, tim_p->tm_mday, tim_p->tm_hour, tim_p->tm_min,
     tim_p->tm_sec);
  return result;
}

char * ICACHE_FLASH_ATTR 
asctime_time(struct tm *tim_p, char *result)
{
  os_sprintf (result, "%02d:%02d", tim_p->tm_hour, tim_p->tm_min);
  return result;
}

