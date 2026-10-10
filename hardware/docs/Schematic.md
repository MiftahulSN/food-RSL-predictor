# Skema Pengkabelan (Electrical Wiring) — food-RSL-predictor

Diagram visual tersedia di: [`../images/Schematic.png`](../images/Schematic.png)

Sumber: `README.md` (Hardware Overview) dan definisi pin di `main/main.ino:55-66`.
Kendali utama: **ESP32-S3 N16R8** (dual-core Xtensa LX7, 16 MB Flash, 8 MB PSRAM, Wi-Fi + BLE).

---

## 1. Gambaran Umum

Sistem terdiri dari lima kelompok rangkaian:

| Kelompok | Komponen | Aktif pada |
|---|---|---|
| Kendali | ESP32-S3 N16R8 | Semua config |
| Sensor CFG 1 | MH-Z19C (CO₂), DHT22 (Temp/RH) | `SENSOR_CONFIG 1` |
| Sensor CFG 2 | SCD41 (CO₂/Temp), Soil Moisture | `SENSOR_CONFIG 2` |
| Penyimpanan | Modul MicroSD (SPI) | Semua config |
| Daya & monitor | 18650 1S1P, MH-CD41, switch, AMS1117, voltage divider | Semua config |

> Catatan: folder `hardware/datasheet/` juga berisi datasheet **MQ-7** (sensor CO) —
> komponen cadangan yang **tidak terpakai** pada desain saat ini.

---

## 2. Arsitektur Daya

```
                        (USB charger)
                             │
[18650 1S1P 3,7 V] ⇄ B+/B− ⇄ [MH-CD41]──►[Switch LED]──► RAIL 5V ──►[AMS1117]──► RAIL 3.3V
 3000 mAh Li-Ion           charger + BMS                                  │                 │
     │                     + boost 5 V                                    ▼                 ▼
     │ BATT+ (sense)                                              MH-Z19C (CFG 1)    ESP32-S3
     └──►[Divider R1 100k / R2 100k]──► GPIO4 (ADC)               MicroSD            DHT22 (CFG 1)
                                                                                  SCD41 (CFG 2)
                                                                                  Soil Moisture (CFG 2)
```

### Tabel rail daya

| Rail | Sumber | Beban |
|---|---|---|
| **5 V** | MH-CD41 (via switch) | MH-Z19C, Modul MicroSD |
| **3.3 V** | AMS1117 (dari rail 5 V) | ESP32-S3, DHT22, SCD41, Soil Moisture |
| **BATT+** | Langsung dari sel 18650 (sebelum switch) | Voltage divider monitor → GPIO4 |
| **GND** | Common (semua komponen) | — |

---

## 3. Wiring per Modul

### 3.1 MH-Z19C — NDIR CO₂ (SENSOR_CONFIG 1, catu 5 V)

| Pin MH-Z19C | Terhubung ke | Fungsi |
|---|---|---|
| Vin | Rail 5 V | Catu daya |
| GND | GND | Ground |
| TX | **GPIO18** | UART1 RX (data sensor → MCU) |
| RX | **GPIO17** | UART1 TX (perintah MCU → sensor) |
| PWM | **GPIO6** | Alternatif pembacaan CO₂ via PWM (tidak dipakai bila UART) |

### 3.2 DHT22 / AM2302 — Temp & RH (SENSOR_CONFIG 1, catu 3.3 V)

| Pin DHT22 | Terhubung ke | Fungsi |
|---|---|---|
| VCC | Rail 3.3 V | Catu daya |
| DATA | **GPIO2** | Data 1-wire |
| GND | GND | Ground |

### 3.3 SCD41 — Sensirion (SENSOR_CONFIG 2, catu 3.3 V)

| Pin SCD41 | Terhubung ke | Fungsi |
|---|---|---|
| VDD | Rail 3.3 V | Catu daya |
| GND | GND | Ground |
| SDA | **GPIO8** | I²C data |
| SCL | **GPIO9** | I²C clock |

### 3.4 Soil Moisture (SENSOR_CONFIG 2, catu 3.3 V)

| Pin Sensor | Terhubung ke | Fungsi |
|---|---|---|
| VCC | Rail 3.3 V | Catu daya |
| AOUT | **GPIO1** | Analog (ADC) |
| GND | GND | Ground |

### 3.5 Modul MicroSD (catu 5 V)

