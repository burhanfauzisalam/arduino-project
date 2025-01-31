#include <WiFiManager.h> // Library WiFiManager
#include <PubSubClient.h> // Library MQTT

// Inisialisasi broker MQTT
const char* mqtt_server = "54.179.124.238"; // Ganti dengan alamat broker Anda
const int mqtt_port = 8883;
const char* mqtt_topic_status = "esp01/status/device01";
const char* mqtt_topic_control = "esp01/relay/device01";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousMillis = 0; // Variabel untuk menyimpan waktu sebelumnya
const long interval = 3000;       // Interval pengiriman data dalam milidetik

// Pin relay
const int relayPin1 = 0;
const int relayPin2 = 2;

// Fungsi callback untuk menerima pesan MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Pesan diterima di topik: ");
  Serial.println(topic);
  Serial.print("Isi pesan: ");
  Serial.println(message);

  // Kontrol relay berdasarkan topik dan pesan
  if (String(topic) == mqtt_topic_control) {
    if (message == "RELAY1_ON") {
      digitalWrite(relayPin1, HIGH); // Aktifkan relay 1 (LOW untuk aktif tergantung modul)
      Serial.println("Relay 1 ON");
    } else if (message == "RELAY1_OFF") {
      digitalWrite(relayPin1, LOW); // Matikan relay 1
      Serial.println("Relay 1 OFF");
    } else if (message == "RELAY2_ON") {
      digitalWrite(relayPin2, HIGH); // Aktifkan relay 2
      Serial.println("Relay 2 ON");
    } else if (message == "RELAY2_OFF") {
      digitalWrite(relayPin2, LOW); // Matikan relay 2
      Serial.println("Relay 2 OFF");
    } else {
      Serial.println("Perintah tidak dikenali!");
    }
  }
}

// Fungsi untuk menghubungkan ke broker MQTT
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke broker MQTT...");
    if (client.connect("ESP01Client")) {
      Serial.println("Terhubung ke broker MQTT");
      // Subscribe ke topik kontrol
      client.subscribe(mqtt_topic_control);
      Serial.println("Subscribe ke topik kontrol berhasil!");
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
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);

  Serial.begin(115200);

  // Membuat objek WiFiManager
  WiFiManager wifiManager;

  // Memulai auto-connect ke WiFi
  if (!wifiManager.autoConnect("ESP01-Relay")) {
    Serial.println("Gagal menyambung ke WiFi. Restart...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("Berhasil terhubung ke WiFi.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Mengatur server dan port MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); // Mengatur callback untuk menerima pesan MQTT
}

void loop() {
  // Cek status WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Koneksi WiFi terputus! Mencoba menyambung ulang...");
    WiFiManager wifiManager;
    if (!wifiManager.autoConnect("ESP8266-Relay")) {
      Serial.println("Gagal menyambung ulang. Restart...");
      delay(3000);
      ESP.restart();
    }
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

    if (client.publish(mqtt_topic_status, payload)) {
      Serial.println("Data berhasil dikirim!");
    } else {
      Serial.println("Gagal mengirim data!");
    }
  }
}
