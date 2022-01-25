#ifndef ADAGFX_H
#define ADAGFX_H
#include <Adafruit_GFX.h>
#endif

#ifndef ADASSD_H
#define ADASSD_H
#include <Adafruit_SSD1306.h>
#endif

class Display
{
private:
    Adafruit_SSD1306 *display;

public:
    Display(Adafruit_SSD1306 *display);
    void begin();
    void addText(int x, int y, String text);
    void print();
    void clear();
    ~Display();
};

Display::Display(Adafruit_SSD1306 *display)
{
    this->display = display;
}

void Display::begin()
{
    this->display->begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
    this->display->clearDisplay();
}

void Display::addText(int x, int y, String text)
{
    display->setCursor(x, y);
    display->setTextSize(1);
    display->setTextColor(WHITE);
    display->println(text);
}

void Display::print()
{
    display->display();
}

void Display::clear()
{
    display->clearDisplay();
}

Display::~Display()
{
}