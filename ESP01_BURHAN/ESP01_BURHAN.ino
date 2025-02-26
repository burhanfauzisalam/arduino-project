#include <WiFiManager.h> // Library WiFiManager
#include <PubSubClient.h> // Library MQTT
#include <Servo.h> // Library Servo
#include <BearSSLHelpers.h>
#include <WiFiClientSecureBearSSL.h>

// Inisialisasi broker MQTT
const char* mqtt_server = "090cd3b5566041ae929bfd1b7420f5b1.s1.eu.hivemq.cloud"; // Ganti dengan alamat broker Anda
const int mqtt_port = 8883;

// Sertifikat Root CA Let's Encrypt (ISRG Root X1)
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n"
"MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n"
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n"
"WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n"
"ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n"
"MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n"
"h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n"
"0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n"
"A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n"
"T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n"
"B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n"
"B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n"
"KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n"
"OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n"
"jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n"
"qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n"
"rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n"
"HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n"
"hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n"
"ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n"
"3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n"
"NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n"
"ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n"
"TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n"
"jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n"
"oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n"
"4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n"
"mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n"
"emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n"
"-----END CERTIFICATE-----";

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

BearSSL::WiFiClientSecure espClient;
BearSSL::X509List cert(root_ca); // Gunakan X509List untuk menyimpan sertifikat
PubSubClient client(espClient);

unsigned long previousMillis = 0; // Variabel untuk menyimpan waktu sebelumnya
const long interval = 3000;       // Interval pengiriman data dalam milidetik

// Pin untuk kontrol
const int servoPin = 0;  // untuk Servo
const int relayPin = 2;  // untuk Relay

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
      myServo.write(25); // Servo ke posisi 90 derajat
      Serial.println("Servo bergerak ke 90 derajat");
    } else if (message == "SERVO_CLOSE") {
      myServo.write(135); // Servo ke posisi 0 derajat
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
    if (client.connect(clientId, "burhan","CahayaUtara2020")) {
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
  espClient.setTrustAnchors(&cert);
  espClient.setInsecure(); // Jika tidak perlu sertifikat client

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
