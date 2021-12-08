#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <SoftwareSerial.h>

#include "Display.h"
#include "Wifi.h"
#include "eeprom_utils.h"

#include "define_sensors.h"
#include "define_utils.h"

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino

Adafruit_SSD1306 adisplay(OLED_RESET);
Display displayUtils(&adisplay);

SoftwareSerial aserial(SOFT_S_RX, SOFT_S_TX); // RX | TX
Wifi wifi(&aserial);

EepromUtils eepromUtils;

int screenStatus = 0;

unsigned long startTime = millis();
bool pkgStatus = false;
int soilmoisturepercent = 0;
float hum = 0; // use int ?
float temperature = 0;
bool connected = false;
bool instantBtnPrestate = false;
bool screenBtnPrestate = false;

void serialPrintln(String text)
{
  Serial.println(text);
}

bool connectToWifi()
{
  wifi.setupClientMode();
  wifi.turnOffEchoMode();
  wifi.connectToWifiAP(eepromUtils.readData().ssid, eepromUtils.readData().pwd);

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

  if (eepromUtils.readData().wifiConfigured)
  {
    displayUtils.clear();
    displayUtils.addText(0, 0, "WiFi: " + wifiStatusText);
    displayUtils.addText(0, 10, "Last pkg.: " + lastPkgText);
    displayUtils.print();
  }
  else
  {
    displayUtils.clear();
    displayUtils.addText(0, 0, "WiFi");
    displayUtils.addText(0, 10, "not");
    displayUtils.addText(0, 20, "configured");
    displayUtils.print();
  }
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

void reset()
{
  Store store;
  store.resetState = true;
  store.wifiConfigured = false;
  eepromUtils.saveData(store);

  wifi.resetWifiModule();
  delay(5000);
  wifi.serialClear();
  wifi.setupWiFiAP();
  wifi.setupTcpServer();
  wifi.serialClear();
}

void readWifiConfiguration()
{
  String confMessage = wifi.readMessageFromServer();

  if (confMessage.length() > 0)
  {
    String ssid = confMessage.substring(confMessage.indexOf(':') - 1, confMessage.length());
    String pwd = confMessage.substring(confMessage.indexOf(':') + 1, confMessage.length());

    Store store = eepromUtils.readData();
    store.resetState = false;
    store.wifiConfigured = true;
    store.ssid = ssid;
    store.pwd = pwd;

    eepromUtils.saveData(store);

    wifi.resetWifiModule();
    delay(5000);
    wifi.serialClear();

    connectToWifi();
  }
}

void exitFromResetState()
{
  if (eepromUtils.readData().resetState)
  {
    Store store = eepromUtils.readData();
    store.resetState = false;

    eepromUtils.saveData(store);
  }
}

void handleButtons()
{
  if (checkButton(INSTANT_BUTTON_PIN, instantBtnPrestate))
  {
    instantBtnPrestate = true;
    screenStatus = 3;

    exitFromResetState();
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

      reset();
    }
    else
    {
      exitFromResetState();
    }

    screenBtnPrestate = true;
  }
  else
  {
    screenBtnPrestate = false;
  }
}

void initEEPROM()
{
#ifdef DEBUG
  serialPrintln("EEPROM present");
  serialPrintln(eepromUtils.isPresent() ? "true" : "false");
  serialPrintln("wifiConfigured");
  serialPrintln(eepromUtils.readData().wifiConfigured ? "true" : "false");
  serialPrintln("resetState");
  serialPrintln(eepromUtils.readData().resetState ? "true" : "false");
  serialPrintln("wifiConfigured");
  serialPrintln(String(eepromUtils.readData().airValue));
  serialPrintln("resetState");
  serialPrintln(String(eepromUtils.readData().waterValue));
#endif
  if (!eepromUtils.isPresent())
  {
    eepromUtils.clearEEPROM();
    Store store;
    eepromUtils.saveData(store);
  }
}

void setup()
{
#ifdef DEBUG
  Serial.begin(BAUD_RATE);
#endif

  dht.begin();
  wifi.begin();
  displayUtils.begin();
  initEEPROM();

  if (eepromUtils.readData().wifiConfigured)
  {
    connectToWifi();
  }

  pinMode(INSTANT_BUTTON_PIN, INPUT);
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
  if (!eepromUtils.readData().resetState)
  {
    if (((millis() - startTime) > 600000) || instantBtnPrestate == true) // 10 minutes
    {
      startTime = millis();
      int soilMoistureValue = analogRead(A0);
      soilmoisturepercent = map(soilMoistureValue, eepromUtils.readData().airValue, eepromUtils.readData().waterValue, 0, 100);
      hum = dht.readHumidity();
      temperature = dht.readTemperature();

      if (soilmoisturepercent >= 100)
      {
        soilmoisturepercent = 100;
      }

      if (soilmoisturepercent <= 0)
      {
        soilmoisturepercent = 0;
      }

#ifdef DEBUG
      serialPrintln("T " + String(temperature) + " H " + String(hum) + " SM " + String(soilmoisturepercent) + " SMV " + String(soilMoistureValue));
#endif

      if (eepromUtils.readData().wifiConfigured)
      {
        connected = wifi.isConnected();
        if (connected)
        {
          wifi.connectToTCPServer("192.168.88.207", "3333");
          wifi.sendData("aa-1;0;1.1;2.2;3.11");
          pkgStatus = wifi.sendData2("aa-1;0;" + String(temperature) + ";" + String(hum) + ";" + String(soilmoisturepercent));
          // wifi.closeConnection();
        }
        else
        {
          connected = connectToWifi();
        }
      }
    }
  }

  handleButtons();

  displayStationScreens(connected,
                        pkgStatus,
                        String(soilmoisturepercent),
                        String(hum),
                        String(temperature));

  if (eepromUtils.readData().resetState && !eepromUtils.readData().wifiConfigured)
  {
    readWifiConfiguration();
  }

  delay(500);
}
