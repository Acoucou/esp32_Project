#include "esp_camera.h"
#include "RTOS.h"

#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <TFT_eSPI.h>      // Hardware-specific library
#include <TFT_eFEX.h>

#include <lvgl.h>
#include "lv_port_indev.h"
#include "lv_home_page.h"
#include "lv_page_common.h"
#include "lv_scroll_page.h"
#include "lv_weather_page.h"

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
TFT_eFEX fex = TFT_eFEX(&tft);

static lv_disp_draw_buf_t draw_buf;    // 定义显示器变量
static lv_color_t buf[TFT_WIDTH * 35]; // 定义刷新缓存

String uart_rev;
static lv_obj_t *slider_label;
extern lv_indev_t *indev_encoder; 


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
int16_t Encoder_get_new_move() //方向键接口
{
    static uint8_t state = 0;
    int16_t new_move = 0;
    if (state == 0)
    {
        state = 1;
        if (digitalRead(16) == 0)
        {
            new_move = -1;
            // new_move = LV_KEY_NEXT;
            // new_move = LV_KEY_DOWN;
            // new_move = LV_KEY_RIGHT;
        }

        if (digitalRead(16) == 0)
        {
            new_move = 1;
            // new_move = LV_KEY_PREV;
            // new_move = LV_KEY_UP;
            // new_move = LV_KEY_LEFT;
        }
    }

    if (digitalRead(16) == 1 && digitalRead(16) == 1)
    {
        state = 0;
    }
    return new_move;
}

uint8_t Encoder_get_key_enter_state() //确认键接口
{
    if (digitalRead(16) == 0)
    {
        return LV_KEY_ENTER;
    }
    return 0;
}
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) //传递键值给LVGL
{

    if (Encoder_get_key_enter_state())
    {
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
        data->enc_diff = Encoder_get_new_move();
    }

}
static void event_slider(lv_event_t *e) //滑条触发事件, 触发条件:数值改变;
{
    lv_obj_t *slider = lv_event_get_target(e);                               //这里的 slider 就是指向触发该事件的滑条
    char buf[8];                                                             //分配8个char 的内存
    lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider)); // lv_slider_get_value(slider)返回的是滑条改变后的值
    lv_label_set_text(slider_label, buf);   //设置label文本
    static String s;
    s = buf;                           //传给字符串s
    Serial.println(s);                 //串口输出s
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
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, TFT_WIDTH * 35);

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = TFT_WIDTH;
    disp_drv.ver_res = TFT_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);


    pinMode(16, INPUT_PULLUP); 
     /*初始化输入设备*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = my_touchpad_read;
    indev_encoder = lv_indev_drv_register(&indev_drv);


    lv_group_t *g = lv_group_create();   //创建组
    lv_group_set_default(g);         //设置组(group)为默认模式,后面的部件创建时会自动加入组(group)
    lv_indev_set_group(indev_encoder, g);    //将输入设备(indev)和组(group)关联起来

    lv_obj_t *button = lv_btn_create(lv_scr_act());   //一个按钮
    lv_obj_align(button, LV_ALIGN_CENTER, 0, -60);
    lv_obj_set_size(button, 30, 30);
    lv_obj_set_style_bg_color(button, lv_color_hex3(0xc43e1c), LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *button1 = lv_btn_create(lv_scr_act());   //又一个按钮
    lv_obj_align(button1, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_size(button1, 30, 30);
    lv_obj_set_style_bg_color(button1, lv_color_hex3(0xc43e1c), LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *slider = lv_slider_create(lv_scr_act());   //创建个滑条
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, -20);
    lv_obj_set_size(slider, 50, 10);

    slider_label = lv_label_create(lv_scr_act());    //标签
    lv_label_set_text(slider_label, "0%");
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_RIGHT_MID, 10, 0);  //放在滑条的右边

    // lv_group_add_obj(g, button);    //如果组(group)没有设置默认模式,这里就手动添加
    // lv_group_add_obj(g, button1);
    // lv_group_add_obj(g, slider);



    lv_obj_add_event_cb(slider, event_slider, LV_EVENT_VALUE_CHANGED, NULL); 
}

void loop()
{
    lv_timer_handler(); /* let the GUI do its work */



    if(digitalRead(16) == 0){
        Serial.println("1");
    }
    delay(5);
}