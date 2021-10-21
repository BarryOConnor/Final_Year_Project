/*
  WeatherStation.h - Library for the automated weather station
  N0813926
*/
#ifndef WeatherSensor_h
#define WeatherSensor_h

#ifndef DEBUG
#define DEBUG true
#endif

#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include <Wire.h>
//#include <NeoSWSerial.h>
#include "WSSIM800L.h"


// DEFINITIONS
// Analog Pins
#define RAIN_SENSOR_PIN A0
#define WIND_SPEED_PIN A3
// Digital Pins
#define DS18B20_PIN 2
#define SIM_RESET_PIN 3
#define SIM_SLEEP_PIN 4
#define SIM_POWER_PIN 5
#define GPS_TX_PIN 6
#define GPS_RX_PIN 7
#define SIM_RX_PIN 8 // AltSoftSerial needs 8 & 9 for sommunication
#define SIM_TX_PIN 9
#define SENSORS_POWER_PIN 12
#define NEO6MGPS_POWER_PIN 13
// other
#define BME280_CHANNEL (0x76)

// constants
#define SEALEVELPRESSURE_HPA 1013.25
#define FIVE_MINS 300
#define SERIAL_MAX_LENGTH 40


// slight adjustment needed for external temp as it is a little low when compared to a mercury thermometer
const float ext_adjustment = 1.5;


struct WeatherValues{
  // arduino sprintf sunction doesn't include the %f identifier so we must use character representations of the float values
  char temp_external[10] = "0.00";
  char temp_internal[10] = "0.00";
  char pressure[10] = "0.00";
  char humidity[10] = "0.00";
  uint16_t windspeed = 0;
  uint16_t rain = 0;
};


class WeatherSensor
{
  private: 
    uint16_t highest_wind = 0;
    
    // DS18B20
    OneWire* DS18B20TempSensor;  
    DallasTemperature* DS18B20ExternalTemp;
    // BME280 
    Adafruit_BME280 BME280; // I2C no softwareserial needed
    //SIM800L
    WSSIM800L* sim800l;
    //NEO6MGPS
    //NeoSWSerial* NEO6MGPS_Serial;  

  public:
    static const uint8_t ID_LENGTH = 8;
    static const uint8_t MOBILE_LENGTH = 12;
    struct WeatherValues currentValues;
    WeatherSensor();
    void begin(uint16_t baudrate);

    bool validateRegData(const char* mobile, const char* id, const char* apn);
    bool setupWeatherSensor(const char* mobile, const char* id, const char* apn);
    const char* getMobile();
    const char* getId();
    const char* getApn();
    void readSensors();

    WeatherValues getCurrentValues();

    void powerToSensor(bool power, uint8_t sensor);

    // Windspeed
    void updateWindspeed();
    // EEPROM
    const char* EEPROM_read(uint16_t offset, uint16_t = 0);
    bool EEPROM_write(const char *content, uint16_t offset);
    // SIM800L
    bool SIM800L_SendSMS(const char *message);
    const char* SIM800L_HTTPGet(const char* url);
    // BME280
    bool BME280_IsPresent();
    // DS18B20
    bool DS18B20_IsPresent();
};
#endif
