#include <Arduino.h>

struct Store
{
    bool wifiConfigured = false;
    bool resetState = true;
    int airValue = 615;
    int waterValue = 249;

    String ssid = "";
    String pwd = "";
};
