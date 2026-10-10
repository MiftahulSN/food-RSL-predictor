# Arsitektur Program — food-RSL-predictor

Diagram visual tersedia di: [`../images/Arsitektur.png`](../images/Arsitektur.png)

---

## 1. Gambaran Umum

Sistem IoT berbasis **ESP32/ESP8266** untuk memprediksi *Remaining Shelf Life* (RSL) makanan:

1. Membaca kondisi lingkungan (CO₂, suhu, kelembapan/soil moisture, baterai)
2. Menghitung prediksi umur simpan dengan **model Arrhenius** (`RSLPredictor`)
3. Mengirim telemetri ke **Blynk Cloud** (dashboard mobile/web)
4. Mencatat data ke **MicroSD** dalam format CSV

Arsitektur berlapis dengan orkestrator tunggal (`main.ino`) dan 4 modul manager yang saling independen.

---

## 2. Diagram Arsitektur

```
                          ┌─────────────────────────┐
                          │  Blynk Cloud/Dashboard  │
                          │ ▲ V0–V9  ·  ▼ V10–V13   │
                          └────────────▲────────────┘
                                       │ WiFi / Internet
┌──────────────────────────────────────┴──────────────────────────────────┐
│                      main.ino — ORKESTRATOR                             │
│    setup(): init semua modul  ·  loop(): main_loop() tiap 1500 ms       │
│             (non-blocking millis, build flags compile-time)             │
└──────┬──────────────────────┬──────────────────────┬────────────────────┘
       ▼                      ▼                      ▼
┌──────────────┐  1  ┌──────────────┐  2  ┌──────────────┐  2  ┌──────────────┐
│SensorManager │────►│ RSLPredictor │────►│StorageManager│     │ BlynkManager │◄═► Cloud
│  readAll()   │     │  predict()   │     │  saveData()  │     │  sendData()  │
└──────┬───────┘     └──────────────┘     └──────┬───────┘     └──────▲───────┘
       ▼                  (Arrhenius)            ▼                   │
┌─────────────────────────────────────┐    ┌───────────┐       ┌───────────┐
│      LAPISAN SENSOR (compile-time)  │    │ MicroSD   │       │ WiFi      │
│  CFG 1: MH-Z19C (UART) + DHT22      │    │ (SPI)     │       │ (ESP32 /  │
│  CFG 2: SCD41 (I²C) + Soil (ADC)    │    └───────────┘       │  ESP8266) │
│  Battery Monitor (ADC) — semua cfg  │                        └───────────┘
└─────────────────────────────────────┘

 1 = SensorData {co2, temp, hum, batt_voltage, batt_percent}
 2 = PredictionResult {score, status A/B/C, days}
```

---

## 3. Struktur Direktori

```
main/
├── main.ino                  # Orkestrator: setup() & loop()
├── test_*.ino                # Sketch pengujian per komponen (tidak dipakai build final)
├── calibration/              # Sketch kalibrasi (soil moisture)
└── src/
    ├── Types.h               # Struktur data bersama (SensorData, PredictionResult)
    ├── Config.h              # Kredensial Blynk & WiFi (tidak di-commit)
    ├── SensorManager.{h,cpp} # Driver + agregasi sensor
    ├── RSLPredictor.{h,cpp}  # Model prediksi Arrhenius
    ├── StorageManager.{h,cpp}# Logging CSV ke MicroSD
    └── BlynkManager.{h,cpp}  # Konektivitas Blynk Cloud
```

---

## 4. Modul

| Modul | File | Tanggung Jawab | API Utama |
|---|---|---|---|
| **Orkestrator** | `main.ino` | Inisialisasi, siklus utama 1500 ms, build flags | `setup()`, `main_loop()` |
| **SensorManager** | `src/SensorManager.*` | Driver sensor (pola *null-pointer* sesuai config) & agregasi pembacaan | `begin()`, `readAll() → SensorData` |
| **RSLPredictor** | `src/RSLPredictor.*` | Hitung umur simpan: faktor suhu (Arrhenius) × CO₂ × kelembapan | `predict(SensorData) → PredictionResult` |
| **StorageManager** | `src/StorageManager.*` | Tulis CSV + header, cek ukuran file, baca file | `saveData()`, `getFileSize()` |
| **BlynkManager** | `src/BlynkManager.*` | Telemetri keluar, perintah masuk (singleton untuk callback BLYNK_WRITE) | `sendData()`, `run()`, flag perintah |

