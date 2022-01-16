#ifndef MEASUREMENTVALUES_H
#define MEASUREMENTVALUES_H
#include "measurement_values.h"
#endif

#ifndef PERSISTENT_STATE_H
#define PERSISTENT_STATE_H
#include "persistent_state.h"
#endif

#ifndef DISPLAY_H
#define DISPLAY_H
#include "display.h"
#endif

#ifndef EEPROMSTORE_H
#define EEPROMSTORE_H
#include "EEPROMStore.h"
#endif

#ifndef WIFI_H
#define WIFI_H
#include "Wifi.h"
#endif

class StationEventHandler
{
private:
    MeasurementValues *measurementValues;
    EEPROMStore<PersistentState> *persistentState;
    Display *display;
    Wifi *wifi;

    unsigned stateIndex;
    bool lastTransferedPackageStatus;

    void displaySensorScreen();
    void displayNetworScreen();
    void displayInstantScreen();
    void displayResetScreen();
    void displayEmptyScreen();

    int calculateSoilMoisturePercent();

    String sensorValuesToJSON();

public:
    StationEventHandler(MeasurementValues *measurementValues, EEPROMStore<PersistentState> *persistentState, Display *display, Wifi *wifi);

    void displayCurrentState();
    void nextScreenState();
    void instantMeasurementState();
    void resetState();

    String generateRequestBody();

    ~StationEventHandler();
};

StationEventHandler::StationEventHandler(MeasurementValues *measurementValues, EEPROMStore<PersistentState> *persistentState, Display *display, Wifi *wifi)
{
    this->measurementValues = measurementValues;
    this->persistentState = persistentState;
    this->display = display;
    this->wifi = wifi;
}

void StationEventHandler::nextScreenState() {}
void StationEventHandler::instantMeasurementState() {}
void StationEventHandler::resetState() {}

void StationEventHandler::displayCurrentState()
{
    switch (stateIndex)
    {
    case 0:
        this->displaySensorScreen();
        break;
    case 1:
        this->displayNetworScreen();
        break;
    case 2:
        this->displayEmptyScreen();
        break;
    case 3:
        this->displayInstantScreen();
        break;
    case 4:
        this->displayResetScreen();
        break;
    }
}

void StationEventHandler::displaySensorScreen()
{
    display->clear();
    display->addText(0, 0, "Soil moisture: " + String(this->calculateSoilMoisturePercent()) + " %");
    display->addText(0, 10, "Humidity: " + String(this->measurementValues->getHumidity()) + " %");
    display->addText(0, 20, "Temperature: " + String(this->measurementValues->getTemperature()) + " C");
    display->print();
}

void StationEventHandler::displayNetworScreen()
{
    String wifiStatusText = wifi->isConnectedToNetwork() ? "ok" : "err";
    String lastPkgText = this->lastTransferedPackageStatus ? "ok" : "err";

    display->clear();
    display->addText(0, 0, "WiFi status: " + wifiStatusText);
    display->addText(0, 10, "Last package: " + lastPkgText);
    display->print();
}

void StationEventHandler::displayInstantScreen()
{
    display->clear();
    display->addText(0, 0, "Instant");
    display->print();
}

void StationEventHandler::displayResetScreen()
{
    display->clear();
    display->addText(0, 0, "Reset");
    display->print();
}

void StationEventHandler::displayEmptyScreen()
{
    display->clear();
    display->print();
}

int StationEventHandler::calculateSoilMoisturePercent()
{
    int soilMoisturePercent = map(this->measurementValues->getSoilMoistureValue(),
                                  this->persistentState->Data.airValue, this->persistentState->Data.waterValue,
                                  0,
                                  100);

    if (soilMoisturePercent >= 100)
    {
        soilMoisturePercent = 100;
    }

    if (soilMoisturePercent <= 0)
    {
        soilMoisturePercent = 0;
    }

    return soilMoisturePercent;
}

String StationEventHandler::generateRequestBody()
{
    return "{ \"sensor_id\": \"" + String(STATION_ID) + "\", \"sensor_data\": " + sensorValuesToJSON() + "}";
}

// {"temperature":1.1, "humidity":2.22, "soil_moisture":3.33}
String StationEventHandler::sensorValuesToJSON()
{
    return "{\"temperature\":\"" + String(this->measurementValues->getTemperature()) + "\", \"humidity\":\"" + String(this->measurementValues->getHumidity()) + "\", \"soil_moisture\": \"" + String(calculateSoilMoisturePercent()) + "\" }";
}

StationEventHandler::~StationEventHandler()
{
}
