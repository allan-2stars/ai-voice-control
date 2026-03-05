#include <pins.h>
#include <Arduino.h>
#include "driver/i2s.h"


#define I2S_MIC_PORT I2S_NUM_1

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

    i2s_driver_install(I2S_MIC_PORT, &config, 0, NULL);
    i2s_set_pin(I2S_MIC_PORT, &pins);
}

void mic_read()
{
    int32_t buffer[256];
    size_t bytesRead;

    i2s_read(I2S_MIC_PORT, buffer, sizeof(buffer), &bytesRead, 0);

    if (bytesRead > 0)
    {
        int samples = bytesRead / 4;
        long level = 0;

        for (int i = 0; i < samples; i++)
        {
            int16_t pcm = buffer[i] >> 14;
            level += abs(pcm);
        }

        level /= samples;

        if (level > 2000)
            Serial.println("Voice...");
    }
}