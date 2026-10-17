#include "StorageManager.h"

StorageManager::StorageManager(uint8_t cs_pin, uint8_t sck_pin, uint8_t mosi_pin, uint8_t miso_pin) {
  _cs_pin = cs_pin;
  _sck_pin = sck_pin;
  _mosi_pin = mosi_pin;
  _miso_pin = miso_pin;
  _status = false;
}

bool StorageManager::begin() {
  SPI.begin(_sck_pin, _miso_pin, _mosi_pin, _cs_pin);
  _status = SD.begin(_cs_pin);
  return _status;
}

bool StorageManager::saveData(const String& filename, const SensorData& sensor, const PredictionResult& prediction) {
  if (!_status) return false;

  // [BUGFIX] FILE_WRITE on ESP32 truncates the file on every open, so each
  // save wiped the previous rows. FILE_APPEND keeps existing data.
  String path = "/"+ filename + ".csv";
  File file = SD.open(path, FILE_APPEND);

  if (file) {
    if (file.size() == 0) {
      file.println("millis,co2,temp,hum,batt_v,batt_pct,score,status,days");
    }

    file.print(millis());
    file.print(',');
    file.print(sensor.co2);
    file.print(',');
    file.print(sensor.temp);
    file.print(',');
    file.print(sensor.hum);
    file.print(',');
    file.print(sensor.batt_voltage);
    file.print(',');
    file.print(sensor.batt_percent);
    file.print(',');
    file.print(prediction.score);
    file.print(',');
    file.print(prediction.status);
    file.print(',');
    file.println(prediction.days);
    file.close();
    return true;
  }

  return false;
}

uint32_t StorageManager::getFileSize(const String& filename) {
  if (!_status) return 0;

  String path = "/" + filename + ".csv";
  if (!SD.exists(path)) return 0;

  File file = SD.open(path, FILE_READ);
  if (!file) return 0;

  uint32_t size = file.size();
  file.close();
  return size;
}

bool StorageManager::readFile(const String& filename) {
  if (!_status) return false;

  String path = "/" + filename + ".csv";
  if (!SD.exists(path)) return false;

  File file = SD.open(path, FILE_READ);
  if (!file) return false;

  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
  return true;
}