| Pin Modul | Terhubung ke | Fungsi |
|---|---|---|
| VCC | Rail 5 V | Catu (regulator 3.3 V + level-shifter onboard) |
| GND | GND | Ground |
| CS | **GPIO10** | Chip select |
| MOSI | **GPIO11** | Data MCU → kartu |
| CLK | **GPIO12** | Clock |
| MISO | **GPIO13** | Data kartu → MCU |

### 3.6 Battery Monitor (voltage divider)

| Elemen | Terhubung ke | Fungsi |
|---|---|---|
| R1 = 100 kΩ (terukur 97,5 kΩ) | BATT+ → node ADC | Bagi tegangan |
| R2 = 100 kΩ (terukur 97,5 kΩ) | Node ADC → GND | Bagi tegangan |
| Node ADC | **GPIO4** | Pembacaan V_batt/2 |

---

## 4. Pin Map ESP32-S3

| GPIO | Kanal/Periferal | Arah | Terhubung | Config |
|---|---|---|---|---|
| 1 | ADC1_CH0 | In | Soil AOUT | 2 |
| 2 | ADC1_CH1 | In | DHT22 DATA | 1 |
| 4 | ADC1_CH3 | In | Divider baterai | semua |
| 6 | — (bebas) | In | MH-Z19C PWM | 1 |
| 8 | I²C SDA (default) | Bi | SCD41 SDA | 2 |
| 9 | I²C SCL (default) | Bi | SCD41 SCL | 2 |
| 10 | SPI CS (FSPI) | Out | MicroSD CS | semua |
| 11 | SPI MOSI (FSPI) | Out | MicroSD MOSI | semua |
| 12 | SPI CLK (FSPI) | Out | MicroSD CLK | semua |
| 13 | SPI MISO (FSPI) | In | MicroSD MISO | semua |
| 17 | UART1 TX (default) | Out | MH-Z19C RX | 1 |
| 18 | UART1 RX (default) | In | MH-Z19C TX | 1 |

Catatan desain pin:
- Kedua input analog (GPIO1, GPIO4) berada di **ADC1** — tidak berkonflik dengan Wi-Fi
- Tidak ada pin yang memakai **strapping pin** ESP32-S3 (GPIO0, 3, 45, 46)
- GPIO19/20 (USB D−/D+) dan GPIO43/44 (UART0 debug) tidak dipakai sensor — tetap tersedia

---

## 5. Perhitungan Voltage Divider Baterai

$$V_{ADC} = V_{BATT} \times \frac{R_2}{R_1 + R_2} = \frac{V_{BATT}}{2}$$

| Kondisi baterai | V_BATT | V_ADC (ke GPIO4) |
|---|---|---|
| Penuh | 4,2 V | 2,1 V |
| Nominal | 3,7 V | 1,85 V |
| Low-batt (0%) | 3,0 V | 1,5 V |

- Maksimum 2,1 V aman untuk ADC ESP32-S3 (≤ 3,3 V)
- Kode (`SensorManager.cpp`): `readVoltage() = analogReadMilliVolts(BATT_PIN)/1000 × divider_ratio` dengan `divider_ratio = 2.0`
- `readPercent()` memetakan 3,0–4,2 V → 0–100%

---

## 6. Catatan Kelistrikan

- **Common ground**: seluruh GND komponen (5 V dan 3.3 V) terhubung ke satu bus ground
- **MH-Z19C**: catu 5 V, tetapi level I/O UART/PWM-nya TTL 3,3 V → aman langsung ke ESP32-S3
- **Modul MicroSD**: catu 5 V (modul umum punya regulator AMS1117 + level-shifter onboard sehingga logikanya aman untuk 3,3 V)
- **MH-CD41**: modul charge/discharge dengan proteksi BMS; BATT+ untuk sense divider diambil langsung dari sel (sebelum switch) agar pembacaan level baterai tetap tersedia
- **Pilihan UART vs PWM MH-Z19C**: desain memakai salah satu (default UART, `SensorManager.cpp:195`)

---

## 7. Referensi Datasheet (lokal)

`hardware/datasheet/`: `MH-Z19C.pdf` · `dht-22.pdf` · `SCD4x.pdf` · `soil-moisture-sensor.pdf` · `micro-sd-card-module.pdf` · `ESP32-S3 pinout.jpg`
