#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <WiFiUdp.h>

#define LED 2

const char *ssid = "";
const char *password = "";
int wifi_flag = 0;                         // 配网标志为
const char *AP_NAME = "ESP-01S WIFI 配网"; // wifi名字
// 暂时存储wifi账号密码
char sta_ssid[32] = {0};
char sta_password[64] = {0};
// 配网页面代码 //style='left: 50px; position: relative;'      style="color:blue;text-align:center"
//  style='position: relative; padding-top: 10rem;'

const char *END_String = "&";
struct config_type
{
    char sta_ssid[32];     // 定义得到的主题(最大32字节)
    char sta_password[64]; // 定义配网得到的UID(最大64字节)
};

config_type config; // 声明定义内容

void saveConfig() // 保存函数
{
    EEPROM.begin(1024); // 向系统申请1024kb ROM
    // 开始写入
    uint8_t *p = (uint8_t *)(&config);
    for (int i = 0; i < sizeof(config); i++)
    {
        EEPROM.write(i, *(p + i)); // 在闪存内模拟写入
    }
    EEPROM.commit(); // 执行写入ROM
}

void loadConfig() // 读取函数
{
    EEPROM.begin(1024);
    uint8_t *p = (uint8_t *)(&config);
    for (int i = 0; i < sizeof(config); i++)
    {
        *(p + i) = EEPROM.read(i);
    }
    EEPROM.commit();
    password = config.sta_password;
    ssid = config.sta_ssid;
}

const char *page_html = "\
<!DOCTYPE html>\r\n\
<html lang='en'>\r\n\
<head>\r\n\
  <meta charset='UTF-8'>\r\n\
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>\r\n\
  <title>ESP Web Server</title>\r\n\
</head>\r\n\
<body align='center' style='background-color:#F5FFFA;color:blue;text-align:center;font-size:21px;'  >\r\n\
  <form align='center'  name='input' action='/' method='POST'>\r\n\   
        wifi名称: <br>\r\n\    
        <input style='width:200px;height=25px;font-size:20px'  contenteditable='true' type='text' name='ssid'><br>\r\n\
        wifi密码:<br>\r\n\
        <input style='width:200px;height=25px;font-size:20px' type='password' name='password' ><br>\r\n\
        <input style='width:100px;height=25px;font-size:20px' type='submit' value='保存'>\r\n\
    </form>\r\n\
</body>\r\n\
</html>\r\n\
";
// 配网成功页面
const char *page_htm2 = "\
<!DOCTYPE html>\r\n\
<html lang='en'>\r\n\
<head>\r\n\
  <meta charset='UTF-8'>\r\n\
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>\r\n\
  <title>ESP Web Server</title>\r\n\
</head>\r\n\
<body align='center' style='background-color:#F5FFFA;color:blue;text-align:center;font-size:21px;'  >\r\n\
 配网已完成\r\n\
</body>\r\n\
</html>\r\n\
";
const byte DNS_PORT = 53;       // DNS端口号
IPAddress apIP(192, 168, 4, 1); // esp8266-AP-IP地址
DNSServer dnsServer;            // 创建dnsServer实例
ESP8266WebServer server(80);    // 创建WebServer

WiFiUDP Udp;
unsigned int localPort = 8888; // local port to listen on
// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1]; // buffer to hold incoming packet,
char ReplyBuffer[] = "success\r\n";            // a string to send back

void handleRoot()
{ // 访问主页回调函数
    server.send(200, "text/html", page_html);
}

void handleRootPost()
{ // Post回调函数
    Serial.println("handleRootPost &");
    if (server.hasArg("ssid"))
    { // 判断是否有账号参数
        Serial.println("get ssid: &");
        strcpy(sta_ssid, server.arg("ssid").c_str()); // 将账号参数拷贝到sta_ssid中
        Serial.println(sta_ssid);
        strcpy(config.sta_ssid, server.arg("ssid").c_str()); // 复制ssid
        Serial.println(END_String);
    }
    else
    { // 没有参数
        Serial.println("error, not found ssid &");
        server.send(200, "text/html", "<meta charset='UTF-8'>error, not found ssid"); // 返回错误页面
        return;
    }
    // 密码与账号同理
    if (server.hasArg("password"))
    {
        Serial.println("get password: & ");
        strcpy(sta_password, server.arg("password").c_str());
        strcpy(config.sta_password, server.arg("password").c_str()); // 复制password
        Serial.println(sta_password);
        Serial.println(END_String);
    }
    else
    {
        Serial.println("error, not found password");
        Serial.println(END_String);
        server.send(200, "text/html", "<meta charset='UTF-8'>error, not found password");
        return;
    }

    server.send(200, "text/html", page_htm2); // 返回保存成功页面
    delay(500);
    ssid = sta_ssid;
    password = sta_password;

    saveConfig(); // 调用保存函数
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    delay(5000);
    // 连接wifi
    connectNewWifi();
}

