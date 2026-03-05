#include "led.h"
#include <pins.h>
#include <Arduino.h>



void led_init()
{
    pinMode(LED_PIN, OUTPUT);
}

void led_on()
{
    digitalWrite(LED_PIN, HIGH);
}

void led_off()
{
    digitalWrite(LED_PIN, LOW);
}