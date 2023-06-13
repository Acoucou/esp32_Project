#include "esp_camera.h"
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <TFT_eSPI.h>      // Hardware-specific library
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
#include <TFT_eFEX.h>
TFT_eFEX fex = TFT_eFEX(&tft);

#include <lvgl.h>

static lv_disp_draw_buf_t draw_buf;    // 定义显示器变量
static lv_color_t buf[TFT_WIDTH * 10]; // 定义刷新缓存

String rev;

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

void camera_init()
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
    config.pixel_format = PIXFORMAT_JPEG;

    config.frame_size = FRAMESIZE_240X240; // 240*240
    config.jpeg_quality = 12;              // 0-63 lower number means higher quality
    config.fb_count = 2;

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
}

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();                                        // 使能写功能
    tft.setAddrWindow(area->x1, area->y1, w, h);             // 设置填充区域
    tft.pushColors((uint16_t *)&color_p->full, w * h, true); // 写入颜色缓存和缓存大小
    tft.endWrite();                                          // 关闭写功能

    lv_disp_flush_ready(disp); // 调用区域填充颜色函数
}

void setup()
{
    Serial.begin(115200);

    camera_init();

    tft.begin();
    tft.fillScreen(TFT_BLACK);
    fex.listSPIFFS();

    tft.setRotation(0);
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, TFT_WIDTH * 10);

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = TFT_WIDTH;
    disp_drv.ver_res = TFT_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello world!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void loop()
{
    camera_fb_t *fb = esp_camera_fb_get();

    tft.setCursor(10, 230, 1); // 设置起始坐标(10, 10)，2 号字体
    tft.println("TFT_Text");   // 显示文字

    fex.drawJpg((const uint8_t *)fb->buf, fb->len, 0, 0, 240, 240, 0, 0);

    esp_camera_fb_return(fb);

    // lv_timer_handler(); /* let the GUI do its work */

    if (Serial.available())
    {
        rev = Serial.readString();
        Serial.print("rev=");
        Serial.println(rev);
    }
}