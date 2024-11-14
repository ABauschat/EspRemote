// Tab.h
#ifndef TAB_H
#define TAB_H

#include <Arduino_GFX.h>
#include <Arduino.h>
#include <vector>
#include <String.h>
#include "Colors.h"

namespace NuggetsInc {

enum LineStyle {
    STYLE_NONE,      // Just display lines as they are
    STYLE_NUMBERED,  // 1, 2, 3, ...
    STYLE_BULLETS    // •, •, •, ...
};

struct DisplayArea {
    int x;
    int y;
    int width;
    int height;
};

class Tab {
public:
    // Constructor with gfx injection
    Tab(String name, DisplayArea area, Arduino_GFX* display);

    // Methods to handle tab content and interactions
    void setStyle(LineStyle newStyle);
    void addLine(String line);
    void clearTab();
    void refreshTab();
    void scrollUp();
    void scrollDown();
    void switchTab();
    void selectTab();  // Simply returns the selected tab's name
    void RemoveAllLines();

    // Getters
    String getName() const;
    DisplayArea getArea() const;
    LineStyle getStyle() const;
    std::vector<String> getLines() const;

private:
    String name;                 // Tab name
    DisplayArea area;            // Area on the screen where this tab is displayed
    std::vector<String> lines;   // Lines of data/content in this tab
    LineStyle style;             // Line style for the tab (none, numbered, bullets)
    int currentLine;             // Current visible line for scrolling
    Arduino_GFX* gfx;            // Graphics display pointer
    bool tabNeedsRefresh = true; // Flag to indicate if the tab content needs to be refreshed

    std::vector<String> wrappedLinesCache; // Cached wrapped lines

    // Method for wrapping text based on width and prefix
    std::vector<String> wrapText(String text, int maxWidth, String prefix = "");

    // Method to calculate line height dynamically based on style
    int calculateLineHeight();

    // Helper method to clamp currentLine within valid range
    void clampCurrentLine();
};

} // namespace NuggetsInc

#endif // TAB_H
