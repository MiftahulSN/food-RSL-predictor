#ifndef RSL_PREDICTOR_H
#define RSL_PREDICTOR_H

#include <Arduino.h>
#include "Types.h"

class RSLPredictor {
private:
  static constexpr float ACTIVATION_ENERGY  = 50000.0f;
  static constexpr float GAS_CONSTANT       = 8.314f;
  static constexpr float PRE_EXPONENTIAL    = 1e10f;
  static constexpr float REFERENCE_TEMP     = 298.15f;
  static constexpr float CO2_DECAY_FACTOR   = 0.001f;
  static constexpr float HUM_OPTIMAL_RANGE  = 10.0f;
  static constexpr float BASE_SHELF_LIFE    = 6.0f;

public:
  RSLPredictor() = default;

  PredictionResult predict(const SensorData& data) const;
};

#endif