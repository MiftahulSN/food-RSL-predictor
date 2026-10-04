/*
  MHZ19 Loop
    [NOTES]
        This function is for testing MH-Z19 sensor reading.
        Only available when SENSOR_CONFIG is 1 (uses the mhz global object).
*/

#if (SENSOR_CONFIG == 1)

void mhz19_loop() {
  static bool print_menu = false;
  static int option_menu = 0;
  static bool abc_on = true;

  if (!print_menu) {
    mhz19_menu();
    print_menu = true;
  }

  if (Serial.available() > 0) {
    option_menu = Serial.parseInt();
    while (Serial.available() > 0) { Serial.read(); }

    Serial.printf("\n[MENU] Selected Option : %d\n", option_menu);
    if (option_menu == 0) print_menu = false;

    if (option_menu == 3) {
      abc_on = !abc_on;
      mhz.selfCalibration(abc_on);
      Serial.printf("[TEST 3] Self-Calibration: %s\n", abc_on ? "ON" : "OFF");
    }

    if (option_menu == 4) {
      mhz.calibrateZero();
      Serial.println(F("[TEST 4] Zero-Point Calibration command sent."));
      Serial.println(F("[TEST 4] Ensure the sensor was in stable fresh air (~400 ppm)."));
    }
  }

  switch (option_menu) {

    case 1: {
      static unsigned long last_read = 0;
      if (millis() - last_read >= 2000) {
        last_read = millis();
        int co2 = mhz.readCO2UART();
        Serial.print(F("[TEST 1] CO2 (UART): "));
        Serial.println(co2 > 0 ? String(co2) + " ppm" : "n/a");
      }
      break;
    }

    case 2: {
      static unsigned long last_read = 0;
      if (millis() - last_read >= 2000) {
        last_read = millis();
        int co2 = mhz.readCO2Pwm();
        Serial.print(F("[TEST 2] CO2 (PWM): "));
        Serial.println(co2 > 0 ? String(co2) + " ppm" : "n/a");
      }
      break;
    }

    default:
      break;
  }
}

void mhz19_menu() {
  Serial.println(F("\n[MENU] MH-Z19 Test Options:"));
  Serial.println(F("1. Read CO2 Concentration [UART]"));
  Serial.println(F("2. Read CO2 Concentration [PWM]"));
  Serial.println(F("3. Enable/Disable Self-Calibration"));
  Serial.println(F("4. Zero Point Calibration"));
  Serial.println(F("0. Back to Main Menu"));
  Serial.print(F("[MENU] Select Option: "));
}

#endif
