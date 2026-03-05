很好。现在进入 语音链路中最关键的一部分：HTTP 音频上传协议 + ESP32 实现 + 可测试方案。
这一部分要保证 稳定、可调试、可扩展。

一、系统通信协议设计

ESP32 需要发送 原始 PCM 音频流 到 AI Core。

标准语音参数：

Sample Rate : 16000 Hz
Bit Depth   : 16 bit
Channels    : Mono
Format      : PCM

上传协议：

POST /stt
Content-Type: application/octet-stream

请求体：

RAW PCM AUDIO STREAM

返回 JSON：

{
  "text": "turn on the kitchen light",
  "audio_url": "http://192.168.50.205:8000/audio/abc123.mp3"
}
二、ESP32 端流程

完整流程：

Button Press
      ↓
Start Recording
      ↓
Mic Task → RingBuffer
      ↓
Network Task
      ↓
HTTP Upload
      ↓
AI Core
      ↓
Return audio_url
      ↓
Speaker Play
三、HTTP 上传模块

新增模块：

lib/audio_uploader/
    audio_uploader.h
    audio_uploader.cpp
audio_uploader.h
#pragma once

/*
--------------------------------------------------
Audio Uploader Module

Purpose:
Upload recorded PCM audio to AI Core server.

Input:
Audio data from ring buffer

Output:
AI server JSON response
--------------------------------------------------
*/

void audio_upload_start();
audio_uploader.cpp
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

#include <audio_ring_buffer.h>
#include <speaker.h>
#include <config.h>

#include "audio_uploader.h"

/*
--------------------------------------------------
Upload audio from ring buffer to AI Core
--------------------------------------------------
*/

void audio_upload_start()
{
    HTTPClient http;
    WiFiClient client;

    http.begin(client, "http://192.168.50.205:8000/stt");

    http.addHeader("Content-Type", "application/octet-stream");

    const size_t chunk_size = 1024;

    uint8_t chunk[chunk_size];

    int total = 0;

    http.collectHeaders(NULL, 0);

    int httpCode = http.POST((uint8_t *)"", 0);

    if (httpCode <= 0)
    {
        Serial.println("HTTP connection failed");
        return;
    }

    while (audio_buffer_available() > 0)
    {
        size_t read = audio_buffer_read(chunk, chunk_size);

        client.write(chunk, read);

        total += read;
    }

    Serial.print("Uploaded bytes: ");
    Serial.println(total);

    String response = http.getString();

    Serial.println("AI Response:");
    Serial.println(response);

    JsonDocument doc;

    deserializeJson(doc, response);

    const char *audio_url = doc["audio_url"];

    if (audio_url)
    {
        speaker_play(String(audio_url));
    }

    http.end();
}
四、在 Network Task 中触发上传

修改：

lib/network_audio/network_audio.cpp

录音完成后：

if(recording_finished)
{
    audio_upload_start();
}
五、AI Core FastAPI 实现

在 Raspberry Pi：

from fastapi import FastAPI, Request
import uuid
import wave

app = FastAPI()

@app.post("/stt")
async def speech_to_text(request: Request):

    audio_bytes = await request.body()

    filename = f"/tmp/{uuid.uuid4()}.wav"

    with wave.open(filename, "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(16000)
        wf.writeframes(audio_bytes)

    text = "hello world"

    return {
        "text": text,
        "audio_url": "http://192.168.50.205:8000/audio/test.mp3"
    }
六、测试方案（必须先验证）

测试步骤：

Step 1

启动 AI Core

uvicorn main:app --host 0.0.0.0 --port 8000
Step 2

确认 ESP32 能访问：

浏览器打开：

http://192.168.50.205:8000
Step 3

ESP32 串口输出：

Uploaded bytes: 32000
AI Response:
{
 "text":"hello world",
 "audio_url":"http://192.168.50.205/audio/test.mp3"
}
Step 4

Speaker 自动播放返回音频。

七、第一阶段测试目标

现在先验证：

1. Mic → RingBuffer
2. RingBuffer → HTTP upload
3. AI Core 接收音频
4. 返回 JSON
5. Speaker 播放

如果全部成功，你的系统就完成了 完整语音闭环。

下一阶段（系统升级）

接下来会实现：

Push-to-Talk 状态机
录音开始 / 结束控制

然后升级：

Wake Word

并加入：

OLED 表情显示
Web 控制面板
MQTT 家庭设备控制

这样你的设备就成为一个 完整家庭 AI 语音终端