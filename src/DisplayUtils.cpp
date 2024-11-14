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
    gfx->setCursor(10, 60);
    gfx->println(message);

    previousMessage = message;
}

void DisplayUtils::newTerminalDisplay(const String& message) {
    gfx->fillScreen(COLOR_BLACK);
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(0, 60);
    gfx->println(message);
    delay(100);
}

void DisplayUtils::addToTerminalDisplay(const String& message) {
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    gfx->println(message);
    delay(100);
}

} // namespace NuggetsInc
