/*
  MHZ19 Loop
    [NOTES]
        This function is for testing MH-Z19 sensor reading.
*/

void mhz19_loop(){
  
}

void mhz19_menu() {
  Serial.println(F("\n[MENU] MH-Z19 Test Options:"));
  Serial.println(F("1. Read CO2 Concentration"));
  Serial.println(F("0. Back to Main Menu"));
  Serial.print(F("[MENU] Select Option: "));
}