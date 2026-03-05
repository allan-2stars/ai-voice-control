/*
--------------------------------------------------
Network Audio Module

Purpose:
Upload recorded audio from the ring buffer to the AI Core.

Architecture:
Mic Task  → push audio → RingBuffer
Network Task → read audio → HTTP upload
AI Core → JSON response → Speaker playback

Task Core:
Runs on Core0
--------------------------------------------------
*/

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

#include <audio_ring_buffer.h>
#include <speaker.h>
#include <config.h>

#include "network_audio.h"

/*
--------------------------------------------------
Configuration
--------------------------------------------------
*/

#define AUDIO_CHUNK_SIZE 1024
#define AUDIO_UPLOAD_URL "http://192.168.50.205:8000/stt"

/*
--------------------------------------------------
FreeRTOS Network Task
--------------------------------------------------
*/

static void network_audio_task(void *param)
{
    uint8_t chunk[AUDIO_CHUNK_SIZE];

    while (true)
    {
        /*
        Wait until enough audio data is available
        */

        if (audio_buffer_available() < AUDIO_CHUNK_SIZE)
        {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            continue;
        }

        Serial.println("Starting audio upload...");

        HTTPClient http;
        WiFiClient client;

        /*
        Connect to AI Core
        */

        http.begin(client, AUDIO_UPLOAD_URL);
        http.addHeader("Content-Type", "application/octet-stream");

        /*
        Start HTTP POST request
        */

        int httpCode = http.POST((uint8_t *)"", 0);

        if (httpCode <= 0)
        {
            Serial.println("HTTP connection failed");
            http.end();
            continue;
        }

        int total_uploaded = 0;

        /*
        Stream audio data from ring buffer
        */

        while (audio_buffer_available() > 0)
        {
            size_t read = audio_buffer_read(chunk, AUDIO_CHUNK_SIZE);

            if (read > 0)
            {
                client.write(chunk, read);
                total_uploaded += read;
            }

            vTaskDelay(1);
        }

        Serial.print("Audio uploaded bytes: ");
        Serial.println(total_uploaded);

        /*
        Read AI response
        */

        String response = http.getString();

        Serial.println("AI Response:");
        Serial.println(response);

        /*
        Parse JSON response
        */

        JsonDocument doc;

        DeserializationError err = deserializeJson(doc, response);

        if (err)
        {
            Serial.println("JSON parse failed");
            http.end();
            continue;
        }

        const char *audio_url = doc["audio_url"];

        /*
        Play returned TTS audio
        */

        if (audio_url)
        {
            Serial.print("Playing audio from: ");
            Serial.println(audio_url);

            speaker_play(String(audio_url));
        }

        http.end();

        /*
        Small delay before next cycle
        */

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

/*
--------------------------------------------------
Start Network Audio Task
--------------------------------------------------
*/

void network_audio_start_task()
{
    xTaskCreatePinnedToCore(
        network_audio_task,   // Task function
        "network_audio_task", // Task name
        8192,                 // Stack size
        NULL,                 // Parameter
        1,                    // Priority
        NULL,                 // Task handle
        0                     // Run on Core0
    );
}