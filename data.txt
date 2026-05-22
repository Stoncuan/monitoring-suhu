// ============================================================
// LIBRARY
// Mengimpor library yang dibutuhkan agar fitur-fiturnya bisa dipakai
// ============================================================

#include <DHT.h>        // Library untuk membaca sensor suhu & kelembaban DHT
#include <HTTPClient.h> // Library untuk mengirim request HTTP (ke Firebase)
#include <WiFi.h>       // Library untuk koneksi WiFi pada ESP32

// ============================================================
// KONFIGURASI WiFi & FIREBASE
// ============================================================

#define WIFI_SSID "Wokwi-GUEST"   // Nama jaringan WiFi yang akan dikoneksikan
#define WIFI_PASSWORD ""           // Password WiFi (kosong = tanpa password)

// URL endpoint Firebase Realtime Database tempat data akan disimpan
#define DATABASE_URL "https://monitoring-suhu-2692e-default-rtdb.asia-southeast1.firebasedatabase.app/data.json"

// ============================================================
// KONFIGURASI SENSOR DHT
// ============================================================

#define DHTPIN 14    // Pin GPIO ESP32 yang terhubung ke kaki DATA sensor DHT
#define DHTTYPE DHT22 // Tipe sensor: DHT22 (lebih akurat dibanding DHT11)

DHT dht(DHTPIN, DHTTYPE); // Membuat objek 'dht' untuk mengakses sensor

// ============================================================
// SETUP — Hanya berjalan SEKALI saat ESP32 pertama kali menyala
// ============================================================

void setup() {

  Serial.begin(115200);
  // Mengaktifkan komunikasi serial dengan baud rate 115200
  // Digunakan untuk menampilkan pesan debug di Serial Monitor

  dht.begin();
  // Menginisialisasi sensor DHT agar siap membaca data

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // Memulai proses koneksi ke jaringan WiFi

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    // Selama belum terhubung ke WiFi, terus tunggu
    Serial.print(".");  // Cetak titik setiap 500ms sebagai indikator loading
    delay(500);         // Tunggu 500 milidetik sebelum cek ulang
  }

  Serial.println("\nConnected!");
  // Jika sudah terhubung, cetak "Connected!" dan lanjut ke loop()
}

// ============================================================
// LOOP — Berjalan TERUS-MENERUS setelah setup() selesai
// ============================================================

void loop() {

  // ----------------------------------------------------------
  // LANGKAH 1: Baca data dari sensor DHT22
  // ----------------------------------------------------------

  float suhu = dht.readTemperature();
  // Membaca suhu dalam satuan Celsius, disimpan sebagai angka desimal (float)

  float kelembaban = dht.readHumidity();
  // Membaca kelembaban dalam satuan persen (%), disimpan sebagai float

  Serial.println("=================================");
  // Garis pemisah di Serial Monitor agar output lebih rapi

  // ----------------------------------------------------------
  // LANGKAH 2: Validasi data sensor
  // ----------------------------------------------------------

  if (!isnan(suhu) && !isnan(kelembaban)) {
    // isnan() = cek apakah nilai adalah "Not a Number" (NaN)
    // Sensor mengembalikan NaN jika gagal membaca (kabel lepas, error, dll)
    // !isnan() = pastikan nilai BUKAN NaN (artinya data valid)
    // Kedua nilai harus valid agar data dikirim ke Firebase

    // ----------------------------------------------------------
    // LANGKAH 3: Tampilkan data di Serial Monitor
    // ----------------------------------------------------------

    Serial.print("Suhu        : ");
    Serial.print(suhu);          // Cetak nilai suhu
    Serial.println(" °C");       // Cetak satuan dan pindah baris

    Serial.print("Kelembaban  : ");
    Serial.print(kelembaban);    // Cetak nilai kelembaban
    Serial.println(" %");        // Cetak satuan dan pindah baris

    // ----------------------------------------------------------
    // LANGKAH 4: Siapkan koneksi HTTP ke Firebase
    // ----------------------------------------------------------

    HTTPClient http;
    // Membuat objek HTTPClient untuk mengelola koneksi HTTP

    http.begin(DATABASE_URL);
    // Membuka koneksi ke URL Firebase yang sudah didefinisikan di atas

    http.addHeader("Content-Type", "application/json");
    // Memberitahu server bahwa data yang dikirim berformat JSON

    // ----------------------------------------------------------
    // LANGKAH 5: Buat data JSON
    // ----------------------------------------------------------

    String jsonData = "{\"suhu\":" + String(suhu) +
                      ",\"kelembaban\":" + String(kelembaban) + "}";
    // Membuat string JSON secara manual
    // Hasil contoh: {"suhu":27.50,"kelembaban":65.30}
    // Tanda \" digunakan karena tanda " harus di-escape dalam String C++

    Serial.println("Mengirim data ke Firebase...");
    Serial.println("JSON: " + jsonData); // Tampilkan JSON yang akan dikirim

    // ----------------------------------------------------------
    // LANGKAH 6: Kirim data ke Firebase dengan metode PUT
    // ----------------------------------------------------------

    int httpResponseCode = http.PUT(jsonData);
    // Mengirim request HTTP PUT ke Firebase dengan isi data jsonData
    // PUT = menimpa/memperbarui data yang sudah ada di database
    // Menyimpan kode respons HTTP dari server ke variabel httpResponseCode

    // Perbedaan PUT vs POST:
    // PUT  → Update/timpa data yang sudah ada (digunakan di sini)
    // POST → Tambah data baru dengan ID otomatis dari Firebase

    // ----------------------------------------------------------
    // LANGKAH 7: Cek hasil pengiriman
    // ----------------------------------------------------------

    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);
    // Menampilkan kode respons HTTP:
    // 200 = OK / Berhasil
    // 400 = Bad Request (format data salah)
    // 401 = Unauthorized (tidak punya akses)
    // 500 = Internal Server Error (masalah di server)
    // -1  = Tidak bisa konek ke server

    if (httpResponseCode == 200) {
      Serial.println("Status: BERHASIL terkirim ✅");
    } else {
      Serial.println("Status: GAGAL ❌");
    }

    // ----------------------------------------------------------
    // LANGKAH 8: Tutup koneksi HTTP
    // ----------------------------------------------------------

    http.end();
    // Menutup koneksi HTTP dan membebaskan resource memori
    // Penting dilakukan setelah selesai agar tidak terjadi memory leak

  } else {
    // Jika salah satu atau kedua nilai sensor adalah NaN
    Serial.println("Gagal membaca sensor DHT!");
    // Kemungkinan penyebab: kabel longgar, sensor rusak, atau pin salah
  }

  // ----------------------------------------------------------
  // LANGKAH 9: Tunggu sebelum membaca ulang
  // ----------------------------------------------------------

  Serial.println("Menunggu 5 detik...\n");
  delay(5000);
  // Menunda eksekusi selama 5000 milidetik = 5 detik
  // Setelah 5 detik, loop() akan kembali ke LANGKAH 1
}