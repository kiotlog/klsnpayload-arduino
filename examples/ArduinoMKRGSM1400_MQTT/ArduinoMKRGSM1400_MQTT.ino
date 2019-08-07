#include "Clock.h"
#include "Secrets.h"


/***** BME280 *****/
#include <Adafruit_BME280.h>
Adafruit_BME280 bme;

/***** TinyGSM *****/
// Select your modem:
#define TINY_GSM_MODEM_UBLOX

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial
#define SerialAT SerialGSM

//#define DUMP_AT_COMMANDS
//#define TINY_GSM_DEBUG SerialMon

// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "tm";
const char gprsUser[] = "";
const char gprsPass[] = "";

#include <TinyGsmClient.h>
#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif
TinyGsmClient client(modem);

/***** MQTT *****/
#include <MQTT.h>
MQTTClient mqtt;
uint8_t mqttPayload[256];
size_t mqttPayloadLen;
// MQTT details
const char* broker = "hetzner-01.trmpln.com";
const char* topic = "/klsnts/trmpln/nu";
const char* topicInit = "/ctrl/init";

/***** Klsn *****/
#include <KlsnPayload.h>
KlsnPayload klsnPayload;

const uint8_t * key = KLSN_KEY;

// Use a packed struct to collect and send data
typedef struct __attribute__ ((packed)) data_struct {
  int16_t temperature;
  uint16_t pressure;
  uint16_t humidity;
  uint16_t battery;
} Data;
Data data;

/***** RTCZero *****/
#include <RTCZero.h>
RTCZero rtc;
uint32_t timestamp;

const unsigned int interval = 10 * 1000;
unsigned int now = millis();

/***** Battery *****/
#define ADC_RESOLUTION 12

void setup() {

  SerialMon.begin(115200);
  while (!SerialMon);

  SerialMon.println("Hello, Kiotlog!");

  // Setup battery stuff
  pinMode(ADC_BATTERY, INPUT);
  analogReadResolution(ADC_RESOLUTION);

  // Power-up UBlox
  powerUpUBlox();

  // Set clock
  rtc.begin();
  setCompileDateTime(-2);
  SerialMon.print("Clock set to: "); SerialMon.println(rtc.getEpoch());

  // Init BME
  bme_init();

  // Init Klsn
  klsnPayload.begin(key);

  SerialMon.println("Wait...");

  // Set GSM module baud rate
  SerialAT.begin(115200);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  //  modem.restart();
  modem.init();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem: ");
  SerialMon.println(modemInfo);

  // Setup MQTT
  mqtt.begin(broker, client);

  connect();
}

void connect() {

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }

  if (!modem.isGprsConnected()) {
    SerialMon.print(F("Connecting to APN: "));
    SerialMon.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      SerialMon.println(" fail");
      delay(10000);
      return;
    }
    SerialMon.println(" success");
  }

  SerialMon.print("Connecting to Broker: ");
  SerialMon.print(broker);

  while (!mqtt.connect("Kiotlg MQTT Nu", "", "")) {
    SerialMon.print(".");
    delay(1000);
  }
  SerialMon.println(" success");
  mqtt.publish(topicInit, "Kiotlg MQTT Nu started");

}

void loop() {

  mqtt.loop();

  if (!mqtt.connected()) {
    SerialMon.println("=== MQTT NOT CONNECTED ===");
    connect();
  }

  if (millis() - now > interval) {

    SerialMon.print("Sending data: ");

    collectMeasures();

    klsnPayload.create(
      (const uint8_t *)&data, sizeof(data),
      timestamp,
      mqttPayload, &mqttPayloadLen);

    const uint8_t* p = (const uint8_t *)&mqttPayload;
    Serial.print("0x");
    for (int i = 0; i < mqttPayloadLen; i++) {
      if (p[i] < 16) Serial.print(0);
      Serial.print(p[i], HEX);
    }
    Serial.print(" ");
    SerialMon.print(mqttPayloadLen);
    SerialMon.println(" bytes.");

    mqtt.publish(topic, (const char*)&mqttPayload, mqttPayloadLen);
    now = millis();
  }
}

void collectMeasures()
{

  data.battery = readBattery() * 1000;

  bme.takeForcedMeasurement();
  data.temperature = bme.readTemperature() * 100;
  data.pressure = bme.readPressure() / 10;
  data.humidity = bme.readHumidity() * 100;
  timestamp = rtc.getEpoch();

}

void bme_init()
{
  bool status;

  status = bme.begin();
  if (!status) {
    SerialUSB.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, // temperature
                  Adafruit_BME280::SAMPLING_X1, // pressure
                  Adafruit_BME280::SAMPLING_X1, // humidity
                  Adafruit_BME280::FILTER_OFF);
}

float readBattery()
{
  const float maxADC = (1 << ADC_RESOLUTION) - 1;
  const float maxVoltage = 4.2;

  float battery_volts = (analogRead(ADC_BATTERY) * maxVoltage) / maxADC;

  return battery_volts;
}

void powerUpUBlox()
{
  pinMode(GSM_RESETN, OUTPUT);
  digitalWrite(GSM_RESETN, HIGH);
  delay(100);
  digitalWrite(GSM_RESETN, LOW);
}
