
#include "app_controller.h"
#include "button.h"
#include "led.h"
#include "mic.h"
#include "app_state.h"
#include <Arduino.h>

static AppState state = STATE_IDLE;

void app_setup()
{
    button_init();
    led_init();
    mic_init();
}

void app_loop()
{
    bool pressed = button_pressed();

    if (pressed && state == STATE_IDLE)
    {
        state = STATE_RECORDING;
        Serial.println("Recording Start");
        led_on();
    }

    if (!pressed && state == STATE_RECORDING)
    {
        state = STATE_PROCESSING;
        Serial.println("Recording Stop");
        led_off();
    }

    if (state == STATE_RECORDING)
    {
        mic_read();
    }
}