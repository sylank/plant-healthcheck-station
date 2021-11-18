#ifndef WIFI_H
#define WIFI_H

#include <SoftwareSerial.h>

#endif

#include <Arduino.h>

class Wifi
{
private:
    SoftwareSerial *serial;
    String writeToESPModule(String data);
    String writeToESPModule2(String data);

public:
    Wifi(SoftwareSerial *serial);
    void begin();

    String pingModule();
    String setupBaudRateOnModule();
    String clientMode();
    String listWifiAPs();
    String connectToWifiAP(String name, String pwd);
    String connectToTCPServer(String host, String port);
    String sendData(String data);
    String sendData2(String data);
    String recvWithEndMarker();
    String closeConnection();

    ~Wifi();
};