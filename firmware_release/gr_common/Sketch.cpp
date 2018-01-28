/* GR-CITRUS Sketch Template V2.20 */
#include <Arduino.h>

void setup(){
    Serial.begin(9600);
    pinMode(PIN_LED0, OUTPUT);
}

void loop(){
    Serial.println("hello");
    digitalWrite(PIN_LED0, HIGH);
    delay(200);
    digitalWrite(PIN_LED0, LOW);
    delay(200);
}
