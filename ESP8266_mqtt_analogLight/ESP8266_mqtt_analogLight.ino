#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define LED_PIN D6

// Ganti dengan SSID dan Password WiFi Anda
const char* ssid = "TP-Link_Lantai1";
const char* password = "Nola2024";

// Ganti dengan alamat broker MQTT Anda
const char* mqtt_server = "34.68.99.84";
const int mqtt_port = 8883; // Port standar untuk MQTT

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // Menghubungkan ke WiFi
  setupWiFi();

  // Mengatur MQTT Client
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); // Mengatur callback untuk menangani pesan MQTT
}

unsigned long previousMillis = 0;  // Store the last time the LED blinked
const long interval = 100;         // Blinking interval (100 ms)

bool ledState = LOW;               // LED state

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // Control LED blinking based on interval
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // Update previous time
    ledState = !ledState;           // Toggle LED state
    digitalWrite(LED_BUILTIN, ledState);
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

  // Periksa topik dan isi pesan
  if (strcmp(topic, "light/status") == 0) {
    if (strcmp(message, "ON") == 0) {
      digitalWrite(LED_PIN, HIGH); // Nyalakan lampu
      Serial.println("Lampu dinyalakan melalui MQTT");
    } else if (strcmp(message, "OFF") == 0) {
      digitalWrite(LED_PIN, LOW); // Matikan lampu
      Serial.println("Lampu dimatikan melalui MQTT");
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

  Serial.println("Terhubung ke WiFi");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());
}

// Fungsi untuk menghubungkan kembali ke broker MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Terhubung ke broker MQTT");

      // Subscribe ke topik "light/status"
      client.subscribe("light/status");
      Serial.println("Subscribed ke topik: light/status");
    } else {
      Serial.print("Gagal, mencoba lagi dalam 5 detik... ");
      delay(5000);
    }
  }
}
