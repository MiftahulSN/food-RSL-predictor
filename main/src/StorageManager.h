#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include "Types.h"

/*
  SD Card reader module
*/
class StorageManager {
  private:
    uint8_t _cs_pin;
    uint8_t _sck_pin;
    uint8_t _mosi_pin;
    uint8_t _miso_pin;
    bool _status;

  public:
    StorageManager(uint8_t cs_pin, uint8_t sck_pin, uint8_t mosi_pin, uint8_t miso_pin);
    bool begin();
    bool saveData(const String& filename, const SensorData& sensor, const PredictionResult& prediction);
    uint32_t getFileSize(const String& filename);
};

#endif