#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

// Definisi pin untuk 3 lampu
#define LED1_PIN D5  // Lampu 1 di Pin D5
#define LED2_PIN D6  // Lampu 2 di Pin D6
#define LED3_PIN D7  // Lampu 3 di Pin D7

// Ganti dengan SSID dan Password WiFi Anda
const char* ssid = "TP-Link_Lantai1";
const char* password = "Nola2024";

// Ganti dengan alamat broker MQTT Anda
const char* mqtt_server = "34.68.99.84";
const int mqtt_port = 8883; // Port standar untuk MQTT

WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);

  // Atur pin LED sebagai output
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);  // LED built-in

  // Connect to WiFi
  wifiManager.autoConnect("ESP8266AP", "12345678");
  // setupWiFi();

  // Mengatur MQTT Client
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); // Callback untuk menangani pesan MQTT
}

unsigned long previousMillis = 0;  // Waktu terakhir LED berkedip
const long interval = 100;         // Interval kedipan (100 ms)
bool ledState = LOW;               // State LED built-in

void loop() {
  // Jika MQTT belum terhubung, hubungkan kembali
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Control LED built-in berkedip
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);
  }

  // WiFi connection check
  if (WiFi.status() != WL_CONNECTED) {
      wifiManager.autoConnect("ESP8266AP", "12345678");
  }
}

// Fungsi untuk menangani pesan dari broker MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Pesan diterima dari topik: ");
  Serial.println(topic);

  // Konversi payload ke string
  char message[length + 1];
  strncpy(message, (char*)payload, length);
  message[length] = '\0';
  Serial.print("Isi pesan: ");
  Serial.println(message);

  // Kontrol Lampu 1
  if (strcmp(topic, "light1/status") == 0) {
    if (strcmp(message, "ON") == 0) {
      digitalWrite(LED1_PIN, HIGH);  // Nyalakan Lampu 1
      Serial.println("Lampu 1 dinyalakan");
    } else if (strcmp(message, "OFF") == 0) {
      digitalWrite(LED1_PIN, LOW);   // Matikan Lampu 1
      Serial.println("Lampu 1 dimatikan");
    }
  }

  // Kontrol Lampu 2
  else if (strcmp(topic, "light2/status") == 0) {
    if (strcmp(message, "ON") == 0) {
      digitalWrite(LED2_PIN, HIGH);  // Nyalakan Lampu 2
      Serial.println("Lampu 2 dinyalakan");
    } else if (strcmp(message, "OFF") == 0) {
      digitalWrite(LED2_PIN, LOW);   // Matikan Lampu 2
      Serial.println("Lampu 2 dimatikan");
    }
  }

  // Kontrol Lampu 3
  else if (strcmp(topic, "light3/status") == 0) {
    if (strcmp(message, "ON") == 0) {
      digitalWrite(LED3_PIN, HIGH);  // Nyalakan Lampu 3
      Serial.println("Lampu 3 dinyalakan");
    } else if (strcmp(message, "OFF") == 0) {
      digitalWrite(LED3_PIN, LOW);   // Matikan Lampu 3
      Serial.println("Lampu 3 dimatikan");
    }
  }
}

// Fungsi untuk menghubungkan ke WiFi
void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nTerhubung ke WiFi");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());
}

// Fungsi untuk menghubungkan kembali ke broker MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Terhubung ke broker MQTT");

      // Subscribe ke topik masing-masing lampu
      client.subscribe("light1/status");
      client.subscribe("light2/status");
      client.subscribe("light3/status");
      
      Serial.println("Subscribed ke topik: light1/status, light2/status, light3/status");
    } else {
      Serial.print("Gagal, mencoba lagi dalam 5 detik... ");
      delay(5000);
    }
  }
}
