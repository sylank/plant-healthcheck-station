#include <DHT.h>
#ifndef EEPROMSTORE_H
#define EEPROMSTORE_H
#include "EEPROMStore.h"
#endif

#ifndef DISPLAY_H
#define DISPLAY_H
#include "display.h"
#endif

#ifndef ADASSD_H
#define ADASSD_H
#include <Adafruit_SSD1306.h>
#endif

#ifndef SOFTWARE_SERIAL_H
#define SOFTWARE_SERIAL_H
#include <SoftwareSerial.h>
#endif

#ifndef WIFI_H
#define WIFI_H
#include "Wifi.h"
#endif

#include "define_sensors.h"

#ifndef STATIONEVENTHANDLER_H
#define STATIONEVENTHANDLER_H
#include "station_event_handler.h"
#endif

#ifndef MEASUREMENTVALUES_H
#define MEASUREMENTVALUES_H
#include "measurement_values.h"
#endif

#ifndef PERSISTENT_STATE_H
#define PERSISTENT_STATE_H
#include "persistent_state.h"
#endif

DHT dht(DHTPIN, DHTTYPE);

Adafruit_SSD1306 adisplay(OLED_RESET);
Display display(&adisplay);

SoftwareSerial aserial(SOFT_S_RX, SOFT_S_TX); // RX | TX
Wifi wifi(&aserial);

EEPROMStore<PersistentState> persistentState;
MeasurementValues *measurementValues;
StationEventHandler *stationEventHandler;

unsigned long startTime = millis();
bool resetBtnPrestate = false;
bool screenBtnPrestate = false;

void serialPrintln(String text)
{
  Serial.println(text);
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

    stationEventHandler->resetState();
  }
  else
  {
    resetBtnPrestate = false;
  }

  if (checkButton(SCREEN_BUTTON_PIN, screenBtnPrestate))
  {
    screenBtnPrestate = true;

    stationEventHandler->nextScreenState();
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
  display.begin();

  persistentState.Load();

  measurementValues = new MeasurementValues();
  stationEventHandler = new StationEventHandler(measurementValues, &persistentState, &display, &wifi);

  pinMode(RESET_BUTTON_PIN, INPUT);
  pinMode(SCREEN_BUTTON_PIN, INPUT);
}

void loop()
{
  if (stationEventHandler->isResetModeActive())
  {
    measurementValues->setSoilMoistureValue(analogRead(A0));
  }
  else
  {
    if ((millis() - startTime) > TEN_MINUTES)
    {
      startTime = millis();

      measurementValues->setSoilMoistureValue(analogRead(A0));
      measurementValues->setHumidity(dht.readHumidity());
      measurementValues->setTemperature(dht.readTemperature());

      if (stationEventHandler->isEnabledToSendData())
      {
        wifi.httpPostData("http://192.168.88.252:3000/insert", stationEventHandler->generateRequestBody());
      }
    }
  }

  handleButtons();

  stationEventHandler->displayCurrentState();
  stationEventHandler->processWiFiEvents();
}
