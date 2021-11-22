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

//Constants
#define DEBUG true

#define DHTPIN 7      // what pin we're connected to
#define DHTTYPE DHT22 // DHT 22  (AM2302)

#define OLED_RESET 4

#define SOFT_S_RX 2
#define SOFT_S_TX 3

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino

Adafruit_SSD1306 adisplay(OLED_RESET);
Display displayUtils(&adisplay);

SoftwareSerial aserial(SOFT_S_RX, SOFT_S_TX); // RX | TX
Wifi wifi(&aserial);

#define AIR_VALUE 615   //you need to replace this value with Value_1
#define WATER_VALUE 249 //you need to replace this value with Value_2

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

void displayStationScreens(String wifiStatus, String lastPackageStatus, String soilmoisturepercent, String hum, String temp)
{
  displayUtils.clear();
  displayUtils.addText(0, 0, "WiFi: " + wifiStatus);
  displayUtils.addText(0, 10, "Pkg.: " + lastPackageStatus);
  displayUtils.print();

  delay(3000);
  displayUtils.clear();

  displayUtils.addText(0, 0, "Soil: " + soilmoisturepercent + " %");
  displayUtils.addText(0, 10, "Hum: " + hum + " %");
  displayUtils.addText(0, 20, "Temp: " + temp + " C");
  displayUtils.print();
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
  int soilMoistureValue = analogRead(A0);
  int soilmoisturepercent = map(soilMoistureValue, AIR_VALUE, WATER_VALUE, 0, 100);
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

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

  bool connected = wifi.isConnected();
  bool pkgStatus = false;
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

  displayStationScreens(connected ? "connected" : "disconnected",
                        pkgStatus ? "OK" : "ERR",
                        String(soilmoisturepercent),
                        String(hum),
                        String(temp));

  // delay(600000); // 10 minutes
  delay(3000); // 10 minutes
}
