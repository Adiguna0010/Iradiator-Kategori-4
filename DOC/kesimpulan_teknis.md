# Kesimpulan Teknis & Configuration Pinout
## Purwarupa Iradiator Peraga (Simulasi Iradiator Kategori 4)

---

### 1. Ringkasan Proyek
- **Nama Proyek:** Purwarupa Iradiator Peraga (Simulasi Iradiator Kategori 4 IAEA)
- **Target Lokasi Penempatan:** Taman Pintar, Yogyakarta
- **Tujuan Utama:** Perangkat edukasi interaktif dan aman (*zero-radiation hazard*) untuk mensimulasikan mekanisme kerja, sistem keselamatan (*interlock*), dan otomatisasi kontrol fasilitas iradiasi nuklir komersial bagi pelajar dan masyarakat umum.
- **Pusat Kontrol (Otak Sistem):** Arduino Mega 2560
- **Antarmuka Pengguna (HMI):** HMI Nextion Touchscreen (Komunikasi Serial)

---

### 2. Pemetaan Pin (Pinout)

Pemetaan pin (*Pinout*) antara **Arduino Mega** dan komponen periferal disusun dengan konfigurasi berikut:

| No | Komponen Periferal | Pin / Detail Signal | Pin Arduino Mega | Deskripsi Fungsi |
|:--:|:-------------------|:-------------------:|:----------------:|:-----------------|
| **1** | **HMI Nextion** | `TX3` | Pin 14 | Transmit Serial 3 |
| | | `RX3` | Pin 15 | Receive Serial 3 |
| **2** | **Driver Motor Stepper 1 & 2 TB6600 (Sumber)** | `PULL` | Pin 3 | Pulse / Step Signal |
| | | `DIR` | Pin 2 | Direction Signal |
| **3** | **Driver Motor Stepper 3 (Rotary Table)** | `PULL` | Pin 6 | Pulse / Step Signal |
| | | `DIR` | Pin 5 | Direction Signal |
| **4** | **Lampu Indikator (Tower Light)** | Lampu Hijau | Pin 8 | Status Standby / Ready |
| | | Lampu Kuning | Pin 9 | Status Transisi Mekanis |
| | | Lampu Merah | Pin 10 | Status Penyinaran / Sumber Aktif |
| | | Buzzer | Pin 7 | Peringatan Suara / Warning Alert |

---

### 3. Arsitektur Hardware & Mekanisme Kerja

1. **HMI Nextion (Pin TX3/RX3):**
   - Berfungsi sebagai antarmuka sentuh visual tempat operator memilih parameter target (*cycle time* / dosis simulasi). Berkomunikasi dua arah dengan Arduino Mega 2560 melalui jalur **Serial 3**.

2. **Driver Motor Stepper 1 & 2 TB6600 - Sumber (PULL: 3, DIR: 2):**
   - Mengontrol pergerakan vertikal *lead screw* untuk menaikkan rak sumber radioaktif simulasi (*source rack*) ke area iradiasi dan meletakkannya kembali ke posisi aman.

3. **Driver Motor Stepper 3 TB6600 - Rotary Table (PULL: 6, DIR: 5):**
   - Menggerakkan meja pemutar (*Rotary Table*) tempat sampel diletakkan agar seluruh permukaan sampel menerima ekspos simulasi penyinaran secara merata.

4. **Lampu Indikator / Tower Light & Buzzer (Pin 7, 8, 9, 10):**
   - **Lampu Hijau (Pin 8):** Indikator sistem dalam keadaan siap (*Standby/Ready*).
   - **Lampu Kuning (Pin 9) & Buzzer (Pin 7):** Peringatan visual dan suara saat terjadi gerakan mekanis (naik/turun rak sumber atau perputaran *Rotary Table*).
   - **Lampu Merah (Pin 10):** Indikator bahwa proses simulasi penyinaran iradiasi sedang berlangsung.

---

### 4. Kesimpulan Teknis

1. **Integrasi HMI & Kontrol Utama:** Pertukaran data antara HMI Nextion dan Arduino Mega 2560 berlangsung secara *real-time* tanpa latensi berarti.
2. **Sinkronisasi Aktuator Stepper:** Konfigurasi pemetaan pin driver TB6600 (Stepper 1, 2, dan 3) memungkinkan pengontrolan independen yang presisi antara pergerakan vertikal rak sumber dan rotasi *Rotary Table*.
3. **Sistem Keamanan Visual (Tower Light):** Indikasi warna (Hijau, Kuning, Merah) dan Buzzer memberikan representasi sistem *safety interlock* yang jelas dan sesuai dengan standar fasilitas iradiasi nuklir komersial.

---

### 5. Rencana Pengembangan (Future Works)

1. **Desain Pintu Safety:** Menambahkan sistem interlock pintu enclosure untuk keamanan ekstra.
2. **Fitur Data Logging:** Mencatat histori proses dan parameter iradiasi.
3. **Fitur Login Multi-User:** Penambahan halaman Login Admin dan User pada HMI Nextion.
4. **Peningkatan Sistem Mekanis:** Menyempurnakan desain sistem agar semakin kompatibel dengan standar Iradiator Kategori 4.
5. **Pengembangan Conveyor:** Membuat konveyor yang lebih mendekati bentuk fisik Iradiator Kategori 4 komersial.
