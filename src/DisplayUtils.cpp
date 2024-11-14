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

void DisplayUtils::displayDataTab(const std::vector<String>& dataLines, int currentScrollLine, int maxVisibleLines) {
    // Define the area you want to clear before writing the text
    int x = 10;                     // Starting x position
    int y = 40;                     // Starting y position
    int width = gfx->width() - x * 2;   // Width of the text area
    int lineHeight = 20;            // Approximate height of each line (adjust as needed)
    int clearHeight = lineHeight * maxVisibleLines; // Exact height to clear only the visible lines

    // Clear the exact area of the visible lines only
    gfx->fillRect(x, y, width, clearHeight, COLOR_BLACK);

    // Set text properties for drawing each line individually
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_WHITE);

    // Write each line at a specific y-coordinate to avoid automatic line breaks
    for (int i = 0; i < maxVisibleLines; i++) {
        int lineIndex = currentScrollLine + i;
        if (lineIndex < dataLines.size()) {
            gfx->setCursor(x, y + i * lineHeight); // Position each line manually
            gfx->print(dataLines[lineIndex]);      // Use print() instead of println() to avoid automatic line break
        }
    }
}


void DisplayUtils::displayInfoTab(const String& tagType, const std::vector<NDEFRecord>& parsedRecords, const String& availableSpace, int uidLength, uint8_t* uid) {
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_WHITE);
    gfx->setCursor(10, 40); // Adjust cursor position as needed
    
    gfx->println("Tag Type: " + tagType);
    
    // Display UID
    String uidStr = "";
    for (int i = 0; i < uidLength; i++) {
        if (uid[i] < 0x10) {
            uidStr += "0"; // Leading zero
        }
        uidStr += String(uid[i], HEX);
        uidStr += " ";
    }
    gfx->println("UID: " + uidStr);
    
    // Display Parsed Records
    gfx->println("\nParsed Records:");
    for (size_t i = 0; i < parsedRecords.size(); i++) {
        gfx->println(String(i + 1) + ". Type: " + parsedRecords[i].type);
        gfx->println("   Payload: " + parsedRecords[i].payload);
    }
    
    // Display Available Space
    gfx->println("\nAvailable Space: " + availableSpace);
}

void DisplayUtils::displayTagInfo(const String& tagType, const String& tagData, const std::vector<NDEFRecord>& parsedRecords, const String& availableSpace, int uidLength, uint8_t* uid, DisplayTab currentTab, const std::vector<String>& dataLines, int currentScrollLine, int maxVisibleLines) {
    gfx->fillScreen(COLOR_BLACK);
    
    // Display Header
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_ORANGE);
    gfx->setCursor(10, 10);
    gfx->println("NFC Tag Info");

    // Display Tabs
    gfx->setTextSize(1);
    // Indicate current tab with different color
    gfx->setTextColor(currentTab == TAB_DATA ? COLOR_WHITE : COLOR_ORANGE);
    gfx->print("Data\t");
    gfx->setTextColor(currentTab == TAB_INFO ? COLOR_WHITE : COLOR_ORANGE);
    gfx->println("Info");

    // Display Content based on currentTab
    if (currentTab == TAB_DATA) {
        displayDataTab(dataLines, currentScrollLine, maxVisibleLines);
    }
    else if (currentTab == TAB_INFO) {
        displayInfoTab(tagType, parsedRecords, availableSpace, uidLength, uid);
    }

    // Instructions
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_WHEAT_CREAM);
    gfx->println("\n\nPress Select to clone");
    gfx->println("Use Up/Down to scroll");
    gfx->println("Press Back to return");
}

} // namespace NuggetsInc
