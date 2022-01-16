#ifndef SOFTWARE_SERIAL_H
#define SOFTWARE_SERIAL_H
#include <SoftwareSerial.h>
#endif

#include <Arduino.h>

class Wifi
{
private:
    SoftwareSerial *serial;

    bool waitForResponse(String target, unsigned long timeout);

public:
    Wifi(SoftwareSerial *serial);
    void begin();

    void initConfigServer(String ssid, String password);
    void connectToStoredNetwork();
    void httpPostData(String serverURL, String jsonStr);
    void idleMode();

    bool isConnectedToNetwork();

    String readDataFromWiFiModule();

    ~Wifi();
};

Wifi::Wifi(SoftwareSerial *serial)
{
    this->serial = serial;
}

void Wifi::begin()
{
    serial->begin(9600);
}

void Wifi::initConfigServer(String ssid, String password)
{
    serial->println("0#" + ssid + "!" + password);
}

void Wifi::connectToStoredNetwork()
{
    serial->println("1");
}

void Wifi::httpPostData(String serverURL, String jsonStr)
{
    serial->println("2#" + serverURL + "!" + jsonStr);
}

void Wifi::idleMode()
{
    serial->println("1");
}

bool Wifi::isConnectedToNetwork()
{
    serial->println("3");
}

String Wifi::readDataFromWiFiModule()
{
    String responseBuffer;
    char charIn;

    unsigned long startTime = millis();

    while ((millis() - startTime) < 3000)
    {
        if (Serial.available())
        {
            charIn = Serial.read();
            responseBuffer += charIn;
        }
        if (responseBuffer.endsWith("\r\n\0"))
        {
            responseBuffer.trim();
            return responseBuffer;
        }
    }
    if (!responseBuffer.endsWith("\r\n\0"))
    {
        responseBuffer.trim();
        return "";
    }

    return "";
}

Wifi::~Wifi()
{
}
