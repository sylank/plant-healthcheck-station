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

#define SENSOR_ACTIVATE_PIN PIN_A6

#define FIVE_MINUTES 300000

unsigned long lastSensorReadTime = millis();

SoftwareSerial serial(RX, TX);
Wifi wifi(serial);
dht DHT;

EEPROMStore<PersistentState> persistentState;

bool calculatedSend = true;

void sensorsTurnOn()
{
  digitalWrite(SENSOR_ACTIVATE_PIN, HIGH);

  delay(1000);
}

void sensorsTurnOff()
{
  digitalWrite(SENSOR_ACTIVATE_PIN, LOW);
}

void sendDataOnSerial(const unsigned int &soilMoisturePercent, const float &hum, const float &temp)
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

  // #1!0!0!1 --> calculated
  // #1!0!0!0 --> raw
  if (message.indexOf("#1") == 0)
  {
    String airValue = getMessageElement(message, '!', 1);
    String waterValue = getMessageElement(message, '!', 2);
    String modeValue = getMessageElement(message, '!', 3);

    calculatedSend = modeValue == "1";
    if (calculatedSend)
    {
      persistentState.Data.airValue = atoi(airValue.c_str());
      persistentState.Data.waterValue = atoi(waterValue.c_str());
    }

    persistentState.Save();
  }
}

void setup()
{
  pinMode(RX, INPUT);
  pinMode(TX, OUTPUT);

  pinMode(SENSOR_ACTIVATE_PIN, OUTPUT);
  digitalWrite(SENSOR_ACTIVATE_PIN, LOW);

  persistentState.Load();

  serial.begin(19200);
}

void loop()
{
  if (((millis() - lastSensorReadTime) > FIVE_MINUTES) || !calculatedSend)
  {
    lastSensorReadTime = millis();
    sensorsTurnOn();

    unsigned int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);

    DHT.read22(DHT_PIN);

    sensorsTurnOff();

    if (calculatedSend)
    {
      unsigned int soilMoisturePercent = map(soilMoistureValue,
                                             persistentState.Data.airValue,
                                             persistentState.Data.waterValue,
                                             0,
                                             100);

      if (soilMoisturePercent < 0)
      {
        soilMoisturePercent = 0;
      }

      if (soilMoisturePercent > 100)
      {
        soilMoisturePercent = 100;
      }

      sendDataOnSerial(soilMoisturePercent, DHT.humidity, DHT.temperature);
    }
    else
    {
      sendDataOnSerial(soilMoistureValue, DHT.humidity, DHT.temperature);
    }
  }

  processIncommingWiFiData();
}
