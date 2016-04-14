/* GR-SAKURA Sketch Template E2.08a */
#include <Arduino.h>

#define INTERVAL 100

void setup()
{
    pinMode(PIN_LED0,OUTPUT);
    pinMode(PIN_LED1,OUTPUT);
    pinMode(PIN_LED2,OUTPUT);
    pinMode(PIN_LED3,OUTPUT);
}

void loop()
{
    digitalWrite(PIN_LED0, 1);
    delay(INTERVAL);
    digitalWrite(PIN_LED1, 1);
    delay(INTERVAL);
    digitalWrite(PIN_LED2, 1);
    delay(INTERVAL);
    digitalWrite(PIN_LED3, 1);
    delay(INTERVAL);
    digitalWrite(PIN_LED0, 0);
    delay(INTERVAL);
    digitalWrite(PIN_LED1, 0);
    delay(INTERVAL);
    digitalWrite(PIN_LED2, 0);
    delay(INTERVAL);
    digitalWrite(PIN_LED3, 0);
    delay(INTERVAL);

}
