//Device Remote (wireless display)
#ifndef DISPLAYUTILS_H
#define DISPLAYUTILS_H

#include <Arduino_GFX.h>
#include <Arduino.h>
#include <vector>
#include <String.h>
#include "NFCLogic.h"
#include "Colors.h"

namespace NuggetsInc
{
    class DisplayUtils
    {
    public:
        DisplayUtils(Arduino_GFX *display);
        ~DisplayUtils();

        // Display functions
        void clearDisplay();
        void displayMessage(const String &message);
        void newTerminalDisplay(const String &message);
        void addToTerminalDisplay(const String &message);
        
        void println(const String &message);
        void print(const String &message);
        void setCursor(int16_t x, int16_t y);
        void setTextSize(uint8_t size);
        void setTextColor(uint16_t color);
        void fillScreen(uint16_t color);
        void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
        void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    private:
        Arduino_GFX *gfx;
        String previousMessage;
    };

} // namespace NuggetsInc

#endif // DISPLAYUTILS_H
