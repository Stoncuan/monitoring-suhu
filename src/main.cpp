// CODE DARI TUTOR
// Note: Make sure these libraries are installed in platformio.ini:
// lib_deps = adafruit/DHT sensor library
#include <DHT.h>
#include <HTTPClient.h>
#include <WiFi.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

#define DATABASE_URL "https://monitoring-suhu-2692e-default-rtdb.asia-southeast1.firebasedatabase.app/data.json"
// #define DATABASE_URL "https://project-ds-iot-mufid-hr-default-rtdb.asia-southeast1.firebasedatabase.app/sensor%20dht.json"

#define DHTPIN 14
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);


void setup() {
  Serial.begin(115200);
  dht.begin();
  randomSeed(analogRead(0));

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected!");
}

void loop() {

  //float suhu = dht.readTemperature();
  float suhu = dht.readTemperature() + random(-7, 15) * 0.1;
  float kelembaban = dht.readHumidity() + random(-5, 10) * 0.1;

  Serial.println("=================================");
  
  if (!isnan(suhu) && !isnan(kelembaban)) {

    Serial.print("Suhu        : ");
    Serial.print(suhu);
    Serial.println(" °C");

    Serial.print("Kelembaban  : ");
    Serial.print(kelembaban);
    Serial.println(" %");

    HTTPClient http;
    http.begin(DATABASE_URL);
    http.addHeader("Content-Type", "application/json");

    String jsonData = "{\"suhu\":" + String(suhu) +
                      ",\"kelembaban\":" + String(kelembaban) + "}";

    Serial.println("Mengirim data ke Firebase...");
    Serial.println("JSON: " + jsonData);

    int httpResponseCode = http.PUT(jsonData);
    //int httpResponseCode = http.POST(jsonData);

    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) {
      Serial.println("Status: BERHASIL terkirim ✅");
    } else {
      Serial.println("Status: GAGAL ❌");
    }

    http.end();
  }
  else {
    Serial.println("Gagal membaca sensor DHT!");
  }

  Serial.println("Menunggu 5 detik...\n");
  delay(100);
}