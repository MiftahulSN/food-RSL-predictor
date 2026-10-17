#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include "src/Debug.h"
#include "src/BlynkManager.h"
#include "src/RSLPredictor.h"
#include "src/SensorManager.h"
#include "src/StorageManager.h"
#include "src/Types.h"

/*
  Build Configuration
    DEBUG                : see src/Debug.h
    SENSOR_CONFIG        : 1 = MH-Z19C (CO2) + DHT22 (Temp & Hum)
                           2 = SCD41 (CO2, Temp & Hum) + Soil Moisture
    MHZ_SELF_CALIBRATION : 1 = enable MH-Z19C self calibration (factory default)
                           0 = disable MH-Z19C self calibration
    MHZ_ZERO_CALIBRATION : 1 = send zero-point calibration at boot (ONLY in stable fresh air ~400 ppm)
                           0 = disable zero-point calibration
    SCD_SELF_CALIBRATION : 1 = enable SCD41 automatic self calibration (ASC)
                           0 = disable SCD41 automatic self calibration
    SCD_FACTORY_RESET    : 1 = SCD41 factory reset at boot (erases EEPROM settings + FRC/ASC history)
                           0 = disable SCD41 factory reset
    SCD_ZERO_CALIBRATION : 1 = SCD41 FRC at boot + 10 min settle (fresh air ~420 ppm)
                           0 = disable SCD41 forced recalibration
*/

#define SENSOR_CONFIG 1
#define MHZ_SELF_CALIBRATION 0
#define MHZ_ZERO_CALIBRATION 0
#define SCD_SELF_CALIBRATION 0
#define SCD_FACTORY_RESET 0
#define SCD_ZERO_CALIBRATION 0

#if (SENSOR_CONFIG != 1) && (SENSOR_CONFIG != 2)
  #error "SENSOR_CONFIG must be 1 (MH-Z19C + DHT22) or 2 (SCD41 + Soil Moisture)"
#endif
#if (MHZ_SELF_CALIBRATION != 0) && (MHZ_SELF_CALIBRATION != 1)
  #error "MHZ_SELF_CALIBRATION must be 1 or 0"
#endif
#if (MHZ_ZERO_CALIBRATION != 0) && (MHZ_ZERO_CALIBRATION != 1)
  #error "MHZ_ZERO_CALIBRATION must be 1 or 0"
#endif
#if (SCD_SELF_CALIBRATION != 0) && (SCD_SELF_CALIBRATION != 1)
  #error "SCD_SELF_CALIBRATION must be 1 or 0"
#endif
#if (SCD_FACTORY_RESET != 0) && (SCD_FACTORY_RESET != 1)
  #error "SCD_FACTORY_RESET must be 1 or 0"
#endif
#if (SCD_ZERO_CALIBRATION != 0) && (SCD_ZERO_CALIBRATION != 1)
  #error "SCD_ZERO_CALIBRATION must be 1 or 0"
#endif

#define SOIL_PIN    1
#define DHT_PIN     2
#define BATT_PIN    4
#define MHZ_RX_PIN  18 
#define MHZ_TX_PIN  17 
#define MHZ_PWM_PIN 6
#define SDA_PIN     8
#define SCL_PIN     9
#define SD_CS_PIN   10
#define SD_SCK_PIN  12
#define SD_MOSI_PIN 11
#define SD_MISO_PIN 13

/*
  Object Initialization
*/

BatteryMonitor batt(BATT_PIN);

#if (SENSOR_CONFIG == 1)
MHZ19Sensor mhz(MHZ_RX_PIN, MHZ_TX_PIN, MHZ_PWM_PIN);
DHT22Sensor dht(DHT_PIN);
SensorManager sensor(&mhz, &dht, nullptr, nullptr, &batt);
#else
SCD41Sensor scd(SDA_PIN, SCL_PIN);
SoilSensor soil(SOIL_PIN, 3402, 1291);  // two-point calibration (dry air / water)
SensorManager sensor(nullptr, nullptr, &scd, &soil, &batt);
#endif

StorageManager storage(SD_CS_PIN, SD_SCK_PIN, SD_MOSI_PIN, SD_MISO_PIN);
BlynkManager blynk;
RSLPredictor predictor;
SensorData data;
PredictionResult prediction;

