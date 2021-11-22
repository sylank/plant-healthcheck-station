#ifndef WIFI_H
#define WIFI_H

#include <SoftwareSerial.h>

#endif

#include <Arduino.h>

class Wifi
{
private:
    SoftwareSerial *serial;
    bool writeToESPModule(String data);
    bool writeToESPModule2(String data);
    bool waitForResponse(String target, unsigned long timeout);

public:
    Wifi(SoftwareSerial *serial);
    void begin();

    bool pingModule();
    bool setupBaudRateOnModule();
    bool connectToWifiAP(String name, String pwd);
    bool connectToTCPServer(String host, String port);
    bool sendData(String data);  // calculate
    bool sendData2(String data); // send
    bool closeConnection();

    bool setupWifiCard();
    bool setupClientMode();
    void turnOffEchoMode();

    bool isConnected();

    ~Wifi();
};