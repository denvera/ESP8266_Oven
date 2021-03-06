#ifdef USE_SSD1306
#include <brzo_i2c.h>
//#include <Wire.h> 
#include "SSD1306.h"
#warning "Display Enabled"

#define temp_width 20
#define temp_height 46
static const char PROGMEM temp_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x80, 0x3f, 0x00,
   0xc0, 0x31, 0x00, 0xc0, 0x60, 0x00, 0x40, 0x60, 0x00, 0xc0, 0x60, 0x00,
   0x40, 0x60, 0x00, 0xc0, 0x60, 0x00, 0x40, 0x60, 0x00, 0xc0, 0x60, 0x00,
   0x40, 0x7c, 0x00, 0xc0, 0x74, 0x00, 0x40, 0x60, 0x00, 0xc0, 0x60, 0x00,
   0x40, 0x7c, 0x00, 0xc0, 0x78, 0x00, 0x40, 0x60, 0x00, 0xc0, 0x60, 0x00,
   0x40, 0x7c, 0x00, 0xc0, 0x7c, 0x00, 0x40, 0x60, 0x00, 0xc0, 0x60, 0x00,
   0x40, 0x60, 0x00, 0xc0, 0x66, 0x00, 0x40, 0x66, 0x00, 0xc0, 0x66, 0x00,
   0x40, 0x66, 0x00, 0xc0, 0x66, 0x00, 0x40, 0x66, 0x00, 0xe0, 0x66, 0x00,
   0x70, 0xc6, 0x00, 0x30, 0x8e, 0x01, 0xb0, 0x9f, 0x01, 0x98, 0x3f, 0x01,
   0x98, 0xbf, 0x03, 0x98, 0x3f, 0x01, 0xb0, 0x9f, 0x01, 0x30, 0x8f, 0x01,
   0x70, 0xc0, 0x00, 0xe0, 0x71, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x0a, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define timer_width 27
#define timer_height 46
static const char PROGMEM  timer_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xf8, 0xff, 0xff, 0x00, 0xfc, 0xff, 0xff, 0x01, 0xfc, 0xff, 0xff, 0x01,
   0xfc, 0xff, 0xff, 0x01, 0xf8, 0xff, 0xff, 0x00, 0x30, 0x00, 0x60, 0x00,
   0x10, 0x00, 0x60, 0x00, 0x30, 0x00, 0x40, 0x00, 0x10, 0x00, 0x60, 0x00,
   0x30, 0x00, 0x60, 0x00, 0x10, 0x00, 0x40, 0x00, 0x30, 0xff, 0x67, 0x00,
   0x30, 0xfe, 0x63, 0x00, 0x20, 0xfe, 0x23, 0x00, 0x60, 0xf8, 0x30, 0x00,
   0xc0, 0x70, 0x18, 0x00, 0x80, 0x21, 0x0c, 0x00, 0x00, 0x03, 0x06, 0x00,
   0x00, 0x8e, 0x03, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00,
   0x00, 0xac, 0x01, 0x00, 0x00, 0x07, 0x03, 0x00, 0x80, 0x01, 0x0e, 0x00,
   0x80, 0x61, 0x18, 0x00, 0xc0, 0xf8, 0x10, 0x00, 0x60, 0xfc, 0x31, 0x00,
   0x30, 0xfe, 0x23, 0x00, 0x30, 0xff, 0x67, 0x00, 0x10, 0xff, 0x67, 0x00,
   0x10, 0xff, 0x47, 0x00, 0x30, 0xff, 0x67, 0x00, 0x10, 0xff, 0x67, 0x00,
   0x30, 0x00, 0x40, 0x00, 0x10, 0x00, 0x60, 0x00, 0xf0, 0x7b, 0x6f, 0x00,
   0xfc, 0xff, 0xff, 0x01, 0xfc, 0xff, 0xff, 0x01, 0xfc, 0xff, 0xff, 0x01,
   0xfc, 0xff, 0xff, 0x01, 0xf0, 0xde, 0x7b, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00 };

