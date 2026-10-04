/*
  SD Card Loop
    [NOTES]
      This function is for testing SD Card reading and writing.
*/

void sdcard_loop(){
  
}

void sdcard_menu() {
  Serial.println(F("\n[MENU] SD Card Test Options:"));
  Serial.println(F("1. Save Dummy Data to SD Card"));
  Serial.println(F("2. Read Data from SD Card"));
  Serial.println(F("0. Back to Main Menu"));
  Serial.print(F("[MENU] Select Option: "));
}