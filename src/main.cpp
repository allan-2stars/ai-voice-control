#include <Arduino.h>

#include <wifi_manager.h>
#include "app_controller.h"

void setup()
{
    Serial.begin(115200);

    wifi_connect();

    app_setup();
}

void loop()
{
    app_loop();
}