#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <time.h>

// ตั้งค่า WiFi
const char *ssid = "POCO F5";
const char *password = "password";

// ตั้งค่า NETPIE 2020
const char *mqtt_server = "broker.netpie.io";
const char *mqtt_Client = "514a960c-9265-464f-84ed-d64abe7d1e1f"; // เอามาจากหน้า Device ใน NETPIE
const char *mqtt_username = "TfPwVh4ghYh2wVXgeHFfyPeJ2zGyZVfT";   // เอามาจากหน้า Device ใน NETPIE
const char *mqtt_password = "9VjLWrNwLyAsyhyxhgReS4d2sAhfVApW";   // เอามาจากหน้า Device ใน NETPIE

const int MAX_TIMES = 10;    // กำหนดจำนวนเวลาที่รับได้สูงสุด (เปลี่ยนได้)
String timeArray[MAX_TIMES]; // Array เก็บข้อความเวลา เช่น "16:00", "18:30"
int timeCount = 0;           // ตัวนับว่าตอนนี้มีเวลากี่ค่าใน Array

int lastMinute = -1;      // ใช้จำว่านาทีล่าสุดคือนาทีอะไร
bool isTriggered = false; // ป้องกันการทำงานซ้ำใน 1 นาทีเดิม

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

int LED_red = 14;
int LED_green = 12;
int buzzer = 5;
int IR_sensor = 17;
int button = 16;

void setup_wifi()
{
    delay(10);
    // เริ่มต้นเชื่อมต่อ WiFi
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
    // แปลง payload เป็น String
    String message = "";
    for (int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    // เช็ค Topic
    if (String(topic) == "@msg/settime")
    {

        if (message == "reset")
        {
            timeCount = 0; // ล้างค่าใน Array โดยการรีเซ็ตตัวนับกลับไปเป็น 0
            Serial.println("ล้างข้อมูลเวลาทั้งหมดแล้ว (Reset)");
        }
        else
        {
            // ถ้าไม่ใช่ reset ให้ถือว่าเป็นเวลา และนำไปเก็บใน Array
            if (timeCount < MAX_TIMES)
            {
                timeArray[timeCount] = message; // เก็บเวลาลง Array
                timeCount++;                    // ขยับตัวนับ

                Serial.print("บันทึกเวลาแล้ว: ");
                Serial.print(message);
                Serial.print(" (จำนวนทั้งหมด: ");
                Serial.print(timeCount);
                Serial.println(")");
            }
            else
            {
                Serial.println("Array เต็ม! ไม่สามารถเพิ่มเวลาได้");
            }
        }
    }
}

void reconnect()
{
    // วนลูปจนกว่าจะเชื่อมต่อ MQTT สำเร็จ
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");

        // เชื่อมต่อด้วย Client ID, Token (Username), Secret (Password)
        if (client.connect(mqtt_Client, mqtt_username, mqtt_password))
        {
            Serial.println("connected!");
            // ส่งข้อความทักทายเมื่อเชื่อมต่อสำเร็จ (ใช้ @msg/ นำหน้า topic เสมอ)
            client.publish("@msg/outTopic", "hello world");
            // ติดตาม (Subscribe) เพื่อรับข้อความ
            client.subscribe("@msg/inTopic");
            client.subscribe("@msg/settime");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // รอ 5 วินาทีก่อนลองใหม่
            delay(5000);
        }
    }
}

void setup()
{
    pinMode(IR_sensor, INPUT);
    pinMode(button, INPUT);

    pinMode(LED_red, OUTPUT);
    pinMode(LED_green, OUTPUT);
    pinMode(buzzer, OUTPUT);

    // ปิดอุปกรณ์ทั้งหมดตอนเริ่มต้น
    digitalWrite(LED_red, LOW);
    digitalWrite(LED_green, LOW);
    digitalWrite(buzzer, LOW);

    Serial.begin(9600);

    setup_wifi();

    configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("Waiting for NTP time sync...");

    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    // 1. ดึงเวลาปัจจุบัน
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    // เช็คว่าเวลาซิงค์สำเร็จหรือยัง (ถ้าปีมากกว่า 2000 แสดงว่าดึงเวลาสำเร็จ)
    if (timeinfo->tm_year > (2000 - 1900))
    {

        // 2. แปลงเวลาปัจจุบันให้อยู่ในรูปแบบ "HH:MM"
        char timeStringBuff[6];
        strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M", timeinfo);
        String currentTime = String(timeStringBuff);

        // 3. รีเซ็ตตัวป้องกันการทำงานซ้ำเมื่อเปลี่ยนนาทีใหม่
        if (timeinfo->tm_min != lastMinute)
        {
            isTriggered = false;
            lastMinute = timeinfo->tm_min;
        }

        // 4. วนลูปเทียบเวลากับใน Array (ถ้ายังไม่เคยทำงานในนาทีนี้)
        if (!isTriggered)
        {

            for (int i = 0; i < timeCount; i++)
            {
                if (currentTime == timeArray[i])
                {
                    // ---- เมื่อถึงเวลาที่กำหนด ----
                    Serial.println("เวลาตรงกันแล้ว! ระบบกำลังทำงาน...");
                    client.publish("@msg/status", 0);
                    digitalWrite(LED_red, HIGH);    // เปิด LED สีแดง
                    digitalWrite(LED_green, LOW);   // ปิด LED สีเขียว
                    digitalWrite(buzzer, LOW);     // เปิด Buzzer (อิงตาม Flowchart)

                    isTriggered = true; // ล็อคไว้ไม่ให้ทำงานซ้ำ
                    break;              
                }
            }
        }
    }

    // 5. ถ้าระบบอยู่ในสถานะแจ้งเตือน (isTriggered == true) ให้รอรับการกดปุ่มและเซนเซอร์
    if (isTriggered == true)
    {
        // เช็คว่ากดปุ่มและบังเซนเซอร์พร้อมกันหรือไม่ (สมมติว่าสถานะ LOW คือตรวจจับได้/ถูกกด)
        if (digitalRead(button) == LOW && digitalRead(IR_sensor) == LOW)
        {
            // ---- เมื่อเงื่อนไขหยุดสำเร็จ ----
            digitalWrite(LED_red, LOW);      // ปิด LED สีแดง
            digitalWrite(buzzer, HIGH);       // ปิด Buzzer
            digitalWrite(LED_green, HIGH);   // เปิด LED สีเขียว
            
            Serial.println("หยุดการแจ้งเตือนสำเร็จ (Done)");
            
            // ส่งข้อมูลไปยัง NETPIE Dashboard
            client.publish("@msg/status", "1", true);
            
            isTriggered = false; // ปลดล็อคสถานะ เพื่อรอรอบถัดไป
        }
    }
}