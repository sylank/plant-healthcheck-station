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
#include "define_utils.h"

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino

Adafruit_SSD1306 adisplay(OLED_RESET);
Display displayUtils(&adisplay);

SoftwareSerial aserial(SOFT_S_RX, SOFT_S_TX); // RX | TX
Wifi wifi(&aserial);

int screenStatus = 0;

unsigned long startTime = millis();
bool pkgStatus = false;
int soilmoisturepercent = 0;
float hum; // use int ?
float temp;
bool connected;
bool resetBtnPrestate;
bool screenBtnPrestate;

void serialPrintln(String text)
{
#ifdef DEBUG
  Serial.println(text);
#endif
}

bool connectToWifi()
{
  wifi.setupClientMode();
  wifi.turnOffEchoMode();
  wifi.connectToWifiAP("szentkuti_serleg_2ghz", "Jeromos210523");

  return wifi.isConnected();
}

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
    delay(3000);
    displayNetworScreen(wifiStatus, lastPackageStatus);
    delay(3000);
    break;
  case 1:
    displayNetworScreen(wifiStatus, lastPackageStatus);
    break;
  case 2:
    displaySensorScreen(soilmoisturepercent, hum, temp);
    break;
  case 3: // OFF
    displayUtils.clear();
    displayUtils.print();
    break;
  case 4:
    displayInstantScreen();
    break;
  case 5:
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
  if (checkButton(RESET_BUTTON_PIN, resetBtnPrestate))
  {
    resetBtnPrestate = true;
    screenStatus = 4;
  }
  else
  {
    resetBtnPrestate = false;
  }

  if (checkButton(SCREEN_BUTTON_PIN, screenBtnPrestate))
  {
    screenStatus++;

    if (screenStatus > 3)
    {
      screenStatus = 0;
    }

    if (resetBtnPrestate)
    {
      screenStatus = 5;
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
#ifdef DEBUG
  Serial.begin(9600);
#endif

  dht.begin();
  wifi.begin();
  displayUtils.begin();

  connectToWifi();

  pinMode(RESET_BUTTON_PIN, INPUT);
  pinMode(SCREEN_BUTTON_PIN, INPUT);

#ifndef DEBUG
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#endif

#ifdef DEBUG
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
#endif
}

void loop()
{
  if (((millis() - startTime) > 600000) || resetBtnPrestate == true) // 10 minutes
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

#ifdef DEBUG
    serialPrintln("T " + String(temp) + " H " + String(hum) + " SM " + String(soilmoisturepercent) + " SMV " + String(soilMoistureValue));
#endif

    connected = wifi.isConnected();
    if (connected)
    {
      wifi.connectToTCPServer("192.168.88.207", "3333");
      wifi.sendData("aa-1;0;1.1;2.2;3.11");
      pkgStatus = wifi.sendData2("aa-1;0;" + String(temp) + ";" + String(hum) + ";" + String(soilmoisturepercent));
      // wifi.closeConnection();
    }
    else
    {
      connected = connectToWifi();
    }
  }

  handleButtons();

  displayStationScreens(connected,
                        pkgStatus,
                        String(soilmoisturepercent),
                        String(hum),
                        String(temp));
}
