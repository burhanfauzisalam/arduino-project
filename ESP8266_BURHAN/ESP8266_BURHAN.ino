#include <WiFiManager.h> // Library WiFiManager
#include <PubSubClient.h> // Library MQTT

// Inisialisasi broker MQTT
const char* mqtt_server = "47.129.50.58"; // Ganti dengan alamat broker Anda
const int mqtt_port = 8883;
const char* mqtt_topic = "esp8266/status";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousMillis = 0; // Variabel untuk menyimpan waktu sebelumnya
const long interval = 5000;       // Interval pengiriman data dalam milidetik

// Fungsi untuk menghubungkan ke broker MQTT
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke broker MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Terhubung ke broker MQTT");
      // Anda dapat menambahkan subscribe di sini jika diperlukan
    } else {
      Serial.print("Gagal, rc=");
      Serial.print(client.state());
      Serial.println(" Coba lagi dalam 5 detik...");
      delay(5000);
    }
  }
}

void setup() {
  // Konfigurasi pin dan Serial
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  // Membuat objek WiFiManager
  WiFiManager wifiManager;

  // Memulai auto-connect ke WiFi
  if (!wifiManager.autoConnect("ESP8266-V3")) {
    Serial.println("Gagal menyambung ke WiFi. Restart...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("Berhasil terhubung ke WiFi.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_BUILTIN, LOW);

  // Mengatur server dan port MQTT
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  // Cek status WiFi
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("Koneksi WiFi terputus! Mencoba menyambung ulang...");
    WiFiManager wifiManager;
    if (!wifiManager.autoConnect("ESP8266-V3")) {
      Serial.println("Gagal menyambung ulang. Restart...");
      delay(3000);
      ESP.restart();
    }
    digitalWrite(LED_BUILTIN, LOW);
  }

  // Cek koneksi ke broker MQTT
  if (!client.connected()) {
    reconnectMQTT();
  }

  // Kirim data ke MQTT menggunakan millis()
  client.loop(); // Pastikan koneksi tetap berjalan
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Contoh data yang akan dikirimkan
    const char* payload = "ONLINE";
    Serial.print("Mengirim data ke MQTT: ");
    Serial.println(payload);

    if (client.publish(mqtt_topic, payload)) {
      Serial.println("Data berhasil dikirim!");
    } else {
      Serial.println("Gagal mengirim data!");
    }
  }
}
