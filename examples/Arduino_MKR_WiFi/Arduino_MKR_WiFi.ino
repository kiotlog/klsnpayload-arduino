/*
  ArduinoMqttClient - WiFi Simple Sender

  This example connects to a MQTT broker and publishes a message to
  a topic once a second.

  The circuit:
  - Arduino MKR 1000, MKR 1010 or Uno WiFi Rev.2 board

  This example code is in the public domain.
*/

#include <ArduinoMqttClient.h>
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
#include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
#include <WiFi101.h>
#elif defined(ARDUINO_ESP8266_ESP12)
#include <ESP8266WiFi.h>
#endif

/***** RTCZero *****/
#include <RTCZero.h>
RTCZero rtc;
uint32_t timestamp;

#include <KlsnPayload.h>
KlsnPayload klsnPayload;

#include "Clock.h"
#include "Secrets.h"

const uint8_t* key = KLSN_KEY;

typedef struct __attribute__((packed)) data_struct {
    int16_t temperature;
    uint16_t pressure;
    uint16_t humidity;
    uint16_t battery;
} Data;
Data data;

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password (use for WPA, or use as key for WEP)

// To connect with SSL/TLS:
// 1) Change WiFiClient to WiFiSSLClient.
// 2) Change port value from 1883 to 8883.
// 3) Change broker value to a server with a known SSL/TLS root certificate
//    flashed in the WiFi module.

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "192.168.1.110";
int port = 1883;
const char topic[] = "/klsnts/trmpln/rp2040-connect";

const long interval = 1000;
unsigned long previousMillis = 0;

int count = 0;

void setup()
{
    //Initialize serial and wait for port to open:
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    // Set clock
    rtc.begin();
    setCompileDateTime(-2);
    Serial.print("Clock set to: ");
    Serial.println(rtc.getEpoch());

    // attempt to connect to Wifi network:
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
        // failed, retry
        Serial.print(".");
        delay(5000);
    }

    Serial.println("You're connected to the network");
    Serial.println();

    // You can provide a unique client ID, if not set the library uses Arduino-millis()
    // Each client must have a unique client ID
    // mqttClient.setId("clientId");

    // You can provide a username and password for authentication
    // mqttClient.setUsernamePassword("username", "password");

    Serial.print("Attempting to connect to the MQTT broker: ");
    Serial.println(broker);

    if (!mqttClient.connect(broker, port)) {
        Serial.print("MQTT connection failed! Error code = ");
        Serial.println(mqttClient.connectError());

        while (1)
            ;
    }

    Serial.println("You're connected to the MQTT broker!");
    Serial.println();

    // Init Klsn
    // klsnPayload.begin(key, 8);
    klsnPayload.begin(key);
    for (int i = 0; i < 32; i++) {
        if (key[i] < 16)
            Serial.print(0);
        Serial.print(key[i], HEX);
    }
    Serial.println();
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
    }

    if (!mqttClient.connected()) {
        // MQTT client is disconnected, connect
        connectMQTT();
    }
    // call poll() regularly to allow the library to send MQTT keep alives which
    // avoids being disconnected by the broker
    mqttClient.poll();

    // avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
    // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        // save the last time a message was sent
        previousMillis = currentMillis;

        Serial.print("Sending message to topic: ");
        Serial.println(topic);

        size_t mqttPayloadLen;
        uint8_t mqttPayload[256] {};

        Serial.print("Creating packet...");
        Serial.flush();

        collectMeasures();

        klsnPayload.create(
            (const uint8_t*)&data, sizeof(data),
            timestamp,
            mqttPayload, &mqttPayloadLen);
        Serial.println(" done.");

        const uint8_t* p = (const uint8_t*)&mqttPayload;
        Serial.print("0x");
        for (int i = 0; i < mqttPayloadLen; i++) {
            if (p[i] < 16)
                Serial.print(0);
            Serial.print(p[i], HEX);
        }
        Serial.print(" ");
        Serial.print(mqttPayloadLen);
        Serial.println(" bytes.");

        // send message, the Print interface can be used to set the message contents
        mqttClient.beginMessage(topic);
        mqttClient.write(mqttPayload, mqttPayloadLen);
        mqttClient.endMessage();

        Serial.println();

        count++;
    }
}

void collectMeasures()
{

    data.battery = 3.72 * 1000;

    data.temperature = 27.8 * 100;
    data.pressure = 1012 / 10;
    data.humidity = 50.23 * 100;
    timestamp = rtc.getEpoch();
}

void connectWiFi()
{
    Serial.print("Attempting to connect to SSID: ");
    Serial.print(ssid);
    Serial.print(" ");

    while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
        // failed, retry
        Serial.print(".");
        delay(5000);
    }
    Serial.println();

    Serial.println("You're connected to the network");
    Serial.println();
}

void connectMQTT()
{
    Serial.print("Attempting to MQTT broker: ");
    Serial.print(broker);
    Serial.println(" ");

    while (!mqttClient.connect(broker, port)) {
        // failed, retry
        Serial.print(".");
        delay(5000);
    }
    Serial.println();

    Serial.println("You're connected to the MQTT broker");
    Serial.println();
}
