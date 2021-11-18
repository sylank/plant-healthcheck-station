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

const int AirValue = 615;   //you need to replace this value with Value_1
const int WaterValue = 249; //you need to replace this value with Value_2

void setup()
{
  Serial.begin(9600);

  dht.begin();
  wifi.begin();
  displayUtils.begin();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.println("Setup completed...");
}

void loop()
{
  int soilMoistureValue = analogRead(A0);
  int soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
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

  Serial.println("T " + String(temp) + " H " + String(hum) + " SM " + String(soilmoisturepercent) + " SMV " + String(soilMoistureValue));

  wifi.connectToTCPServer("192.168.88.207", "3333");
  wifi.sendData("aa-1;0;1.1;2.2;3.11");
  wifi.sendData2("aa-1;0;" + String(temp) + ";" + String(hum) + ";" + String(soilmoisturepercent));
  wifi.closeConnection();

  displayUtils.clear();
  displayUtils.addText(0, 0, "WiFi status");
  displayUtils.addText(0, 10, "unknown");
  displayUtils.print();

  delay(3000);
  displayUtils.clear();

  displayUtils.addText(0, 0, "Soil moisture: " + String(soilmoisturepercent) + " %");
  displayUtils.addText(0, 10, "Humidity: " + String(hum) + " %");
  displayUtils.addText(0, 20, "Temperature: " + String(temp) + " C");
  displayUtils.print();

  delay(600000); // 10 minutes
}

//AT+UART_DEF=9600, 8, 1, 0, 0

//AT+CWMODE=1 client mode

//AT+CWLAP list wifi

//AT+CWJAP="Liberty Media", "nlj-hyiyk187" connect

//AT+CIFSR get IP

//AT+CIPSTART="TCP","192.168.88.207",3333 connect to tcp

//AT+CIPSEND=19 send n bytes

//aa-1;0;1.1;2.2;3.11

//AT+CIPCLOSE
