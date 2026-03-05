#include "audio_ring_buffer.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/*
--------------------------------------------------
Internal buffer variables
--------------------------------------------------
*/

static uint8_t *buffer = NULL;
static size_t buffer_size = 0;

static size_t write_index = 0;
static size_t read_index = 0;

static SemaphoreHandle_t mutex;

/*
--------------------------------------------------
Initialize ring buffer
--------------------------------------------------
*/

void audio_buffer_init(size_t size)
{
    buffer = (uint8_t *)malloc(size);

    buffer_size = size;

    write_index = 0;
    read_index = 0;

    mutex = xSemaphoreCreateMutex();
}

/*
--------------------------------------------------
Write data into ring buffer

Returns number of bytes written
--------------------------------------------------
*/

size_t audio_buffer_write(uint8_t *data, size_t length)
{
    if (!buffer)
        return 0;

    xSemaphoreTake(mutex, portMAX_DELAY);

    size_t written = 0;

    for (size_t i = 0; i < length; i++)
    {
        size_t next = (write_index + 1) % buffer_size;

        /* Buffer full */
        if (next == read_index)
            break;

        buffer[write_index] = data[i];
        write_index = next;

        written++;
    }

    xSemaphoreGive(mutex);

    return written;
}

/*
--------------------------------------------------
Read data from ring buffer
--------------------------------------------------
*/

size_t audio_buffer_read(uint8_t *data, size_t length)
{
    if (!buffer)
        return 0;

    xSemaphoreTake(mutex, portMAX_DELAY);

    size_t read = 0;

    while (read_index != write_index && read < length)
    {
        data[read] = buffer[read_index];

        read_index = (read_index + 1) % buffer_size;

        read++;
    }

    xSemaphoreGive(mutex);

    return read;
}

/*
--------------------------------------------------
Check how many bytes available
--------------------------------------------------
*/

size_t audio_buffer_available()
{
    if (write_index >= read_index)
        return write_index - read_index;

    return buffer_size - read_index + write_index;
}

/*
--------------------------------------------------
Reset buffer
--------------------------------------------------
*/

void audio_buffer_reset()
{
    xSemaphoreTake(mutex, portMAX_DELAY);

    write_index = 0;
    read_index = 0;

    xSemaphoreGive(mutex);
}