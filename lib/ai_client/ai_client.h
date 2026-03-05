#pragma once

#include <Arduino.h>

void ai_send_text(String message);
void ai_send_audio(uint8_t* data, size_t length);