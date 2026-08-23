#include "SensorManager.h"

/*
  MH-Z19 Sensor Implementation
*/

MHZ19Sensor::MHZ19Sensor(uint8_t rx_pin, uint8_t tx_pin)
  : _rx_pin(rx_pin), _tx_pin(tx_pin), _mhz19_serial(rx_pin, tx_pin) {}

void MHZ19Sensor::begin() {
  _mhz19_serial.begin(9600);
  _mhz.begin(_mhz19_serial);
  _mhz.autoCalibration();
}

uint16_t MHZ19Sensor::readCO2() {
  return _mhz.getCO2();
}

int MHZ19Sensor::readTemperature() {
  return _mhz.getTemperature();
}

/*
  Soil Moisture Sensor Implementation
*/

SoilSensor::SoilSensor(uint8_t pin) : _pin(pin) {}

void SoilSensor::begin() {
  pinMode(_pin, INPUT);
}

uint16_t SoilSensor::readMoisture() {
  return analogRead(_pin);
}

/*
  Sensor Manager Implementation
*/

SensorManager::SensorManager(MHZ19Sensor& mhz19Sensor, SoilSensor& soilSensor)
  : _mhz19Sensor(mhz19Sensor), _soilSensor(soilSensor) {}

void SensorManager::begin() {
  _mhz19Sensor.begin();
  _soilSensor.begin();
}

SensorData SensorManager::readAll() {
  SensorData data;
  data.co2  = _mhz19Sensor.readCO2();
  data.temp = _mhz19Sensor.readTemperature();
  data.hum = _soilSensor.readMoisture();
  return data;
}