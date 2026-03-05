#pragma once

enum AppState
{
    STATE_IDLE,
    STATE_RECORDING,
    STATE_PROCESSING,
    STATE_PLAYING
};


extern bool recording;
extern bool recording_finished;