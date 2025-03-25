/************************************************************
  ===================ESP8266简易WIFI天气时钟==================
  Author：码农爱学习
  Date: 2022/05/28
  Version: V1.0
  Blog: https://xxpcb.gitee.io/
  BiliBili: https://space.bilibili.com/146899653
  图文教程：https://www.bilibili.com/read/cv16758967
  ------------------------------------------------------------
  硬件：ESP8266 NodeMCU + 12864 SSD1306 OLED显示屏（4pin IIC引脚）
  接线：[ESP8266]  --- [OLED]
       3.3V     ---  VCC
       G (GND)  ---  GND
       D1(GPIO5)---  SCL
       D2(GPIO4)---  SDA
************************************************************/

#include <TimeLib.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <WiFiUdp.h>

#define SCL 5
#define SDA 4
#define HTTP_TIMEOUT 5000
#define CAPACITY (JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 70)

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /*clock=*/SCL, /*data=*/SDA, /*reset=*/U8X8_PIN_NONE);   // All Boards without Reset of the Display

//---------------WIFI信息------------------------------
const char ssid[] = "ChinaNet-snVy";      //WiFi名(<---------替换为自己的)
const char pass[] = "zyz18920200023";      //WiFi密码(<---------替换为自己的)
//-----------------------------------------------------

WiFiClient client;
HTTPClient http;

void initOLED();
void initWiFi();
void initNTP();

//--------- weather ---------------
const char* host = "api.seniverse.com";     // 将要连接的服务器地址
const int httpPort = 80;                    // 将要连接的服务器端口
//心知天气HTTP请求所需信息
String reqUserKey = "S48EXgBmbWspefEj7";   // 私钥(<---------替换为自己的)
String reqLocation = "Tianjin";           // 城市
String reqUnit = "c";                      // 摄氏/华氏
//建立心知天气API当前天气请求资源地址
String reqRes = "/v3/weather/now.json?key=" + reqUserKey +
                + "&location=" + reqLocation +
                "&language=en&unit=" + reqUnit;

//--------网络校时的相关配置---------
#define timeZone (8) //时区设置，采用东8区
#define NTP_PACKET_SIZE (48)     // NTP时间在消息的前48个字节里
static const char ntpServerName[] = "ntp1.aliyun.com"; //NTP服务器，使用阿里云
byte packetBuffer[NTP_PACKET_SIZE]; // 输入输出包的缓冲区
unsigned int localPort = 8888; // 用于侦听UDP数据包的本地端口
boolean isNTPConnected = false;
WiFiUDP Udp;

//----------时间、天气信息自定义结构体--------
typedef struct 
{
  int tm_year;
  int tm_mon;
  int tm_mday;
  int tm_hour;
  int tm_min;
  int tm_sec;
  int tm_week;
}rtc_time_t;

typedef struct 
{
  String weather; //天气字符串
  int iconIdx;    //天气对应的图标索引
  int temp;       //温度
}weather_info_t;

#define IOCN_OVERCAST 0
#define IOCN_CLOUDY   1
#define IOCN_RAIN     2
#define IOCN_SUNNY    3
char* icon_index[4]={"@","A","C","E"};//图标在天气字库里分别代表 阴(0x40)，云，雨，晴

typedef struct
{
  int XinZhi_code;
  int icon_idx;
}weather_code_t;

weather_code_t weather_icon_table[4] = {
  {0, IOCN_SUNNY},
  {4, IOCN_CLOUDY},
  {9, IOCN_OVERCAST},
  {13, IOCN_RAIN}
};

//天气相关变量
String g_strWeather = "";
String g_strUpdateTime = "";
int g_iTemperature = 18;
int g_iCode = -1;
int g_iIdx = -1;

//==============================seup&loop=============================
void setup()
{
  Serial.begin(9600);
  Serial.println("NTP time and WiFi Weather test");
  initOLED();
  initWiFi();
  initNTP();
}

void loop()
{
  static int times = 0;
  //Serial.println(times);

  if ((times == 0) && (WiFi.status() == WL_CONNECTED))
  {
    // 向心知天气服务器服务器请求信息并对信息进行解析
    httpRequest(reqRes);
  }

  updateShowTime();

  if (++times >= 6000)
  {
    times = 0;
  }
  delay(300);
}
//======================================================================