void initSoftAP(void)
{ // 初始化AP模式
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    if (WiFi.softAP(AP_NAME))
    {
        Serial.write("ESP8266 SoftAP is right");
        Serial.write(END_String);
    }
}

void initWebServer(void)
{ // 初始化WebServer
    // server.on("/",handleRoot);
    // 上面那行必须以下面这种格式去写否则无法强制门户
    server.on("/", HTTP_GET, handleRoot);      // 设置主页回调函数
    server.onNotFound(handleRoot);             // 设置无法响应的http请求的回调函数
    server.on("/", HTTP_POST, handleRootPost); // 设置Post请求回调函数
    server.begin();                            // 启动WebServer
    Serial.write("WebServer started!");
    Serial.write(END_String);
}

void initDNS(void)
{ // 初始化DNS服务器
    if (dnsServer.start(DNS_PORT, "*", apIP))
    { // 判断将所有地址映射到esp8266的ip上是否成功
        Serial.println("start dnsserver success. &");
    }
    else
        Serial.write("start dnsserver failed. &");
}

void connectNewWifi(void)
{
    WiFi.hostname("Smart-ESP-01s"); // 设置ESP8266设备名
    WiFi.mode(WIFI_STA);            // 切换为STA模式
    // WiFi.setAutoConnect(true);      // 设置自动连接
    WiFi.begin(ssid, password); // 连接上一次连接成功的wifi
    Serial.println("start Connect to wifi &");
    int count = 0;

    while (WiFi.status() != WL_CONNECTED)
    {
        digitalWrite(LED, LOW);
        delay(1000);
        digitalWrite(LED, HIGH);
        delay(1000);
        digitalWrite(LED, LOW);
        count++;
        if (count > 4)
        { // 如果8秒内没有连上，就开启Web配网 可适当调整这个时间
            initSoftAP();
            initWebServer();
            initDNS();
            break; // 跳出 防止无限初始化
        }
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED)
    { // 如果连接上 就输出IP信息 防止未连接上break后会误输出
        Serial.println("WIFI Connected!");
        wifi_flag = 1; // 标志为1
        Serial.println("IP address:  &");
        Serial.println(WiFi.localIP()); // 打印esp8266的IP地址
        Serial.println("Wifi connect success &");
        server.stop();
        digitalWrite(LED, LOW);
        Udp.begin(localPort);
    }
}

void setup()
{
    Serial.begin(9600);
    delay(500);
    loadConfig(); // 读取信息
    Serial.println("ssid :   &");
    Serial.println(ssid);
    Serial.println(END_String);
    Serial.println("password: &");
    Serial.println(password);
    Serial.println(END_String);
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    connectNewWifi();
}

void loop()
{
    if (wifi_flag == 0)
    {
        digitalWrite(LED, LOW);
        delay(150);
        digitalWrite(LED, HIGH);
        delay(150);
        digitalWrite(LED, LOW);
        server.handleClient();
        dnsServer.processNextRequest();
    }
    else if (wifi_flag == 1)
    {
        // if there's data available, read a packet
        int packetSize = Udp.parsePacket();
        if (packetSize)
        {
            Serial.printf("Received packet of size %d from %s:%d\n    (to %s:%d, free heap = %d B) &", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort(), Udp.destinationIP().toString().c_str(), Udp.localPort(), ESP.getFreeHeap());

            // read the packet into packetBufffer
            int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
            packetBuffer[n] = 0;
            Serial.write("Contents:");
            Serial.write(END_String);
            Serial.write(packetBuffer);
            String commend;
            commend += packetBuffer;
            if (commend.indexOf("ClearConfig") != -1)
            {
                strcpy(config.sta_password, "");
                strcpy(config.sta_ssid, "");    
                saveConfig();
            }

            // Serial.write(END_String);
            // send a reply, to the IP address and port that sent us the packet we received
            Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
            Udp.write(ReplyBuffer);
            Udp.endPacket();
        }
    }
}