### Hierarki Sensor

Semua sensor mewarisi `BaseSensor` (interface `begin()`). `SensorManager` menerima pointer
dan hanya memanggil sensor yang tidak `nullptr` — konfigurasi dipilih saat **compile-time**:

| `SENSOR_CONFIG` | CO₂ | Suhu | Kelembapan | Lainnya |
|---|---|---|---|---|
| `1` | MH-Z19C (UART/PWM) | DHT22 | DHT22 (%RH) | Battery |
| `2` | SCD41 (I²C) | SCD41 | Soil Moisture (ADC, %) | Battery |

---

## 5. Alur Data (per siklus 1500 ms)

```
Sensor ─► SensorManager.readAll() ─► SensorData ─► RSLPredictor.predict()
                                                       │
                                       ┌───────────────┴───────────────┐
                                       ▼                               ▼
                          jika V11 = 1 (READ)              jika V12 = 1 (SAVE)
                            Blynk sendData()              StorageManager.saveData()
                              (V0–V9)                         (CSV ke SD)
```

Perintah masuk dari dashboard (via `BLYNK_WRITE` + singleton instance):

| Pin | Fungsi | Perilaku |
|---|---|---|
| V10 | Nama file target | Text input → `setFilename()` |
| V11 | Kirim data ke cloud | Tombol 0/1 → kirim kontinu |
| V12 | Simpan data ke SD | Tombol 0/1 → simpan kontinu |
| V13 | Minta ukuran file | *One-shot* → `getFileSize()` → V6 |

---

## 6. Pemetaan Virtual Pin Blynk (Keluar)

| Pin | Data | Satuan | Sumber |
|---|---|---|---|
| V0 | Suhu | °C | DHT22 / SCD41 |
| V1 | CO₂ | ppm | MH-Z19C / SCD41 |
| V2 | Kelembapan | % | DHT22 (CFG 1) / Soil (CFG 2) |
| V3 | Sisa hari umur simpan | hari | `prediction.days` |
| V4 | Skor kelayakan | 0–100 | `prediction.score` |
| V5 | Status | 0=A, 1=B, 2=C | `prediction.status` |
| V6 | Ukuran file | byte | `getFileSize()` |
| V8 | Tegangan baterai | V | BatteryMonitor |
| V9 | Persentase baterai | % | BatteryMonitor |

---

## 7. Konfigurasi Build (`main.ino`)

| Flag | Nilai | Efek |
|---|---|---|
| `DEBUG` | 0/1 | Log serial 115200 baud |
| `SENSOR_CONFIG` | 1/2 | Pilihan set sensor (dijaga `#error` guard) |
| `MHZ_SELF_CALIBRATION` | 0/1 | Kalibrasi otomatis ABC MH-Z19C |
| `MHZ_ZERO_CALIBRATION` | 0/1 | Kalibrasi titik nol saat boot (butuh udara segar) |
| `SCD_SELF_CALIBRATION` | 0/1 | ASC SCD41 (default off; persist ke EEPROM hanya saat nilai berubah) |
| `SCD_ZERO_CALIBRATION` | 0/1 | FRC SCD41 saat boot: tunggu 3 menit di udara segar ±420 ppm (blocking) |

Pin I/O didefinisikan sebagai konstanta `#define` di `main.ino:55-66`.

---

## 8. Prinsip Desain

- **Non-blocking**: siklus utama memakai `millis()` (1500 ms), tanpa `delay()` di jalur normal
- **Compile-time config**: pilihan sensor via preprocessor → tidak ada biaya runtime
- **Modular**: tiap manager bisa diuji sendiri (sketch `test_*.ino`)
- **Singleton pattern** pada `BlynkManager` untuk menjembatani macro callback Blynk (`BLYNK_WRITE`) ke method class
- **Umur simpan 12 hari max**, skor 0–100, status A/B/C — detail formula di [`Arrhenius.md`](Arrhenius.md)
