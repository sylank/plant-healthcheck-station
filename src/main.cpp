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

Adafruit_SSD1306 adisplay(OLED_RESET);
Display displayUtils(&adisplay);

SoftwareSerial aserial(SOFT_S_RX, SOFT_S_TX); // RX | TX
Wifi wifi(&aserial);

String stationId = "MUlnRG11emptOVAyRU1wWQo=";

int screenStatus = 0;

unsigned long startTime = millis();
bool pkgStatus = false;
int soilmoisturepercent = 0;
float hum = 0; // use int ?
float temp = 0;
bool connected = false;
bool instantBtnPrestate = false;
bool screenBtnPrestate = false;

void displaySensorScreen(String soilmoisturepercent, String hum, String temp)
{
  displayUtils.clear();
  displayUtils.addText(0, 0, "Soil: " + soilmoisturepercent + " %");
  displayUtils.addText(0, 10, "Hum: " + hum + " %");
  displayUtils.addText(0, 20, "Temp: " + temp + " C");
  displayUtils.print();
}

void displayNetworScreen(bool wifiStatus, bool lastPackageStatus)
{
  String wifiStatusText = wifiStatus ? "ok" : "err";
  String lastPkgText = lastPackageStatus ? "ok" : "err";

  displayUtils.clear();
  displayUtils.addText(0, 0, "WiFi: " + wifiStatusText);
  displayUtils.addText(0, 10, "Last pkg.: " + lastPkgText);
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

void setup()
{
  Serial.begin(9600);

  dht.begin();
  wifi.begin();
  displayUtils.begin();

  pinMode(INSTANT_BUTTON_PIN, INPUT);
  pinMode(SCREEN_BUTTON_PIN, INPUT);

  aserial.println("1");
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

    Serial.println("T " + String(temp) + " H " + String(hum) + " SM " + String(soilmoisturepercent) + " SMV " + String(soilMoistureValue));

    String payload = "{\"sensor_id\":\"" + stationId + "\", \"temperature\": " + String(temp) + ", \"humidity\": " + String(hum) + ", \"soil_moisture\": " + String(soilmoisturepercent) + "}";
    Serial.println(payload);
    wifi.httpPostData(F("http://192.168.88.252:3000/insert"), payload);
    Serial.println("sent");
  }

  handleButtons();

  displayStationScreens(connected,
                        pkgStatus,
                        String(soilmoisturepercent),
                        String(hum),
                        String(temp));

  String data = wifi.readDataFromWiFiModule();
  if (data.length() != 0)
  {
    Serial.println(data);
  }
}
