#include "mqtt.h"

const char* mqttServer = "bemfa.com";
const char*  uid1 = "52160dd34d0b4f7be1ccb551b77c63ac";    //用户私钥，巴法云控制台获取
const char* user = NULL;
const char* pass = NULL;
const char* willTopic = "test";
const uint8_t willQos = 0;
const boolean willRetain = false;
const char* willMessage = "OFFLINE";
const boolean cleanSession = false;

// 建立订阅主题
const char* subTopic = "test";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void mqtt_init(){
      // 设置MQTT服务器和端口号
  mqttClient.setServer(mqttServer, 9501);
  // 设置MQTT订阅回调函数
  mqttClient.setCallback(receiveCallback);
 
  // 连接MQTT服务器
  connectMQTTServer();
}

// 订阅指定主题
void subscribeTopic(){
  // 通过串口监视器输出是否成功订阅主题以及订阅的主题名称
  if(mqttClient.subscribe(subTopic)) {
    Serial.println("Subscrib Topic:");
    Serial.println(subTopic);
  }
}

void connectMQTTServer(){
  // 连接MQTT服务器
  if (mqttClient.connect(uid1, user, pass, willTopic, willQos, willRetain, willMessage, cleanSession)) { 
    Serial.println("MQTT Server Connected.");
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
  Serial.print("Message Received :");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}
