  const char html1[] PROGMEM =
"<html> \
  <head> \
    <script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script> \
    <script type=\"text/javascript\"> \
      google.charts.load('current', {'packages':['corechart']}); \
      google.charts.setOnLoadCallback(drawChart); \
      function drawChart() { \
        fetch('/temps').then((resp) => resp.json()).then(function(jsonData) { \
          console.log(JSON.stringify(jsonData)); \
          var data = new google.visualization.DataTable(); \
          data.addColumn('datetime', 'Time'); \
          data.addColumn('number', 'Temp \\xB0C'); \
          data.addColumn({type: 'boolean', role: 'certainty' }); \
          var chart_data  = jsonData['temps'].map(function(e) { return [new Date(e[0]), e[1], (e[3] > 0) ? true : false ] }); \
          data.addRows(chart_data); \
          var options = {'title':'Oven Temperature', colors: ['red'], pointsVisible: true }; \
          var chart = new google.visualization.LineChart(document.getElementById('temps')); \
          chart.draw(data, options); \
        }); \        
      }; \
    </script> \
    <title>ESP8266 Oven Controller</title> \
    <style> \
        body { \
            font-family: Arial, Helvetica, Sans-Serif; \
        } \
    </style> \
  </head> \
  <body> \
    <div id=\"temps\"></div> \
    <p align='center'>Uptime: ";


void ICACHE_FLASH_ATTR handleRoot() {
  char temp[48];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  sprintf(temp, "%02d:%02d:%02d", hr, min % 60, sec % 60);
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send ( 200, "text/html", "");

    
  const static char html2[] PROGMEM = "</p> \
  </body> \
</html>";
  server.sendContent(html1);
  server.sendContent(temp);
  server.sendContent(F("</p><p align='center'>"));
  sprintf(temp, "Free Heap: %u", ESP.getFreeHeap());
  server.sendContent(temp);
  server.sendContent(html2);
  server.sendContent("");
  
}
void ICACHE_FLASH_ATTR handleTemps() {
  String temps_json = F("{ \"temps\": [\r\n");
  int numTemps = RingBufNumElements(temps);
  server.sendHeader(F("Access-Control-Allow-Origin"), "*");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, F("text/json"), "");
  server.sendContent(temps_json);
  for (int i = 0; i < numTemps; i++) {
    struct TempSample *t = (struct TempSample *)RingBufPeek(temps, i);
    //t->temp = random(10,1000);
    if (t) {
      server.sendContent("\t[ \"" + String(t->timestamp) + "\", " \
                             + ((isnan(t->temp)) ? "null" : String(t->temp)) + ", " \
                             + String(t->set_temp) + ", " \
                             + String(t->output) + \
                             + " ]" + ((i<(numTemps-1)) ? "," : "") + "\r\n");
    }
  }
  server.sendContent("] }");
  server.sendContent("");
  //server.client().flush();
  server.client().stop();  
  temps_json = String();  
}
void ICACHE_FLASH_ATTR handleSetPID() {    
  if (server.hasArg("p"))
    Kp = server.arg("p").toFloat();
  if (server.hasArg("i"))
    Ki = server.arg("i").toFloat();
  if (server.hasArg("d"))
    Kd = server.arg("d").toFloat();  
  OvenPID.SetTunings(Kp, Ki, Kd);
  server.send(200, "text/plain", "");
  saveConfig();
}
void ICACHE_FLASH_ATTR handleGetPID() {
  server.send(200, "text/plain", "{ \"p\": \"" + String(Kp) + "\", \"i\": \"" + String(Ki) + "\", \"d\": \"" + String(Kd) + "\" }");
}
void ICACHE_FLASH_ATTR handleSetPrefs() {
  if (server.hasArg("controller")) {
    server.arg("controller").toUpperCase();
    if (server.arg("controller") == "PID") {
      current_controller = CTRL_PID;
    } else if (server.arg("controller") == "DUMB") {
      current_controller = CTRL_DUMB;
    }
  }  
  saveConfig();
  server.send(200, "text/plain", "");
}

void ICACHE_FLASH_ATTR handleSetOutput() {
  
}

void ICACHE_FLASH_ATTR handleSetTime() {
  
}


void ICACHE_FLASH_ATTR setupWebServer() {
  server.on ("/", handleRoot);
  server.on("/temps", handleTemps);
  server.on("/pid", HTTP_GET, handleGetPID);
  server.on("/pid", HTTP_POST, handleSetPID);
  server.on("/controller", HTTP_POST, handleSetPrefs);
}



