/*
  SCD41 Loop
    [NOTES]
        This function is for testing SCD41 sensor reading.
        Only available when SENSOR_CONFIG is 2 (uses the scd global object).
        The sensor updates its measurement every 5 seconds in periodic mode,
        so identical consecutive readings are normal.
*/

#if (SENSOR_CONFIG == 2)

void scd41_loop() {
  static bool print_menu = false;
  static int option_menu = 0;

  if (!print_menu) {
    scd41_menu();
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
        scd.read();
        Serial.print(F("[TEST 1] CO2: "));
        Serial.print(scd.readCO2());
        Serial.println(F(" ppm"));
      }
      break;
    }

    case 2: {
      static unsigned long last_read = 0;
      if (millis() - last_read >= 2000) {
        last_read = millis();
        scd.read();
        Serial.print(F("[TEST 2] Temp: "));
        Serial.print(scd.readTemperature());
        Serial.println(F(" C"));
      }
      break;
    }

    default:
      break;
  }
}

void scd41_menu() {
  Serial.println(F("\n[MENU] SCD41 Test Options:"));
  Serial.println(F("1. Read CO2 Concentration"));
  Serial.println(F("2. Read Temperature"));
  Serial.println(F("0. Back to Main Menu"));
  Serial.print(F("[MENU] Select Option: "));
}

#endif