void initOLED()
{
  u8g2.begin();
  u8g2.enableUTF8Print();//用于汉字显示
  u8g2.setFont(u8g2_font_wqy14_t_gb2312);

  u8g2.clearBuffer();
  u8g2.setCursor(0, 14);
  u8g2.print("等待WiFi连接");
  u8g2.setCursor(0, 30);
  u8g2.print("WiFi connection...");
  u8g2.sendBuffer();
}

void initWiFi()
{
  Serial.print("Connecting WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void initNTP()
{
  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
  setSyncInterval(1000); //每300秒同步一次时间
}

/*-------- 网络天气 代码 ----------*/
// 向心知天气服务器服务器请求信息并对信息进行解析
void httpRequest(String reqRes)
{
  WiFiClient client;

  // 建立http请求信息
  String httpRequest = String("GET ") + reqRes + " HTTP/1.1\r\n" +
                       "Host: " + host + "\r\n" +
                       "Connection: close\r\n\r\n";
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.print(host);

  // 尝试连接服务器
  if (client.connect(host, 80))
  {
    Serial.println(" Success!");

    // 向服务器发送http请求信息
    client.print(httpRequest);
    Serial.println("Sending request: ");
    Serial.println(httpRequest);

    // 获取并显示服务器响应状态行
    String status_response = client.readStringUntil('\n');
    Serial.print("status_response: ");
    Serial.println(status_response);

    // 使用find跳过HTTP响应头
    if (client.find("\r\n\r\n"))
    {
      Serial.println("Found Header End. Start Parsing.");
    }

    // 利用ArduinoJson库解析心知天气响应信息
    parseInfo(client);
  }
  else
  {
    Serial.println(" connection failed!");
  }
  //断开客户端与服务器连接工作
  client.stop();
}

// 利用ArduinoJson库解析心知天气响应信息
void parseInfo(WiFiClient client)
{
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + 230;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, client);
  Serial.println(doc.as<String>());

  JsonObject result0 = doc["results"][0];
  JsonObject result0_now = result0["now"];

  // 通过串口监视器显示以上信息
  g_strWeather = result0_now["text"].as<String>();// "Sunny"
  g_iCode = result0_now["code"].as<int>();// "0"
  g_iTemperature = result0_now["temperature"].as<int>();// "32"
  g_strUpdateTime = result0["last_update"].as<String>();// "2020-06-02T14:40:00+08:00"

  Serial.println(F("======Weahter Now======="));
  Serial.print(F("Weather Now: "));
  Serial.print(g_strWeather);
  Serial.print(F(" -> "));
  Serial.println(g_iCode);
  Serial.print(F("Temperature: "));
  Serial.println(g_iTemperature);
  Serial.print(F("Last Update: "));
  Serial.println(g_strUpdateTime);
  Serial.println(F("========================"));

  for (int i=0; i< 4; i++)
  {
    if(g_iCode == weather_icon_table[i].XinZhi_code)
    {
      g_iIdx = weather_icon_table[i].icon_idx;
      break;
    }
  }
}

void my_strcat(char* srcstr, int t)
{
  char str_tmp[5];
  if(t < 10) 
  {
    strcat(srcstr, "0");
  }
  itoa(t, str_tmp, 10); 
  strcat(srcstr, str_tmp); 
}
void updateShowTime()
{
  rtc_time_t now_time;
  now_time.tm_year = year();
  now_time.tm_mon = month();
  now_time.tm_mday = day();
  now_time.tm_hour = hour();
  now_time.tm_min = minute();
  now_time.tm_sec = second();
  now_time.tm_week = weekday();
  
  weather_info_t weather_info;
  weather_info.weather = g_strWeather;
  weather_info.iconIdx = g_iIdx;
  weather_info.temp = g_iTemperature;
  
  testShowTimeAndWeather(now_time, weather_info);
}

void testShowTimeAndWeather(rtc_time_t &now_time, weather_info_t &weather_info)
{
  u8g2.clearBuffer();
  int tm_year = now_time.tm_year;
  int tm_month = now_time.tm_mon;
  int tm_day = now_time.tm_mday;
  int tm_hour = now_time.tm_hour;
  int tm_minute = now_time.tm_min;
  int tm_sec = now_time.tm_sec;
  int tm_week = now_time.tm_week;
  
  //时分
  char str_big_time[] = "";
  my_strcat(str_big_time, tm_hour);
  strcat(str_big_time,":");
  my_strcat(str_big_time, tm_minute);
  u8g2.setFont(u8g2_font_logisoso24_tf); 
  u8g2.drawStr(0, 30, str_big_time);

  //秒
  char str_small_sec[] = "";
  my_strcat(str_small_sec, tm_sec);
  u8g2.setFont(u8g2_font_wqy14_t_gb2312);
  u8g2.drawStr(73, 30, str_small_sec);

  //日期
  char str_date[] = "";
  char str_temp[6];
  itoa(tm_year,str_temp,10);
  strcat(str_date,str_temp);
  strcat(str_date,"-");
  my_strcat(str_date, tm_month);
  strcat(str_date,"-");
  my_strcat(str_date, tm_day);
  u8g2.drawStr(0, 47, str_date);
  
  u8g2.setCursor(0, 63);
  u8g2.print("星期");
  switch (tm_week)
  {
    case 1: u8g2.print("日"); break;
    case 2: u8g2.print("一"); break;
    case 3: u8g2.print("二"); break;
    case 4: u8g2.print("三"); break;
    case 5: u8g2.print("四"); break;
    case 6: u8g2.print("五"); break;
    case 7: u8g2.print("六"); break;
    default: break;
  }
  u8g2.setCursor(60, 63);
  u8g2.print("天津");

  //分割线
  u8g2.drawLine(90, 0, 90, 63);
  
  //天气
  if (weather_info.iconIdx<0 || weather_info.iconIdx>3) //没有对应的天气图标
  {
    Serial.print("no icon for weather: ");
    Serial.println(weather_info.weather);
  }
  else
  {
    u8g2.setFont(u8g2_font_open_iconic_weather_4x_t );
    u8g2.drawStr(96, 34, icon_index[0]);
  }
  char temperature_tmp[25];
  itoa(weather_info.temp, temperature_tmp, 10);
  strcat(temperature_tmp,"℃");
  u8g2.setFont(u8g2_font_wqy16_t_gb2312);
  u8g2.setCursor(96, 55);
  u8g2.print(temperature_tmp);
  
  u8g2.sendBuffer();
}


/*-------- NTP 代码 ----------*/
time_t getNtpTime()
{
    IPAddress ntpServerIP; // NTP服务器的地址
 
    while(Udp.parsePacket() > 0); // 丢弃以前接收的任何数据包
    Serial.println("Transmit NTP Request");
    // 从池中获取随机服务器
    WiFi.hostByName(ntpServerName, ntpServerIP);
    Serial.print(ntpServerName);
    Serial.print(": ");
    Serial.println(ntpServerIP);
    sendNTPpacket(ntpServerIP);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500)
    {
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE)
        {
            Serial.println("Receive NTP Response");
            isNTPConnected = true;
            Udp.read(packetBuffer, NTP_PACKET_SIZE); // 将数据包读取到缓冲区
            unsigned long secsSince1900;
            // 将从位置40开始的四个字节转换为长整型，只取前32位整数部分
            secsSince1900 = (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            Serial.println(secsSince1900);
            Serial.println(secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR);
            return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
        }
    }
    Serial.println("No NTP Response :-("); //无NTP响应
    isNTPConnected = false;
    return 0; //如果未得到时间则返回0
}
 
// 向给定地址的时间服务器发送NTP请求
void sendNTPpacket(IPAddress &address)
{
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    packetBuffer[0] = 0b11100011; // LI, Version, Mode
    packetBuffer[1] = 0;          // Stratum, or type of clock
    packetBuffer[2] = 6;          // Polling Interval
    packetBuffer[3] = 0xEC;       // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;
    Udp.beginPacket(address, 123); //NTP需要使用的UDP端口号为123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}
