/*
  Battery Loop
    [NOTES]
        This function is for testing battery monitor reading.
        Available in both sensor configurations (uses the batt global object).
*/

void batt_loop() {
  static bool print_menu = false;
  static int option_menu = 0;

  if (!print_menu) {
    batt_menu();
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
        Serial.print(F("[TEST 1] Battery: "));
        Serial.print(batt.readVoltage());
        Serial.print(F("V ("));
        Serial.print(batt.readPercent());
        Serial.println(F("%)"));
      }
      break;
    }

    default:
      break;
  }
}

void batt_menu() {
  Serial.println(F("\n[MENU] Battery Test Options:"));
  Serial.println(F("1. Read Battery Voltage & Level"));
  Serial.println(F("0. Back to Main Menu"));
  Serial.print(F("[MENU] Select Option: "));
}
