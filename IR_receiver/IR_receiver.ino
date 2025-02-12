#include <IRremote.h>

#define IR_RECEIVE_PIN 7  // Sesuaikan dengan pin sensor IR

void setup() {
    Serial.begin(115200);
    Serial.println("IR Receiver Siap...");
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
}

void loop() {
    if (IrReceiver.decode()) {
        Serial.println("Data IR Diterima:");
        
        // Menampilkan panjang bit
        Serial.print("Bit Length: ");
        Serial.println(IrReceiver.decodedIRData.numberOfBits);

        // Menampilkan data RAW
        Serial.print("rawData[");
        Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawlen - 1);
        Serial.print("]: {");
        for (int i = 1; i < IrReceiver.decodedIRData.rawDataPtr->rawlen; i++) {
            Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawbuf[i] * 50);
            if (i < IrReceiver.decodedIRData.rawDataPtr->rawlen - 1) {
                Serial.print(", ");
            }
        }
        Serial.println("}");
        
        Serial.println("-----------------------------------");
        
        IrReceiver.resume(); // Siap menerima sinyal berikutnya
    }
}
