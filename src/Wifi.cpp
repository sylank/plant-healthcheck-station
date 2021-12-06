#include "Wifi.h"

Wifi::Wifi(SoftwareSerial *serial)
{
    this->serial = serial;
}

void Wifi::begin()
{
    serial->begin(9600);
}

bool Wifi::pingModule()
{
    return writeToESPModule("AT");
}

bool Wifi::setupBaudRateOnModule()
{
    return writeToESPModule("AT+UART_DEF=9600,8,1,0,0");
}

bool Wifi::connectToWifiAP(String name, String pwd)
{
    return writeToESPModule("AT+CWJAP=\"" + name + "\", \"" + pwd + "\"");
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

    return waitForStatusResponse("OK\r\n\0", 5000L);
}

bool Wifi::writeToESPModule2(String data)
{
    String command = data + "\n";
    serial->write(command.c_str());

    return waitForStatusResponse("OK\r\n\0", 5000L);
}

bool Wifi::waitForStatusResponse(String target, unsigned long timeout)
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
    return writeToESPModule("AT+UART_DEF=9600,8,1,0,0");
}

bool Wifi::setupClientMode()
{
    return writeToESPModule("AT+CWMODE=1");
}

bool Wifi::isConnected()
{
    String command = "AT+CIFSR\r\n";
    serial->write(command.c_str());

    return !waitForStatusResponse("0.0.0.0\"\r\n\0", 5000L);
}

void Wifi::turnOffEchoMode()
{
    writeToESPModule("ATE0");
}

bool Wifi::setupWiFiAP()
{
    writeToESPModule("AT+CWMODE=2");
    return writeToESPModule("AT+CWSAP_CUR=\"ps\",\"1\",5,3");
}

bool Wifi::setupTcpServer()
{
    writeToESPModule("AT+CIPMUX=1");
    return writeToESPModule("AT+CIPSERVER=1,1001");
}

String Wifi::readMessageFromServer()
{
    if (serial->available())
    {
        String msg = waitForStringResponse(3000);
        if (msg.indexOf(':') != -1)
        {
            return msg.substring(msg.indexOf(':') + 1, msg.length());
        }

        return msg;
    }

    return "";
}

String Wifi::waitForStringResponse(unsigned long timeout)
{
    String responseBuffer;
    unsigned long startTime = millis();

    char charIn;

    while ((millis() - startTime) < timeout)
    {
        if (serial->available())
        {
            charIn = serial->read();
            responseBuffer += charIn;
        }
        if (responseBuffer.endsWith("\r\n\0"))
        {
            Serial.println("res:" + responseBuffer);
            return responseBuffer;
        }
    }
    if (!responseBuffer.endsWith("\r\n\0"))
    {
        Serial.println(responseBuffer);
        return "";
    }

    return "";
}

void Wifi::resetWifiModule()
{
    writeToESPModule("AT+RESTORE");
}

void Wifi::serialClear()
{
    if (serial->available())
    {
        while (serial->available())
        {
            serial->read();
        }
    }
}

Wifi::~Wifi()
{
}
