#include <config.h>
#include <WiFi.h>


void wifi_connect()
{
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    Serial.print("Connecting WiFi");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi Connected");
    Serial.println(WiFi.localIP());
}

bool wifi_is_connected()
{
    return WiFi.status() == WL_CONNECTED;
}