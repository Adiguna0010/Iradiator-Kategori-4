#include <Servo.h>

// --- DEKLARASI PIN INDIKATOR & RELAY (Active LOW) ---
const int pinLampuHijau  = 22; // Sistem Ready / Proses Jalan Aman
const int pinLampuKuning = 23; // Standby / Menunggu Parameter SOP
const int pinLampuMerah  = 24; // Bahaya / Emergency / Pintu Terbuka
const int pinBuzzer      = 25; // Alarm Warning

// --- DEKLARASI PIN DRIVER STEPPER ---
// Driver 1: Conveyor / Meja Putar (Maju - Mundur)
const int pinStepConv = 2;
const int pinDirConv  = 3;

// Driver 2 & 3: Motor Sumber Radiasi (Naik - Turun) digabung pararel/sinkron
const int pinStepSumber = 4;
const int pinDirSumber  = 5;

// --- DEKLARASI SERVO (PINTU RUANG IRADIASI) ---
Servo servoPintu;
const int pinServoPintu = 9;
const int POSISI_LOCK   = 0;   // Derajat saat pintu terkunci rapat
const int POSISI_UNLOCK = 90;  // Derajat saat kunci pintu terbuka

// --- VARIABEL OPERASIONAL & FISIKA ---
double doseRate = 0.456; // Ketetapan Laju Dosis (Gy/sec)
double targetDose = 0.0;
unsigned long totalIrradTime = 0; // Dalam milidetik
unsigned long sisaWaktu = 0;      // Dalam milidetik
bool isIrradiating = false;
bool isDoorLocked = false;
bool isDoorClosed = false;

// --- VARIABEL BUFFER SERIAL NEXTION ---
String inputBuffer = "";

void setup() {
  // Pemicuan Komunikasi Serial
  Serial.begin(9600);   // Serial Debug ke Laptop
  Serial1.begin(9600);  // Serial Komunikasi ke Layar Nextion
  
  // Konfigurasi Pin Out Indikator Relay
  pinMode(pinLampuHijau, OUTPUT);
  pinMode(pinLampuKuning, OUTPUT);
  pinMode(pinLampuMerah, OUTPUT);
  pinMode(pinBuzzer, OUTPUT);
  
  // Kondisi Awal Standby: Lampu Kuning Aktif
  digitalWrite(pinLampuHijau, HIGH); 
  digitalWrite(pinLampuKuning, LOW); // ON (Active Low)
  digitalWrite(pinLampuMerah, HIGH);
  digitalWrite(pinBuzzer, HIGH);

  // Konfigurasi Pin Stepper
  pinMode(pinStepConv, OUTPUT);   pinMode(pinDirConv, OUTPUT);
  pinMode(pinStepSumber, OUTPUT); pinMode(pinDirSumber, OUTPUT);
  
  // Inisialisasi Kunci Pintu Servo
  servoPintu.attach(pinServoPintu);
  servoPintu.write(POSISI_UNLOCK); // Awalnya unlock demi keamanan mekanik
}

void loop() {
  // 1. Baca input data dari HMI Nextion
  bacaSerialNextion();
  
  // 2. Jalankan Routine Utama jika Proses Iradiasi Aktif
  if (isIrradiating) {
    eksekusiSiklusIradiasi();
  }
}

// ==========================================
//          MANAJEMEN SERIAL NEXTION
// ==========================================
void bacaSerialNextion() {
  while (Serial1.available() > 0) {
    char incomingChar = Serial1.read();
    
    // Pola pembacaan string data kustom atau data angka dari Nextion
    if (incomingChar == '\n' || incomingChar == '\r') {
      prosesSinyalHMI(inputBuffer);
      inputBuffer = ""; // Reset buffer
    } else if ((byte)incomingChar != 0xFF) { // Abaikan byte penutup Nextion
      inputBuffer += incomingChar;
    }
  }
  
  // Membaca Hex data tunggal (printh) dari event tombol
  if (Serial1.available() >= 2) {
    byte header = Serial1.read();
    byte command = Serial1.read();
    if (header == 0x55) {
      eksekusiCommandHex(command);
    }
  }
}

void eksekusiCommandHex(byte cmd) {
  switch(cmd) {
    case 0x01: // Buka Pintu
      if(!isIrradiating && !isDoorLocked) {
        Serial.println("HMI: Perintah Buka Pintu");
        isDoorClosed = false;
        digitalWrite(pinLampuMerah, LOW); // Nyalakan indikator merah (Pintu Terbuka)
      }
      break;
      
    case 0x02: // Tutup Pintu
      Serial.println("HMI: Perintah Tutup Pintu");
      isDoorClosed = true;
      digitalWrite(pinLampuMerah, HIGH); // Matikan merah
      break;
      
    case 0x07: // Unlock Kunci Servo
      if (!isIrradiating) {
        servoPintu.write(POSISI_UNLOCK);
        isDoorLocked = false;
        Serial.println("Sistem Kunci Pintu: UNLOCKED");
      }
      break;
      
    case 0x08: // Lock Kunci Servo
      if (isDoorClosed) {
        servoPintu.write(POSISI_LOCK);
        isDoorLocked = true;
        Serial.println("Sistem Kunci Pintu: LOCKED (Aman)");
      }
      break;
      
    case 0x03: // Tombol START ditekan
      SOPValidationCheck();
      break;
      
    case 0x04: // Tombol EMERGENCY STOP ditekan
      pemicuEmergencyStop();
      break;
      
    case 0x06: // Tombol RETRACT ditekan
      Serial.println("HMI: Retract Aktif. Mengembalikan Sumber ke bawah...");
      retractSumberKePosisiAman();
      break;
  }
}

