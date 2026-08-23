#include "BlynkManager.h"

#if defined(ESP32)
  #include <WiFi.h>
  #include <BlynkSimpleEsp32.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <BlynkSimpleEsp8266.h>
#endif

BlynkManager* BlynkManager::_instance = nullptr;

BlynkManager::BlynkManager() {
    _instance = this; 
    _filename.reserve(32);
}

void BlynkManager::begin(const char* auth, const char* ssid, const char* pass) {
    Blynk.connectWiFi(ssid, pass);
    Blynk.config(auth);
    Blynk.connect();
}

void BlynkManager::run() {
    if (WiFi.status() == WL_CONNECTED) {
        if (!Blynk.connected()) Blynk.connect(1000); 
        else Blynk.run();
    }
}

bool BlynkManager::isConnected() const {
    return Blynk.connected();
}

// outbound: send data to Blynk
bool BlynkManager::sendData(const SensorData& sensor, const PredictionResult& prediction) {
    if (!isConnected()) return false;

    Blynk.virtualWrite(V0, sensor.temp);
    Blynk.virtualWrite(V1, sensor.co2);
    Blynk.virtualWrite(V2, sensor.hum);
    Blynk.virtualWrite(V3, prediction.days);
    Blynk.virtualWrite(V4, prediction.score);
    Blynk.virtualWrite(V5, prediction.status);
    return true;
}

// outbound: send file size to Blynk
bool BlynkManager::sendFileSize(uint32_t filesize) {
    if (!isConnected()) return false;

    Blynk.virtualWrite(V6, filesize);
    return true;
}

// inbound: Blynk event handlers
BLYNK_CONNECTED() {
    Blynk.syncVirtual(V10, V11, V12);
}

// inbound: filename text input
BLYNK_WRITE(V10) {
  String text = param.asString();
  if (text.length() > 0 && BlynkManager::_instance != nullptr) {
    BlynkManager::_instance->setFilename(text);
  }
}

// inbound: continuous operation read data command
BLYNK_WRITE(V11) {
  if (BlynkManager::_instance != nullptr) {
    bool read = (param.asInt() == 1);
    BlynkManager::_instance->setReadCMD(read);
  }
}

// inbound: continuous operation save data command
BLYNK_WRITE(V12) {
  if (BlynkManager::_instance != nullptr) {
    bool save = (param.asInt() == 1);
    BlynkManager::_instance->setSaveCMD(save);
  }
}

// inbound: one-shot operation file size request command
BLYNK_WRITE(V13) {
  uint8_t req = param.asInt();
  if (req == 1 && BlynkManager::_instance != nullptr) {
    BlynkManager::_instance->setFileSizeREQ();
  }
}