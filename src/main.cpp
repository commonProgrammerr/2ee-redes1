#include "main.h"

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, pwd);

  // inicia radio wifi e tenta estabelecer conexão ao roteador
  setup_wifi();

  // inicia e adiciona as tasks
  timeScheduler.init();
  timeScheduler.addTask(plusTZ);
  timeScheduler.addTask(minusTZ);
  timeScheduler.addTask(udpTSK);

  // input watch
  InputHandle.addInput(up_button);
  InputHandle.addInput(down_button);
  InputHandle.addInput(update_button);
  InputHandle.init();

  udp.begin(udpPort);
}

void log_time(Time &tm)
{
  Serial.printf(
      "%02u/%02u/%u %02u:%02u:%02u.%04u %+02d\n",
      now.day, now.mon, now.year,
      now.hour, now.min, now.sec,
      millis() % 1000, now.gmt);
}

void loop()
{
  timeScheduler.execute();
}

void updateTime()
{
  udp.beginPacket(udpAddress, 3030);
  udp.write((byte *)&time_zone, sizeof(int8_t));
  udp.endPacket();

  for (byte i = 0; i < 5; i++)
    if (udp.parsePacket())
      break;

  for (byte i = 0; i < 5; i++)
    if (udp.read((byte *)&now, sizeof(Time)) > 0)
    {
      Serial.print("Server to client: ");
      log_time(now);
      break;
    }
}

// pass to next time zone
void nextTZ()
{
  time_zone = time_zone >= 12 ? -11 : time_zone + 1;
  Serial.printf("Current time zone GMT: %d\n", time_zone);
};

// pass to previus time zone
void previusTZ()
{
  time_zone = time_zone <= -11 ? 12 : time_zone - 1;
  Serial.printf("Current time zone GMT: %d\n", time_zone);
}
