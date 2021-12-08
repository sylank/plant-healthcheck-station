#include <Arduino.h>

struct Store
{
    bool wifiConfigured = false;
    bool resetState = false;
    int airValue = 615;
    int waterValue = 249;

    String ssid = "";
    String pwd = "";

    // String sensorGateAddress = "192.168.88.207";
    // String sensorGatePort = "3333";
};
