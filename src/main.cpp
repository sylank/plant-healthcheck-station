#include <Wire.h>

#ifndef DISPLAY_H
#define DISPLAY_H
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif
#include <DHT.h>
#ifndef WIFI_H
#define WIFI_H
#include <SoftwareSerial.h>
#endif

#include "Display.h"
#include "Wifi.h"

#include "define_sensors.h"

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino

//CHECK THE SETTINGS, IT CAN SHOW MORE LINES
Adafruit_SSD1306 adisplay(OLED_RESET);
Display displayUtils(&adisplay);

SoftwareSerial aserial(SOFT_S_RX, SOFT_S_TX); // RX | TX
Wifi wifi(&aserial);

String stationId = "1";

int screenStatus = 0;

unsigned long startTime = millis();
bool pkgStatus = false;
String transferStatusCode = "0";
bool connected = false;
String ipAddress = "0.0.0.0";
int soilmoisturepercent = 0;
float hum = 0; // use int ?
float temp = 0;
bool instantBtnPrestate = false;
bool screenBtnPrestate = false;

void displaySensorScreen(String soilmoisturepercent, String hum, String temp)
{
  displayUtils.clear();
  displayUtils.addText(0, 0, "Soil moisture:" + soilmoisturepercent + " %");
  displayUtils.addText(0, 10, "Humidity:" + hum + " %");
  displayUtils.addText(0, 20, "Temperature:" + temp + " C");
  displayUtils.print();
}

void displayNetworScreen(bool wifiStatus, bool lastPackageStatus)
{
  String wifiStatusText = wifiStatus ? "ok" : "err";
  String lastPkgText = lastPackageStatus ? "ok" : "err";

  displayUtils.clear();
  displayUtils.addText(0, 0, "Connected:" + wifiStatusText);
  displayUtils.addText(0, 10, "IP:" + ipAddress);
  displayUtils.addText(0, 20, "HTTP status:" + lastPkgText + "-" + transferStatusCode);
  displayUtils.print();
}

void displayInstantScreen()
{
  displayUtils.clear();
  displayUtils.addText(0, 0, "Instant");
  displayUtils.print();
  screenStatus = 0;
  delay(3000);
}

void displayResetScreen()
{
  displayUtils.clear();
  displayUtils.addText(0, 0, "Reset");
  displayUtils.print();
}

void displayStationScreens(bool wifiStatus, bool lastPackageStatus, String soilmoisturepercent, String hum, String temp)
{
  switch (screenStatus)
  {
  case 0:
    displaySensorScreen(soilmoisturepercent, hum, temp);
    break;
  case 1:
    displayNetworScreen(wifiStatus, lastPackageStatus);
    break;
  case 2: // OFF
    displayUtils.clear();
    displayUtils.print();
    break;
  case 3:
    displayInstantScreen();
    break;
  case 4:
    displayResetScreen();
    break;
  }
}

bool checkButton(int pinNumber, bool prestate)
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
  if (checkButton(INSTANT_BUTTON_PIN, instantBtnPrestate))
  {
    instantBtnPrestate = true;
    screenStatus = 3;
  }
  else
  {
    instantBtnPrestate = false;
  }

  if (checkButton(SCREEN_BUTTON_PIN, screenBtnPrestate))
  {
    screenStatus++;

    if (screenStatus > 2)
    {
      screenStatus = 0;
    }

    if (instantBtnPrestate)
    {
      screenStatus = 4;
    }

    screenBtnPrestate = true;
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

  // message.trim();
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

  // if (message.indexOf("#AP_READY") == 0)
  // {
  //   apIpAddress = message.substring(message.indexOf('!') + 1, message.length());
  //   stateIndex = 6;
  //   apCreationRetryCount = 0;
  // }

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

  // if (message == "#CONFIG_DONE")
  // {
  //   persistentState->Data.wifiConfigured = true;
  //   persistentState->Data.resetState = false;
  //   persistentState->Save();

  //   stateIndex = 0;
  // }
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

void setup()
{
  Serial.begin(9600);

  dht.begin();
  wifi.begin();
  displayUtils.begin();

  pinMode(INSTANT_BUTTON_PIN, INPUT);
  pinMode(SCREEN_BUTTON_PIN, INPUT);

  wifi.isConnectedToNetwork();
}

void loop()
{
  if (((millis() - startTime) > TEN_MINUTES) || instantBtnPrestate == true)
  {
    startTime = millis();
    int soilMoistureValue = analogRead(A0);
    soilmoisturepercent = map(soilMoistureValue, AIR_VALUE, WATER_VALUE, 0, 100);
    hum = dht.readHumidity();
    temp = dht.readTemperature();

    if (soilmoisturepercent >= 100)
    {
      soilmoisturepercent = 100;
    }

    if (soilmoisturepercent <= 0)
    {
      soilmoisturepercent = 0;
    }

    wifi.httpPostData(F("http://192.168.88.207:3000/insert"), "{\"sensor_id\":\"" + stationId + "\", \"temperature\": " + String(temp) + ", \"humidity\": " + String(hum) + ", \"soil_moisture\": " + String(soilmoisturepercent) + "}");
  }

  handleButtons();

  displayStationScreens(connected,
                        pkgStatus,
                        String(soilmoisturepercent),
                        String(hum),
                        String(temp));

  processWiFiEvents();
}