// Memproses penerimaan string nilai dosis dari HMI
void prosesSinyalHMI(String data) {
  if (data.startsWith("target=")) {
    String nilaiStr = data.substring(7);
    targetDose = nilaiStr.toFloat();
    
    if (targetDose > 0.0) {
      // Kalkulasi Otomatis Durasi Waktu berdasarkan Rumus Fisika (t = D / Laju)
      double hitungDetik = targetDose / doseRate;
      totalIrradTime = hitungDetik * 1000; // Konversi ke milidetik untuk timer
      
      Serial.print("Target Dosis Di-set: "); Serial.print(targetDose); Serial.println(" Gy");
      Serial.print("Durasi Waktu Terhitung: "); Serial.print(hitungDetik); Serial.println(" Detik");
      
      // Kirim hasil perhitungan kembali ke layar Nextion untuk di-update ke tTime
      Serial1.print("page2.tTime.txt=\"");
      Serial1.print(hitungDetik, 1);
      Serial1.print(" s\"");
      kirimEndCommand();
    }
  }
}

// ==========================================
//          VALIDASI SAFETY INTERLOCK
// ==========================================
void SOPValidationCheck() {
  // Syarat Utama: Target dosis sudah diisi, pintu wajib tertutup rapat, dan terkunci elektrik oleh servo
  if (targetDose > 0.0 && isDoorClosed && isDoorLocked) {
    Serial.println("SOP TERPENUHI. Memulai proses iradiasi...");
    
    // Bunyikan Buzzer Peringatan 2 detik sebelum alat bergerak
    digitalWrite(pinBuzzer, LOW);
    digitalWrite(pinLampuKuning, HIGH);
    digitalWrite(pinLampuMerah, LOW); // Merah kedip peringatan awal
    delay(2000);
    digitalWrite(pinBuzzer, HIGH);
    digitalWrite(pinLampuMerah, HIGH);
    
    digitalWrite(pinLampuHijau, LOW); // Indikator hijau aktif (Proses Radiasi Berjalan)
    isIrradiating = true;
    sisaWaktu = totalIrradTime;
    
    // Langkah Awal: Angkat Sumber Radiasi ke Posisi Aktif di Atas
    gerakkanSumber(true); 
  } 
  else {
    Serial.println("SOP GAGAL: Periksa kembali Dosis Target atau Kunci Pintu Ruang Radiasi!");
    // Tolak tombol start di HMI, paksa balik ke kondisi mati
    Serial1.print("page2.bStart.val=0");
    kirimEndCommand();
    
    // Bunyikan Alarm Pendek tanda error SOP
    digitalWrite(pinBuzzer, LOW); delay(300); digitalWrite(pinBuzzer, HIGH);
  }
}

// ==========================================
//          AKSI PROSES IRADIASI (MAIN CYCLE)
// ==========================================
void eksekusiSiklusIradiasi() {
  unsigned long waktuMulaiSiklus = millis();
  unsigned long waktuLalu = waktuMulaiSiklus;
  
  bool arahMajuConveyor = true;
  
  while (sisaWaktu > 0 && isIrradiating) {
    unsigned long waktuSekarang = millis();
    unsigned long delta = waktuSekarang - waktuLalu;
    
    if (delta >= 100) { // Update data HMI setiap 100 milidetik (Real-time & Smooth)
      if (sisaWaktu >= delta) {
        sisaWaktu -= delta;
      } else {
        sisaWaktu = 0;
      }
      waktuLalu = waktuSekarang;
      
      // 1. Hitung kalkulasi dosis akumulasi aktual saat ini
      unsigned long waktuBerjalanDetik = (totalIrradTime - sisaWaktu) / 1000;
      double dosisAktual = waktuBerjalanDetik * doseRate;
      
      // 2. Kirim update Sisa Waktu ke HMI
      Serial1.print("page2.tRemain.txt=\"");
      Serial1.print((double)sisaWaktu / 1000.0, 1);
      Serial1.print(" s\"");
      kirimEndCommand();
      
      // 3. Kirim update Akumulasi Dosis ke HMI
      Serial1.print("page2.tActual.txt=\"");
      Serial1.print(dosisAktual, 3);
      Serial1.print(" Gy\"");
      kirimEndCommand();
    }
    
    // --- GERAKKAN CONVEYOR / MEJA PUTAR (Maju-Mundur Non Blocking) ---
    // Simulasi mekanik meja bergerak bolak-balik membawa produk agar penyinaran homogen
    if (arahMajuConveyor) {
      digitalWrite(pinDirConv, HIGH);
    } else {
      digitalWrite(pinDirConv, LOW);
    }
    
    // Kirim pulsa langkah motor
    digitalWrite(pinStepConv, HIGH);
    delayMicroseconds(800); 
    digitalWrite(pinStepConv, LOW);
    delayMicroseconds(800);
    
    // Pembatas loop simulasi arah (misal berbalik setiap 3 detik)
    if ((millis() - waktuMulaiSiklus) % 6000 < 3000) {
      arahMajuConveyor = true;
    } else {
      arahMajuConveyor = false;
    }
    
    // Jalankan pengecekan serial darurat di tengah pergerakan motor
    if (Serial1.available() > 0) {
      bacaSerialNextion(); 
    }
  }
  
  if (isIrradiating) { // Jika keluar loop karena waktu habis alami
    selesaiSiklusNormal();
  }
}

