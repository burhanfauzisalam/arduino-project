#include "Wire.h"
#include "PN532_I2C.h"
#include "PN532.h"
#include <WiFiManager.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

//#define LED_BUILTIN 2

// Initialize PN532 NFC Reader
PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

// LED pin definitions
const int led1 = D1;  // Red
const int led2 = D8; // Green
const int led3 = D2;  // Blue
const int led4 = D9;  // Yellow

// RFID card IDs
const String redCard = "BECE816E";
const String greenCard = "6EF18B6E";
const String yellowCard = "BEE0806E";
const String blueCard = "4E637F6E";

// RFID card IDs
//const String redCard = "6E9E8C6E";
//const String greenCard = "111DF01C";
//const String yellowCard = "D355C095";
//const String blueCard = "BA4E2F59";

String userRFID = "";

// Button pin definition
const int buttonPin = D3;  // Pin tombol

bool buttonState = LOW;       // Status tombol saat ini
bool lastButtonState = LOW;   // Status tombol sebelumnya
unsigned long pressTime = 0;  // Waktu saat tombol ditekan
unsigned long releaseTime = 0; // Waktu saat tombol dilepas
bool isLongPress = false;     // Indikator apakah long press

// Durasi untuk mendeteksi long press (1 detik)
const unsigned long longPressDuration = 1000;

const char* apiURL = "http://server.habito.id/light-status?id=habito_001";
const char* apiReset = "http://server.habito.id/reset-data?id=habito_001";

// MQTT broker details
const char* mqttServer = "090cd3b5566041ae929bfd1b7420f5b1.s1.eu.hivemq.cloud"; // Corrected server address
const int mqttPort = 8883;                     // Non-secure port

const char* topicStatus= "habito/status/habito_001";
const char* topicLight= "habito/light/#";

// Timing variables
unsigned long previousMillis1 = 0;
const long interval1 = 5000; // Interval for "habito/status" topic (in ms)

// Blinking variables
unsigned long blinkPreviousMillis = 0;
const long blinkInterval = 500; // Blinking interval (500ms)
bool ledStatus = false;

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


// Initialize WiFi and MQTT clients
WiFiClientSecure espClient;       // Use WiFiClient for non-secure connections
PubSubClient client(espClient);

// WiFiManager instance
WiFiManager wm;

// MQTT callback function
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();


}

// Reconnect to MQTT broker
void reconnect() {
  unsigned long startAttemptTime = millis();
  while (!client.connected() && millis() - startAttemptTime < 15000) { // Max retry 15 seconds
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), "burhan","CahayaUtara2020")) {
      Serial.println("connected");
      client.subscribe(topicStatus);
      client.subscribe(topicLight);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 3 seconds");
      delay(3000);
    }
  }
}



void setup() {
  Serial.begin(115200);
  
  // Initialize LEDs
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
//
//  digitalWrite(led1, HIGH);
//  digitalWrite(led2, HIGH);
//  digitalWrite(led3, HIGH);
//  digitalWrite(led4, HIGH);

  // Initialize NFC reader
  Serial.println("NFC/RFID Reader");
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("Didn't find PN53x board");
    while (1); // Halt
  }

  // Print NFC firmware version
  Serial.print("Found chip PN5"); 
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); 
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); 
  Serial.println((versiondata >> 8) & 0xFF, DEC);

  // Configure NFC reader
  nfc.setPassiveActivationRetries(0x01);
  nfc.SAMConfig();

  Serial.println("Waiting for an ISO14443A Card ...");

  // Connect to WiFi using WiFiManager
  WiFi.mode(WIFI_STA);
  bool res = wm.autoConnect("habito_002"); // Password-protected AP

  if (!res) {
    Serial.println("Failed to connect");
    wm.resetSettings();
    // Optionally, you can restart the device or enter a safe state
  } else {
    Serial.println("Connected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
//    digitalWrite(led1, LOW);
//    digitalWrite(led2, LOW);
//    digitalWrite(led3, LOW);
//    digitalWrite(led4, LOW);
  }

  // Setup MQTT client
    espClient.setCACert(root_ca);  // Gunakan sertifikat
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {

  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
//    digitalWrite(led1, HIGH);
//    digitalWrite(led2, HIGH);
//    digitalWrite(led3, HIGH);
//    digitalWrite(led4, HIGH);
    Serial.println("WiFi disconnected! Resetting settings...");
    wm.resetSettings();
    delay(1000); // Give some time before attempting to reconnect
    // Optionally, you can trigger a WiFiManager reconnect or restart
  }

  // Reconnect to MQTT if disconnected
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // MQTT connection status LED blinking
//  if (client.connected()) {
//    unsigned long currentMillis = millis();
//    if (currentMillis - blinkPreviousMillis >= blinkInterval) {
//      blinkPreviousMillis = currentMillis;
//      ledStatus = !ledStatus;
//      digitalWrite(LED_BUILTIN, ledStatus); // Toggle built-in LED
//    }
//  } else {
//    digitalWrite(LED_BUILTIN, LOW); // Turn off LED if not connected
//  }

  // Publish "ONLINE" status every second
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis1 >= interval1) {
    previousMillis1 = currentMillis;
    client.publish(topicStatus, "ONLINE");
  }

  // Read RFID cards
  readRFID();

  // Membaca status tombol
  buttonState = digitalRead(buttonPin);

  // Tombol ditekan (LOW karena pull-up digunakan)
  if (buttonState == LOW && lastButtonState == HIGH) {
    pressTime = millis(); // Catat waktu tekan
    isLongPress = false;
  }

  // Tombol dilepas
  if (buttonState == HIGH && lastButtonState == LOW) {
    releaseTime = millis(); // Catat waktu lepas

    unsigned long pressDuration = releaseTime - pressTime;

    if (pressDuration < longPressDuration) {
      // Short press
      Serial.println("Short Press Detected");
      fetchAPIData();
    } else {
      // Long press
      Serial.println("Long Press Detected");
      resetData();
    }
  }
  

  // Simpan status tombol sebelumnya
  lastButtonState = buttonState;
}

