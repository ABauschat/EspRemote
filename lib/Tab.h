#ifndef TAB_H
#define TAB_H

#include <Arduino_GFX.h>
#include <Arduino.h>
#include <vector>
#include <String.h>
#include "Colors.h"

namespace NuggetsInc {

enum LineStyle {
    STYLE_NONE,      
    STYLE_NUMBERED, 
    STYLE_BULLETS  
};

struct DisplayArea {
    int x;
    int y;
    int width;
    int height;
};

class Tab {
public:
    Tab(String name, DisplayArea area, Arduino_GFX* display);

    void setStyle(LineStyle newStyle);
    void addLine(String line);
    void RemoveAllLines();
    void clearTab();
    void refreshTab();
    void scrollUp();
    void scrollDown();
    void DrawTabHeaders(const std::vector<Tab>& tabs, int currentIndex);

    String getName() const;
    DisplayArea getArea() const;
    LineStyle getStyle() const;
    std::vector<String> getLines() const;
    String getTitle() const;
    void setNeedsRefresh(bool needsRefresh);

private:
    String name;                 // Tab name
    DisplayArea area;            // Area on the screen where this tab is displayed
    std::vector<String> lines;   // Original lines added to the tab
    LineStyle style;             // Line style for the tab (none, numbered, bullets)
    int scrollOffset;            // Scroll offset for scrolling through wrapped lines
    Arduino_GFX* gfx;            // Graphics display pointer
    bool tabNeedsRefresh = true; // Flag to indicate if the tab content needs to be refreshed
    std::vector<String> wrappedLinesCache; // Cached wrapped lines
    int maxVisibleLines;         // Maximum number of lines that can be displayed at once

    std::vector<String> wrapText(String text, int maxWidth, String prefix = "");
    int calculateLineHeight();
    void clampScrollOffset();
};

} // namespace NuggetsInc

#endif // TAB_H
