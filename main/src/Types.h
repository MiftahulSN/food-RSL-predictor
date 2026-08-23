#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

/*
  Shared Data Structure
*/

struct SensorData {
  uint16_t co2;
  int temp;
  uint16_t hum;
};

struct PredictionResult {
  float score;
  char status;
  uint8_t days;
};

#endif