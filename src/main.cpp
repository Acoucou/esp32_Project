#include "esp_camera.h"
#include <time.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "esp_http_client.h"
#include <ArduinoJson.h>
#include <lvgl.h>
#include <BleKeyboard.h>
#include "config.h"
#include "arduinoFFT.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

unsigned char button_state;
unsigned char light_fun = 0;
extern unsigned char page_idx;

extern PubSubClient mqttClient;

extern lv_obj_t *chart;
extern lv_chart_series_t *fft_ser;

bool camera_flag = false, gui_flag = true, fft_flag = false;

void GUI_Task();
void UART_Task();
void Camera_Task();
void Light_Task();
void Mqtt_Task();
void Fft_Task();

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

#define CHANNEL A10
const uint16_t samples = 64;          // This value MUST ALWAYS be a power of 2
const double samplingFrequency = 100; // Hz, must be less than 10000 due to ADC
unsigned int sampling_period_us;
unsigned long microseconds;
double vReal[samples];
double vImag[samples];
uint16_t toData[samples];

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable   detector
  all_init();                                // 初始化

  lv_app_load_start_scr(); // 开始动画
  // lv_app_load_fft_scr();
  // lv_app_load_home_scr();
  // lv_app_load_light_scr();
  // lv_app_load_voice_scr();
}

void GUI_Task()
{
  lv_timer_handler(); /* let the GUI do its work */
}

void Camera_Task()
{
  camera_show();
  if (button_state == 3)
  {
    camera_flag = false;
    button_state = 0;
  }
  if (button_state == 4)
  {
    // Take picture
    take_send_photo();
    button_state = 0;
  }
}

void Mqtt_Task()
{
  if (mqttClient.connected())
  {
    // 如果开发板成功连接服务器
    // 保持心跳
    mqttClient.loop();
  }
  // else
  // {                      // 如果开发板未能成功连接服务器
  //   connectMQTTServer(); // 则尝试连接服务器
  // }
}

void Fft_Task()
{
  int i = 0;
  microseconds = micros();
  for (i = 0; i < samples; i++)
  {
    vReal[i] = analogRead(CHANNEL);
    vImag[i] = 0;
    while (micros() - microseconds < sampling_period_us)
    {
      // empty loop
    }
    microseconds += sampling_period_us;
  }

  /* Print the results of the sampling according to time */
  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD); /* Weigh data */
  FFT.Compute(vReal, vImag, samples, FFT_FORWARD);                 /* Compute FFT */
  FFT.ComplexToMagnitude(vReal, vImag, samples);                   /* Compute magnitudes */

  for (i = 0; i < 50; i++)
  {
    // Serial.println(vReal[i+5]);
    toData[i] = vReal[i + 6];
    toData[i] = constrain(toData[i], 300, 1600);
    toData[i] = map(toData[i], 300, 1600, 0, 100);
  }

  for (i = 0; i < 50; i++)
  {
    lv_chart_set_next_value(chart, fft_ser, toData[i]);
  }
}

void Light_Task()
{
  if (light_fun == 1)
  {
    rainbow(10);
  }
  else if (light_fun == 2)
  {
    theaterChaseRainbow(10);
  }
  else if (light_fun == 3)
  {
    sound_light();
  }
}

void Voice_Task()
{
  if (button_state == 5)
  {
    button_state = 0;
    lv_app_load_voice_scr();
  }
}

void UART_Task()
{
  String uart_rev; // 接收字符串命令

  if (Serial.available())
  {
    uart_rev = Serial.readString();
    // Serial.print("rev=");
    // Serial.println(uart_rev);

    if (uart_rev.compareTo("RIGHT") == 0)
    {
      button_state = 1;
    }
    else if (uart_rev.compareTo("LEFT") == 0)
    {
      button_state = 2;
    }
    else if (uart_rev.compareTo("ENTER") == 0) // 确认按钮
    {
      button_state = 3;
      fft_flag = false;
    }
    else if (uart_rev.compareTo("EXIT") == 0)
    {
      button_state = 4;
    }
    else if (uart_rev.compareTo("TAKEPIC") == 0) // 拍照
    {
      button_state = 4;
    }
    else if (uart_rev.compareTo("VOICE1") == 0)
    {
      button_state = 5; // 语音命令
    }
    else if (uart_rev.compareTo("OPENLED") == 0)
    {
      light_fun = 3;    // LED语音命令
      button_state = 6; // 语音命令
    }
    else if (uart_rev.compareTo("CLOSELED") == 0)
    {
      light_fun = 0;    // LED语音命令
      button_state = 6; // 语音命令
    }
    else if (uart_rev.compareTo("CLASS") == 0)
    {
      page_idx = 1;
      button_state = 6; // 语音命令
      // lv_app_load_class_scr();
    }
    else if (uart_rev.compareTo("HOME") == 0)
    {
      page_idx = 5;
      button_state = 6; // 语音命令
      // lv_app_load_home_scr();
    }
    else if (uart_rev.compareTo("WIFI") == 0)
    {
      // wifi_init();
      // /*
      // 初始化时间
      // */
      // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

      // // 更新天气
      // lv_app_weather_update();

      button_state = 6; // 语音命令
    }
    else
    {
      button_state = 0;
    }

    // Serial.println(button_state);
  }
}

void loop()
{
  UART_Task(); // 交互接口

  // Mqtt_Task();

  if (camera_flag)
  {
    gui_flag = false;
    Camera_Task(); // 摄像头
  }
  else
  {
    if (!gui_flag)
    {
      WiFi.disconnect(true, true);
      lv_app_scroll(); // 仅执行一次
      gui_flag = true;
    }
    GUI_Task(); // lvgl心跳
  }

  Light_Task(); // 灯光任务

  if (fft_flag)
  {
    Fft_Task(); // FFT任务
  }

  Voice_Task(); // 语音交互任务
}
