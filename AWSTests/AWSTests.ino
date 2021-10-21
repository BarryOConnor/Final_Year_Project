#include "WeatherSensor.h"
WeatherSensor ws;
uint8_t passed = 0;
uint8_t failed = 0;
bool interrupt_fired = false;


#define THIRTY_SECONDS 30

void assertIsTrue(bool comparison){
  if(comparison){
    Serial.println(F(": TEST PASSED"));
    passed ++;
  } else {
    Serial.println(F(": TEST FAILED"));
    failed ++;
  }
  return;
}

void assertIsFalse(bool comparison){
  if(!comparison){    
    Serial.println(F(": TEST PASSED"));
    passed ++;
  } else {
    Serial.println(F(": TEST FAILED"));
    failed ++;
  }
  return;
}

template <typename Template> void assertIsEqual(Template comparison1, Template comparison2){
  if(comparison1 == comparison2){    
    Serial.println(F(": TEST PASSED"));
    passed ++;
  } else {
    Serial.println(F(": TEST FAILED"));
    failed ++;
  }
  return;
}

void assertStringIsEqual(const char * comparison1, const char * comparison2){
  if(strcmp(comparison1,comparison2) == 0){    
    Serial.println(F(": TEST PASSED"));
    passed ++;
  } else {
    Serial.println(F(": TEST FAILED"));
    failed ++;
  }
  return;
}

void assertIsEqual(int comparison1, int comparison2){
  if(comparison1 == comparison2){    
    Serial.println(F(": TEST PASSED"));
    passed ++;
  } else {
    Serial.println(F(": TEST FAILED"));
    failed ++;
  }
  return;
}

uint8_t countStructMatches(WeatherValues comparison1, WeatherValues comparison2){
  uint8_t matches = 0;
  if (comparison1.humidity == comparison2.humidity) matches++;
  if (comparison1.pressure == comparison2.pressure) matches++;
  if (comparison1.rain == comparison2.rain) matches++;
  if (comparison1.temp_external == comparison2.temp_external) matches++;
  if (comparison1.temp_internal == comparison2.temp_internal) matches++;
  if (comparison1.windspeed == comparison2.windspeed) matches++;
  return matches;
}

uint8_t countStructMatches(WeatherValues comparison1){
  WeatherValues comparison2;
  return countStructMatches(comparison1, comparison2);
}


/* ========== Test setupValidation ========== */

 //testing Setup validation
void testMobileEmpty() {
  Serial.print(F("testMobileEmpty"));
  assertIsFalse(ws.validateRegData("", "id_value", "someapnvalue"));
}

void testMobilePlus() {
  Serial.print(F("testMobilePlus"));
  assertIsFalse(ws.validateRegData("+01234567891", "id_value", "someapnvalue"));
}

void testMobileTooShort() {
  Serial.print(F("testMobileTooShort"));
  assertIsFalse(ws.validateRegData("01234", "id_value", "someapnvalue"));
}

void testMobileTooLong() {
  Serial.print(F("testMobileTooLong"));
  assertIsFalse(ws.validateRegData("012341111111111111", "id_value", "someapnvalue"));
}

// id tests
void testIDEmpty() {
  Serial.print(F("testIDEmpty"));
  assertIsFalse(ws.validateRegData("447860444555", "", "someapnvalue"));
}

void testIDTooShort() {
  Serial.print(F("testIDTooShort"));
  assertIsFalse(ws.validateRegData("447860444555", "id23", "someapnvalue"));
}

void testIDTooLong() {
  Serial.print(F("testIDTooLong"));
  assertIsFalse(ws.validateRegData("447860444555", "id_value_too_long44444", "someapnvalue"));
}

// apn tests
void testAPNEmpty() {
  Serial.print(F("testAPNEmpty"));
  assertIsFalse(ws.validateRegData("447860444555", "id_value", ""));
}

void testAPNShort() {
  Serial.print(F("testAPNShort"));
  assertIsTrue(ws.validateRegData("447860444555", "id_value", "s"));
}

void testAPNLong() {
  Serial.print(F("testAPNLong"));
  assertIsTrue(ws.validateRegData("447860444555", "id_value", "someapnvaluesomeapnvalue"));
}

// everything correct
void testEverythingValid() {
  Serial.print(F("testEverythingValid"));
  assertIsTrue(ws.validateRegData("447860444555", "id_value", "someapnvalue"));
} 

