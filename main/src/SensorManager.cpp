#include "SensorManager.h"

/*
  MH-Z19 Sensor Implementation
*/

MHZ19Sensor::MHZ19Sensor(uint8_t rx_pin, uint8_t tx_pin, uint8_t pwm_pin)
  : _rx_pin(rx_pin), _tx_pin(tx_pin), _pwm_pin(pwm_pin), _serial(1) {}

void MHZ19Sensor::begin() {
  _serial.begin(9600, SERIAL_8N1, _rx_pin, _tx_pin);
  pinMode(_pwm_pin, INPUT);
}

uint8_t MHZ19Sensor::cmdChecksum(const uint8_t* cmd) const {
  uint8_t sum = 0;
  for (uint8_t i = 1; i < 8; i++) {
    sum += cmd[i];
  }
  return (uint8_t)(0xFF - sum + 1);
}

int MHZ19Sensor::readCO2UART() {
  while (_serial.available() > 0) {
    _serial.read();
  }

  const uint8_t readCmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  _serial.write(readCmd, 9);
  delay(100);

  if (_serial.available() < 9) {
    return -1;
  }

  uint8_t response[9];
  _serial.readBytes(response, 9);

  if (response[0] != 0xFF || response[1] != 0x86) {
    return -1;
  }
  if (cmdChecksum(response) != response[8]) {
    return -1;
  }

  return (response[2] << 8) | response[3];
}

int MHZ19Sensor::readCO2Pwm() {
  unsigned long th_us = pulseIn(_pwm_pin, HIGH, 2000000);
  unsigned long tl_us = pulseIn(_pwm_pin, LOW, 2000000);
  if (th_us == 0 || tl_us == 0) return -1;

  float th = th_us / 1000.0f;
  float tl = tl_us / 1000.0f;
  return (int)(2000.0f * (th - 2.0f) / (th + tl - 4.0f));
}

