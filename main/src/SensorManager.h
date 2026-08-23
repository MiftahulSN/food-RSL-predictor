#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <MHZ19.h>
#include <SoftwareSerial.h>
#include "Types.h"

/*
  Base Class
*/
class BaseSensor {
  public:
    virtual void begin() = 0;
    virtual ~BaseSensor() {}
};

/*
  MH-Z19 Sensor
*/
class MHZ19Sensor : public BaseSensor {
  private:
    uint8_t _rx_pin;
    uint8_t _tx_pin;
    SoftwareSerial _mhz19_serial;
    MHZ19 _mhz;

  public:
    MHZ19Sensor(uint8_t rx_pin, uint8_t tx_pin);
    void begin() override;
    uint16_t readCO2();
    int readTemperature();
};

/*
  Soil Moisture Sensor 
*/
class SoilSensor : public BaseSensor {
  private:
    uint8_t _pin;

  public:
    SoilSensor(uint8_t pin);
    void begin() override;
    uint16_t readMoisture();
};

/*
  Sensor Manager
*/
class SensorManager {
  private:
    MHZ19Sensor& _mhz19Sensor;
    SoilSensor&  _soilSensor;

  public:
    SensorManager(MHZ19Sensor& mhz19Sensor, SoilSensor& soilSensor);
    void begin();
    SensorData readAll();
};

#endif