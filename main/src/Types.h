#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

/*
  Shared Data Structure
*/

struct SensorData {
  uint16_t co2;
  float temp;
  float hum;
  float batt_voltage;
  uint8_t batt_percent;
};

struct PredictionResult {
  float score;
  char status;
  uint8_t days;
};

#endif
