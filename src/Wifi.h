#ifndef SOFTWARE_SERIAL_H
#define SOFTWARE_SERIAL_H
#include <SoftwareSerial.h>
#endif

#include <Arduino.h>

class Wifi
{
private:
    SoftwareSerial &serial;

public:
    Wifi(SoftwareSerial &serial);

    void sendDataOnSerial(const unsigned int &soilMoisture, const float &hum, const float &temp);
    String readDataFromWiFiModule();

    ~Wifi();
};

Wifi::Wifi(SoftwareSerial &aserial) : serial(aserial)
{
}

void Wifi::sendDataOnSerial(const unsigned int &soilMoisture, const float &hum, const float &temp)
{
    char buff[15];
    sprintf(buff, "#%d;%d;%d", soilMoisture, (unsigned)hum, (unsigned)temp);
    serial.println(buff);
}

String Wifi::readDataFromWiFiModule()
{
    if (serial.available())
    {
        String responseBuffer;
        char charIn;

        unsigned long startTime = millis();

        while ((millis() - startTime) < 3000)
        {
            if (serial.available())
            {
                charIn = serial.read();
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
