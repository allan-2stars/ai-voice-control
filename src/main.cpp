#include <Arduino.h>
#include "driver/i2s.h"

void setupMicI2S(){

  i2s_config_t config;
  memset(&config, 0, sizeof(config));

  config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
  config.sample_rate = 16000;
  config.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
  config.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
  config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  config.intr_alloc_flags = 0;
  config.dma_buf_count = 4;
  config.dma_buf_len = 256;
  config.use_apll = false;

  i2s_pin_config_t pins;
  memset(&pins, 0, sizeof(pins));

  pins.bck_io_num = 14;              // SCK
  pins.ws_io_num = 15;               // WS
  pins.data_out_num = I2S_PIN_NO_CHANGE;
  pins.data_in_num = 32;             // SD
  pins.mck_io_num = I2S_PIN_NO_CHANGE;

  i2s_driver_install(I2S_NUM_0, &config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pins);

  Serial.println("Mic I2S Ready");
}

void setup(){
  Serial.begin(115200);
  delay(2000);
  setupMicI2S();
}

void loop(){

  int32_t buffer[256];
  size_t bytesRead;

  i2s_read(I2S_NUM_0, buffer, sizeof(buffer), &bytesRead, portMAX_DELAY);

  long volume = 0;

  for(int i=0;i<256;i++){
    int32_t sample = buffer[i] >> 8;   // INMP441 24bit left aligned
    volume += abs(sample);
  }

  Serial.println(volume);
}