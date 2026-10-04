#include "RSLPredictor.h"

PredictionResult RSLPredictor::predict(const SensorData& data) const {
  PredictionResult result;

  // 1. Temperature factor via Arrhenius Equation
  float temp_kelvin = data.temp + 273.15f;
  float k_reference = PRE_EXPONENTIAL * exp(-ACTIVATION_ENERGY / (GAS_CONSTANT * REFERENCE_TEMP));
  float k_actual    = PRE_EXPONENTIAL * exp(-ACTIVATION_ENERGY / (GAS_CONSTANT * temp_kelvin));
  float rate_ratio  = k_actual / k_reference;

  // 2. CO2 degradation factor
  float co2_factor = 1.0f;
  if (data.co2 > 400) {
    co2_factor = 1.0f + ((data.co2 - 400) * CO2_DECAY_FACTOR);
  }

  // 3. Humidity degradation factor
  float humidity_factor = 1.0f;
  float humidity_deviation = abs(data.hum - 60.0f);
  if (humidity_deviation > HUM_OPTIMAL_RANGE) {
    humidity_factor = 1.0f + ((humidity_deviation - HUM_OPTIMAL_RANGE) / 10.0f * 0.2f);
  }
  if (humidity_factor > 3.0f) humidity_factor = 3.0f;

  // 4. Shelf life estimation
  float total_degradation_rate = rate_ratio * co2_factor * humidity_factor;
  float predicted_shelf_life   = BASE_SHELF_LIFE / total_degradation_rate;

  if (predicted_shelf_life > 12.0f) predicted_shelf_life = 12.0f;
  if (predicted_shelf_life < 0.0f)  predicted_shelf_life = 0.0f;

  float days_elapsed = BASE_SHELF_LIFE - predicted_shelf_life;
  if (days_elapsed < 0.0f) days_elapsed = 0.0f;

  // 5. Score calculation (0 - 100%)
  result.score = 100.0f - (days_elapsed / BASE_SHELF_LIFE * 100.0f);
  if (result.score < 0.0f)   result.score = 0.0f;
  if (result.score > 100.0f) result.score = 100.0f;

  // 6. Status determination
  float days_age = BASE_SHELF_LIFE - predicted_shelf_life;
  if (days_age <= 3.0f) {
    result.status = 'A';
  } else if (days_age <= 6.0f) {
    result.status = 'B';
  } else {
    result.status = 'C';
  }

  // 7. Days remaining
  result.days = (uint8_t)(predicted_shelf_life);

  return result;
}