/* ========== Test Startup functions ========== */
void testSetupAndRetrieve() {

  Serial.print(F("testSetupAndRetrieve"));
  if(ws.setupWeatherSensor("447860497508", "AWS-P001", "wap.vodafone.co.uk")){

    if(strcmp(ws.getMobile(), "447860497508") == 0 && strcmp(ws.getId(), "AWS-P001") == 0 && strcmp(ws.getApn(), "wap.vodafone.co.uk") == 0 ){
      Serial.println(F(": TEST PASSED"));
      passed ++;
      return;
    } 
  }
  Serial.println(F(": TEST FAILED"));
  failed ++;
}

/* ========== Test EEPROM functions ========== */

void testEEPROMReadPastEnd() {
  Serial.print(F("testEEPROMReadPastEnd"));
  assertStringIsEqual(ws.EEPROM_read(1020, 200),"");
}

void testEEPROMWritePastEnd() {
  Serial.print(F("testEEPROMWritePastEnd"));
  assertIsFalse(ws.EEPROM_write("1234567890", 1020));
}

void testEEPROMWriteEmpty() {
  Serial.print(F("testEEPROMWriteEmpty"));
  assertIsFalse(ws.EEPROM_write("", 0));
}

void testEEPROMWrite() {
  Serial.print(F("testEEPROMWrite"));
  assertIsTrue(ws.EEPROM_write("1234567890", 0));
}

void testEEPROMRead() {
  Serial.print(F("testEEPROMRead"));
  assertStringIsEqual(ws.EEPROM_read(0, 10), "1234567890");
}

void testEEPROMWriteRead() {
  Serial.print(F("testEEPROMWriteRead"));
  ws.EEPROM_write("X", 0);
  assertStringIsEqual(ws.EEPROM_read(0,1), "X");
} 



/* ========== Test SMS Send ========== */

void testSMSSend() {  
  //ws.powerToSensor(true, SIM_POWER_PIN);
  Serial.print(F("testSMSSend"));
  assertIsTrue(ws.SIM800L_SendSMS("Testing 1234567890"));
}

void testHTTPGet() {
  //ws.powerToSensor(true, SIM_POWER_PIN);
  Serial.print(F("testHTTPGet"));
  assertStringIsEqual(ws.SIM800L_HTTPGet("http://weather.barryoconnor.co.uk/test.php?text=This%20is%20a%20Test!"), "This is a Test!");
}


/* ========== Function Pins ========== */
void testSensors_Off() {
  Serial.print(F("testSensors_Off"));
  ws.powerToSensor(true, SENSORS_POWER_PIN);
  ws.powerToSensor(false, SENSORS_POWER_PIN);
  digitalRead(SENSORS_POWER_PIN);
  assertIsEqual(digitalRead(SENSORS_POWER_PIN), 0);
}

void testSensors_On() {
  Serial.print(F("testSensors_On"));
  ws.powerToSensor(true, SENSORS_POWER_PIN);
  Serial.print(digitalRead(SENSORS_POWER_PIN));
  assertIsEqual(digitalRead(SENSORS_POWER_PIN), 1);
  delay(1000);
  ws.powerToSensor(false, SENSORS_POWER_PIN);
}

void testNEO6MGPS_Off() {
  Serial.print(F("testNEO6MGPS_Off"));
  ws.powerToSensor(true, NEO6MGPS_POWER_PIN);
  ws.powerToSensor(false, NEO6MGPS_POWER_PIN);
  digitalRead(NEO6MGPS_POWER_PIN);
  assertIsEqual(digitalRead(NEO6MGPS_POWER_PIN), 0);
}

void testNEO6MGPS_On() {
  Serial.print(F("testNEO6MGPS_On"));
  ws.powerToSensor(true, NEO6MGPS_POWER_PIN);
  digitalRead(NEO6MGPS_POWER_PIN);
  assertIsEqual(digitalRead(NEO6MGPS_POWER_PIN), 1);
  ws.powerToSensor(false, NEO6MGPS_POWER_PIN);
}
 


void testSIM_Sleep() {
  Serial.print(F("testSIM_Sleep"));
  digitalRead(SIM_SLEEP_PIN);
  assertIsEqual(digitalRead(SIM_SLEEP_PIN), 0);
}

void testSIM_Wake() {
  Serial.print(F("testSIM_Wake"));
  ws.powerToSensor(true, SIM_SLEEP_PIN);
  digitalRead(SIM_SLEEP_PIN);
  assertIsEqual(digitalRead(SIM_SLEEP_PIN), 1);
  ws.powerToSensor(false, SIM_SLEEP_PIN);
}

