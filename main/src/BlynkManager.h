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
      V0: Temperature (°C)
      V1: CO2 (ppm)
      V2: Soil Moisture
      V3: Remaining Days
      V4: Prediction Score
      V5: Status String ("Good", "Warning", "Expired")
      V6: File Size

    Inbound (Blynk Cloud -> MCU):
      V10: Target Filename (Text Input Widget)
      V11: Read Data Command (Button Widget)
      V12: Save Data Command (Button Widget)
      V13: File Size Command (Button Widget)
*/
