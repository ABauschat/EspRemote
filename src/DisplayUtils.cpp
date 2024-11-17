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

} // namespace NuggetsInc