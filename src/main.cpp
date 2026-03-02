#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "MP3DecoderHelix.h"
#include "driver/i2s.h"

using namespace libhelix;

// ---------------- WIFI ----------------

const char* ssid="BlueberryHouse";
const char* password="allansy5-777";

String AI_CHAT="http://192.168.50.205:8000/chat";

const char* mqtt_server="192.168.50.205";

// ------------ RELAY ------------------

#define RELAY_PIN 18

// ------------ WIFI MQTT -------------

WiFiClient espClient;
PubSubClient client(espClient);

// ------------ AUDIO -----------------

TaskHandle_t audioTaskHandle=NULL;

volatile bool stopAudio=false;
volatile bool audioRunning=false;

MP3DecoderHelix decoder(
[](MP3FrameInfo &info,int16_t *pcm,size_t len,void*){

 static int lastRate=0;

 if(info.samprate!=lastRate){

  i2s_set_clk(
   I2S_NUM_0,
   info.samprate,
   I2S_BITS_PER_SAMPLE_16BIT,
   I2S_CHANNEL_MONO);

  lastRate=info.samprate;
 }

 size_t written;

 i2s_write(
  I2S_NUM_0,
  pcm,
  len*sizeof(int16_t),
  &written,
  portMAX_DELAY);
});

// ------------ I2S -------------------

void setupI2S(){

 i2s_config_t config={

 .mode=(i2s_mode_t)(I2S_MODE_MASTER|I2S_MODE_TX),

 .sample_rate=44100,

 .bits_per_sample=I2S_BITS_PER_SAMPLE_16BIT,

 .channel_format=I2S_CHANNEL_FMT_ONLY_LEFT,

 .communication_format=I2S_COMM_FORMAT_STAND_I2S,

 .intr_alloc_flags=0,

 .dma_buf_count=16,

 .dma_buf_len=1024,

 .use_apll=false};

 i2s_pin_config_t pins = {
    .mck_io_num = I2S_PIN_NO_CHANGE,   // ⭐ 不使用 MCLK
    .bck_io_num = 26,
    .ws_io_num = 25,
    .data_out_num = 22,
    .data_in_num = I2S_PIN_NO_CHANGE
};

 i2s_driver_install(I2S_NUM_0,&config,0,NULL);

 i2s_set_pin(I2S_NUM_0,&pins);
}

// ------------ AUDIO TASK ------------

void audioTask(void *parameter){

 String url=*(String*)parameter;

 delete (String*)parameter;

 HTTPClient http;

 http.begin(url);

 int code=http.GET();

 if(code!=200){

  Serial.println("Audio HTTP Fail");

  vTaskDelete(NULL);
  return;
 }

 WiFiClient *stream=http.getStreamPtr();

 decoder.begin();

 i2s_zero_dma_buffer(I2S_NUM_0);

 audioRunning=true;

 uint8_t buffer[1024];

 unsigned long lastData=millis();

 while(true){

  if(stopAudio){

   Serial.println("Audio Interrupted");

   break;
  }

  int available=stream->available();

  if(available>0){

   int read=stream->readBytes(
     buffer,
     min(available,1024));

   decoder.write(buffer,read);

   lastData=millis();
  }
  else{

   if(millis()-lastData>500)
     break;

   delay(5);
  }
 }

 decoder.end();

 http.end();

 i2s_zero_dma_buffer(I2S_NUM_0);

 i2s_stop(I2S_NUM_0);

 delay(10);

 i2s_start(I2S_NUM_0);

 audioRunning=false;

 stopAudio=false;

 Serial.println("Playback Done");

 vTaskDelete(NULL);
}

// ------------ START AUDIO -----------

void startAudio(String url){

 if(audioRunning){

  stopAudio=true;

  delay(80);
 }

 String *copy=new String(url);

 xTaskCreatePinnedToCore(

  audioTask,

  "audio",

  8192,

  copy,

  1,

  &audioTaskHandle,

  0);
}

// ------------ AI CALL ---------------

void callAI(String msg){

 HTTPClient http;

 http.begin(AI_CHAT);

 http.addHeader("Content-Type","application/json");

 String payload="{\"message\":\""+msg+"\"}";

 int code=http.POST(payload);

 if(code==200){

  String res=http.getString();

  Serial.println(res);

  JsonDocument doc;

  deserializeJson(doc,res);

  const char* url=doc["audio_url"];

  if(url){

   startAudio(String(url));
  }
 }

 http.end();
}

// ------------ MQTT ------------------

void mqttCallback(char* topic,byte* payload,unsigned int len){

 String msg;

 for(int i=0;i<len;i++)
  msg+=(char)payload[i];

 Serial.println(msg);

 if(msg=="on"){

  digitalWrite(RELAY_PIN,HIGH);

  callAI("kitchen light on");
 }

 else if(msg=="off"){

  digitalWrite(RELAY_PIN,LOW);

  callAI("kitchen light off");
 }
}

void reconnectMQTT(){

 while(!client.connected()){

  Serial.println("MQTT connecting");

  if(client.connect("esp32node")){

   client.subscribe("home/light/kitchen");

   Serial.println("MQTT OK");
  }
  else{

   delay(2000);
  }
 }
}

// ------------ WIFI ------------------

void connectWiFi(){

 Serial.println("WiFi");

 WiFi.begin(ssid,password);

 while(WiFi.status()!=WL_CONNECTED){

  delay(500);
  Serial.print(".");
 }

 Serial.println("\nWiFi OK");
}

// ------------ SETUP -----------------

void setup(){

 Serial.begin(115200);

 pinMode(RELAY_PIN,OUTPUT);

 digitalWrite(RELAY_PIN,LOW);

 connectWiFi();

 client.setServer(mqtt_server,1883);

 client.setCallback(mqttCallback);

 setupI2S();

 Serial.println("READY");
}

// ------------ LOOP ------------------

void loop(){

 if(!client.connected())
  reconnectMQTT();

 client.loop();
}