/* opposite way around since we are sending a signal to the 
*  DC Buck Converter to turn on the power rather than managing the
*  power directly 
*/


void testSIM_Power_On() {
  Serial.print(F("testSIM_Power_On"));
  ws.powerToSensor(true, SIM_POWER_PIN);
  delay(1000);
  assertIsEqual(digitalRead(SIM_POWER_PIN), 1);
  //ws.powerToSensor(false, SIM_POWER_PIN);
}

void testSIM_Power_Off() {
  Serial.print(F("testSIM_Power_Off"));
  assertIsEqual(digitalRead(SIM_POWER_PIN), 0);
}


/* ========== Test BME280 ========== */
void testBME280_Present() {
  Serial.print(F("testBME280_Present"));
  ws.powerToSensor(true, SENSORS_POWER_PIN);
  delay(1000);
  assertIsTrue(ws.BME280_IsPresent());
  ws.powerToSensor(false, SENSORS_POWER_PIN);
}


/* ========== Test DS18B20 ========== */
void testDS18B20_Present() {
  Serial.print(F("testDS18B20_Present"));
  ws.powerToSensor(true, SENSORS_POWER_PIN);
  assertIsTrue(ws.DS18B20_IsPresent());
  ws.powerToSensor(false, SENSORS_POWER_PIN);
}

/* ========== Sensor Tests ========== */
void testSensorDataNotZero() {
  WeatherValues empty_values;
  ws.updateWindspeed();
  ws.readSensors();
  uint8_t matches = countStructMatches(ws.getCurrentValues());

  if (matches == 0) {
    Serial.println(F("testSensorDataNotZero: TEST PASSED"));
    passed ++;
  } else {
    Serial.println(F("testSensorDataNotZero: TEST FAILED"));
    failed ++;
  }

}

void testSensorDataInRange() {
  ws.updateWindspeed();
  ws.readSensors();
  WeatherValues curr_values = ws.getCurrentValues();  
  if (curr_values.temp_internal >= -55.00 && curr_values.temp_internal <= 125.00
      && curr_values.temp_internal >= -40.00 && curr_values.temp_internal <= 85.00 
      && curr_values.humidity >= 0.00 && curr_values.humidity <= 100.00
      && curr_values.pressure >= 300.00 && curr_values.pressure <= 1100.00
      && curr_values.rain >= 0 && curr_values.pressure <= 1024
      && curr_values.windspeed >= 0 && curr_values.windspeed <= 1024){
    Serial.println(F("testSensorDataInRange: TEST PASSED"));
    passed ++;
  } else {
    Serial.println(F("testSensorDataInRange: TEST FAILED"));
    failed ++;
  }
} 


void setup() {
  Serial.begin(DEFAULT_BAUD);
  while (!Serial) {}
  ws.begin();
  
  Serial.println(F("\nTesting data validation"));
  testMobileEmpty();
  testMobilePlus();
  testMobileTooShort();
  testMobileTooLong();
  testIDEmpty();
  testIDTooShort();
  testIDTooLong();
  testAPNEmpty();
  testAPNShort();
  testAPNLong();
  testEverythingValid();

  Serial.println(F("\nTesting EEPROM functionality"));
  testEEPROMReadPastEnd();
  testEEPROMWritePastEnd();
  testEEPROMWriteEmpty();
  testEEPROMWrite();
  testEEPROMRead();
  testEEPROMWriteRead();

  Serial.println(F("\nTesting Setup data storage and retrieval"));
  testSetupAndRetrieve();

  Serial.println(F("\nTesting functional pins (power etc)"));
  testSensors_Off();
  testSensors_On();
  testNEO6MGPS_Off();
  testNEO6MGPS_On();
  testSIM_Sleep();
  testSIM_Wake();
  testSIM_Power_On();
  testSIM_Power_Off();


  //Serial.println(F("\nTesting SMS & HTTP capability"));
  //testSMSSend();
  //testHTTPGet();
  

  Serial.println(F("\nTesting presence of BME280 and DS18B20 Chips"));
  testBME280_Present();
  testDS18B20_Present();


 Serial.println(F("\nTesting sensor data validation"));
 testSensorDataNotZero();
 testSensorDataInRange(); 



  Serial.print(F("\nTotal Tests: "));
  Serial.println(failed + passed);
  Serial.print(F("Passed: "));
  Serial.println(passed);
  Serial.print(F("Failed: "));
  Serial.println(failed);
}

void loop() {

}
