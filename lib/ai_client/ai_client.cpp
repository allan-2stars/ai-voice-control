#include "ai_client.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <config.h>
#include <speaker.h>



void ai_send_text(String message)
{
    HTTPClient http;

    http.begin(AI_CHAT_URL);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"message\":\"" + message + "\"}";

    int code = http.POST(payload);

    if (code == 200)
    {
        String res = http.getString();

        Serial.println("AI Response:");
        Serial.println(res);

        JsonDocument doc;
        deserializeJson(doc, res);

        const char* url = doc["audio_url"];

        if (url)
        {
            speaker_play(String(url));
        }
    }

    http.end();
}