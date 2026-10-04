/*
  SD Card Loop
    [NOTES]
        This function is for testing SD Card reading and writing.
*/

void sdcard_loop() {
  static bool print_menu = false;
  static int option_menu = 0;

  if (!print_menu) {
    sdcard_menu();
    print_menu = true;
  }

  if (Serial.available() > 0) {
    option_menu = Serial.parseInt();
    while (Serial.available() > 0) { Serial.read(); }

    Serial.printf("\n[MENU] Selected Option : %d\n", option_menu);
    if (option_menu == 0) print_menu = false;

    if (option_menu == 1) {
      SensorData dummy_sensor;
      dummy_sensor.co2 = 412;
      dummy_sensor.temp = 25.5f;
      dummy_sensor.hum = 60.0f;
      dummy_sensor.batt_voltage = 4.15f;
      dummy_sensor.batt_percent = 95;

      PredictionResult dummy_pred;
      dummy_pred.days = 5;
      dummy_pred.score = 88.5f;
      dummy_pred.status = 'B';

      const char* test_filename = "sd_test.csv";
      if (storage.saveData(test_filename, dummy_sensor, dummy_pred)) {
        Serial.printf("[TEST 1] Data saved to '%s' (%u bytes)\n",
                      test_filename, (unsigned)storage.getFileSize(test_filename));
      } else {
        Serial.println(F("[TEST 1] Save failed! Check the SD Card."));
      }
    }

    if (option_menu == 2) {
      const char* test_filename = "sd_test.csv";
      Serial.printf("[TEST 2] Contents of '%s':\n", test_filename);
      Serial.println(F("----------------------------------------"));
      if (storage.readFile(test_filename)) {
        Serial.println(F("----------------------------------------"));
        Serial.println(F("[TEST 2] Read complete."));
      } else {
        Serial.println(F("[TEST 2] File not found or read failed! Run Test 1 first."));
      }
    }
  }
}

void sdcard_menu() {
  Serial.println(F("\n[MENU] SD Card Test Options:"));
  Serial.println(F("1. Save Dummy Data to SD Card"));
  Serial.println(F("2. Read Data from SD Card"));
  Serial.println(F("0. Back to Main Menu"));
  Serial.print(F("[MENU] Select Option: "));
}
