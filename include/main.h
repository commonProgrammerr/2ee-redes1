#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "input_task.h"

/* WiFi network name and password */
const char *ssid = "Renato z5";
const char *pwd = "55515552";
// const char *udpAddress = "192.168.21.117";
// const char *udpAddress = "192.168.1.38";
const char *udpAddress = "192.168.73.117";
const int udpPort = 8080;
int8_t time_zone = -3;

typedef struct
{

  byte day;      // 8 bits
  byte mon;      // 16 bits
  byte hour;     // 24 bits
  byte min;      // 32 bits
  byte sec;      // 40 bits
  byte week_day; // 48 bits
  int8_t gmt;    // 56 bits
  byte _;        // 64 bits
  uint16_t year; // 80 bits
} Time;

Time now;
WiFiUDP udp;

void setup_wifi()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

#define bt_callback(task) [](Input &bt) \
{                                       \
  switch (bt.getStatus())               \
  {                                     \ 
  case Input::KEYDOWN:                  \
    task.enable();                      \
    task.restart();                     \
    break;                              \
                                        \
  case Input::HOLD:                     \
    task.setInterval(600);              \
    task.setIterations(TASK_FOREVER);   \
    task.restart();                     \
    break;                              \
                                        \
  case Input::KEYUP:                    \
    task.setInterval(100);              \
    task.setIterations(1);              \
    break;                              \
                                        \
  default:                              \
    break;                              \
  } }

void log_time(Time &tm);
void updateTime();
void nextTZ();    // pass to next time zone
void previusTZ(); // pass to previus time zone

Scheduler timeScheduler;

Task plusTZ(100, 1, &nextTZ);
Task minusTZ(100, 1, &previusTZ);
Task udpTSK(0, 1, &updateTime);

void update_button_callback(Input &bt)
{
  switch (bt.getStatus())
  {

  case Input::KEYDOWN:
    Serial.println("Tempo atual:");
    log_time(now);
    break;

  case Input::HOLD:
    Serial.println("Requisitando atualização... ");
    udpTSK.enable();
    udpTSK.restart();
    break;

  case Input::KEYUP:
  default:
    udpTSK.setIterations(1);
    udpTSK.setInterval(0);
    break;
  }
}

Input up_button(4, INPUT_PULLUP, bt_callback(plusTZ));
Input down_button(16, INPUT_PULLUP, bt_callback(minusTZ));
Input update_button(9, INPUT_PULLUP, update_button_callback);