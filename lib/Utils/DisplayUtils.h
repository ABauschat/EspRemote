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

        void beginPlot(const String &xTitle, const String &yTitle, int minX, int maxX, int minY, int maxY);
        void plotPoint(int xValue, int yValue, uint16_t color);

    private:
        Arduino_GFX *gfx;
        String previousMessage;

        static const uint16_t maxPlotPoints = 2000;
        int plotBuffer[maxPlotPoints];  // stores y-values of points
        uint16_t plotCount;             // number of plotted points
        int plotMin;                    // minimum y-value seen
        int plotMax;                    // maximum y-value seen
        String xAxisTitle;
        String yAxisTitle;

        int minX, maxX, minY, maxY; // x and y axis limits
        
        // Graph drawing area in the display:
        int graphX, graphY, graphW, graphH;
    };

} // namespace NuggetsInc

#endif // DISPLAYUTILS_H
