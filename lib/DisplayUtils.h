// DisplayUtils.h
#ifndef DISPLAYUTILS_H
#define DISPLAYUTILS_H

#include <Arduino_GFX.h>
#include <Arduino.h>
#include <vector>
#include <String.h>
#include "NFCLogic.h"
#include "Colors.h"

namespace NuggetsInc {

// Define display tabs
enum DisplayTab {
    TAB_DATA,
    TAB_INFO
};

class DisplayUtils {
public:
    DisplayUtils(Arduino_GFX* display);
    ~DisplayUtils();

    // Display functions
    void displayMessage(const String& message);
    void newTerminalDisplay(const String& message);
    void addToTerminalDisplay(const String& message);
    void displayTagInfo(const String& tagType, const String& tagData, const std::vector<NDEFRecord>& parsedRecords, const String& availableSpace, int uidLength, uint8_t* uid, DisplayTab currentTab, const std::vector<String>& dataLines, int currentScrollLine, int maxVisibleLines);
    void displayDataTab(const std::vector<String>& dataLines, int currentScrollLine, int maxVisibleLines);
    void displayInfoTab(const String& tagType, const std::vector<NDEFRecord>& parsedRecords, const String& availableSpace, int uidLength, uint8_t* uid);

private:
    Arduino_GFX* gfx;
    String previousMessage;
};

} // namespace NuggetsInc

#endif // DISPLAYUTILS_H
