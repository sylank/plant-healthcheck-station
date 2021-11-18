#include "Wifi.h"

Wifi::Wifi(SoftwareSerial *serial)
{
    this->serial = serial;
}

void Wifi::begin()
{
    serial->begin(9600);
}

String Wifi::pingModule()
{
    return writeToESPModule("AT");
}

String Wifi::setupBaudRateOnModule()
{
    return writeToESPModule("AT+UART_DEF=9600,8,1,0,0");
}

String Wifi::clientMode()
{
    return writeToESPModule("AT+CWMODE=1");
}

String Wifi::listWifiAPs()
{
    return writeToESPModule("AT+CWLAP");
}

String Wifi::connectToWifiAP(String name, String pwd)
{
    return writeToESPModule("AT+CWJAP=\"" + name + "\", \"" + pwd + "\"");
}

String Wifi::connectToTCPServer(String host, String port)
{
    return writeToESPModule("AT+CIPSTART=\"TCP\",\"" + host + "\"," + port + "");
}

String Wifi::sendData(String data)
{
    return writeToESPModule("AT+CIPSEND=21");
}

String Wifi::sendData2(String data)
{
    return writeToESPModule2(data);
}

String Wifi::closeConnection()
{
    return writeToESPModule("AT+CIPCLOSE");
}

String Wifi::writeToESPModule(String data)
{
    String command = data + "\r\n";
    serial->write(command.c_str());

    delay(1000);
    return recvWithEndMarker();
}

String Wifi::writeToESPModule2(String data)
{
    String command = data + "\n";
    serial->write(command.c_str());

    delay(1000);
    return recvWithEndMarker();
}

String Wifi::recvWithEndMarker()
{
    bool newData = false;
    int numChars = 32;
    char receivedChars[numChars];

    static int ndx = 0;
    char endMarker = '\n';
    char rc;
    if (serial->available() > 0)
    {
        while (serial->available() > 0 && newData == false)
        {
            rc = serial->read();

            if (rc != endMarker)
            {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars)
                {
                    ndx = numChars - 1;
                }
            }
            else
            {
                receivedChars[ndx] = '\0'; // terminate the string
                ndx = 0;
                newData = true;
            }
        }
    }

    return String(receivedChars);
}

Wifi::~Wifi()
{
}