void selesaiSiklusNormal() {
  Serial.println("Proses Penyinaran Selesai Penuh.");
  isIrradiating = false;
  
  // Turunkan Kembali Sumber ke Posisi Aman (Dasar Kolam / Shielding)
  gerakkanSumber(false);
  
  // Kembalikan status tombol start di HMI
  Serial1.print("page2.bStart.val=0"); kirimEndCommand();
  
  // Nyalakan alarm tanda sukses proses
  digitalWrite(pinLampuHijau, HIGH);
  digitalWrite(pinLampuKuning, LOW); // Kembali standby
  digitalWrite(pinBuzzer, LOW); delay(1000); digitalWrite(pinBuzzer, HIGH);
}

// ==========================================
//          KENDALAKAN MOTOR SUMBER (STEPPER)
// ==========================================
void gerakkanSumber(bool naik) {
  if (naik) {
    Serial.println("Mekanik: Menaikkan Rak Sumber Radiasi...");
    digitalWrite(pinDirSumber, HIGH);
  } else {
    Serial.println("Mekanik: Menurunkan Rak Sumber Radiasi...");
    digitalWrite(pinDirSumber, LOW);
  }
  
  // Misalkan butuh 4000 pulsa step untuk pergerakan penuh leadscrew
  for (int i = 0; i < 4000; i++) {
    digitalWrite(pinStepSumber, HIGH);
    delayMicroseconds(600);
    digitalWrite(pinStepSumber, LOW);
    delayMicroseconds(600);
  }
}

void retractSumberKePosisiAman() {
  // Fungsi manual paksa untuk memastikan motor menurunkan sumbunya kembali
  gerakkanSumber(false);
  Serial.println("Sumber berhasil diretraksi ke posisi dasar.");
}

// ==========================================
//          SISTEM KESELAMATAN EMERGENCY
// ==========================================
void pemicuEmergencyStop() {
  Serial.println("\n!!! EMERGENCY STOP DIAKTIFKAN !!!");
  isIrradiating = false;
  
  // 1. Hidupkan Alarm Bahaya & Lampu Merah Total
  digitalWrite(pinLampuHijau, HIGH);
  digitalWrite(pinLampuKuning, HIGH);
  digitalWrite(pinLampuMerah, LOW);  // ON Merah
  digitalWrite(pinBuzzer, LOW);     // Sirene ON Continuous
  
  // 2. Turunkan Rak Sumber Detik Itu Juga ke Posisi Paling Aman
  digitalWrite(pinDirSumber, LOW);
  for (int i = 0; i < 4000; i++) {
    digitalWrite(pinStepSumber, HIGH); delayMicroseconds(400); // Gerak cepat ke bawah
    digitalWrite(pinStepSumber, LOW);  delayMicroseconds(400);
  }
  
  // 3. Putar Balik Conveyor / Meja Sampel Keluar ke Jalur Awal
  digitalWrite(pinDirConv, LOW);
  for (int i = 0; i < 2000; i++) {
    digitalWrite(pinStepConv, HIGH); delayMicroseconds(500);
    digitalWrite(pinStepConv, LOW);  delayMicroseconds(500);
  }
  
  // 4. Buka Pengunci Pintu Otomatis agar Evakuasi Sampel Bisa Dilakukan
  servoPintu.write(POSISI_UNLOCK);
  isDoorLocked = false;
  
  // 5. Reset Tampilan Seluruh Grafik HMI ke Kondisi Nol / Mati
  Serial1.print("page2.bStart.val=0"); kirimEndCommand();
  Serial1.print("page2.tRemain.txt=\"0.0 s\""); kirimEndCommand();
  Serial1.print("page2.tActual.txt=\"0.000 Gy\""); kirimEndCommand();
  
  Serial.println("Sistem Berhasil Diamankan total. Menunggu reset manual hardware.");
  
  // Kunci sistem di loop tak terbatas sampai unit dimatikan/di-reboot operator
  while(true) {
    // Flash Lampu Merah untuk Menandakan Keadaan Bahaya Belum Direset
    digitalWrite(pinLampuMerah, LOW); delay(200);
    digitalWrite(pinLampuMerah, HIGH); delay(200);
  }
}

void kirimEndCommand() {
  Serial1.write(0xFF);
  Serial1.write(0xFF);
  Serial1.write(0xFF);
}
