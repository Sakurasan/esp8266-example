// #include <Arduino.h>
// #include <U8g2lib.h>

// #ifdef U8X8_HAVE_HW_SPI
// #include <SPI.h>
// #endif
// #ifdef U8X8_HAVE_HW_I2C
// #include <Wire.h>
// #endif

// U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ D2, /* data=*/ D1, /* reset=*/ U8X8_PIN_NONE);   // 此处 D1 D2是对应焊接的脚针

// void setup(void) {
//   pinMode(LED_BUILTIN,OUTPUT);
//   u8g2.begin();
//   u8g2.enableUTF8Print();		// enable UTF8 support for the Arduino print() function
// }

// void loop(void) {
//   digitalWrite(LED_BUILTIN,HIGH);
//   delay(1000);
//   digitalWrite(LED_BUILTIN,LOW);
//   u8g2.setFont(u8g2_font_unifont_t_chinese2);  // use chinese2 for all the glyphs of "你好世界"
//   //u8g2.setFont(u8g2_font_b10_t_japanese1);  // all the glyphs of "こんにちは世界" are already included in japanese1: Lerning Level 1-6
//   u8g2.setFontDirection(0);
//   u8g2.firstPage();
//   do {
//     u8g2.setCursor(0, 15);
//     u8g2.print("Hello World!");
//     u8g2.setCursor(0, 50);
//     u8g2.print("你好世界");		// Chinese "Hello World" 
//     //u8g2.print("こんにちは世界");		// Japanese "Hello World" 
//   } while ( u8g2.nextPage() );
//   delay(1000);
// }


// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor


#include <Chrono.h>
#include <Servo.h>
#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>

#include <U8g2lib.h>
#include <U8g2wqy.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
 
#define DHTPIN 1              //定义DHT11模块连接管脚
#define RELAYPIN 0
#define DHTTYPE DHT11           //使用 DHT 11模块
//#define DHTTYPE DHT22       //使用DHT 22  (AM2302), AM2321模块
//#define DHTTYPE DHT21        //使用 DHT 21 (AM2301)模块

// 设置wifi接入信息(请根据您的WiFi信息进行修改)
const char* ssid = "CCC";
const char* password = "ccc123456";
const char* mqttServer = "test.ranye-iot.net";

Chrono chronoA;
Chrono chronoB;
Chrono chronoC;

Ticker ticker;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
int count;    // Ticker计数用变量
String stringOne, stringTwo, stringThree, stringfour, stringfive, string0, string1;
int i = 0;
float t,h;

DHT dht(DHTPIN, DHTTYPE);   //定义dht

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ D2, /* data=*/ D1, /* reset=*/ U8X8_PIN_NONE);   // 此处 D1 D2是对应焊接的脚针

Servo myServo;  // 定义Servo对象来控制
int pos = 0;    // 角度存储变量

void tickerCount(){
  count++;
}

void connectMQTTServer(){
  // 根据ESP8266的MAC地址生成客户端ID（避免与其它ESP8266的客户端ID重名）
  String clientId = "esp8266-" + WiFi.macAddress();
 
  // 连接MQTT服务器
  if (mqttClient.connect(clientId.c_str())) { 
    Serial.println("MQTT Server Connected.");
    Serial.println("Server Address: ");
    Serial.println(mqttServer);
    Serial.println("ClientId:");
    Serial.println(clientId);
  } else {
    Serial.print("MQTT Server Connect Failed. Client State:");
    Serial.println(mqttClient.state());
    delay(3000);
  }   
}
 
 // 发布信息
void pubMQTTmsg(float temp ,float humi){
  // 建立发布主题。主题名称以Taichi-Maker-为前缀，后面添加设备的MAC地址。
  // 这么做是为确保不同用户进行MQTT信息发布时，ESP8266客户端名称各不相同，
  String topicString = "Maisan-Pub-" + WiFi.macAddress();
  char publishTopic[topicString.length() + 1];  
  strcpy(publishTopic, topicString.c_str());
 
  // 建立发布信息。信息内容以Hello World为起始，后面添加发布次数。
  String messageString = String(temp)+"|"+String(humi); 
  char publishMsg[messageString.length() + 1];   
  strcpy(publishMsg, messageString.c_str());
  
  // 实现ESP8266向主题发布信息
  if(mqttClient.publish(publishTopic, publishMsg)){
    Serial.println("Publish Topic:");Serial.println(publishTopic);
    Serial.println("Publish message:");Serial.println(publishMsg);    
  } else {
    Serial.println("Message Publish Failed."); 
  }
}

void subMQTTmsg(){
  String topicString = "Maisan-Sub-" + WiFi.macAddress();
  char subTopic[topicString.length() + 1];  
  strcpy(subTopic, topicString.c_str());
  // strcpy(sub)
  if (mqttClient.subscribe(subTopic))
  {
    Serial.println("Subscrib Topic:");
    Serial.println(subTopic);
  }else{
     Serial.print("Subscribe Fail...");
  }
  
}

// 控制舵机
void steering(){
  for (pos = 0; pos <= 180; pos ++) { // 0°到180°
    // in steps of 1 degree
    myServo.write(pos);              // 舵机角度写入
    delay(15);                       // 等待转动到指定角度
  } 
  delay(2000);//延时2s
  for (pos = 180; pos >= 0; pos --) { // 从180°到0°
    myServo.write(pos);              // 舵机角度写入
    delay(15);                       // 等待转动到指定角度
  }
  delay(2000);//延时2s 
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

  if ((char)payload[0] == '1') {     // 如果收到的信息以“1”为开始
    digitalWrite(RELAYPIN, LOW);  // 则点亮LED。
    Serial.println("ON");
  } else if ((char)payload[0] == 'a'){                           
    digitalWrite(RELAYPIN, HIGH); // 否则熄灭LED。
    Serial.println("OFF");
  }
}

