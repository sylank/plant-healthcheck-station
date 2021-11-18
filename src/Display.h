#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
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