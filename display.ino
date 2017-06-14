#ifdef USE_SSD1306
#include <brzo_i2c.h>
//#include <Wire.h> 
#include "SSD1306.h"
#warning "Display Enabled"

SSD1306  display(0x3c, D2, D1);

unsigned long lastActive = 0;

void ICACHE_FLASH_ATTR statusScreen();

void ICACHE_FLASH_ATTR initScreen() {
  display.init();
  statusScreen("Initializing...");  
}
void ICACHE_FLASH_ATTR statusScreen(String msg) {
  display.clear();
  display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 20, msg);
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


#else
// Just fill with empty stubs if screen not used
void initScreen() {}
void statusScreen(String msg) {}
void idleScreen() {}
void readTagScreen(String msg, int percent) {}
#endif
