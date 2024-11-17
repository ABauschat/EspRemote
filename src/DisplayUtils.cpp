//Device Remote (wireless display)
// DisplayUtils.cpp
#include "DisplayUtils.h"

namespace NuggetsInc {

DisplayUtils::DisplayUtils(Arduino_GFX* display)
    : gfx(display) {}

DisplayUtils::~DisplayUtils() {}

void DisplayUtils::displayMessage(const String& message) {
    if (previousMessage == message) return;
    
    gfx->fillScreen(COLOR_BLACK);
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(10, 10);
    gfx->println(message);

    previousMessage = message;
}

void DisplayUtils::clearDisplay() {
    gfx->fillScreen(COLOR_BLACK);
    previousMessage = "";
    gfx->setCursor(10, 10);
}

void DisplayUtils::newTerminalDisplay(const String& message) {
    gfx->fillScreen(COLOR_BLACK);
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(10, 10);
    gfx->println(message);
    delay(100);
}

void DisplayUtils::addToTerminalDisplay(const String& message) {
    if (gfx->getCursorY() > 200) {
        gfx->fillScreen(COLOR_BLACK);
        gfx->setCursor(0, 10);
    }

    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->println(message);
    delay(100);
}

} // namespace NuggetsInc
