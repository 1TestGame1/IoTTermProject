#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <time.h>

const char *ssid = "wifi";
const char *password = "password";

const char *mqtt_server = "broker.netpie.io";
const char *mqtt_Client = "514a960c-9265-464f-84ed-d64abe7d1e1f";
const char *mqtt_username = "TfPwVh4ghYh2wVXgeHFfyPeJ2zGyZVfT";
const char *mqtt_password = "9VjLWrNwLyAsyhyxhgReS4d2sAhfVApW";

const int MAX_TIMES = 10;
String timeArray[MAX_TIMES];
int timeCount = 0;

int lastMinute = -1;
bool isTriggered = false;
bool isAlarming = false;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

int LED_red = 14;
int LED_green = 12;
int buzzer = 4;
int IR_sensor = 17;
int button = 5;

void setup_wifi()
{
    delay(10);

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

    String message = "";
    for (int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    if (String(topic) == "@msg/settime")
    {

        if (message == "reset")
        {
            timeCount = 0;
            Serial.println("ล้างข้อมูลเวลาทั้งหมดแล้ว (Reset)");
        }
        else
        {
            if (timeCount < MAX_TIMES)
            {
                timeArray[timeCount] = message;
                timeCount++;

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
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");

        if (client.connect(mqtt_Client, mqtt_username, mqtt_password))
        {
            Serial.println("connected!");
            client.publish("@msg/outTopic", "hello world");

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
    pinMode(button, INPUT_PULLUP);

    pinMode(LED_red, OUTPUT);
    pinMode(LED_green, OUTPUT);
    pinMode(buzzer, OUTPUT);

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

    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    if (timeinfo->tm_year > (2000 - 1900))
    {
        char timeStringBuff[6];
        strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M", timeinfo);
        String currentTime = String(timeStringBuff);

        if (timeinfo->tm_min != lastMinute)
        {
            isTriggered = false;
            lastMinute = timeinfo->tm_min;
        }

        {
            for (int i = 0; i < timeCount; i++)
            {
                if (currentTime == timeArray[i])
                {
                    Serial.println("ถึงเวลาทานยาแล้ว ระบบแจ้งเตือนกำลังทำงาน");
                    client.publish("@msg/status", "0");
                    digitalWrite(LED_red, HIGH);
                    digitalWrite(LED_green, LOW);
                    tone(buzzer, 1000);

                    isTriggered = true;
                    isAlarming = true;
                    break;
                }
            }
        }
    }

    if (isAlarming == true)
    {
        if (digitalRead(button) == LOW && analogRead(ir_sensor))
        {
            digitalWrite(LED_red, LOW);
            noTone(buzzer);
            digitalWrite(LED_green, HIGH);

            Serial.println("ผู้ป่วยรับยาแล้ว");
            client.publish("@msg/status", "1");

            isAlarming = false;
        }
    }
}