#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include "esp_camera.h"

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

void setupCamera()
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_RGB565;

  config.frame_size = FRAMESIZE_HQVGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
  config.jpeg_quality = 12;            // 0-63 lower number means higher quality
  config.fb_count = 2;

  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void camera_show(void)
{
  unsigned long i = 0;
  camera_fb_t *fb = esp_camera_fb_get();
  //单帧图像数据输出
  delay(2000);

  Serial.println("fb->buf, fb->len");
  Serial.write(fb->buf, fb->len);

  delay(2000);

  for (uint8_t y = 0; y < 240; y++)
  {
    for (uint8_t x = 0; x < 176; x++)
    {
      // RGB565格式 240X176
      tft.drawPixel(x, 60 + y, ((fb->buf[2 * i]) << 8) | (fb->buf[2 * i + 1]));
      i++;
    }
  }
  esp_camera_fb_return(fb);
}

void setup()
{
  Serial.begin(115200);
  tft.begin();
  tft.fillScreen(TFT_BLACK);
  delay(500);

  setupCamera();
  Serial.println("fbgsdg-tern");
}

void loop()
{

  tft.setCursor(10, 80, 1); //设置起始坐标(10, 10)，2 号字体
  tft.println("TFT_Text");  //显示文字
  delay(3000);

  camera_show();
}
