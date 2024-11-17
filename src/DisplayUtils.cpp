//Device Remote (wireless display)
#include "DisplayUtils.h"

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

} // namespace NuggetsInc