#include "lv_page_common.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include <iostream>

extern unsigned char button_state;
extern unsigned char page_idx;
extern unsigned char light_fun;

const char *host = "api.seniverse.com";
/*privateKey改成自己的*/
const char *privateKey = "pqe1fgv45lrdruq7"; 
const char *city = "meizhou";
const char *language = "zh-Hans";

static lv_obj_t * slider_label;
static lv_obj_t * slider_label2;
lv_timer_t *timer_time1, *voice_timer;
static lv_obj_t * slider, * start_scr;
int temp, img_idx;
static lv_style_t font_16_label_style;
static lv_obj_t *voice_img;
static lv_obj_t *hello_label;

LV_IMG_DECLARE(plane);
LV_IMG_DECLARE(voice1);
LV_IMG_DECLARE(voice2);
LV_IMG_DECLARE(voice3);
LV_IMG_DECLARE(voice4);

lv_user_weather_info_t user_weather_info = {0};

// APP切换
void app_exit_event_handler(lv_event_t* e)
{
    light_fun = 0;
    lv_app_scroll();
}

static void lv_start_timer_update(lv_timer_t *timer)
{
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d%%", temp);
    lv_label_set_text(slider_label, buf);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_slider_set_value(slider, temp, LV_ANIM_ON);
    if(temp == 0){
        lv_style_init(&font_16_label_style);
        lv_style_set_text_font(&font_16_label_style, &myfont_16);
        slider_label2 = lv_label_create(start_scr);
        lv_obj_add_style(slider_label2, &font_16_label_style, 0);
        lv_label_set_text_fmt(slider_label2, "%s", "WIFI初始化");
        lv_obj_align_to(slider_label2, slider, LV_ALIGN_CENTER, 0, -25);
        
    }
    else if(temp == 20){
      lv_label_set_text_fmt(slider_label2, "%s", "MQTT初始化");
    }
    else if(temp == 40){
      lv_label_set_text_fmt(slider_label2, "%s", "摄像头初始化");
    }
    else if(temp == 60){
      lv_label_set_text_fmt(slider_label2, "%s", "获取时间");
    }
    else if(temp == 80){
      lv_label_set_text_fmt(slider_label2, "%s", "获取天气");
    }
    else if(temp == 100){
      lv_timer_del(timer_time1);
      lv_app_load_home_scr();
    }
    temp++;
}
static void lv_voice_timer_update(lv_timer_t *timer){
  if(img_idx == 4){
    img_idx = 0;
  }
  if(button_state == 6){
    button_state = 0;
    lv_timer_del(voice_timer);
    lv_label_set_text_fmt(hello_label, "%s", "#0000ff 好的#");
    switch (page_idx)
    {
    case 0:
    lv_app_load_camera_scr();
      break;
    case 1:
    lv_app_load_class_scr();
      break;
    case 2:
    lv_app_load_weather_scr();
      break;
    case 3:
    lv_app_load_light_scr();
      break;
    case 4:
    lv_app_load_fft_scr();
      break;
    default:
    lv_app_load_home_scr();
      break;
    }
  }
  switch (img_idx)
  {
  case 0:
    lv_img_set_src(voice_img, &voice1);
    break;
  case 1:
    lv_img_set_src(voice_img, &voice2);
  break;
  case 2:
    lv_img_set_src(voice_img, &voice3);
  break;
  default:
    lv_img_set_src(voice_img, &voice4);
    break;
  }
  img_idx++;
}
// 开始动画
void lv_app_load_start_scr(){
    /*Create a slider in the center of the display*/
    start_scr = lv_obj_create(NULL);
    lv_obj_set_size(start_scr, 240, 240);  // 设置大小
    slider = lv_slider_create(start_scr);
    lv_obj_align_to(slider, start_scr, LV_ALIGN_BOTTOM_MID, 30, -50);
    lv_obj_set_size(slider, 200, 10);

    /*Create a label below the slider*/
    slider_label = lv_label_create(start_scr);
    lv_label_set_text(slider_label, "0%");
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    // 你好世界
    static lv_obj_t *hello_label = lv_label_create(start_scr);
    lv_label_set_recolor(hello_label, true);//颜色重绘
    static lv_style_t font_20_label_style;
    lv_style_init(&font_20_label_style);
    lv_style_set_text_font(&font_20_label_style, &myfont);
    lv_obj_add_style(hello_label, &font_20_label_style, 0);
    lv_label_set_text_fmt(hello_label, "%s", "#0000ff 你好##ff00ff 世界#");
    lv_obj_align_to(hello_label, start_scr, LV_ALIGN_TOP_MID, 0, 80);

    // 纸飞机
    lv_obj_t *plane_img = lv_img_create(start_scr);
    lv_img_set_src(plane_img, &plane);
    lv_obj_align_to(plane_img, start_scr, LV_ALIGN_TOP_MID, 50, 30);

    timer_time1 = lv_timer_create(lv_start_timer_update, 100, NULL);
    lv_scr_load_anim(start_scr, LV_SCR_LOAD_ANIM_OVER_LEFT, 500, 0, true);
}

