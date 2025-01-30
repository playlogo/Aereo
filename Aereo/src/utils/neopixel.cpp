#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel pixels(1, 19, NEO_BRG + NEO_KHZ800);

void setColor(uint8_t r, uint8_t g, uint8_t b)
{
    pixels.setPixelColor(0, pixels.Color(r, g, b));
    pixels.show();
}

void neopixelSetup()
{
    pixels.begin();
    pixels.clear();

    setColor(0, 0, 40);
}