// ESP8266连接wifi
void connectWifi(){
 
  WiFi.begin(ssid, password);
 
  //等待WiFi连接,成功连接后输出成功信息
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  // for (int i = 0; i < 100; i++)
  // {
  //   if (WiFi.status() != WL_CONNECTED)
  //   {
  //     delay(1000);
  //     Serial.print(".");
  //   }
  // }
  // if (WiFi.status() != WL_CONNECTED)
  // {
  //   Serial.println("无法连接WiFi，跳过链接");
  //   return ;
  // }
  
  
  Serial.println("");
  Serial.println("WiFi Connected!");  
  Serial.println(""); 
}


void initwifi(){
  //设置ESP8266工作模式为无线终端模式
  WiFi.mode(WIFI_STA);
  
  // 连接WiFi
  connectWifi();
  
  // 设置MQTT服务器和端口号
  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(receiveCallback);
 
  // 连接MQTT服务器
  connectMQTTServer();
 
  // Ticker定时对象
  ticker.attach(1, tickerCount); 
}

void initdisplay(){
  u8g2.begin();
  u8g2.enableUTF8Print();
}

void display( String arr[2] ){
// 字体名	                   字符数	字节 
// 12,13,14,15,16
// u8g2_font_wqy12_t_chinese1	411	9,491
// u8g2_font_wqy12_t_chinese2	574	13,701
// u8g2_font_wqy12_t_chinese3	993	25,038
// u8g2_font_wqy12_t_gb2312a	4041	111,359
// u8g2_font_wqy12_t_gb2312b	4531	120,375
// u8g2_font_wqy12_t_gb2312	7539	208,228
  u8g2.setFont(u8g2_font_wqy15_t_gb2312a); 
  u8g2.setFontDirection(0);
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 15);
    if (WiFi.status() != WL_CONNECTED)
    {
      u8g2.print("离线使用中 !");
    }else{
      u8g2.print("WiFi已连接");
    }
    
    
    u8g2.setCursor(0, 35);
    u8g2.print(arr[0]);		// Chinese "Hello World" 
    u8g2.setCursor(0, 55);
    u8g2.print(arr[1]);

  } while ( u8g2.nextPage() );
  // delay(1000);
}


void setup() {                                               //主程序
  Serial.begin(115200);  
  // pinMode(LED_BUILTIN,OUTPUT);
  // digitalWrite(LED_BUILTIN,HIGH);
  pinMode(D5,OUTPUT);
  pinMode(2,OUTPUT); 
  pinMode(RELAYPIN,OUTPUT);
  digitalWrite(RELAYPIN,HIGH);

  initwifi(); 
  initdisplay();
  myServo.attach(D5); //舵机
                                  //设置串口
  Serial.println(F("DHT开始测试!"));             //串口打印DHT开始测试!
  dht.begin();                                              //DHT开始工作
//字符串用于后面拼接
  stringOne = String("温度:");
  stringTwo = String("℃");  
  stringfour = String(" 湿度:");
  stringfive = String("%");
  stringThree = String();
  string1 = String("传感器出错");
}

void loop() {   
  digitalWrite(LED_BUILTIN,HIGH);  //关灯                                               
  if (chronoA.hasPassed(1000))
  {                                                      //等待2秒，读取温度或湿度大约需要250毫秒！避免发生错误
    h = dht.readHumidity();                            //将湿度度赋予h
    t = dht.readTemperature();                      //读取摄氏度赋予t
    if (isnan(h) || isnan(t)) {                                      //读取失败显示
      Serial.println(F("读取失败，找不到传感器!"));
      // WiFi.softAP(string1);                     
      // return;
    }
    float hic = dht.computeHeatIndex(t, h, false);   //用摄氏度计算酷热指数，摄氏度设参数为false
    Serial.print(F("湿度: "));                                  //串口打印
    Serial.print(h);                                              //串口打印湿度值
    Serial.print(F("%,温度: "));                            //串口打印
    Serial.print(t);                                             //串口打印温度值
    Serial.print(F("°C "));                                    //串口打印
    Serial.print(F(", 体感温度: "));                      //串口打印
    Serial.print(hic);                                      //串口打印体感温度
    Serial.println(F("°C "));                             //串口打印
    stringThree =  stringOne + t + stringTwo + stringfour + h + stringfive;

    String arr[2];
    arr[0] = "温度: "+String(t)+ " °C ";
    arr[1] = "湿度: "+String(h) +" % ";
    display(arr);

    if (i < 10){i=i+1;}else{i=i-10;};                //引入i，在串口调试的时候每次发送不同数字，便于区别
    Serial.println("测试段落开始 ");
    Serial.println(stringThree);
    Serial.println(i);
    Serial.println("测试段落结束 ");
    delay(2500);
  }
  
  if (chronoB.hasPassed(500))
  {
    if (mqttClient.connected()) { // 如果开发板成功连接服务器
    // 每隔3秒钟发布一次信息
    // if (count >= 3){
    //   count = 0;
    // }  
    pubMQTTmsg(t,h);  
    
    // 保持心跳
      mqttClient.loop();
    } else {                  // 如果开发板未能成功连接服务器
      connectMQTTServer();    // 则尝试连接服务器
    }
  }
  
  if (chronoC.hasPassed(100))
  {
    /* code */
    if (mqttClient.connected()) { // 如果开发板成功连接服务器  
      subMQTTmsg();
    // 保持心跳
      mqttClient.loop();
    } else {                  // 如果开发板未能成功连接服务器
      connectMQTTServer();    // 则尝试连接服务器
      Serial.println("-------------------------");
    }
  }
  

}
