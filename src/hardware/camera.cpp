#include "camera.h"

extern TFT_eSPI tft;
extern TFT_eFEX fex;

/*********************需要修改的地方**********************/
const char *uid = "52160dd34d0b4f7be1ccb551b77c63ac"; // 用户私钥，巴法云控制台获取
const char *topic = "mypic";                          // 主题名字，可在控制台新建
const char *wechatMsg = NULL;                         // 如果不为空，会推送到微信，可随意修改，修改为自己需要发送的消息
const char *wecomMsg = NULL;                          // 如果不为空，会推送到企业微信，推送到企业微信的消息，可随意修改，修改为自己需要发送的消息
const char *urlPath = NULL;                           // 如果不为空，会生成自定义图片链接，自定义图片上传后返回的图片url，url前一部分为巴法云域名，第二部分：私钥+主题名的md5值，第三部分为设置的图片链接值。
/********************************************************/
const char *post_url = "http://images.bemfa.com/upload/v1/upimages.php"; // 默认上传地址
static String httpResponseString;                                        // 接收服务器返回信息
bool internet_connected = false;

void camera_init(){
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

void camera_show(void)
{
    camera_fb_t *fb = esp_camera_fb_get();

    // Serial.println("fb->buf, fb->len");
    // Serial.write(fb->buf, fb->len);

    fex.drawJpg((const uint8_t *)fb->buf, fb->len, 0, 0, 240, 240, 0, 0);

    esp_camera_fb_return(fb);
}


/********http请求处理函数*********/
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
  if (evt->event_id == HTTP_EVENT_ON_DATA)
  {
    httpResponseString.concat((char *)evt->data);
  }
  return ESP_OK;
}
/********推送图片*********/
esp_err_t take_send_photo()
{
  Serial.println("Taking picture...");
  esp_err_t res = ESP_OK;

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb)
  {
    Serial.println("Camera capture failed");
    return ESP_FAIL;
  }

  httpResponseString = "";
  esp_http_client_handle_t http_client;
  esp_http_client_config_t config_client = {0};
  config_client.url = post_url;
  config_client.event_handler = _http_event_handler;
  config_client.method = HTTP_METHOD_POST;
  http_client = esp_http_client_init(&config_client);
  esp_http_client_set_post_field(http_client, (const char *)fb->buf, fb->len); // 设置http发送的内容和长度
  esp_http_client_set_header(http_client, "Content-Type", "image/jpg");        // 设置http头部字段
  esp_http_client_set_header(http_client, "Authorization", uid);               // 设置http头部字段
  esp_http_client_set_header(http_client, "Authtopic", topic);                 // 设置http头部字段
  esp_http_client_set_header(http_client, "wechatmsg", wechatMsg);             // 设置http头部字段
  esp_http_client_set_header(http_client, "wecommsg", wecomMsg);               // 设置http头部字段
  esp_http_client_set_header(http_client, "picpath", urlPath);                 // 设置http头部字段
  esp_err_t err = esp_http_client_perform(http_client);                        // 发送http请求
  if (err == ESP_OK)
  {
    Serial.println(httpResponseString); // 打印获取的URL
    // json数据解析
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, httpResponseString);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
    }
    String url = doc["url"];
    Serial.println(url); // 打印获取的URL
  }
  esp_http_client_cleanup(http_client);
  esp_camera_fb_return(fb);
  return res;
}