#define info_width 27
#define info_height 47
static const char PROGMEM info_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0xfc, 0x01, 0x00,
   0x00, 0xfe, 0x03, 0x00, 0x00, 0xff, 0x03, 0x00, 0x00, 0xff, 0x07, 0x00,
   0x00, 0xff, 0x07, 0x00, 0x80, 0xff, 0x07, 0x00, 0x00, 0xff, 0x07, 0x00,
   0x00, 0xff, 0x07, 0x00, 0x00, 0xff, 0x07, 0x00, 0x00, 0xfe, 0x03, 0x00,
   0x00, 0xfc, 0x01, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xe8, 0xff, 0x07, 0x00, 0xf8, 0xff, 0x07, 0x00, 0xf8, 0xff, 0x07, 0x00,
   0x00, 0xff, 0x07, 0x00, 0x00, 0xff, 0x07, 0x00, 0x00, 0xfe, 0x07, 0x00,
   0x00, 0xff, 0x07, 0x00, 0x00, 0xfe, 0x07, 0x00, 0x00, 0xff, 0x07, 0x00,
   0x00, 0xfe, 0x07, 0x00, 0x00, 0xff, 0x07, 0x00, 0x00, 0xfe, 0x07, 0x00,
   0x00, 0xff, 0x07, 0x00, 0x00, 0xfe, 0x07, 0x00, 0x00, 0xff, 0x07, 0x00,
   0x00, 0xfe, 0x07, 0x00, 0x00, 0xff, 0x07, 0x00, 0x00, 0xfe, 0x07, 0x00,
   0x00, 0xff, 0x07, 0x00, 0x00, 0xfe, 0x07, 0x00, 0x00, 0xff, 0x07, 0x00,
   0x00, 0xfe, 0x07, 0x00, 0x00, 0xff, 0x07, 0x00, 0x00, 0xff, 0x0f, 0x00,
   0xf0, 0xff, 0xff, 0x00, 0xf0, 0xff, 0xff, 0x00, 0xf8, 0xff, 0xff, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


SSD1306  display(0x3c, D2, D1);

unsigned long lastActive = 0;

void ICACHE_FLASH_ATTR displayWake() {
  lastActive = millis();
}

//void ICACHE_FLASH_ATTR statusScreen(String msg, int = 0, int = 0);
void ICACHE_FLASH_ATTR statusBar(bool elemOn, int currentTemp) {
  display.clear();  
  display.setFont(ArialMT_Plain_16);
  if (elemOn) display.fillRect(0, 0, 128, 16);
  if (elemOn) display.setColor(INVERSE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, (elemOn) ? "ON" : "OFF");
  
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  asctime_time(&timeinfo, timestamp);
  display.drawString(40, 0, timestamp);
  
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 0, String(currentTemp) + "°C");
}

void ICACHE_FLASH_ATTR initScreen() {
  display.init();
  display.clear();
  display.flipScreenVertically();  
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 15, "Initialising...");
}
void ICACHE_FLASH_ATTR statusScreen(String msg, int currentTemp, int setTemp) {
  display.clear();  
  /*display.setFont(ArialMT_Plain_16);
  //display.fillRect(0, 0, 128, 16);
  //display.setColor(INVERSE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "ON/OFF");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 0, String(currentTemp) + " °C");*/
  statusBar(output, current_temp);

  //display.drawXbm(0, 22, temp_width, temp_height, timer_bits);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setColor(WHITE);
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 24, msg);
  if (setTemp > 0) {
    if (currentTemp <= setTemp) {
      display.drawProgressBar(0, 50, 120, 10, (int)(((float)currentTemp/(float)setTemp)*100));
    } else {
      display.drawProgressBar(0, 50, 120, 10, 100);
    }
  }
  display.display();
  lastActive = millis();
}
void ICACHE_FLASH_ATTR statusScreen(String msg) {
  display.clear();  
  statusBar(output, current_temp);
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 24, msg);
  display.display();
  lastActive = millis();
}

void ICACHE_FLASH_ATTR setTempScreen(String temp) {
  display.clear(); 
  statusBar(output, current_temp);  
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  //display.setColor(WHITE);
  display.drawXbm(0, 18, temp_width, temp_height, temp_bits);
  display.drawString(30, 24, "Set Temp");
  display.drawString(50, 40, temp + " °C");
  display.display();
}

void ICACHE_FLASH_ATTR setTimeScreen(String sTime) {
  display.clear(); 
  statusBar(output, current_temp);  
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  //display.setColor(WHITE);
  display.drawXbm(0, 18, timer_width, timer_height, timer_bits);
  display.drawString(30, 24, "Set Timer");
  display.drawString(50, 40, sTime);
  display.display();  
}

void ICACHE_FLASH_ATTR progressScreen(String msg, int percent) {   
  lastActive = millis();
  display.clear();
  // draw the percentage as String
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 10, msg);
  display.drawString(64, 30, String(percent) + "%");
  display.drawProgressBar(0, 50, 120, 10, percent);
  display.display();
}

void ICACHE_FLASH_ATTR idleScreen() {
  display.clear();
  if ((millis() - lastActive < 300000) && (lastActive > 0)) {      
    display.clear(); 
    statusBar(output, current_temp);  
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    //display.setColor(WHITE);
    display.drawXbm(0, 18, info_width, info_height, info_bits);
    display.drawString(28, 24, String(current_temp) + "/" + String(set_temp) + " °C");
    display.drawString(28, 40, (set_time > 0) ? String(set_time) + " m" : "Off");
  }
  display.display();
}
void ICACHE_FLASH_ATTR idleScreen(String msg) {
  display.clear();
  if ((millis() - lastActive < 60000) && (lastActive > 0)) {      
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 20, msg);    
  }
  display.display();
}

#else
// Just fill with empty stubs if screen not used
void initScreen() {}
void statusScreen(String msg, int currentTemp, int setTemp) {}
void idleScreen() {}
void readTagScreen(String msg, int percent) {}
#endif
