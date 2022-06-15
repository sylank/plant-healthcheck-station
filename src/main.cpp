#include <Arduino.h>

#include <SoftwareSerial.h>

// https://github.com/RobTillaart/Arduino/blob/master/libraries/DHTlib/examples/dht22_test/dht22_test.ino
#include <dht.h>

#define DHT_PIN PIN_B2
#define DHTTYPE DHT22 // DHT 22  (AM2302)

#define RX PIN_B0
#define TX PIN_B1
#define SOIL_MOISTURE_PIN A7

#define RESET_BUTTON_PIN PIN_A3

const byte COLOR_BLACK = 0b000;
const byte COLOR_RED = 0b100;
const byte COLOR_GREEN = 0b010;
const byte COLOR_BLUE = 0b001;
const byte COLOR_MAGENTA = 0b101;
const byte COLOR_CYAN = 0b011;
const byte COLOR_YELLOW = 0b110;
const byte COLOR_WHITE = 0b111;

const byte PIN_LED_R = PIN_A0;
const byte PIN_LED_G = PIN_A1;
const byte PIN_LED_B = PIN_A2;

const byte PIN_WIFI_STATUS = PIN_A4;

SoftwareSerial serial(RX, TX);

dht DHT;

bool resetBtnPrestate = false;

void displayColor(byte color)
{
  digitalWrite(PIN_LED_R, !bitRead(color, 2));
  digitalWrite(PIN_LED_G, !bitRead(color, 1));
  digitalWrite(PIN_LED_B, !bitRead(color, 0));
}

void setup()
{
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);

  pinMode(PIN_WIFI_STATUS, OUTPUT);

  displayColor(COLOR_BLACK);

  digitalWrite(PIN_WIFI_STATUS, HIGH);

  pinMode(RX, INPUT);
  pinMode(TX, OUTPUT);

  pinMode(RESET_BUTTON_PIN, INPUT);

  serial.begin(19200);
}

void loop()
{
  int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
  int soilMoisturePercent = map(soilMoistureValue, 850, 450, 0, 100);

  DHT.read22(DHT_PIN);
  float humidity = DHT.humidity;
  float temperature = DHT.temperature;

  displayColor(COLOR_WHITE);

  int resetValue = digitalRead(RESET_BUTTON_PIN);

  if (resetValue == HIGH)
  {
    if (!resetBtnPrestate)
    {
      resetBtnPrestate = true;
      serial.println("r");
    }
  }
  else
  {
    resetBtnPrestate = false;
  }
}
