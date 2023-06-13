#include "esp_camera.h"
#include "RTOS.h"
#include <time.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "esp_http_client.h"
#include <ArduinoJson.h>
#include "PubSubClient.h"
#include <lvgl.h>
#include "lv_page_common.h"

#include <BleKeyboard.h>
#include "camera.h"
#include "config.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

/*********************需要修改的地方**********************/
const char* mqttServer = "bemfa.com";

const char* user = NULL;
const char* pass = NULL;
const char* willTopic = "填写主题";
const uint8_t willQos = 0;
const boolean willRetain = false;
const char* willMessage = "OFFLINE";
const boolean cleanSession = false;

// 建立订阅主题
const char* subTopic = "填写主题";


const char*  uid = "52160dd34d0b4f7be1ccb551b77c63ac";    //用户私钥，巴法云控制台获取
const char*  topic = "mypic";     //主题名字，可在控制台新建
const char*  wechatMsg = NULL;          //如果不为空，会推送到微信，可随意修改，修改为自己需要发送的消息
const char*  wecomMsg = NULL;         //如果不为空，会推送到企业微信，推送到企业微信的消息，可随意修改，修改为自己需要发送的消息
const char*  urlPath = NULL;          //如果不为空，会生成自定义图片链接，自定义图片上传后返回的图片url，url前一部分为巴法云域名，第二部分：私钥+主题名的md5值，第三部分为设置的图片链接值。
/********************************************************/
const char*  post_url = "http://images.bemfa.com/upload/v1/upimages.php"; // 默认上传地址
static String httpResponseString;  //接收服务器返回信息
bool internet_connected = false;

unsigned char button_state;

TaskHandle_t GUI_Handle;
TaskHandle_t UART_Handle;
TaskHandle_t Camera_Handle;
TaskHandle_t Light_Handle;
TaskHandle_t Mqtt_Handle;
void GUI_Task(void *parameter);
void UART_Task(void *parameter);
void Camera_Task(void *parameter);
void Light_Task(void *parameter);
void Mqtt_Task(void *parameter);

extern lv_obj_t *home_scr;

esp_err_t take_send_photo();
esp_err_t _http_event_handler(esp_http_client_event_t *evt);


void connectMQTTServer(){
  // 连接MQTT服务器
  if (mqttClient.connect(uid, user, pass, willTopic, willQos, willRetain, willMessage, cleanSession)) { 
    Serial.println("MQTT Server Connected.");
    Serial.println("Server Address: ");
    Serial.println(mqttServer);
    Serial.println("ClientId:");
    Serial.println(uid);
    // 订阅指定主题
    subscribeTopic(); 
  } else {
    Serial.print("MQTT Server Connect Failed. Client State:");
    Serial.println(mqttClient.state());
    delay(3000);
  }   
}

