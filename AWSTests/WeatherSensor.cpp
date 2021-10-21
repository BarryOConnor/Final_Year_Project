/*
  WeatherStation.h - Library for the automated weather station
  N0813926
*/
#include "WeatherSensor.h"


WeatherSensor::WeatherSensor() {
  /*  Constructor for the class, empty by design - all of 
  *  the setup happens in the begin() method so that
  *  the board can power up before the class initialises 
  *  pins otherwise issues happen
  */ 
}

void WeatherSensor::begin(uint16_t baudrate){
  Serial.println(F("Starting..."));
  pinMode(SIM_POWER_PIN, OUTPUT);
  powerToSensor(true, SIM_POWER_PIN);
  pinMode(SIM_SLEEP_PIN, OUTPUT);
  powerToSensor(false, SIM_SLEEP_PIN);
  pinMode(SIM_RESET_PIN, OUTPUT);
  powerToSensor(false, SIM_RESET_PIN);
  pinMode(SENSORS_POWER_PIN, OUTPUT);
  powerToSensor(true, SENSORS_POWER_PIN);
  pinMode(NEO6MGPS_POWER_PIN, OUTPUT);
  powerToSensor(false, NEO6MGPS_POWER_PIN);

  sim800l = new WSSIM800L(baudrate);

  DS18B20TempSensor = new OneWire(DS18B20_PIN);  
  DS18B20ExternalTemp = new DallasTemperature(DS18B20TempSensor);
  //NEO6MGPS_Serial = new NeoSWSerial(GPS_RX_PIN, GPS_RX_PIN);
  //NEO6MGPS_Serial->begin(DEFAULT_BAUD);
  delay(1000);
}



bool WeatherSensor::validateRegData(const char* mobile, const char* id, const char* apn) {
  /*  Validates registration data. 
  *  PARAMETERS
  *  String mobile - contact mobile number (used for SMS)
  *  String id - id for the device
  *  String apn - network APN for the SIM card
  *  RETURN
  *  boolean indicating whether the operation was successful
  */
  if (strlen(mobile) != WeatherSensor::MOBILE_LENGTH) {
    #if DEBUG
      Serial.println(F("Weather Station:: ERROR:- mobile number must be: ZZXXXXXXXXXXXX where ZZ is the country code and XXXXXXXXXXXX is the mobile number!"));
    #endif
    return false;
  } else if (strlen(id) != WeatherSensor::ID_LENGTH) {
    #if DEBUG
      Serial.println(F("Weather Station:: ERROR:- ID must be 8 characters long"));
    #endif
    return false;
  } else if (apn == "") {
    #if DEBUG
      Serial.println(F("Weather Station:: ERROR:- APN cannot be blank"));
    #endif
    return false;
  }

  for (int i = 0; i < WeatherSensor::MOBILE_LENGTH; i ++){
    if (!isDigit(mobile[i])){
      #if DEBUG
        Serial.println(F("Weather Station:: ERROR:- mobile number can only contain digits!"));
      #endif
      return false;
    }
  }
  return true;
}

bool WeatherSensor::setupWeatherSensor(const char* mobile, const char* id, const char* apn) {
  if (!validateRegData(mobile, id, apn)) {
    return false;
  }
  
  EEPROM_write(mobile, 0);
  EEPROM_write(id, WeatherSensor::MOBILE_LENGTH);
  EEPROM.update(WeatherSensor::MOBILE_LENGTH + WeatherSensor::ID_LENGTH, strlen(apn));
  EEPROM_write(apn, WeatherSensor::MOBILE_LENGTH + WeatherSensor::ID_LENGTH + 1);
  //write to eeprom and register on the website
  return true;
}

const char* WeatherSensor::getMobile(){
  return EEPROM_read(0, MOBILE_LENGTH);
}

const char* WeatherSensor::getId(){
  return EEPROM_read(MOBILE_LENGTH, ID_LENGTH);
}

const char* WeatherSensor::getApn(){
  return EEPROM_read(WeatherSensor::MOBILE_LENGTH + WeatherSensor::ID_LENGTH + 1, EEPROM.read(WeatherSensor::MOBILE_LENGTH + WeatherSensor::ID_LENGTH));
}

WeatherValues WeatherSensor::getCurrentValues(){
  /*  returns an object with the current sensor values
  */
  return currentValues;
}

