# Persamaan Prediksi RSL (Remaining Shelf Life)

Rumusan matematis yang diimplementasikan pada `main/src/RSLPredictor.cpp` dan `main/src/RSLPredictor.h`.
Ringkasan persamaan tersedia dalam bentuk gambar: [`../images/Arrhenius.png`](../images/Arrhenius.png)

---

## Parameter Konstanta

| Simbol | Nilai | Konstanta Kode | Keterangan |
|---|---|---|---|
| $E_a$ | 50.000 J/mol | `ACTIVATION_ENERGY` | Energi aktivasi |
| $R$ | 8,314 J/(mol·K) | `GAS_CONSTANT` | Konstanta gas universal |
| $A$ | 10¹⁰ | `PRE_EXPONENTIAL` | Faktor pre-eksponensial |
| $T_{ref}$ | 298,15 K | `REFERENCE_TEMP` | Suhu referensi (25 °C) |
| $k_{CO_2}$ | 0,001 ppm⁻¹ | `CO2_DECAY_FACTOR` | Faktor peluruhan CO₂ |
| $RH_{opt}$ | 10 % | `HUM_OPTIMAL_RANGE` | Toleransi deviasi kelembapan |
| $SL_0$ | 6 hari | `BASE_SHELF_LIFE` | Umur simpan dasar |

---

## 1. Faktor Suhu — Persamaan Arrhenius

Konversi suhu ke Kelvin:

$$T = T_C + 273{,}15$$

Konstanta laju reaksi (Arrhenius):

$$k = A \cdot e^{-\dfrac{E_a}{R \cdot T}}, \qquad k_{ref} = A \cdot e^{-\dfrac{E_a}{R \cdot T_{ref}}}$$

Rasio laju degradasi akibat suhu:

$$f_T = \frac{k}{k_{ref}} = e^{\dfrac{E_a}{R}\left(\dfrac{1}{T_{ref}} - \dfrac{1}{T}\right)}$$

> Implementasi: `RSLPredictor.cpp:7-10`

---

## 2. Faktor Degradasi CO₂

$$f_{CO_2} = \begin{cases} 1 + 0{,}001 \,(C - 400) & \text{jika } C > 400 \text{ ppm} \\[6pt] 1 & \text{jika } C \le 400 \text{ ppm} \end{cases}$$

dengan $C$ = konsentrasi CO₂ (ppm). Di atas 400 ppm (ambang udara segar), peningkatan CO₂ mempercepat degradasi.

> Implementasi: `RSLPredictor.cpp:13-16`

---

## 3. Faktor Degradasi Kelembapan

Deviasi dari kelembapan optimal (60 %):

$$d = |\,RH - 60\,|$$

Faktor kelembapan (dibatasi maksimum 3):

$$f_{RH} = \min\left(3,\; \begin{cases} 1 + 0{,}02\,(d - 10) & \text{jika } d > 10\ \% \\[6pt] 1 & \text{jika } d \le 10\ \% \end{cases}\right)$$

> Implementasi: `RSLPredictor.cpp:19-24`

---

## 4. Laju Degradasi Total & Estimasi Umur Simpan

$$D = f_T \cdot f_{CO_2} \cdot f_{RH}$$

Umur simpan tersisa (dibatasi maksimum 12 hari):

$$SL = \min\left(12,\; \frac{SL_0}{D}\right), \qquad SL \ge 0$$

> Implementasi: `RSLPredictor.cpp:27-31`

---

## 5. Skor Kelayakan (0–100 %)

Hari yang telah berlalu:

$$t_{elapsed} = \max\left(0,\; SL_0 - SL\right)$$

Skor kelayakan:

$$Score = \mathrm{clip}\left(100 - \frac{t_{elapsed}}{SL_0} \times 100,\; 0,\; 100\right)$$

> Implementasi: `RSLPredictor.cpp:33-39`

---

## 6. Status Kelayakan & Sisa Hari

$$t_{age} = SL_0 - SL$$

$$\text{Status} = \begin{cases} \text{A (segar)} & t_{age} \le 3 \text{ hari} \\[4pt] \text{B (layak)} & 3 < t_{age} \le 6 \text{ hari} \\[4pt] \text{C (tidak layak)} & t_{age} > 6 \text{ hari} \end{cases}$$

Sisa hari umur simpan (pembulatan ke bawah):

$$\text{days} = \lfloor SL \rfloor$$

> Implementasi: `RSLPredictor.cpp:41-53`

---

## Ringkasan Alur Perhitungan

```
Suhu (°C) ──► Arrhenius ──► f_T ─┐
CO₂ (ppm) ───────────────► f_CO2 ─┼─► D = f_T·f_CO2·f_RH ─► SL = min(12, 6/D)
RH (%) ──────────────────► f_RH ─┘            │
                                              ├─► Score  = clip(100·(1 - t_elapsed/6), 0, 100)
                                              ├─► Status = A | B | C
                                              └─► Days   = floor(SL)
```