// 语音动画
void lv_app_load_voice_scr(){
    /*Create a slider in the center of the display*/
    static lv_obj_t* voice_scr = lv_obj_create(NULL);
    lv_obj_set_size(voice_scr, 240, 240);  // 设置大小
    lv_obj_set_style_bg_color(voice_scr, lv_color_make(18, 18, 23), 0);

    // voice
    voice_img = lv_img_create(voice_scr);
    lv_img_set_src(voice_img, &voice1);
    lv_obj_align_to(voice_img, voice_scr, LV_ALIGN_CENTER, 0, -30);

    // 我在
    hello_label = lv_label_create(voice_scr);
    lv_label_set_recolor(hello_label, true);//颜色重绘
    static lv_style_t font_20_label_style;
    lv_style_init(&font_20_label_style);
    lv_style_set_text_font(&font_20_label_style, &myfont);
    lv_obj_add_style(hello_label, &font_20_label_style, 0);
    lv_label_set_text_fmt(hello_label, "%s", "#0000ff 我在#");
    lv_obj_align_to(hello_label, voice_scr, LV_ALIGN_BOTTOM_MID, 0, -20);

    voice_timer = lv_timer_create(lv_voice_timer_update, 200, NULL);
    lv_scr_load_anim(voice_scr, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, true);
}

void lv_app_weather_update()
{
    WiFiClient client;
    DynamicJsonDocument Weather_Json(1400);

    if (!client.connect(host, 80))
    {
        Serial.println("Connect host failed!");
        return;
    }
    //Serial.println("host Conected!");

    String getUrl = "/v3/weather/daily.json?key=";
    getUrl += privateKey;
    getUrl += "&location=";
    getUrl += city;
    getUrl += "&language=";
    getUrl += language;
    client.print(String("GET ") + getUrl + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    //Serial.println("Get send");

    char endOfHeaders[] = "\r\n\r\n";
    bool ok = client.find(endOfHeaders);
    if (!ok)
    {
        Serial.println("No response or invalid response!");
    }
    //Serial.println("Skip headers");

    String line = "";

    line += client.readStringUntil('\n');

    // Serial.println("Weather_data");
    // Serial.println(line);

    DeserializationError error = deserializeJson(Weather_Json, line);
    if (error)
    {
        Serial.println("deserialize json failed");
        return;
    }
    
    user_weather_info.day_one_text = Weather_Json["results"][0]["daily"][0]["text_day"].as<const int>();
    user_weather_info.day_one_temp_high = Weather_Json["results"][0]["daily"][0]["high"].as<const int>();
    user_weather_info.day_one_temp_low = Weather_Json["results"][0]["daily"][0]["low"].as<const int>();
    sscanf(Weather_Json["results"][0]["daily"][0]["date"].as<const char *>(),"%d-%d-%d",&user_weather_info.day_one_year,&user_weather_info.day_one_month,&user_weather_info.day_one_date);
    user_weather_info.day_one_humi = Weather_Json["results"][0]["daily"][0]["humidity"].as<const int>();
    user_weather_info.day_one_windspeed = Weather_Json["results"][0]["daily"][0]["wind_speed"].as<const int>();
    strcpy(user_weather_info.day_one_wind_direction,Weather_Json["results"][0]["daily"][0]["wind_direction"].as<const char *>());

    user_weather_info.day_two_text = Weather_Json["results"][0]["daily"][1]["text_day"].as<const int>();
    user_weather_info.day_two_temp_high = Weather_Json["results"][0]["daily"][1]["high"].as<const int>();
    user_weather_info.day_two_temp_low = Weather_Json["results"][0]["daily"][1]["low"].as<const int>();
    sscanf(Weather_Json["results"][0]["daily"][1]["date"].as<const char *>(),"%d-%d-%d",&user_weather_info.day_two_year,&user_weather_info.day_two_month,&user_weather_info.day_two_date);
    
    user_weather_info.day_three_text = Weather_Json["results"][0]["daily"][2]["text_day"].as<const int>();
    user_weather_info.day_three_temp_high = Weather_Json["results"][0]["daily"][2]["high"].as<const int>();
    user_weather_info.day_three_temp_low = Weather_Json["results"][0]["daily"][2]["low"].as<const int>();
    sscanf(Weather_Json["results"][0]["daily"][2]["date"].as<const char *>(),"%d-%d-%d",&user_weather_info.day_three_year,&user_weather_info.day_three_month,&user_weather_info.day_three_date);

    Weather_Json.clear();

    client.stop();
}