/*
  Global Variables
    [TIMING] Decoupled scheduler
      The sensor heartbeat is the only source of fresh data; Blynk pushes and
      SD writes run only right AFTER a fresh read, gated by their own minimum
      interval. Effective cadence = first heartbeat after each gate elapses.
*/

#if (SENSOR_CONFIG == 1)
  // DHT22 min sampling period is 2 s (datasheet); MH-Z19C updates ~1 s.
  const unsigned long SENSOR_INTERVAL = 2000;
#else
  // SCD41 periodic measurement outputs a fresh sample every 5 s (hardware).
  const unsigned long SENSOR_INTERVAL = 5000;
#endif
const unsigned long BLYNK_INTERVAL   = 2000;  // 8 writes/tick -> 4/s (Blynk cloud limit 10/s)
const unsigned long STORAGE_INTERVAL = 4000;  // exact multiple of the 2 s heartbeat -> 4 s writes

unsigned long SENSOR_TIME = 0;
unsigned long BLYNK_TIME  = 0;
unsigned long STORE_TIME  = 0;

/*
  Setup
*/

void setup() {

#if DEBUG
  Serial.begin(115200);
  Serial.println(F("[INITIALIZATION]"));
#endif

  sensor.begin();

#if (SENSOR_CONFIG == 1)
  mhz.selfCalibration(MHZ_SELF_CALIBRATION);
#if DEBUG
  Serial.print(F("[MHZ19] Self-Calibration: "));
  Serial.println(MHZ_SELF_CALIBRATION ? F("ON") : F("OFF"));
#endif
#if MHZ_ZERO_CALIBRATION
  mhz.calibrateZero();
#if DEBUG
  Serial.println(F("[MHZ19] Zero-Point Calibration sent (fresh air ~400 ppm required)."));
#endif
#endif
#endif

#if (SENSOR_CONFIG == 2)
#if SCD_FACTORY_RESET
  // Must run BEFORE selfCalibration(): the reset restores the factory-default
  // ASC=ON flag, which selfCalibration() re-applies afterwards. Do NOT power
  // off during the reset (EEPROM erase takes ~1.2 s).
  scd.factoryReset();
#if DEBUG
  Serial.println(F("[SCD41] Factory reset done (EEPROM settings + FRC/ASC history erased)."));
#endif
#endif
  scd.selfCalibration(SCD_SELF_CALIBRATION);
#if DEBUG
  Serial.print(F("[SCD41] Self-Calibration: "));
  Serial.println(SCD_SELF_CALIBRATION ? F("ON") : F("OFF"));
#endif
#if SCD_ZERO_CALIBRATION
  // FRC requires >= 3 minutes of prior operation in periodic measurement mode
  // (SCD4x datasheet section 3.8.1). Keep the device in stable fresh air (~420 ppm).
#if DEBUG
  Serial.println(F("[SCD41] Zero-Point Calibration: keep device in fresh air (~420 ppm)!"));
#endif
  for (uint8_t m = 3; m > 0; m--) {
#if DEBUG
    Serial.print(F("[SCD41] Calibrating in "));
    Serial.print(m);
    Serial.println(F(" minute(s)..."));
#endif
    delay(60000);
  }
#if DEBUG
  int16_t frc_correction = scd.calibrateZero();
  bool frc_ok = (frc_correction != 32767);
  if (frc_ok) {
    Serial.print(F("[SCD41] Forced Recalibration OK. Correction: "));
    Serial.print(frc_correction);
    Serial.println(F(" ppm"));
  } else {
    Serial.println(F("[SCD41] Forced Recalibration FAILED (0xFFFF)."));
  }
#else
  scd.calibrateZero();
  bool frc_ok = true;
#endif

  // [OPERATION] Post-FRC settle period: keeps the sensor running so the FRC
  // history auto-saves to EEPROM (SCD4x datasheet 3.10.1) and verifies the
  // result. Correctly calibrated readings must never go below ~400 ppm.
  if (frc_ok) {
#if DEBUG
    Serial.println(F("[SCD41] Settle 10 min - do NOT power off (calibration auto-saving)..."));
    uint32_t settle_start = millis();
    bool suspicious = false;
    while (millis() - settle_start < 600000UL) {
      delay(5000);
      if (scd.read()) {
        uint16_t co2_now = scd.readCO2();
        if (co2_now < 400) suspicious = true;
        Serial.print(F("[SCD41] Settle "));
        Serial.print((millis() - settle_start) / 1000);
        Serial.print(F("s: CO2 = "));
        Serial.print(co2_now);
        Serial.println(F(" ppm"));
      }
    }
    if (suspicious) {
      Serial.println(F("[SCD41] WARNING: reading below 400 ppm detected!"));
      Serial.println(F("[SCD41] FRC was anchored in wrong air (people too close / not fresh air)."));
      Serial.println(F("[SCD41] Redo FRC in open air with everyone >= 2 m from the device."));
    } else {
      Serial.println(F("[SCD41] Settle done. Calibration saved. Flash with SCD_ZERO_CALIBRATION 0."));
    }
#else
    delay(600000UL);
#endif
  }
#endif
#endif

  storage.begin();
  WiFi.mode(WIFI_STA);
  blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);

  // [TIMING] Anchor the heartbeat so the first sensor read fires immediately
  // on the first loop iteration (push/save gates stay at 0 for the same reason).
  SENSOR_TIME = millis() - SENSOR_INTERVAL;
}

