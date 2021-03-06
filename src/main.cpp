// MEMORY_PRINT_START
// MEMORY_PRINT_HEAPSTART
// MEMORY_PRINT_HEAPEND
// MEMORY_PRINT_STACKSTART
// MEMORY_PRINT_END
// MEMORY_PRINT_HEAPSIZE
// FREERAM_PRINT;
#include <Wire.h>
#include <MemoryUsage.h>

#include <DHT.h>

#ifndef WIFI_H
#define WIFI_H
#include <SoftwareSerial.h>
#endif

#include "Wifi.h"

#include "define_sensors.h"

#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

#include "EEPROMStore.h"
#include "persistent_state.h"

#include "Filter.h"

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino
// https://github.com/greiman/SSD1306Ascii
SSD1306AsciiAvrI2c oled;

SoftwareSerial aserial(SOFT_S_RX, SOFT_S_TX); // RX | TX
Wifi wifi(aserial);

EEPROMStore<PersistentState> persistentState;

ExponentialFilter<long> ADCFilter(5, 0);

String stationId = "1";

int screenStatus = 0;

unsigned long lastMeasurementTime = millis();
unsigned long lastConfigDisplayTime = millis();
unsigned long lastSensorReadTime = millis();

bool pkgStatus = false;
String transferStatusCode = "0";
bool connected = false;
String ipAddress = "0.0.0.0";

bool screenOff = false;

int soilMoistureValue = 0;
int soilMoisturePercent = 0;
float humidity = 0;
float temperature = 0;

bool resetBtnPrestate = false;
bool screenBtnPrestate = false;
bool instantBtnPrestate = false;

String apIPAddress = "0.0.0.0";
bool resetMode = false;
unsigned int apCreationRetryCount = 0;
unsigned const int maxRetryCount = 3;
bool instant = false;

void displaySensorScreen()
{
  oled.clear();
  oled.println("Soil moisture: " + String(soilMoisturePercent) + "%");
  oled.println("Humidity: " + String(humidity) + "%");
  oled.println("Temperature:" + String(temperature) + "C");
}

void displayNetworScreen()
{
  String wifiStatusText = connected ? "ok" : "err";
  String lastPkgText = pkgStatus ? "ok" : "err";

  oled.clear();
  oled.println("Connection:" + wifiStatusText);
  oled.println("IP:" + ipAddress);
  oled.println("HTTP status:" + lastPkgText + "-" + transferStatusCode);
}

void displayConfigScreen()
{
  oled.clear();
  oled.println(F("SSID: PHCS-conf"));
  oled.println(F("PWD: 123456789"));
  oled.println("IP:" + apIPAddress);
  oled.println("Soil value:" + String(soilMoistureValue));
}

void displayLoadingScreen()
{
  oled.clear();
  oled.println(F("Loading..."));
}

void displayErrorScreen(const String &message)
{
  oled.clear();
  oled.println(message);
}

void resetModule()
{
  resetBtnPrestate = true;
  connected = false;
  displayLoadingScreen();
  persistentState.Reset();
  persistentState.Save();
  wifi.initConfigServer("PHCS-conf", "123456789");
}

