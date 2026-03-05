#pragma once

#include <Arduino.h>

/*
--------------------------------------------------
Speaker Module

Responsibilities:
1. Initialize I2S speaker
2. Play MP3 audio stream from URL
3. Stop playback if necessary
--------------------------------------------------
*/

void speaker_init();

void speaker_play(String url);

void speaker_stop();