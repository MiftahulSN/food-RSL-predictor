/*
  Blynk Loop
    [NOTES]
        This function is for testing Blynk Cloud connection and data sending.
*/

void blynk_loop() {
  static bool print_menu = false;
  static int option_menu = 0;
  static int counter = 0;

  blynk.run();

  if (!print_menu) {
    blynk_menu();
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
      static unsigned long last_send = 0;
      if (millis() - last_send >= 3000) {
        last_send = millis();
        counter++;

        SensorData dummy_sensor;
        dummy_sensor.temp = 25.0f + counter;
        dummy_sensor.co2  = 400 + (counter * 10);
        dummy_sensor.hum  = 60.0f + counter;
        dummy_sensor.batt_voltage = 4.2f - (counter * 0.05f);
        dummy_sensor.batt_percent = (counter < 20) ? (100 - (counter * 5)) : 0;

        PredictionResult dummy_pred;
        dummy_pred.days   = (10.0f - counter > 0) ? (10.0f - counter) : 0;
        dummy_pred.score  = 95.0f - counter;
        dummy_pred.status = 'A' + (counter % 3);

        if (blynk.sendData(dummy_sensor, dummy_pred)) {
          Serial.printf("[TEST 1] Stream #%d Berhasil!\n", counter);
        } else {
          Serial.println(F("[TEST 1] Stream Failed!"));
        }
      }
      break;
    }

    case 2: {
      static unsigned long last_print = 0;
      if (millis() - last_print >= 3000) {
        last_print = millis();
        Serial.printf("[TEST 2] Filename Received: '%s'\n", blynk.getFilename().c_str());
      }
      break;
    }

    case 3: {
      static unsigned long last_toggle = 0;
      if (millis() - last_toggle >= 2000) {
        last_toggle = millis();
        bool isReading = blynk.getReadCMD();
        bool isSaving  = blynk.getSaveCMD();

        Serial.printf("[TEST 3] Status Toggle -> READ_DATA (V11): %s | SAVE_DATA (V12): %s\n",
                      isReading ? "ON" : "OFF",
                      isSaving  ? "ON" : "OFF");
      }
      break;
    }

    case 4: {
      if (blynk.fileSizeRequested()) {
        Serial.println(F("[TEST 4] File Size (V13) Request Received!"));
        
        uint32_t dummy_size = 2048;
        if (blynk.sendFileSize(dummy_size)) {
          Serial.printf("[TEST 4] File Size Sent: (%u bytes) to V6\n", dummy_size);
        }
      }
      break;
    }

    default:
      break;
  }
}


void blynk_menu() {
  Serial.println(F("\n--------------- TEST SUITE: BLYNK MANAGER --------------- "));
  Serial.println(F("1. Test Stream Dummy Sensor Data            (V0-V5)"));
  Serial.println(F("2. Test Receive Dummy Filename Blynk        (V10)"));
  Serial.println(F("3. Test Receive SAVE & READ Data Command    (V11-V12)"));
  Serial.println(F("4. Test Receive FILE SIZE Request           (V13)"));
  Serial.println(F("0. Back to Main Menu"));
  Serial.print(F("Choose Menu [0-4] : "));
}