void readRFID() {
  boolean success;
  uint8_t uid[7] = {0};
  uint8_t uidLength = 0;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    userRFID = "";
    for (uint8_t i = 0; i < uidLength; i++) {
      if (uid[i] <= 0xF) {
        userRFID += "0";
      }
      userRFID += String(uid[i], HEX);
    }
    userRFID.toUpperCase();
    Serial.println(userRFID);

    if (userRFID == redCard) {
      client.publish("habito/light/red/habito_001", "ON");
      digitalWrite(led1, HIGH);
      delay(1000);
      digitalWrite(led1, LOW);
    } 
    else if (userRFID == greenCard) {
      client.publish("habito/light/green/habito_001", "ON");
      digitalWrite(led2, HIGH);
      delay(1000);
      digitalWrite(led2, LOW);
    } 
    else if (userRFID == blueCard) {
      client.publish("habito/light/blue/habito_001", "ON");
      digitalWrite(led3, HIGH);
      delay(1000);
      digitalWrite(led3, LOW);
    } 
    else if (userRFID == yellowCard) {
      client.publish("habito/light/yellow/habito_001", "ON");
      digitalWrite(led4, HIGH);
      delay(1000);
      digitalWrite(led4, LOW);
    }
    else {
      digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
      digitalWrite(led3, HIGH);
      digitalWrite(led4, HIGH);
      delay(1000);
      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
      digitalWrite(led3, LOW);
      digitalWrite(led4, LOW);
    }

    delay(400); // Debounce delay
    userRFID = "";
  }
}

void fetchAPIData() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        // Kirim request ke API
        http.begin(apiURL);  // Ganti dengan URL API Anda
        int httpCode = http.GET(); // Kirim HTTP GET

        if (httpCode > 0) {
            String payload = http.getString();
            Serial.println("API Response:");
            Serial.println(payload);

            // Parsing JSON
            DynamicJsonDocument doc(1024); // Sesuaikan ukuran buffer jika diperlukan
            DeserializationError error = deserializeJson(doc, payload);

            if (!error) {
                JsonArray dataArray = doc["data"].as<JsonArray>();

                // Reset LED (matiin semua LED)
//                digitalWrite(led1, LOW);
//                digitalWrite(led2, LOW);
//                digitalWrite(led3, LOW);
//                digitalWrite(led4, LOW);

                // Loop untuk memeriksa data warna
                for (JsonObject item : dataArray) {
                    const char* color = item["color"];
                    const char* status = item["status"];

                    if (strcmp(status, "ON") == 0) {
                        // Menyalakan LED berdasarkan warna
                        if (strcmp(color, "red") == 0) {
                            digitalWrite(led1, HIGH);
                        } else if (strcmp(color, "green") == 0) {
                            digitalWrite(led2, HIGH);
                        } else if (strcmp(color, "blue") == 0) {
                            digitalWrite(led3, HIGH);
                        } else if (strcmp(color, "yellow") == 0) {
                            digitalWrite(led4, HIGH);
                        }
                        
                    }
                }
                delay(1000);
                digitalWrite(led1, LOW);
                digitalWrite(led2, LOW);
                digitalWrite(led3, LOW);
                digitalWrite(led4, LOW);
            } else {
                Serial.println("Failed to parse JSON");
            }
        } else {
            Serial.print("HTTP GET failed, error: ");
            Serial.println(http.errorToString(httpCode).c_str());
        }

        http.end(); // Tutup koneksi HTTP
    } else {
        Serial.println("WiFi not connected!");
    }
}

void resetData() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(apiReset);
        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK) {
            Serial.println("Data reset successfully");
        } else {
            Serial.println("Error resetting data");
        }
        http.end();
    } else {
        Serial.println("WiFi not connected");
    }
}
