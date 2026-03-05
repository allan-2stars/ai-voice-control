#include <Arduino.h>
#include <pins.h>
#include "button.h"

void button_init()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

bool button_pressed()
{
    return digitalRead(BUTTON_PIN) == LOW;
}