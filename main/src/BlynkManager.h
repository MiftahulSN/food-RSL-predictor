#ifndef BLYNK_MANAGER_H
#define BLYNK_MANAGER_H

#include <Arduino.h>
#include "Types.h"
#include "Config.h"

/*
  Blynk Manager
*/
class BlynkManager {
  private:
    bool READ_DATA = false;
    bool SAVE_DATA = false;
    bool FILE_SIZE = false;
    String _filename;

  public:
    static BlynkManager* _instance;
    BlynkManager();
    void begin(const char* auth, const char* ssid, const char* pass);
    void run();
    bool isConnected() const;
    bool sendData(const SensorData& sensor, const PredictionResult& prediction);
    bool sendFileSize(uint32_t filesize);

    // Setters
    void setFilename(const String& name) { _filename = name; }
    void setReadCMD(bool enable) { READ_DATA = enable; }
    void setSaveCMD(bool enable) { SAVE_DATA = enable; }
    void setFileSizeREQ() { FILE_SIZE = true; }

    // Getters
    const String& getFilename() const { return _filename; }
    bool getReadCMD() const { return READ_DATA; }
    bool getSaveCMD() const { return SAVE_DATA; }
    bool fileSizeRequested() {
      bool request = FILE_SIZE;
      FILE_SIZE = false;
      return request;
    }
};

#endif

/*
  Virtual Pin Mapping

    Outbound (MCU -> Blynk Cloud):
      V0: Temperature (°C)                                    [Double]
      V1: CO2 (ppm)                                           [Integer]
      V2: Humidity (Config 1: DHT22 %RH | Config 2: Soil %)   [Double]
      V3: Remaining Days                                      [Integer]
      V4: Prediction Score                                    [Double]
      V5: Status (Good/Warning/Expired)                       [Enum: 0=Good, 1=Warning, 2=Expired]
      V6: File Size (bytes)                                   [Integer]
      V8: Battery Voltage (V)                                 [Double]
      V9: Battery Percent (%)                                 [Integer]

    Inbound (Blynk Cloud -> MCU):
      V10: Target Filename                                    [String, Text Input Widget]
      V11: Read Data Command                                  [Integer 0/1, Button Widget]
      V12: Save Data Command                                  [Integer 0/1, Button Widget]
      V13: File Size Command                                  [Integer 0/1, Button Widget]
*/
