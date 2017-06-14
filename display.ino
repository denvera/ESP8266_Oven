#ifdef USE_SSD1306
#include <brzo_i2c.h>
//#include <Wire.h> 
#include "SSD1306.h"
#warning "Display Enabled"

SSD1306  display(0x3c, D2, D1);

unsigned long lastActive = 0;

//void ICACHE_FLASH_ATTR statusScreen(String msg, int = 0, int = 0);

void ICACHE_FLASH_ATTR initScreen() {
  display.init();
  display.clear();
  display.flipScreenVertically();  
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 15, "Initialising...");
}
void ICACHE_FLASH_ATTR statusScreen(String msg, int currentTemp, int setTemp) {
  display.clear();  
  display.setFont(ArialMT_Plain_16);
  //display.fillRect(0, 0, 128, 16);
  //display.setColor(INVERSE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "ON/OFF");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 0, String(currentTemp) + " °C");
  
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setColor(WHITE);
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 20, msg);
  if (setTemp > 0)
    display.drawProgressBar(0, 50, 120, 10, (int)(((float)currentTemp/(float)setTemp)*100));
  display.display();
  lastActive = millis();
}
void ICACHE_FLASH_ATTR statusScreen(String msg) {
  display.clear();  
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 15, msg);
  display.display();
  lastActive = millis();
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
  if ((millis() - lastActive < 30000) && (lastActive > 0)) {      
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 20, "Idle");    
  }
  display.display();
}
void ICACHE_FLASH_ATTR idleScreen(String msg) {
  display.clear();
  if ((millis() - lastActive < 30000) && (lastActive > 0)) {      
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
