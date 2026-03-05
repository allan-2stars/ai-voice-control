#include <Arduino.h>
#include <HTTPClient.h>
#include <driver/i2s.h>

#include <pins.h>

#include "speaker.h"

/*
--------------------------------------------------
Speaker playback task
--------------------------------------------------
*/

static TaskHandle_t speakerTaskHandle = NULL;

static void speaker_task(void *param)
{
    String url = *(String *)param;

    delete (String *)param;

    HTTPClient http;

    http.begin(url);

    int code = http.GET();

    if (code != 200)
    {
        Serial.println("Speaker HTTP error");
        vTaskDelete(NULL);
        return;
    }

    WiFiClient *stream = http.getStreamPtr();

    uint8_t buffer[1024];

    while (stream->connected())
    {
        int available = stream->available();

        if (available > 0)
        {
            int read = stream->readBytes(buffer, min(available, 1024));

            size_t written;

            i2s_write(
                I2S_NUM_0,
                buffer,
                read,
                &written,
                portMAX_DELAY
            );
        }

        else
        {
            delay(5);
        }
    }

    http.end();

    vTaskDelete(NULL);
}

/*
--------------------------------------------------
Start speaker playback
--------------------------------------------------
*/

void speaker_play(String url)
{
    String *copy = new String(url);

    xTaskCreatePinnedToCore(
        speaker_task,
        "speaker_task",
        8192,
        copy,
        1,
        &speakerTaskHandle,
        0
    );
}

void speaker_stop()
{
    if (speakerTaskHandle)
    {
        vTaskDelete(speakerTaskHandle);
        speakerTaskHandle = NULL;
    }
}