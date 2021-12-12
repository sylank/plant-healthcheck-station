#include "Wifi.h"

Wifi::Wifi(SoftwareSerial *serial)
{
    this->serial = serial;
}

void Wifi::begin()
{
    serial->begin(115200);
}

bool Wifi::pingModule()
{
    return writeToESPModule("AT");
}

bool Wifi::setupBaudRateOnModule()
{
    return writeToESPModule("AT+UART_DEF=115200,8,1,0,0");
}

bool Wifi::connectToWifiAP(String name, String pwd)
{
    return writeToESPModule("AT+CWJAP=\"" + name + "\",\"" + pwd + "\"");
}

bool Wifi::connectToTCPServer(String host, String port)
{
    return writeToESPModule("AT+CIPSTART=\"TCP\",\"" + host + "\"," + port + "");
}

bool Wifi::sendData(String data)
{
    return writeToESPModule("AT+CIPSEND=21");
}

bool Wifi::sendData2(String data)
{
    return writeToESPModule2(data);
}

bool Wifi::closeConnection()
{
    return writeToESPModule("AT+CIPCLOSE");
}

bool Wifi::writeToESPModule(String data)
{
    String command = data + "\r\n";
    serial->write(command.c_str());

    return waitForResponse("OK\r\n\0", 5000L);
}

bool Wifi::writeToESPModule2(String data)
{
    String command = data + "\n";
    serial->write(command.c_str());

    return waitForResponse("OK\r\n\0", 5000L);
}

bool Wifi::waitForResponse(String target, unsigned long timeout)
{
    unsigned long startTime = millis();
    String responseBuffer;
    char charIn;

    while ((millis() - startTime) < timeout)
    {
        if (serial->available())
        {
            charIn = serial->read();
            responseBuffer += charIn;
        }
        if (responseBuffer.endsWith(target))
        {
            return 1; //true
        }
    }
    if (!responseBuffer.endsWith(target))
    {
        Serial.println(responseBuffer);
        return 0; //false
    }

    return 0;
}

bool Wifi::setupWifiCard()
{
    return writeToESPModule("AT+UART_DEF=115200,8,1,0,0");
}

bool Wifi::setupClientMode()
{
    return writeToESPModule("AT+CWMODE=1");
}

bool Wifi::isConnected()
{
    String command = "AT+CIFSR\r\n";
    serial->write(command.c_str());

    return !waitForResponse("0.0.0.0\"\r\n\0", 5000L);
}

void Wifi::turnOffEchoMode()
{
    writeToESPModule("ATE0");
}

Wifi::~Wifi()
{
}
