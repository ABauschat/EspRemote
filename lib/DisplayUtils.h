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

    private:
        Arduino_GFX *gfx;
        String previousMessage;
    };

} // namespace NuggetsInc

#endif // DISPLAYUTILS_H
