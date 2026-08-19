# Purwarupa Iradiator Peraga (Iradiator Kategori 4)

Perangkat peraga edukasi fisik dan sistem otomatisasi kontrol untuk mensimulasikan mekanisme kerja fasilitas **Iradiator Kategori 4 (IAEA)** secara interaktif, aman (*zero-radiation hazard*), dan representatif. Ditargetkan untuk ditempatkan di Taman Pintar, Yogyakarta.

---

## 📌 Inti Spesifikasi Teknis

- **Pusat Kontrol (Brain):** Arduino Mega 2560
- **Antarmuka Pengguna (HMI):** HMI Nextion Touchscreen (Komunikasi Serial 3)
- **Aktuator Mekanis:** Motor Stepper (NEMA) + Driver TB6600 & Mekanisme *Lead Screw*
- **Sistem Indikator Keselamatan:** Tower Light (Hijau, Kuning, Merah) & Buzzer Alert

---

## 🔌 Pemetaan Pin (Pinout Configuration)

Berikut adalah konfigurasi pemetaan pinout resmi antara mikrokontroler **Arduino Mega 2560** dan seluruh periferal kontrol:

| No | Komponen Periferal | Signal / Line | Pin Arduino Mega 2560 | Fungsi Utama |
|:--:|:-------------------|:-------------:|:---------------------:|:-------------|
| **1** | **HMI Nextion** | `TX3` | Pin 14 | Transmit Serial 3 |
| | | `RX3` | Pin 15 | Receive Serial 3 |
| **2** | **Driver Motor Stepper 1 & 2 TB6600 (Sumber)** | `PULL` | Pin 3 | Signal Pulse / Step |
| | | `DIR` | Pin 2 | Signal Direction |
| **3** | **Driver Motor Stepper 3 (Rotary Table)** | `PULL` | Pin 6 | Signal Pulse / Step |
| | | `DIR` | Pin 5 | Signal Direction |
| **4** | **Lampu Indikator (Tower Light)** | Lampu Hijau | Pin 8 | Status *Standby* / *Ready* |
| | | Lampu Kuning | Pin 9 | Status Transisi Mekanis |
| | | Lampu Merah | Pin 10 | Status Penyinaran / Sumber Aktif |
| | | Buzzer | Pin 7 | *Warning Alert* / Peringatan Suara |

---

## ⚙️ Logika Operasional & Interlock System

1. **Standby Mode (Lampu Hijau ON):**
   Sistem siap (*ready*), HMI Nextion menerima input parameter durasi/dosis simulasi iradiasi dari operator.
2. **Transition Mode (Lampu Kuning ON + Buzzer Active):**
   Driver Stepper 1 & 2 (Pin 2, 3) menggerakkan rak sumber (*source rack*) secara vertikal via *lead screw*. Stepper 3 (Pin 5, 6) menggerakkan *Rotary Table* pemutar sampel.
3. **Irradiation Mode (Lampu Merah ON):**
   Sumber simulasi terekspos, proses penyinaran berlangsung sesuai *cycle time* yang ditentukan.
4. **Complete & Return:**
   Sumber kembali ke posisi aman, sistem memberi sinyal selesai dan kembali ke posisi *Standby* (Lampu Hijau ON).

---

## 📂 Struktur Repositori

```text
.
├── CODE PROGRAM/
│   └── programgabungan.ino                  # Source code utama Arduino Mega 2560
├── DOC/
│   ├── Dokumen RDD Iradiator.docx           # Dokumen RDD proyek
│   └── kesimpulan_teknis.md                 # Dokumentasi kesimpulan teknis & pinout
├── HMI/
│   └── HMI NEWW/
│       └── IRADIATOR MONITOR HMI Nextion.HMI # File desain antarmuka HMI Nextion
└── README.md
```

---

## 🚀 Rencana Pengembangan (Future Works)

1. Menambahkan sistem *safety interlock* pintu enclosure.
2. Fitur *Data Logging* untuk mencatat histori penyinaran.
3. Halaman Login Multi-User (Admin & User) pada HMI Nextion.
4. Pengembangan konveyor yang lebih representatif sesuai standar industri.
