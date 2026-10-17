#include "BlynkManager.h"
#include "Debug.h"

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
    _active_filename.reserve(32);
}

void BlynkManager::begin(const char* auth, const char* ssid, const char* pass) {
    Blynk.connectWiFi(ssid, pass);
    Blynk.config(auth);
    Blynk.connect();
}

void BlynkManager::run() {
    unsigned long now = millis();

    // [BUGFIX] No WiFi reconnect before: if WiFi dropped, Blynk stayed down
    // forever. Retry periodically instead of blocking every loop.
    if (WiFi.status() != WL_CONNECTED) {
        if (now - _last_wifi_attempt >= WIFI_RECONNECT_INTERVAL) {
            _last_wifi_attempt = now;
            WiFi.reconnect();
        }
        return;
    }

    if (!Blynk.connected()) {
        // [BUGFIX] Blynk.connect(1000) used to block every loop iteration
        // while disconnected (1s stall of sensor + SD + Blynk processing).
        // Attempt only every BLYNK_RECONNECT_INTERVAL.
        if (now - _last_blynk_attempt >= BLYNK_RECONNECT_INTERVAL) {
            _last_blynk_attempt = now;
            if (Blynk.connect(1000)) {
                Blynk.run();
            }
        }
        return;
    }

    Blynk.run();
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
    int status_index = prediction.status - 'A';
    if (status_index < 0 || status_index > 2) status_index = 0;
    Blynk.virtualWrite(V5, status_index);
    Blynk.virtualWrite(V8, sensor.batt_voltage);
    Blynk.virtualWrite(V9, sensor.batt_percent);
    return true;
}

// outbound: send file size to Blynk
bool BlynkManager::sendFileSize(uint32_t filesize) {
    if (!isConnected()) return false;

    Blynk.virtualWrite(V6, filesize);
    return true;
}

// inbound setter: filename text input
// [BUGFIX] Echo the value back to V10 so (a) the app shows what the device
// actually holds and (b) the device-side write refreshes the server's stored
// value, so a reconnect re-sync can never resurrect a stale filename.
void BlynkManager::setFilename(const String& name) {
    _filename = name;
#if DEBUG
    Serial.print(F("[BLYNK] V10 filename = \""));
    Serial.print(name);
    Serial.println(F("\""));
#endif
    if (isConnected()) {
        Blynk.virtualWrite(V10, name);
    }
}

// inbound setter: continuous read command
void BlynkManager::setReadCMD(bool enable) {
    READ_DATA = enable;
#if DEBUG
    Serial.print(F("[BLYNK] V11 read = "));
    Serial.println(enable ? F("ON") : F("OFF"));
#endif
}

// inbound setter: continuous save command
// [BUGFIX] Latch the filename at session start: one button-ON = one file.
// Renaming mid-session no longer switches the output file silently.
void BlynkManager::setSaveCMD(bool enable) {
    if (enable && !SAVE_DATA) {
        if (_filename.length() > 0) {
            _active_filename = _filename;
#if DEBUG
            Serial.print(F("[BLYNK] Save session start -> \""));
            Serial.print(_active_filename);
            Serial.println(F(".csv\""));
#endif
            if (isConnected()) {
                Blynk.virtualWrite(V10, _active_filename);
            }
        }
#if DEBUG
        else {
            Serial.println(F("[BLYNK] Save session NOT started: filename empty (set V10 first)"));
        }
#endif
    }
    SAVE_DATA = enable;
#if DEBUG
    Serial.print(F("[BLYNK] V12 save = "));
    Serial.println(enable ? F("ON") : F("OFF"));
#endif
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