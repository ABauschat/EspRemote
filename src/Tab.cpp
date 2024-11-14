#include "Tab.h"

namespace NuggetsInc {

Tab::Tab(String name, DisplayArea area, Arduino_GFX* display)
    : name(name), area(area), style(STYLE_NONE), currentLine(0), gfx(display) {}

void Tab::setStyle(LineStyle newStyle) {
    style = newStyle;
}

void Tab::addLine(String line) {
    lines.push_back(line);
}

void Tab::clearTab() {
    // Clear the display area by filling it with black
    gfx->fillRect(area.x, area.y, area.width, area.height, COLOR_BLACK);
}

void Tab::refreshTab() {
    clearTab();  // Clear the tab area before updating content

    // Display lines according to the selected style
    int lineHeight = 20;  // Adjust line height as needed
    gfx->setTextColor(COLOR_WHITE);
    gfx->setCursor(area.x, area.y);

    for (int i = currentLine; i < currentLine + 5 && i < lines.size(); ++i) {
        if (style == STYLE_NUMBERED) {
            gfx->println(String(i + 1) + ". " + lines[i]);
        }
        else if (style == STYLE_BULLETS) {
            gfx->println("• " + lines[i]);
        }
        else {
            gfx->println(lines[i]);
        }
    }
}

void Tab::scrollUp() {
    if (currentLine > 0) {
        --currentLine;
        refreshTab();
    }
}

void Tab::scrollDown() {
    if (currentLine + 5 < lines.size()) {  // Assuming 5 lines fit in the display area
        ++currentLine;
        refreshTab();
    }
}

void Tab::switchTab() {
    // Logic to switch between tabs, to be implemented
}

void Tab::selectTab() {
    // Return the selected tab's name as a string
    Serial.println("Selected Tab: " + name);
}

String Tab::getName() const {
    return name;
}

DisplayArea Tab::getArea() const {
    return area;
}

LineStyle Tab::getStyle() const {
    return style;
}

std::vector<String> Tab::getLines() const {
    return lines;
}

} // namespace NuggetsInc