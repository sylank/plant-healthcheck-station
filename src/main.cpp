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

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino
//https://github.com/greiman/SSD1306Ascii
SSD1306AsciiAvrI2c oled;

SoftwareSerial aserial(SOFT_S_RX, SOFT_S_TX); // RX | TX
Wifi wifi(aserial);

String stationId = "1";

int screenStatus = 0;

unsigned long lastMeasurementTime = millis();
unsigned long lastConfigDisplayTime = millis();

bool pkgStatus = false;
String transferStatusCode = "0";
bool connected = false;
String ipAddress = "0.0.0.0";

int soilMoistureValue = 0;
int soilMoisturePercent = 0;
float humidity = 0;
float temperature = 0;

bool resetBtnPrestate = false;
bool screenBtnPrestate = false;

String apIPAddress = "0.0.0.0";
bool resetMode = false;

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

bool checkButton(const int &pinNumber, const bool &prestate)
{
  int btnState = digitalRead(pinNumber);

  if (btnState == HIGH && prestate == false)
  {
    return true;
  }
  else if (btnState == LOW)
  {
    return false;
  }

  return false;
}

void handleButtons()
{
  int resetValue = digitalRead(RESET_BUTTON_PIN);
  int screenValue = digitalRead(SCREEN_BUTTON_PIN);

  if (resetValue == HIGH)
  {
    if (!resetBtnPrestate)
    {
      resetBtnPrestate = true;
      displayLoadingScreen();
      wifi.initConfigServer("PHCS-conf", "123456789");
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
        break;
      }
    }
  }
  else
  {
    screenBtnPrestate = false;
  }
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

  // if (message == "#MODULE_READY")
  // {
  // }

  // if (message == "#IDLE_STATE_READY")
  // {
  // }

  // if (message == "#AP_FAILED")
  // {
  //   if (apCreationRetryCount < maxRetryCount)
  //   {
  //     delay(2000);
  //     this->resetState();
  //     apCreationRetryCount++;
  //   }
  //   else
  //   {
  //     apCreationRetryCount = 0;
  //     stateIndex = 5;
  //   }
  // }

  if (message.indexOf("#AP_READY") == 0)
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

  // if (message == "#CONFIG_FAILED")
  // {
  //   stateIndex = 6;
  // }

  // if (message.indexOf("#CUSTOM_CFG") == 0)
  // {
  //   String airValue = getMessageElement(message, '!', 1);
  //   String waterValue = getMessageElement(message, '!', 2);

  //   persistentState->Data.airValue = atoi(airValue.c_str());
  //   persistentState->Data.waterValue = atoi(waterValue.c_str());
  //   persistentState->Save();
  // }

  if (message == F("#CONFIG_DONE"))
  {
    resetMode = false;
    displaySensorScreen();
  }
}

// https://stackoverflow.com/questions/9072320/split-string-into-string-array
// String getMessageElement(String data, char separator, int index)
// {
//   int found = 0;
//   int strIndex[] = {0, -1};
//   int maxIndex = data.length() - 1;

//   for (int i = 0; i <= maxIndex && found <= index; i++)
//   {
//     if (data.charAt(i) == separator || i == maxIndex)
//     {
//       found++;
//       strIndex[0] = strIndex[1] + 1;
//       strIndex[1] = (i == maxIndex) ? i + 1 : i;
//     }
//   }

//   return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
// }

void displayBegin()
{
  oled.begin(&Adafruit128x64, DISPLAY_ADDRESS);
  oled.setFont(X11fixed7x14);
  oled.clear();
  oled.setI2cClock(900000L);
}

void setup()
{
  Serial.begin(9600);

  displayBegin();
  dht.begin();
  wifi.begin();

  pinMode(RESET_BUTTON_PIN, INPUT);
  pinMode(SCREEN_BUTTON_PIN, INPUT);

  wifi.isConnectedToNetwork();

  displaySensorScreen();
}

void makeMeasurement()
{
  soilMoistureValue = analogRead(A0);
  soilMoisturePercent = map(soilMoistureValue, AIR_VALUE, WATER_VALUE, 0, 100);
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
  if (resetMode)
  {
    if (((millis() - lastConfigDisplayTime) > TWO_SECONDS))
    {
      lastConfigDisplayTime = millis();
      makeMeasurement();
      displayConfigScreen();
    }
  }

  if (((millis() - lastMeasurementTime) > TEN_MINUTES))
  {
    resetBtnPrestate = false;
    lastMeasurementTime = millis();
    makeMeasurement();

    if (connected)
    {
      wifi.httpPostData(F("http://192.168.88.207:3000/insert"), "{\"sensor_id\":\"" + stationId + "\", \"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + ", \"soil_moisture\": " + String(soilMoisturePercent) + "}");
    }
    else
    {
      wifi.connectToStoredNetwork();
    }

    displaySensorScreen();
    wifi.isConnectedToNetwork();
  }

  handleButtons();

  processWiFiEvents();
}
