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
    static const unsigned long WIFI_RECONNECT_INTERVAL  = 5000;  // ms
    static const unsigned long BLYNK_RECONNECT_INTERVAL = 5000;  // ms

    bool READ_DATA = false;
    bool SAVE_DATA = false;
    bool FILE_SIZE = false;
    String _filename;         // latest V10 value received from the app
    String _active_filename;  // latched when a save session starts (V12 ON)
    unsigned long _last_wifi_attempt = 0;
    unsigned long _last_blynk_attempt = 0;

  public:
    static BlynkManager* _instance;
    BlynkManager();
    void begin(const char* auth, const char* ssid, const char* pass);
    void run();
    bool isConnected() const;
    bool sendData(const SensorData& sensor, const PredictionResult& prediction);
    bool sendFileSize(uint32_t filesize);

    // Setters (implemented in .cpp: session-latch, echo-back & debug logging)
    void setFilename(const String& name);
    void setReadCMD(bool enable);
    void setSaveCMD(bool enable);
    void setFileSizeREQ() { FILE_SIZE = true; }

    // Getters
    const String& getFilename() const { return _filename; }
    const String& getActiveFilename() const { return _active_filename; }
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
