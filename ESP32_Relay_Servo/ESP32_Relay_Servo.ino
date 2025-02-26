#include <WiFiManager.h> // Library WiFiManager
#include <PubSubClient.h> // Library MQTT
#include <ESP32Servo.h> // Library Servo

// Inisialisasi broker MQTT
const char* mqtt_server = "192.168.0.101"; // Ganti dengan alamat broker Anda
const int mqtt_port = 8883;

// Variabel yang bisa diubah
const char* deviceId = "device01"; // Ubah sesuai kebutuhan
char clientId[20]; // Buffer untuk Client ID unik

void generateUniqueClientId() {
  snprintf(clientId, sizeof(clientId), "Smarthome-%d", random(1000, 9999));
}


// Format topik MQTT
typedef struct {
  char status[50];
  char control[50];
} MqttTopics;

MqttTopics mqtt_topics;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousMillis = 0; // Variabel untuk menyimpan waktu sebelumnya
const long interval = 3000;       // Interval pengiriman data dalam milidetik

// Pin untuk kontrol
const int servoPin = 0;  // GPIO 0 untuk Servo
const int relayPin = 2;  // GPIO 2 untuk Relay

Servo myServo; // Objek Servo

// Fungsi untuk membentuk topik MQTT
void updateMqttTopics() {
  snprintf(mqtt_topics.status, sizeof(mqtt_topics.status), "esp01/status/%s", deviceId);
  snprintf(mqtt_topics.control, sizeof(mqtt_topics.control), "esp01/relay/%s", deviceId);
}

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

  // Kontrol berdasarkan topik dan pesan
  if (String(topic) == mqtt_topics.control) {
    if (message == "SERVO_OPEN") {
      myServo.write(0); // Servo ke posisi 90 derajat
      Serial.println("Servo bergerak ke 90 derajat");
    } else if (message == "SERVO_CLOSE") {
      myServo.write(160); // Servo ke posisi 0 derajat
      Serial.println("Servo bergerak ke 0 derajat");
    } else if (message == "RELAY2_ON") {
      digitalWrite(relayPin, HIGH); // Aktifkan relay 
      Serial.println("Relay ON");
    } else if (message == "RELAY2_OFF") {
      digitalWrite(relayPin, LOW); // Matikan relay 
      Serial.println("Relay OFF");
    } else {
      Serial.println("Perintah tidak dikenali!");
    }
  }
}

// Fungsi untuk menghubungkan ke broker MQTT
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke broker MQTT...");
    if (client.connect(clientId)) {
      Serial.println("Terhubung ke broker MQTT");
      client.subscribe(mqtt_topics.control);
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
  pinMode(relayPin, OUTPUT);
  Serial.begin(115200);

  generateUniqueClientId();
  
  // Inisialisasi topik MQTT
  updateMqttTopics();

  // Inisialisasi servo
  myServo.attach(servoPin);
  myServo.write(0); // Pastikan posisi awal servo di 0 derajat

  // Membuat objek WiFiManager
  WiFiManager wifiManager;

  // Menyesuaikan nama AP dengan deviceId
  String apName = "Smarthome-" + String(deviceId);

  // Memulai auto-connect ke WiFi
  if (!wifiManager.autoConnect(apName.c_str())) {
    Serial.println("Gagal menyambung ke WiFi. Restart...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("Berhasil terhubung ke WiFi.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Mengatur server dan port MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  // Cek status WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Koneksi WiFi terputus! Mencoba menyambung ulang...");
    WiFiManager wifiManager;
    String apName = "Smarthome-" + String(deviceId);
    if (!wifiManager.autoConnect(apName.c_str())) {
      Serial.println("Gagal menyambung ulang. Restart...");
      delay(3000);
      ESP.restart();
    }
  }

  // Cek koneksi ke broker MQTT
  if (!client.connected()) {
    reconnectMQTT();
  }

  client.loop();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    const char* payload = "ONLINE";
    Serial.print("Mengirim data ke MQTT: ");
    Serial.println(payload);
    if (client.publish(mqtt_topics.status, payload)) {
      Serial.println("Data berhasil dikirim!");
    } else {
      Serial.println("Gagal mengirim data!");
    }
  }
}
