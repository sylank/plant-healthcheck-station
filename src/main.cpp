#include <Arduino.h>
#include <SoftwareSerial.h>
#include <dht.h> // https://github.com/RobTillaart/Arduino/blob/master/libraries/DHTlib/examples/dht22_test/dht22_test.ino

#include "Wifi.h"
#include "EEPROMStore.h"
#include "persistent_state.h"

#define DHTTYPE DHT22 // DHT 22  (AM2302)

#define SOIL_MOISTURE_PIN A7
#define DHT_PIN PIN_B2
#define RX PIN_B0
#define TX PIN_B1

#define PIN_LED_R PIN_A0
#define PIN_LED_G PIN_A1
#define PIN_LED_B PIN_A2

#define SENSOR_ACTIVATE_PIN PIN_A6

#define TWENTY_SECS 20000

const byte COLOR_RED = 0b100;
const byte COLOR_YELLOW = 0b110;
const byte COLOR_GREEN = 0b010;
const byte COLOR_WHITE = 0b111;

unsigned long lastSensorReadTime = millis();

SoftwareSerial serial(RX, TX);
Wifi wifi(serial);
dht DHT;

EEPROMStore<PersistentState> persistentState;

byte soilMoistureColor = COLOR_WHITE;

void displayColor(const byte &color)
{
  digitalWrite(PIN_LED_R, !bitRead(color, 2));
  digitalWrite(PIN_LED_G, !bitRead(color, 1));
  digitalWrite(PIN_LED_B, !bitRead(color, 0));
}

void sensorsTurnOn()
{
  digitalWrite(SENSOR_ACTIVATE_PIN, HIGH);
}

void sensorsTurnOff()
{
  digitalWrite(SENSOR_ACTIVATE_PIN, LOW);
}

byte determineSoilMoistureColor(const unsigned int &percent)
{
  if (percent < 30)
  {
    return COLOR_RED;
  }

  if (percent >= 30 && percent < 60)
  {
    return COLOR_YELLOW;
  }

  if (percent >= 60)
  {
    return COLOR_GREEN;
  }

  return COLOR_WHITE;
}

void sendCalculatedDataOnSerial(const unsigned int &soilMoisturePercent, const float &hum, const float &temp)
{
  wifi.sendDataOnSerial(soilMoisturePercent, hum, temp);
}

// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String getMessageElement(const String &data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void processIncommingWiFiData()
{
  String message = wifi.readDataFromWiFiModule();

  if (message.indexOf("#1") == 0)
  {
    String airValue = getMessageElement(message, '!', 1);
    String waterValue = getMessageElement(message, '!', 2);
    String modeValue = getMessageElement(message, '!', 3);

    persistentState.Data.airValue = atoi(airValue.c_str());
    persistentState.Data.waterValue = atoi(waterValue.c_str());
    persistentState.Data.calculatedSend = modeValue == "1";
    persistentState.Save();
  }
}

void setup()
{
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);

  displayColor(COLOR_WHITE);

  pinMode(RX, INPUT);
  pinMode(TX, OUTPUT);

  pinMode(SENSOR_ACTIVATE_PIN, OUTPUT);
  digitalWrite(SENSOR_ACTIVATE_PIN, LOW);

  persistentState.Load();

  serial.begin(19200);
}

void loop()
{
  if ((millis() - lastSensorReadTime) > TWENTY_SECS)
  {
    sensorsTurnOn();

    unsigned int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
    unsigned int soilMoisturePercent = map(soilMoistureValue,
                                           persistentState.Data.airValue,
                                           persistentState.Data.waterValue,
                                           0,
                                           100);

    DHT.read22(DHT_PIN);

    sensorsTurnOff();

    soilMoistureColor = determineSoilMoistureColor(soilMoisturePercent);

    if (persistentState.Data.calculatedSend)
    {
      sendCalculatedDataOnSerial(soilMoisturePercent, DHT.humidity, DHT.temperature);
    }
    else
    {
      sendCalculatedDataOnSerial(soilMoistureValue, DHT.humidity, DHT.temperature);
    }
  }

  displayColor(soilMoistureColor);
  processIncommingWiFiData();
}