void handleButtons()
{
  int resetValue = digitalRead(RESET_BUTTON_PIN);
  int screenValue = digitalRead(SCREEN_BUTTON_PIN);
  int instantValue = digitalRead(INSTANT_BUTTON_PIN);

  if (resetValue == HIGH)
  {
    if (!resetBtnPrestate)
    {
      resetModule();
    }
  }
  else
  {
    resetBtnPrestate = false;
  }

  if (screenValue == HIGH)
  {
    if (!screenBtnPrestate)
    {
      screenBtnPrestate = true;
      screenOff = false;
      screenStatus++;

      if (screenStatus > 2)
      {
        screenStatus = 0;
      }

      switch (screenStatus)
      {
      case 0:
        displaySensorScreen();
        break;
      case 1:
        displayNetworScreen();
        break;
      case 2: // OFF
        oled.clear();
        screenOff = true;
        break;
      }
    }
  }
  else
  {
    screenBtnPrestate = false;
  }

  if (instantValue == HIGH)
  {
    if (!instantBtnPrestate)
    {
      instant = true;
    }
  }
  else
  {
    instantBtnPrestate = false;
  }
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

void processWiFiEvents()
{
  String message = wifi.readDataFromWiFiModule();

  if (message.length() != 0)
  {
    Serial.println(message);
  }

  if (message == F("#TRANSPORT_OK"))
  {
    pkgStatus = true;
    transferStatusCode = "200";
  }

  if (message == F("#AP_FAILED"))
  {
    if (apCreationRetryCount < maxRetryCount)
    {
      delay(2000);
      apCreationRetryCount++;
      resetModule();
    }
    else
    {
      displayErrorScreen(F("WiFi AP\nsetup failed :("));
    }
  }

  if (message.indexOf(F("#AP_READY")) == 0)
  {
    apIPAddress = message.substring(message.indexOf('!') + 1, message.length());
    displayConfigScreen();
    resetMode = true;
  }

  if (message.indexOf(F("#WIFI_CONNECTED")) == 0)
  {
    connected = true;
    ipAddress = message.substring(message.indexOf('!') + 1, message.length());
  }

  if (message == F("#WIFI_CONNECTION_FAILED"))
  {
    connected = false;
    ipAddress = "0.0.0.0";
  }

  if (message.indexOf(F("#TRANSPORT_FAILED")) == 0)
  {
    pkgStatus = false;

    transferStatusCode = message.substring(message.indexOf('!') + 1, message.length());
  }

  if (message == F("#CONFIG_FAILED"))
  {
    displayErrorScreen(F("Configuration failed :("));
  }

  if (message.indexOf(F("#CUSTOM_CFG")) == 0)
  {
    String airValue = getMessageElement(message, '!', 1);
    String waterValue = getMessageElement(message, '!', 2);

    persistentState.Data.airValue = atoi(airValue.c_str());
    persistentState.Data.waterValue = atoi(waterValue.c_str());
    persistentState.Save();
  }

  if (message == F("#CONFIG_DONE"))
  {
    resetMode = false;
    displaySensorScreen();
  }
}

void displayBegin()
{
  oled.begin(&Adafruit128x64, DISPLAY_ADDRESS);
  oled.setFont(X11fixed7x14);
  oled.setI2cClock(900000L);
  oled.clear();
  displayLoadingScreen();
}

void setup()
{
  Serial.begin(9600);

  displayBegin();

  delay(2000);

  dht.begin();
  wifi.begin();
  persistentState.Load();

  pinMode(RESET_BUTTON_PIN, INPUT);
  pinMode(SCREEN_BUTTON_PIN, INPUT);

  wifi.isConnectedToNetwork();

  displaySensorScreen();
}

void makeMeasurement()
{
  soilMoistureValue = analogRead(A0);
  ADCFilter.Filter(soilMoistureValue);
  soilMoistureValue = ADCFilter.Current();
  soilMoisturePercent = map(soilMoistureValue, persistentState.Data.airValue, persistentState.Data.waterValue, 0, 100);
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (soilMoisturePercent >= 100)
  {
    soilMoisturePercent = 100;
  }

  if (soilMoisturePercent <= 0)
  {
    soilMoisturePercent = 0;
  }
}

void loop()
{
  if (((millis() - lastSensorReadTime) > TWO_SECONDS))
  {
    makeMeasurement();
  }

  if (resetMode)
  {
    if (((millis() - lastConfigDisplayTime) > TWO_SECONDS))
    {
      lastConfigDisplayTime = millis();
      displayConfigScreen();
    }
  }

  if (instant || (((millis() - lastMeasurementTime) > TEN_MINUTES) && !resetMode))
  {
    instant = false;
    lastMeasurementTime = millis();

    if (connected)
    {
      wifi.httpPostData(F("http://192.168.88.207:3000/insert"), "{\"sensor_id\":\"" + stationId + "\", \"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + ", \"soil_moisture\": " + String(soilMoisturePercent) + "}");
    }
    else
    {
      wifi.connectToStoredNetwork();
    }

    if (!screenOff)
    {
      displaySensorScreen();
    }

    wifi.isConnectedToNetwork();
  }

  handleButtons();

  processWiFiEvents();
}
