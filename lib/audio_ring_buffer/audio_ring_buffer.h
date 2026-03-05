#pragma once

#include <Arduino.h>

/*
--------------------------------------------------
Audio Ring Buffer Module

Purpose:
Provide a circular buffer for continuous audio data.

Architecture:
Producer  : microphone task pushes audio samples
Consumer  : network task reads audio samples

Advantages:
1. Prevent audio loss
2. Non-blocking audio capture
3. Smooth streaming to AI Core
--------------------------------------------------
*/

void audio_buffer_init(size_t size);

/* Push audio samples into buffer */
size_t audio_buffer_write(uint8_t *data, size_t length);

/* Read audio samples from buffer */
size_t audio_buffer_read(uint8_t *data, size_t length);

/* Check how many bytes available */
size_t audio_buffer_available();

/* Clear buffer */
void audio_buffer_reset();