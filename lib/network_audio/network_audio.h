#pragma once

/*
--------------------------------------------------
Network Audio Task

Purpose:
Read audio data from ring buffer
and upload it to AI Core.

Runs as a FreeRTOS task.
--------------------------------------------------
*/

void network_audio_start_task();