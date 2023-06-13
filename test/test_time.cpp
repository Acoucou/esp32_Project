#include <Arduino.h>
#include <time.h>
#include <WiFi.h>

const char *ssid = "CouCou";
const char *password = "12345678";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 8*3600; 
const int   daylightOffset_sec = 3600;

int potValue = 0;
int avg = 0;

const int freq = 20;
const int ledChannel = 0;
const int resolution = 8;

struct tm timeinfo;

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  // 初始化时间
}

void loop()
{
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
  }

  Serial.println(timeinfo.tm_year + 1900);

  delay(1000);

  delay(1000);
}
