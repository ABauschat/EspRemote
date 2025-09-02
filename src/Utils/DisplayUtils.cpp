//Device Remote (wireless display)
#include "DisplayUtils.h"
#include "Device.h"

namespace NuggetsInc {

DisplayUtils::DisplayUtils(Arduino_GFX* display)
    : gfx(display) {}

DisplayUtils::~DisplayUtils() {}

void DisplayUtils::clearDisplay() {
    gfx->fillScreen(COLOR_BLACK);
    gfx->setTextColor(COLOR_WHITE);
    gfx->setCursor(0, 10);
    gfx->setTextSize(2);
    previousMessage = "";
}

void DisplayUtils::displayMessage(const String& message) {
    if (previousMessage == message) return;

    clearDisplay();
    gfx->println(message);
    previousMessage = message;
}

void DisplayUtils::newTerminalDisplay(const String& message) {  
    clearDisplay();
    gfx->println(message);
}

void DisplayUtils::addToTerminalDisplay(const String& message) {
    if (gfx->getCursorY() > 200) {
        clearDisplay();
    }

    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->println(message);
}

void DisplayUtils::println(const String& message) {
    gfx->println(message);
}

void DisplayUtils::print(const String& message) {
    gfx->print(message);
}

void DisplayUtils::setCursor(int16_t x, int16_t y) {
    gfx->setCursor(x, y);
}

void DisplayUtils::setTextSize(uint8_t size) {
    gfx->setTextSize(size);
}

void DisplayUtils::setTextColor(uint16_t color) {
    gfx->setTextColor(color);
}

void DisplayUtils::fillScreen(uint16_t color) {
    gfx->fillScreen(color);
}

void DisplayUtils::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    gfx->drawRect(x, y, w, h, color);
}

void DisplayUtils::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    gfx->fillRect(x, y, w, h, color);
}

void DisplayUtils::beginPlot(const String &xTitle, const String &yTitle, int _minX, int _maxX, int _minY, int _maxY)
{
    // Set the maximum number of points to plot.
    minX = _minX;
    maxX = _maxX;
    minY = _minY;
    maxY = _maxY;  

    // Save the axis titles.
    xAxisTitle = xTitle;
    yAxisTitle = yTitle;
    
    // Reset point count.
    plotCount = 0;
    
    // Define drawing area margins for axis labels:
    // (e.g., left margin: 50 px, top: 10 px, right: 10 px, bottom: 30 px).
    graphX = 50;
    graphY = 10;
    graphW = SCREEN_WIDTH - 60;
    graphH = SCREEN_HEIGHT - 50;
    
    // Clear the entire display and draw the graph frame.
    gfx->fillScreen(COLOR_BLACK);
    gfx->drawRect(graphX, graphY, graphW, graphH, COLOR_WHITE);
    
    // Draw the X-axis title centered along the bottom margin.
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    int xTitleX = graphX + (graphW / 2) - (xAxisTitle.length()); // Approximate centering.
    int xTitleY = graphY + graphH + 5;
    gfx->setCursor(xTitleX, xTitleY);
    gfx->println(xAxisTitle);
    
    // Draw the Y-axis title near the left side.
    gfx->setCursor(5, graphY + (graphH / 2) - 10);
    // Manually render vertical text for the Y-axis title.
    for (size_t i = 0; i < yAxisTitle.length(); ++i) {
        gfx->setCursor(5, graphY + (graphH / 4) + (i * 20)); // Adjust spacing as needed.
        gfx->print(yAxisTitle[i]);
    }
}

void DisplayUtils::plotPoint(int xValue, int yValue, uint16_t color)
{
    // Map the x and y values to the graph area.
    int mappedX = graphX + ((xValue - minX) * graphW) / (maxX - minX);
    int mappedY = graphY + graphH - ((yValue - minY) * graphH) / (maxY - minY);

    // Draw the point on the graph.
    gfx->drawPixel(mappedX, mappedY, color);

    // Store the y-value in the plot buffer.
    plotBuffer[plotCount] = yValue;
    plotCount++;
    
    if (plotCount >= maxPlotPoints)
    {
        beginPlot(xAxisTitle, yAxisTitle, minX, maxX, minY, maxY);
    }
}

} // namespace NuggetsInc