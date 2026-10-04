# Food RSL Predictor

A portable IoT device that predicts the **Remaining Shelf Life (RSL)** of food by monitoring environmental conditions such as CO2 concentration, temperature, and humidity. Built around the **ESP32-S3**, the device logs sensor data locally to an SD card and streams it to the Blynk IoT platform for remote monitoring.

## Hardware Overview

### Main Controller

| Component | Description |
|---|---|
| **ESP32-S3 N16R8** | Dual-core Xtensa LX7 @ 240 MHz, 16 MB Flash, 8 MB PSRAM, Wi-Fi + BLE |

### Sensor Config 1

| Component | Measures | Power | Connections |
|---|---|---|---|
| **MH-Z19C** (NDIR CO2 sensor) | CO2 gas (400–5000 ppm) | 5 V | TX > GPIO17 (UART1), RX > GPIO18 (UART1), PWM > GPIO6 |
| **DHT22** (AM2302) | Temperature & humidity | 3.3 V | Data (AnalogOut) > GPIO2 |

### Sensor Config 2

| Component | Measures | Power | Connections |
|---|---|---|---|
| **Sensirion SCD41** (photoacoustic NDIR sensor) | CO2 gas & temperature | 3.3 V | SDA > GPIO8 (I2C), SCL > GPIO9 (I2C) |
| **Soil Moisture Sensor** | Soil humidity | 3.3 V | AnalogOut > GPIO1 (ADC) |

> **Note:** There are 2 sensor configurations. Either a single config or both can be used.

### Power System

| Component | Description | Connections |
|---|---|---|
| **MH-CD41** | Charging module & BMS for 1S Li-Ion battery | — |
| **AMS1117** | 3.3 V linear regulator | — |
| **Battery 1S1P 18650** | Li-Ion 3000 mAh power source | — |
| **Power Switch LED Button** | Switches the system ON/OFF | — |
| **Analog Battery Monitor** | Battery level indicator via voltage divider | BATT > GPIO4 (ADC) |

**Voltage divider:** R1 = 97.5 kΩ (100k), R2 = 97.5 kΩ (100k) — halves the battery voltage (max 4.2 V → 2.1 V), keeping the ADC input within the ESP32-S3's 3.3 V range.

### Storage

| Component | Description | Power | Connections |
|---|---|---|---|
| **MicroSD Card Module** | Local data logging | 5 V | MOSI > GPIO11, MISO > GPIO13, CLK > GPIO12, CS > GPIO10 (HSPI) |

## Pin Map Summary

| GPIO | Function | Peripheral |
|---|---|---|
| GPIO1 | ADC (Analog Input) | Soil Moisture |
| GPIO2 | Digital Input | DHT22 |
| GPIO4 | ADC (Analog Input) | Battery Monitor |
| GPIO6 | PWM Input | MH-Z19C |
| GPIO8 | I2C SDA | SCD41 |
| GPIO9 | I2C SCL | SCD41 |
| GPIO10 | SPI CS | SD Card |
| GPIO11 | SPI MOSI | SD Card |
| GPIO12 | SPI CLK | SD Card |
| GPIO13 | SPI MISO | SD Card |
| GPIO17 | UART1 RX | MH-Z19C TX |
| GPIO18 | UART1 TX | MH-Z19C RX |
