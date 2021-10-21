#include "WeatherSensor.h"

#define DEBUG true
#define DEFAULT_BAUD 9600
#define DEBUG_BAUD 115200

WeatherSensor ws;
unsigned int reading_interval = 5 * 60 * 1000;      // number of minutes between data readings
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(DEBUG_BAUD);
  while (!Serial) {}
  ws.begin(DEFAULT_BAUD);
  performAction(); 
}


void loop() {
  unsigned long currentMillis = millis();
   
 
  // work out whether enough time has passed to take another set of readings
  if (((currentMillis - previousMillis)) >= reading_interval) {
    performAction();
    // Use the snapshot to set track time until next event
    previousMillis = currentMillis;
   }
}

void performAction(){
    char url[130];
    const char* result;

    Serial.println(F("Reading Sensors"));
    ws.readSensors();

    WeatherValues current = ws.getCurrentValues();
    
    sprintf_P(url, PSTR("http://weather.barryoconnor.co.uk/p.php?d=%s&e=%s&i=%s&p=%s&h=%s&w=%u&r=%u"), ws.getId(), current.temp_external, current.temp_internal, current.pressure, current.humidity, current.windspeed, current.rain);
    
    #if DEBUG
      Serial.println(url);
    #endif

    if(ws.SIM800L_HTTPGet(url) == "100") {
      ws.SIM800L_SendSMS("An attempt to store data from an unregistered device was made");
    };
}
