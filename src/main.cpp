#include <Arduino.h>

#include "wifi_manager.h"
#include "button.h"
#include "mic.h"
#include "led.h"

bool recording = false;

void setup()
{
    Serial.begin(115200);

    wifi_connect();

    button_init();
    mic_init();
    led_init();
}

void loop()
{
    bool pressed = button_pressed();

    if (pressed && !recording)
    {
        recording = true;
        Serial.println("Recording Start");
        led_on();
    }

    if (!pressed && recording)
    {
        recording = false;
        Serial.println("Recording Stop");
        led_off();
    }

    if (recording)
    {
        mic_read();
    }
}