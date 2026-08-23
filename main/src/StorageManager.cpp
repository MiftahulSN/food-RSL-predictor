#include "StorageManager.h" 

StorageManager::StorageManager(uint8_t cs_pin, uint8_t sck_pin, uint8_t mosi_pin, uint8_t miso_pin) {
  _cs_pin = cs_pin;
  _sck_pin = sck_pin;
  _mosi_pin = mosi_pin;
  _miso_pin = miso_pin;
  _status = false;
}

bool StorageManager::begin() {
  SPI.begin();
  _status = SD.begin(_cs_pin);
  return _status;
}

bool StorageManager::saveData(const String& filename, const SensorData& sensor, const PredictionResult& prediction) {
  if (!_status) return 0;

  String path = "/"+ filename;
  File file = SD.open(path, FILE_WRITE);

  if (file) {
    if (!SD.exists(path) || file.size() == 0) {
      file.println("co2,temp,hum,score,status,days");
    }

    String data = String(sensor.co2) + "," + 
                  String(sensor.temp) + "," + 
                  String(sensor.hum) + "," +
                  String(prediction.score) + "," + 
                  String(prediction.status) + "," + 
                  String(prediction.days);

    file.println(data);
    file.close();
    return true;
  }

  return false;
}

uint32_t StorageManager::getFileSize(const String& filename) {
  if (!_status) return 0;

  String path = "/" + filename;
  if (!SD.exists(path)) return 0;

  File file = SD.open(path, FILE_READ);
  if (!file) return 0;

  uint32_t size = file.size();
  file.close();
  return size;
}
