#include <Arduino.h>

// La LED est sur le GPIO 22 sur la Lolin32 Lite
#define LED_PIN 33

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    Serial.println("Lolin32 Lite prête !");
}

void loop() {
    digitalWrite(LED_PIN, LOW); // Allume la LED
    Serial.println("LED ON");
    delay(1000);
    digitalWrite(LED_PIN, HIGH); // Éteint la LED
    Serial.println("LED OFF");
    delay(1000);
}