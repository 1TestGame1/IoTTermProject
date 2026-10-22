#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// ตั้งค่า WiFi
const char *ssid = "POCO F5";
const char *password = "password";

// ตั้งค่า NETPIE 2020
const char *mqtt_server = "broker.netpie.io";
const char *mqtt_Client = "514a960c-9265-464f-84ed-d64abe7d1e1f"; // เอามาจากหน้า Device ใน NETPIE
const char *mqtt_username = "TfPwVh4ghYh2wVXgeHFfyPeJ2zGyZVfT";   // เอามาจากหน้า Device ใน NETPIE
const char *mqtt_password = "9VjLWrNwLyAsyhyxhgReS4d2sAhfVApW";   // เอามาจากหน้า Device ใน NETPIE

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

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
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // ถ้าข้อความที่เข้ามาเป็น '1' ให้เปิดไฟ LED (LOW คือเปิดสำหรับไฟบนบอร์ด NodeMCU)
    if ((char)payload[0] == '1')
    {
        digitalWrite(14, HIGH);
    }
    else
    {
        digitalWrite(14, LOW);
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
    pinMode(14, OUTPUT);
    Serial.begin(9600);

    setup_wifi();

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

    unsigned long now = millis();
    // ส่งข้อมูลทุกๆ 2 วินาที (2000 ms)
    if (now - lastMsg > 2000)
    {
        lastMsg = now;
        ++value;
        snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);

        Serial.print("Publish message: ");
        Serial.println(msg);

        // ส่งข้อความไปที่ Topic @msg/outTopic
        client.publish("@msg/outTopic", msg);
    }
}