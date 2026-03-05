#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

#include <audio_ring_buffer.h>
#include <speaker.h>
#include <config.h>
#include <app_state.h>

#include "network_audio.h"

#define AUDIO_UPLOAD_URL "http://192.168.50.205:8000/stt"
#define CHUNK_SIZE 1024

/*
--------------------------------------------------
Upload audio to AI Core
--------------------------------------------------
*/

static void audio_upload_start()
{
    Serial.println("Uploading audio...");

    HTTPClient http;
    WiFiClient client;

    http.begin(client, AUDIO_UPLOAD_URL);
    http.addHeader("Content-Type", "application/octet-stream");

    uint8_t chunk[CHUNK_SIZE];

    int total = 0;

    int httpCode = http.POST((uint8_t *)"", 0);

    if (httpCode <= 0)
    {
        Serial.println("HTTP error");
        return;
    }

    while (audio_buffer_available() > 0)
    {
        size_t read = audio_buffer_read(chunk, CHUNK_SIZE);

        if (read > 0)
        {
            client.write(chunk, read);
            total += read;
        }

        delay(1);
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

/*
--------------------------------------------------
Network FreeRTOS Task
--------------------------------------------------
*/

static void network_audio_task(void *param)
{
    while (true)
    {
        if (recording_finished)
        {
            recording_finished = false;

            audio_upload_start();

            audio_buffer_reset();
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

/*
--------------------------------------------------
Start task
--------------------------------------------------
*/

void network_audio_start_task()
{
    xTaskCreatePinnedToCore(
        network_audio_task,
        "network_audio_task",
        8192,
        NULL,
        1,
        NULL,
        0
    );
}