void WeatherSensor::readSensors(){
  /*  turns on the power for sensors and reads data into 
  *  the currentValues struct
  */

  // turn on the power to the sensors 
  powerToSensor(true, SENSORS_POWER_PIN);

  delay(1000); // delay for startup

  // read data from BME280
  if (BME280_IsPresent()){
    dtostrf(BME280.readTemperature(), 4, 2, currentValues.temp_internal);    
    dtostrf(BME280.readPressure() / 100, 4, 2, currentValues.pressure);
    dtostrf(BME280.readHumidity(), 4, 2, currentValues.humidity); 

    // safe temperatures for the arduino are between -40 degrees C and 85 degrees C
    if (BME280.readTemperature() > 75.00 || BME280.readTemperature() < -35.00) {
      char sms_message[160];
      sprintf(sms_message, "Internal Temperature is %s degrees C. Damage to the electronics may occur above 85 degrees C or below -45 degrees C.", currentValues.temp_internal);
      SIM800L_SendSMS(sms_message);
    }
  }

  // read data from DS18B20
  if (DS18B20_IsPresent()){
    DS18B20ExternalTemp->requestTemperatures();
    dtostrf(DS18B20ExternalTemp->getTempCByIndex(0) + ext_adjustment, 4, 2, currentValues.temp_external);
  }

  // read the analog value from the rain sensor
  currentValues.rain = analogRead(RAIN_SENSOR_PIN);

  // transfer highest windspeed and reset to 0 for next interval
  currentValues.windspeed = highest_wind;
  highest_wind = 0;

  #if DEBUG
      Serial.print(F("Internal Temp: "));
      Serial.println(currentValues.temp_internal);      
      Serial.print(F("Humidity: "));
      Serial.println(currentValues.humidity);
      Serial.print(F("Pressure: "));
      Serial.println(currentValues.pressure);
      Serial.print(F("External Temp: "));
      Serial.println(currentValues.temp_external);
      Serial.print(F("Rain: "));
      Serial.println(currentValues.rain);
      Serial.print(F("Wind: "));
      Serial.println(currentValues.windspeed);
  #endif


  // all done, turn the power off again
  powerToSensor(false, SENSORS_POWER_PIN);  
  return;
}

void WeatherSensor::updateWindspeed(){
  /*  Wind Speed changes a lot so this function is used to 
  *  store a highest value over the interval so that we can
  *  record this rather than whatever the current reading is
  */
  uint16_t newWindSpeed = analogRead(WIND_SPEED_PIN);
  if (newWindSpeed > highest_wind){
    highest_wind = newWindSpeed;
  }
  return;
}

void WeatherSensor::powerToSensor(bool power, uint8_t sensor){
  /*  Arduino pins can provide a voltage depending on whether
  *  the output from the pin is HIGH or LOW allowing us to switch
  *  the power on and off. 
  *  PARAMETERS
  *  bool power - true = power on, false = power off
  *  int sensor - the pin number of the sensor pin
  */
  
  if(sensor == SIM_POWER_PIN || sensor == SIM_RESET_PIN){
    // invert the signal for these signals
    power = !power;
  }

  if(power){
    // turn on power
    digitalWrite(sensor, HIGH);
  } else {
    // turn off power
    digitalWrite(sensor, LOW);
  } 
      
  return;
}

/*  ==========================
 *   
 *       EEPROM FUNCTIONS
 *       
 *  ========================== */
const char* WeatherSensor::EEPROM_read(uint16_t offset, uint16_t length) {
  char returnValue[length+1];

  // make sure that we're not reading past the end of the eeprom
  if(offset + length > EEPROM.length()) {
    #if DEBUG
      Serial.println(F("Weather Station:: ERROR:- cannot read past the end of memory!"));
    #endif
    return "";
  } else {
    if (length == 0){
      // needed for apn which isn't fixed length
      length = EEPROM.read(offset);
      offset++;
    } 
    for (uint8_t address = 0; address < length; address++) {
      returnValue[address] = char(EEPROM.read(offset + address));
    }
    returnValue[length] = '\0';
  }  
  return strdup(&returnValue[0]);
}


bool WeatherSensor::EEPROM_write(const char *content, uint16_t offset) {
  if(content == "") {
    #if DEBUG
      Serial.println(F("Weather Station - ERROR:- nothing to write!"));
    #endif
    return false;
  } else if(offset + strlen(content) > EEPROM.length()) {
    // make sure that we're not reading past the end of the eeprom
    #if DEBUG
      Serial.println(F("Weather Station:: ERROR:- cannot write past the end of memory!"));
    #endif
    return false;
  }

  for (uint8_t address = 0; address < strlen(content); address++) {
    EEPROM.update(offset + address, content[address]);
  }
  return true;
}


/*  ==========================
 *   
 *       SIM800L FUNCTIONS
 *       
 *  ========================== */


bool WeatherSensor::SIM800L_SendSMS(const char* message) {
  
  powerToSensor(true, SIM_POWER_PIN);
  sim800l->reset();
  bool returnvalue = sim800l->sendSMS(getMobile(), message);
  powerToSensor(false, SIM_POWER_PIN);

  return returnvalue;
}


const char* WeatherSensor::SIM800L_HTTPGet(const char* url) {

  powerToSensor(true, SIM_POWER_PIN);
  sim800l->reset();
  const char* returnvalue = sim800l->httpGet(getApn(), url);
  powerToSensor(false, SIM_POWER_PIN);

  return returnvalue;
}



/*  ==========================
 *   
 *       BME280 FUNCTIONS
 *       
 *  ========================== */


bool WeatherSensor::BME280_IsPresent(){
  if (!BME280.begin(BME280_CHANNEL)) {
    Serial.println(F("BME280 sensor not found!"));
    return false;
  }
  return true;
}

/*  ==========================
 *   
 *       BME280 FUNCTIONS
 *       
 *  ========================== */


bool WeatherSensor::DS18B20_IsPresent(){
  DS18B20ExternalTemp->begin();
  if (DS18B20ExternalTemp->getDeviceCount() < 1){
    Serial.println(F("DS18B20 sensor not found!"));
    return false;
  }
  return true;
}  
