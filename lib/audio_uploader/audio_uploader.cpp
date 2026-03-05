#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

#include <audio_ring_buffer.h>
#include <speaker.h>
#include <config.h>

#include "audio_uploader.h"

/*
--------------------------------------------------
Upload audio from ring buffer to AI Core
--------------------------------------------------
*/

void audio_upload_start()
{
    HTTPClient http;
    WiFiClient client;

    http.begin(client, "http://192.168.50.205:8000/stt");

    http.addHeader("Content-Type", "application/octet-stream");

    const size_t chunk_size = 1024;

    uint8_t chunk[chunk_size];

    int total = 0;

    http.collectHeaders(NULL, 0);

    int httpCode = http.POST((uint8_t *)"", 0);

    if (httpCode <= 0)
    {
        Serial.println("HTTP connection failed");
        return;
    }

    while (audio_buffer_available() > 0)
    {
        size_t read = audio_buffer_read(chunk, chunk_size);

        client.write(chunk, read);

        total += read;
    }

    Serial.print("Uploaded bytes: ");
    Serial.println(total);

    String response = http.getString();

    Serial.println("AI Response:");
    Serial.println(response);

    JsonDocument doc;

    deserializeJson(doc, response);

    const char *audio_url = doc["audio_url"];

    if (audio_url)
    {
        speaker_play(String(audio_url));
    }

    http.end();
}