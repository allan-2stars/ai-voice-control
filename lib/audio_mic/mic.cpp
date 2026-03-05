#include "mic.h"
#include "audio_ring_buffer.h"
#include <pins.h>
#include <Arduino.h>
#include <driver/i2s.h>


#define MIC_PORT I2S_NUM_1

void mic_init()
{
    i2s_config_t config = {};

    config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
    config.sample_rate = 16000;
    config.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
    config.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
    config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
    config.dma_buf_count = 8;
    config.dma_buf_len = 512;

    i2s_pin_config_t pins = {};

    pins.bck_io_num = I2S_MIC_BCLK;
    pins.ws_io_num = I2S_MIC_WS;
    pins.data_in_num = I2S_MIC_DATA;
    pins.data_out_num = I2S_PIN_NO_CHANGE;

    i2s_driver_install(MIC_PORT, &config, 0, NULL);
    i2s_set_pin(MIC_PORT, &pins);
}

void mic_read()
{
    int32_t buffer[256];
    size_t bytesRead;

    i2s_read(MIC_PORT, buffer, sizeof(buffer), &bytesRead, 0);

    if (bytesRead > 0)
    {
        Serial.println("Audio data received");
    }
}

/*
--------------------------------------------------
Microphone FreeRTOS Task

Purpose:
Continuously read audio samples from I2S
and push them into the audio ring buffer.
--------------------------------------------------
*/

static void mic_task(void *param)
{
    int32_t i2sBuffer[256];
    size_t bytesRead;

    while (true)
    {
        /* Read audio samples from microphone */
        i2s_read(I2S_NUM_1, i2sBuffer, sizeof(i2sBuffer), &bytesRead, portMAX_DELAY);

        /* Push audio samples into ring buffer */
        audio_buffer_write((uint8_t *)i2sBuffer, bytesRead);
    }
}

/*
--------------------------------------------------
Start microphone task
--------------------------------------------------
*/

void mic_start_task()
{
    xTaskCreatePinnedToCore(
        mic_task,      // Task function
        "mic_task",    // Task name
        4096,          // Stack size
        NULL,          // Task parameter
        1,             // Priority
        NULL,          // Task handle
        1              // Run on Core 1
    );
}