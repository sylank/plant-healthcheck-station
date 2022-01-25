#ifndef SOFTWARE_SERIAL_H
#define SOFTWARE_SERIAL_H
#include <SoftwareSerial.h>
#endif

#include <Arduino.h>

class Wifi
{
private:
    SoftwareSerial *serial;

public:
    Wifi(SoftwareSerial *serial);
    void begin();

    void initConfigServer(String ssid, String password);
    void connectToStoredNetwork();
    void httpPostData(String serverURL, String jsonStr);
    void idleMode();

    void isConnectedToNetwork();

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

void Wifi::isConnectedToNetwork()
{
    serial->println("3");
}

String Wifi::readDataFromWiFiModule()
{
    if (serial->available())
    {
        String responseBuffer;
        char charIn;

        unsigned long startTime = millis();

        while ((millis() - startTime) < 3000)
        {
            if (serial->available())
            {
                charIn = serial->read();
                responseBuffer += charIn;
            }
            if (responseBuffer.endsWith("\n"))
            {
                responseBuffer.trim();
                return responseBuffer;
            }
        }
        if (!responseBuffer.endsWith("\n"))
        {
            responseBuffer.trim();
            return "";
        }
    }

    return "";
}

Wifi::~Wifi()
{
}
