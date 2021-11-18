#include "Display.h"

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
