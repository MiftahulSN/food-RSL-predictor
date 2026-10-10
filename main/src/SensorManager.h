#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <DFRobot_SCD4X.h>
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
  MH-Z19 Sensor (Sensor Config 1)
*/
class MHZ19Sensor : public BaseSensor {
  private:
    uint8_t _rx_pin;
    uint8_t _tx_pin;
    uint8_t _pwm_pin;
    HardwareSerial _serial;
    uint8_t cmdChecksum(const uint8_t* cmd) const;

  public:
    MHZ19Sensor(uint8_t rx_pin, uint8_t tx_pin, uint8_t pwm_pin);
    void begin() override;
    int readCO2UART();
    int readCO2Pwm();
    void selfCalibration(bool on);
    void calibrateZero();
};

/*
  DHT22 Sensor (Sensor Config 1)
*/
class DHT22Sensor : public BaseSensor {
  private:
    DHT _dht;

  public:
    DHT22Sensor(uint8_t pin);
    void begin() override;
    float readTemperature();
    float readHumidity();
};

/*
  DFRobot SCD4X Calib Wrapper
    Exposes the protected performForcedRecalibration method.
*/
class SCD4XCalib : public DFRobot_SCD4X {
  public:
    using DFRobot_SCD4X::DFRobot_SCD4X;
    int16_t forcedRecalibration(uint16_t co2ppm) {
      return performForcedRecalibration(co2ppm);
    }
};

/*
  SCD41 Sensor (Sensor Config 2)
*/
class SCD41Sensor : public BaseSensor {
  private:
    uint8_t _sda_pin;
    uint8_t _scl_pin;
    SCD4XCalib _scd;
    bool _status;
    DFRobot_SCD4X::sSensorMeasurement_t _last;

  public:
    SCD41Sensor(uint8_t sda_pin, uint8_t scl_pin);
    void begin() override;
    bool read();
    uint16_t readCO2();
    float readTemperature();
    void selfCalibration(bool on);
    int16_t calibrateZero(uint16_t ref_ppm = 420);
};

/*
  Soil Moisture Sensor (Sensor Config 2)
*/
class SoilSensor : public BaseSensor {
  private:
    uint8_t _pin;
    uint16_t _adc_dry;
    uint16_t _adc_wet;

  public:
    SoilSensor(uint8_t pin, uint16_t adc_dry = 3200, uint16_t adc_wet = 1300);
    void begin() override;
    uint8_t readMoisture();
};

/*
  Battery Monitor (Both Configs)
*/
class BatteryMonitor : public BaseSensor {
  private:
    uint8_t _pin;
    float _divider_ratio;

  public:
    BatteryMonitor(uint8_t pin, float divider_ratio = 2.0f);
    void begin() override;
    float readVoltage();
    uint8_t readPercent();
};

/*
  Sensor Manager
*/
class SensorManager {
  private:
    MHZ19Sensor* _mhz19Sensor;
    DHT22Sensor* _dht22Sensor;
    SCD41Sensor* _scd41Sensor;
    SoilSensor*  _soilSensor;
    BatteryMonitor* _battMonitor;

  public:
    SensorManager(MHZ19Sensor* mhz19Sensor, DHT22Sensor* dht22Sensor,
                  SCD41Sensor* scd41Sensor, SoilSensor* soilSensor,
                  BatteryMonitor* battMonitor);
    void begin();
    SensorData readAll();
};

#endif