/*
  Loop
    [NOTES]
      Use only one loop functions, comment the rest functions!
      Main loop as the main function, and the rest functions are for testing purposes!
      Don't forget to change #define DEBUG 1 when using testing functions!
*/

void loop() {
  main_loop();
  // blynk_loop();
  // sdcard_loop();
#if (SENSOR_CONFIG == 1)
  // mhz19_loop();
#endif
#if (SENSOR_CONFIG == 2)
  // scd41_loop();
  // soilmoisture_loop();
#endif
  // batt_loop();
}

/*
  Main Loop
    [NOTES]
      This function is for the main operation of the system, integraring all components and functionalities.
*/

void main_loop() {
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

  // [OPERATION] Sensor Heartbeat - every push/log below uses this fresh data
  if (millis() - SENSOR_TIME >= SENSOR_INTERVAL) {
    SENSOR_TIME += SENSOR_INTERVAL;  // drift-free cadence

    // [OPERATION] Reading Sensor Data and Making Prediction
    data = sensor.readAll();
    prediction = predictor.predict(data);
#if DEBUG
    Serial.print(F("CO2: "));    Serial.print(data.co2);
    Serial.print(F(" | Temp: ")); Serial.print(data.temp);
    Serial.print(F(" | Hum: "));  Serial.print(data.hum);
    Serial.print(F(" | Batt: ")); Serial.print(data.batt_voltage);
    Serial.print(F("V ("));       Serial.print(data.batt_percent);
    Serial.println(F("%)"));
#endif

    // [OPERATION] Continuous Send Data to Blynk Command (rate-limited)
    if (blynk.getReadCMD() && (millis() - BLYNK_TIME >= BLYNK_INTERVAL)) {
      BLYNK_TIME = millis();
      if (blynk.sendData(data, prediction)) {
#if DEBUG
        Serial.println(F("[BLYNK] Pushed data to Blynk Cloud"));
#endif
      }
    }

    // [OPERATION] Continuous Save Data to SD Card Command (wear-limited)
    if (blynk.getSaveCMD() && (millis() - STORE_TIME >= STORAGE_INTERVAL)) {
      STORE_TIME = millis();
      const String& filename = blynk.getActiveFilename();
      if (filename.length() > 0) {
        if (storage.saveData(filename, data, prediction)) {
#if DEBUG
          Serial.print(F("[STORAGE] Saved to \""));
          Serial.print(filename);
          Serial.println(F(".csv\""));
#endif
        }
#if DEBUG
        else {
          Serial.println(F("[STORAGE] Save FAILED (check SD card)"));
        }
#endif
      }
#if DEBUG
      else {
        Serial.println(F("[STORAGE] Save skipped: no session filename (set V10, then toggle V12)"));
      }
#endif
    }
  }
}
