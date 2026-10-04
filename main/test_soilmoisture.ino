/*
  Soil Moisture Loop
    [NOTES]
        This function is for testing soil moisture sensor reading.
        Only available when SENSOR_CONFIG is 2 (uses the soil global object).
        If readings look wrong, recalibrate via the adc_dry/adc_wet
        constructor args in main.ino.
*/

#if (SENSOR_CONFIG == 2)

void soilmoisture_loop() {
  static bool print_menu = false;
  static int option_menu = 0;

  if (!print_menu) {
    soilmoisture_menu();
    print_menu = true;
  }

  if (Serial.available() > 0) {
    option_menu = Serial.parseInt();
    while (Serial.available() > 0) { Serial.read(); }

    Serial.printf("\n[MENU] Selected Option : %d\n", option_menu);
    if (option_menu == 0) print_menu = false;
  }

  switch (option_menu) {

    case 1: {
      static unsigned long last_read = 0;
      if (millis() - last_read >= 2000) {
        last_read = millis();
        Serial.print(F("[TEST 1] Soil Moisture: "));
        Serial.print(soil.readMoisture());
        Serial.println(F(" %"));
      }
      break;
    }

    default:
      break;
  }
}

void soilmoisture_menu() {
  Serial.println(F("\n[MENU] Soil Moisture Test Options:"));
  Serial.println(F("1. Read Soil Moisture"));
  Serial.println(F("0. Back to Main Menu"));
  Serial.print(F("[MENU] Select Option: "));
}

#endif