// 收到信息后的回调函数
void receiveCallback(char* topic, byte* payload, unsigned int length) {
  
  Serial.print("Message Received [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("");
  Serial.print("Message Length(Bytes) ");
  Serial.println(length);
}
 
// 订阅指定主题
void subscribeTopic(){
 
  // 通过串口监视器输出是否成功订阅主题以及订阅的主题名称
  if(mqttClient.subscribe(subTopic)) {
    Serial.println("Subscrib Topic:");
    Serial.println(subTopic);
  } else {
    Serial.print("Subscribe Fail...");
  }  
}

void setup()
{
  all_init();

  // 设置MQTT服务器和端口号
  mqttClient.setServer(mqttServer, 9501);
  // 设置MQTT订阅回调函数
  mqttClient.setCallback(receiveCallback);
 
  // 连接MQTT服务器
  connectMQTTServer();


  // lv_app_load_weather_scr();
  // lv_app_load_home_scr();
  // lv_app_scroll();
  // lv_app_load_class_scr();
  // lv_app_load_pc_state_scr();
  // lv_app_load_ble_scr();
  // lv_app_load_wifi_scr();

  // GUI任务
  xTaskCreate(GUI_Task, "TaskOne", 30000, NULL, 1, &GUI_Handle); 

  // 串口接收任务
  xTaskCreate(UART_Task, "TaskTwo", 10000, NULL, 10, &UART_Handle);

  // 摄像头任务
  xTaskCreate(Camera_Task, "TaskThree", 20000, NULL, 1, &Camera_Handle);
  vTaskSuspend(Camera_Handle);

  // 灯光控制任务
  // xTaskCreate(Light_Task, "TaskFour", 10000, NULL, 1, &Light_Handle);

  // mqtt控制任务
  xTaskCreate(Mqtt_Task, "TaskFive", 5000, NULL, 1, &Mqtt_Handle);
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

  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
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
  esp_http_client_set_post_field(http_client, (const char *)fb->buf, fb->len);//设置http发送的内容和长度
  esp_http_client_set_header(http_client, "Content-Type", "image/jpg"); //设置http头部字段
  esp_http_client_set_header(http_client, "Authorization", uid);        //设置http头部字段
  esp_http_client_set_header(http_client, "Authtopic", topic);          //设置http头部字段
  esp_http_client_set_header(http_client, "wechatmsg", wechatMsg);      //设置http头部字段
  esp_http_client_set_header(http_client, "wecommsg", wecomMsg);        //设置http头部字段
  esp_http_client_set_header(http_client, "picpath", urlPath);          //设置http头部字段
  esp_err_t err = esp_http_client_perform(http_client);//发送http请求
  if (err == ESP_OK) {
    Serial.println(httpResponseString);//打印获取的URL
    //json数据解析
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, httpResponseString);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
    }
    String url = doc["url"];
    Serial.println(url);//打印获取的URL
  }
  esp_http_client_cleanup(http_client);
  esp_camera_fb_return(fb);
  return res;
}

void GUI_Task(void *parameter)
{
  lv_app_load_home_scr();
  Serial.println("guiTask");
  while (1)
  {
    lv_timer_handler(); /* let the GUI do its work */
    vTaskDelay(1);
  }
}

void Camera_Task(void *parameter)
{
  while (1)
  {
    camera_show();

    if (button_state == 5) { // Take picture
      take_send_photo();
    }

    vTaskDelay(1);
  }
}

void Light_Task(void *parameter){

}

void UART_Task(void *parameter)
{
  String uart_rev; // 接收字符串命令
  while (1)
  {
    if (Serial.available())
    {
      uart_rev = Serial.readString();
      Serial.print("rev=");
      Serial.println(uart_rev);

      if (uart_rev.toInt() == 1)
      {
        // vTaskDelete(Camera_Handle);
        // vTaskDelay(100);
        // xTaskCreate(GUI_Task, "TaskOne", 10000, NULL, 1, &GUI_Handle);

        vTaskSuspend(Camera_Handle);
        vTaskDelay(1);
        lv_app_load_home_scr();
        vTaskDelay(1);
        vTaskResume(GUI_Handle);
      }
      else if (uart_rev.toInt() == 2)
      {
        // vTaskDelete(GUI_Handle);
        // vTaskDelay(100);
        // xTaskCreate(Camera_Task, "TaskTwo", 10000, NULL, 1, &Camera_Handle);

        vTaskSuspend(GUI_Handle);
        vTaskDelay(1);
        vTaskResume(Camera_Handle);
      }

      if (uart_rev.compareTo("RIGHT") == 0)
      {
        button_state = 1;
      }
      else if (uart_rev.compareTo("LEFT") == 0)
      {
        button_state = 2;
      }
      else if (uart_rev.compareTo("ENTER") == 0)
      {
        button_state = 3;
      }
      else if (uart_rev.compareTo("EXIT") == 0)
      {
        button_state = 4;
      }
      else if (uart_rev.compareTo("TAKEPIC") == 0)
      {
        button_state = 5;
      }
      else
      {
        button_state = 0;
      }
      Serial.println(button_state);
    }
    vTaskDelay(10);
  }
}

void loop()
{

}
