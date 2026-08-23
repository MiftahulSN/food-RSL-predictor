#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif 
#include "src/BlynkManager.h"
#include "src/RSLPredictor.h"
#include "src/SensorManager.h"
#include "src/StorageManager.h"
#include "src/Types.h"

#define DEBUG 0

#define USE_SOIL_MOISTURE 1
#define SOIL_PIN A0

#define USE_MHZ19 1
#define MHZ_RX_PIN  0
#define MHZ_TX_PIN  2

#define USE_SD_CARD 1
#define SD_CS_PIN PIN_SPI_SS
#define SD_SCK_PIN PIN_SPI_SCK
#define SD_MOSI_PIN PIN_SPI_MOSI
#define SD_MISO_PIN PIN_SPI_MISO

/*
  Object Initialization
*/

MHZ19Sensor mhz(MHZ_RX_PIN, MHZ_TX_PIN);
SoilSensor soil(SOIL_PIN);
SensorManager sensor(mhz, soil);
StorageManager storage(SD_CS_PIN, SD_SCK_PIN, SD_MOSI_PIN, SD_MISO_PIN);
BlynkManager blynk;
RSLPredictor predictor;
SensorData data;
PredictionResult prediction;

/*
  Global Variables
*/

unsigned long MARK_TIME = 0;
const unsigned long INTERVAL_TIME = 500;

/*
  Setup
*/

void setup() {

#if DEBUG
  Serial.begin(115200);
  Serial.println(F("[INITIALIZATION]"));
#endif

  sensor.begin();
  storage.begin();
  WiFi.mode(WIFI_STA);
  blynk.begin(BLYNK_AUTH, WIFI_SSID, WIFI_PASS);
}

/*
  Main Loop
*/

void loop() {
  blynk.run();

  // [OPERATION] File Size Request Command
  if (blynk.fileSizeRequested()) {
    String filename = blynk.getFilename();
    uint32_t filesize = storage.getFileSize(filename);
    blynk.sendFileSize(filesize); 
#if DEBUG
    Serial.print(F("[STORAGE] File size: ")); 
    Serial.print(filesize); 
    Serial.println(" bytes");
#endif
  }

  if (millis() - MARK_TIME >= INTERVAL_TIME) {
    MARK_TIME = millis();
    data = sensor.readAll();
    prediction = predictor.predict(data);
#if DEBUG
    Serial.print(F("CO2: "));   Serial.print(data.co2);
    Serial.print(F(" | Temp: ")); Serial.print(data.temp);
    Serial.print(F(" | Hum: "));  Serial.println(data.hum);
#endif

    // [OPERATION] Continuous Send Data to Blynk Command
    if (blynk.getReadCMD()) {
      blynk.sendData(data, prediction);
#if DEBUG
      Serial.println(F("[BLYNK] Pushed data to Blynk Cloud"));
#endif
    }

    // [OPERATION] Continuous Save Data to SD Card Command
    if (blynk.getSaveCMD()) {
      String filename = blynk.getFilename();
      storage.saveData(filename, data, prediction);
#if DEBUG
      Serial.print(F("[STORAGE] Data saved to SD Card"));
#endif
    }
  }
}
