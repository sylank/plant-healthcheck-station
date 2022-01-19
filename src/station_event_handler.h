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
    static const unsigned maxRetryCount = 2;
    const String configServerSSID = "PHS-conf";
    const String configServerPassowrd = "123456789";

    MeasurementValues *measurementValues;
    EEPROMStore<PersistentState> *persistentState;
    Display *display;
    Wifi *wifi;

    unsigned stateIndex;
    bool lastTransferedPackageStatus;
    int lastTransferedStatusCode;
    bool isConnectedToNetwork;
    String apIpAddress;
    String stationIpAddress;
    unsigned apCreationRetryCount;

    void displaySensorScreen();
    void displayNetworScreen();
    void displayInstantScreen();
    void displayResetScreen();
    void displayEmptyScreen();
    void displayAPFailedScreen();
    void displayAPInfoScreen();

    int calculateSoilMoisturePercent();
    String sensorValuesToJSON();

    String getMessageElement(String data, char separator, int index);

public:
    StationEventHandler(MeasurementValues *measurementValues, EEPROMStore<PersistentState> *persistentState, Display *display, Wifi *wifi);

    void displayCurrentState();
    void nextScreenState();
    void instantMeasurementState();
    void resetState();

    void processWiFiEvents();

    bool isEnabledToSendData();
    bool isResetModeActive();

    String generateRequestBody();

    ~StationEventHandler();
};

StationEventHandler::StationEventHandler(MeasurementValues *measurementValues, EEPROMStore<PersistentState> *persistentState, Display *display, Wifi *wifi)
{
    this->measurementValues = measurementValues;
    this->persistentState = persistentState;
    this->display = display;
    this->wifi = wifi;

    apCreationRetryCount = 0;
}

bool StationEventHandler::isEnabledToSendData()
{
    return persistentState->Data.resetState == false && persistentState->Data.wifiConfigured == true;
}

bool StationEventHandler::isResetModeActive()
{
    return persistentState->Data.resetState == true;
}

void StationEventHandler::nextScreenState() {}
void StationEventHandler::instantMeasurementState() {}

void StationEventHandler::resetState()
{
    stateIndex = 4;
    persistentState->Reset();
    persistentState->Save();

    wifi->initConfigServer("Plant_Healthcheck_Station", "123456789");
}

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
    case 5:
        this->displayAPFailedScreen();
        break;
    case 6:
        this->displayAPInfoScreen();
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
    String wifiStatusText = isConnectedToNetwork ? "ok" : "err";
    String lastPkgText = this->lastTransferedPackageStatus ? "ok" : "err";

    display->clear();
    display->addText(0, 0, "WiFi status: " + wifiStatusText);
    display->addText(0, 10, "Last package: " + lastPkgText);
    display->addText(0, 20, "IP: " + stationIpAddress);
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
    display->addText(0, 0, "Reset started ...");
    display->print();
}

void StationEventHandler::displayEmptyScreen()
{
    display->clear();
    display->print();
}

void StationEventHandler::displayAPFailedScreen()
{
    display->clear();
    display->addText(0, 0, "Failed to create AP");
    display->print();
}

void StationEventHandler::displayAPInfoScreen()
{
    display->clear();
    display->addText(0, 0, "Wifi:" + configServerSSID);
    display->addText(0, 10, "Pwd:" + configServerPassowrd);
    display->addText(0, 20, "Host: http://" + apIpAddress);
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

void StationEventHandler::processWiFiEvents()
{
    String message = wifi->readDataFromWiFiModule();

    if (message[0] != '#')
    {
        return;
    }

    if (message == "#MODULE_READY")
    {
    }

    if (message == "#IDLE_STATE_READY")
    {
    }

    if (message == "#AP_FAILED")
    {
        if (apCreationRetryCount < maxRetryCount)
        {
            delay(2000);
            this->resetState();
            apCreationRetryCount++;
        }
        else
        {
            apCreationRetryCount = 0;
            stateIndex = 5;
        }
    }

    if (message.indexOf("#AP_READY") == 0)
    {
        apIpAddress = message.substring(message.indexOf('!') + 1, message.length());
        stateIndex = 6;
        apCreationRetryCount = 0;
    }

    if (message.indexOf("#WIFI_CONNECTED") == 0)
    {
        stationIpAddress = message.substring(message.indexOf('!') + 1, message.length());
    }

    if (message == "#WIFI_CONNECTION_FAILED")
    {
    }

    if (message == "#TRANSPORT_OK")
    {
        lastTransferedPackageStatus = true;
        lastTransferedStatusCode = 200;
    }

    if (message.indexOf("#TRANSPORT_FAILED") == 0)
    {
        lastTransferedStatusCode = atoi(message.substring(message.indexOf('!') + 1, message.length()).c_str());
        lastTransferedPackageStatus = false;
    }

    if (message == "#CONFIG_FAILED")
    {
        stateIndex = 6;
    }

    if (message == "#CUSTOM_CFG !{ VAL1 } !{ VAL2 }")
    {
        String airValue = getMessageElement(message, '!', 1);
        String waterValue = getMessageElement(message, '!', 2);

        persistentState->Data.airValue = atoi(airValue.c_str());
        persistentState->Data.waterValue = atoi(waterValue.c_str());

        persistentState->Save();
    }

    if (message == "#CONFIG_DONE")
    {
    }
}

// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String StationEventHandler::getMessageElement(String data, char separator, int index)
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

StationEventHandler::~StationEventHandler()
{
}