void MHZ19Sensor::selfCalibration(bool on) {
  uint8_t cmd[9] = {0xFF, 0x01, 0x79, on ? 0xA0 : 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  cmd[8] = cmdChecksum(cmd);
  _serial.write(cmd, 9);
}

void MHZ19Sensor::calibrateZero() {
  uint8_t cmd[9] = {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  cmd[8] = cmdChecksum(cmd);
  _serial.write(cmd, 9);
  delay(100);
}

/*
  DHT22 Sensor Implementation
*/

DHT22Sensor::DHT22Sensor(uint8_t pin) : _dht(pin, DHT22) {}

void DHT22Sensor::begin() {
  _dht.begin();
}

float DHT22Sensor::readTemperature() {
  return _dht.readTemperature();
}

float DHT22Sensor::readHumidity() {
  return _dht.readHumidity();
}

/*
  SCD41 Sensor Implementation
*/

SCD41Sensor::SCD41Sensor(uint8_t sda_pin, uint8_t scl_pin)
  : _sda_pin(sda_pin), _scl_pin(scl_pin), _status(false)
{
  _last.CO2ppm = 0;
  _last.temp = 0.0f;
  _last.humidity = 0.0f;
}

void SCD41Sensor::begin() {
  Wire.begin(_sda_pin, _scl_pin);
  _status = _scd.begin();
  if (_status) {
    _scd.enablePeriodMeasure(SCD4X_START_PERIODIC_MEASURE);
  }
}

bool SCD41Sensor::read() {
  if (!_status) return false;
  if (!_scd.getDataReadyStatus()) return false;
  _scd.readMeasurement(&_last);
  return true;
}

uint16_t SCD41Sensor::readCO2() {
  return _last.CO2ppm;
}

float SCD41Sensor::readTemperature() {
  return _last.temp;
}

void SCD41Sensor::selfCalibration(bool on) {
  _scd.enablePeriodMeasure(SCD4X_STOP_PERIODIC_MEASURE);
  if (_scd.getAutoCalibMode() != on) {
    _scd.setAutoCalibMode(on);
    _scd.persistSettings();
  }
  _scd.enablePeriodMeasure(SCD4X_START_PERIODIC_MEASURE);
}

int16_t SCD41Sensor::calibrateZero(uint16_t ref_ppm) {
  // Returns FRC correction in ppm; 32767 (0xFFFF) means FRC failed.
  _scd.enablePeriodMeasure(SCD4X_STOP_PERIODIC_MEASURE);
  int16_t correction = _scd.forcedRecalibration(ref_ppm);
  _scd.enablePeriodMeasure(SCD4X_START_PERIODIC_MEASURE);
  return correction;
}

/*
  Soil Moisture Sensor Implementation
*/

SoilSensor::SoilSensor(uint8_t pin, uint16_t adc_dry, uint16_t adc_wet)
  : _pin(pin), _adc_dry(adc_dry), _adc_wet(adc_wet) {}

void SoilSensor::begin() {
  pinMode(_pin, INPUT);
}

uint8_t SoilSensor::readMoisture() {
  uint16_t raw = analogRead(_pin);
  float pct = (_adc_dry - raw) * 100.0f / (_adc_dry - _adc_wet);
  if (pct < 0.0f) pct = 0.0f;
  if (pct > 100.0f) pct = 100.0f;
  return (uint8_t)pct;
}

/*
  Battery Monitor Implementation
*/

BatteryMonitor::BatteryMonitor(uint8_t pin, float divider_ratio)
  : _pin(pin), _divider_ratio(divider_ratio) {}

void BatteryMonitor::begin() {
  pinMode(_pin, INPUT);
}

float BatteryMonitor::readVoltage() {
  return (analogReadMilliVolts(_pin) / 1000.0f) * _divider_ratio;
}

uint8_t BatteryMonitor::readPercent() {
  float v = readVoltage();
  float pct = (v - 3.0f) / (4.2f - 3.0f) * 100.0f;
  if (pct < 0.0f) pct = 0.0f;
  if (pct > 100.0f) pct = 100.0f;
  return (uint8_t)pct;
}

/*
  Sensor Manager Implementation
*/

SensorManager::SensorManager(MHZ19Sensor* mhz19Sensor, DHT22Sensor* dht22Sensor,
                             SCD41Sensor* scd41Sensor, SoilSensor* soilSensor,
                             BatteryMonitor* battMonitor)
  : _mhz19Sensor(mhz19Sensor), _dht22Sensor(dht22Sensor),
    _scd41Sensor(scd41Sensor), _soilSensor(soilSensor),
    _battMonitor(battMonitor) {}

void SensorManager::begin() {
  if (_mhz19Sensor) _mhz19Sensor->begin();
  if (_dht22Sensor) _dht22Sensor->begin();
  if (_scd41Sensor) _scd41Sensor->begin();
  if (_soilSensor)  _soilSensor->begin();
  if (_battMonitor) _battMonitor->begin();
}

SensorData SensorManager::readAll() {
  SensorData data;
  data.co2 = 0;
  data.temp = 0.0f;
  data.hum = 0.0f;
  data.batt_voltage = 0.0f;
  data.batt_percent = 0;

  if (_mhz19Sensor) {
    int co2 = _mhz19Sensor->readCO2UART();
    data.co2 = (co2 > 0) ? (uint16_t)co2 : 0;
  }
  if (_dht22Sensor) {
    float t = _dht22Sensor->readTemperature();
    float h = _dht22Sensor->readHumidity();
    data.temp = isnan(t) ? 0.0f : t;
    data.hum = isnan(h) ? 0.0f : h;
  }
  if (_scd41Sensor) {
    _scd41Sensor->read();
    data.co2 = _scd41Sensor->readCO2();
    data.temp = _scd41Sensor->readTemperature();
  }
  if (_soilSensor) {
    // [DESIGN] SENSOR_CONFIG 2: soil moisture intentionally replaces SCD41
    // air humidity as the humidity input for RSL prediction.
    data.hum = _soilSensor->readMoisture();
  }
  if (_battMonitor) {
    data.batt_voltage = _battMonitor->readVoltage();
    data.batt_percent = _battMonitor->readPercent();
  }
  return data;
}
