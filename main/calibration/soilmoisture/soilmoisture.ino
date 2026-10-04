/*
  Soil Moisture Calibration
    [NOTES]
        Standalone sketch - open it separately in Arduino IDE (not compiled with the main sketch).
        Two-point calibration to get adc_dry & adc_wet for SoilSensor() in main.ino.
        Procedure:
          1. Dry the sensor in open air, wait for stable reading  -> option 2 (5 s average)
          2. Submerge the sensor in water, wait for stable reading -> option 3 (5 s average)
          3. Option 4 to show the result and the paste-ready line.
*/

#define SOIL_PIN 1

uint16_t readAveraged() {
  uint32_t sum = 0;
  uint16_t count = 0;
  unsigned long start = millis();
  while (millis() - start < 5000) {
    sum += analogRead(SOIL_PIN);
    count++;
    delay(100);
  }
  return (uint16_t)(sum / count);
}

void setup() {
  Serial.begin(115200);
  pinMode(SOIL_PIN, INPUT);
}

void loop() {
  soilmoisture_loop();
}

void soilmoisture_loop() {
  static bool print_menu = false;
  static int option_menu = 0;
  static bool have_dry = false;
  static bool have_wet = false;
  static uint16_t adc_dry = 0;
  static uint16_t adc_wet = 0;

  if (!print_menu) {
    soilmoisture_menu();
    print_menu = true;
  }

  if (Serial.available() > 0) {
    option_menu = Serial.parseInt();
    while (Serial.available() > 0) { Serial.read(); }

    Serial.printf("\n[MENU] Selected Option : %d\n", option_menu);
    if (option_menu == 0) print_menu = false;

    if (option_menu == 2) {
      Serial.println(F("[CAL] Capturing DRY point (5 s average)... keep sensor in dry air."));
      adc_dry = readAveraged();
      have_dry = true;
      Serial.printf("[CAL] DRY = %u\n", adc_dry);
    }

    if (option_menu == 3) {
      Serial.println(F("[CAL] Capturing WET point (5 s average)... keep sensor submerged."));
      adc_wet = readAveraged();
      have_wet = true;
      Serial.printf("[CAL] WET = %u\n", adc_wet);
    }

    if (option_menu == 4) {
      if (!have_dry || !have_wet) {
        Serial.println(F("[CAL] Capture both DRY (2) and WET (3) points first!"));
      } else if (adc_dry <= adc_wet) {
        Serial.println(F("[CAL] WARNING: adc_dry <= adc_wet (values inverted)."));
        Serial.println(F("[CAL] For capacitive sensors, dry must be HIGHER than wet."));
        Serial.println(F("[CAL] Check wiring/sensor type, then re-capture."));
      } else {
        Serial.println(F("\n[CAL] Two-Point Calibration Result"));
        Serial.printf("  adc_dry = %u (dry air)\n", adc_dry);
        Serial.printf("  adc_wet = %u (water)\n", adc_wet);
        Serial.println(F("  Paste into main.ino (SENSOR_CONFIG 2):"));
        Serial.printf("    SoilSensor soil(SOIL_PIN, %u, %u);\n", adc_dry, adc_wet);
      }
    }
  }

  switch (option_menu) {

    case 1: {
      static unsigned long last_read = 0;
      if (millis() - last_read >= 500) {
        last_read = millis();
        Serial.print(F("[CAL] Raw ADC: "));
        Serial.println(analogRead(SOIL_PIN));
      }
      break;
    }

    default:
      break;
  }
}

void soilmoisture_menu() {
  Serial.println(F("\n[MENU] Soil Moisture Calibration Options:"));
  Serial.println(F("1. Read Real-Time ADC (500 ms)"));
  Serial.println(F("2. Capture DRY Point (5 s average, sensor in dry air)"));
  Serial.println(F("3. Capture WET Point (5 s average, sensor in water)"));
  Serial.println(F("4. Show Calibration Result"));
  Serial.println(F("0. Back to Main Menu"));
  Serial.print(F("[MENU] Select Option: "));
}
